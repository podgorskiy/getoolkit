/*
 * Copyright 2017-2020 Stanislav Pidhorskyi. All rights reserved.
 * License: https://raw.githubusercontent.com/podgorskiy/bimpy/master/LICENSE.txt
 */

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Camera2D.h"
#include "Render/DebugRenderer.h"
#include "simpletext.h"
#include "Render/GLDebugMessage.h"
#include "Render/Shader.h"
#include "2DEngine/Renderer2D.h"
#include "2DEngine/Encoder.h"
#include "Render/VertexSpec.h"
#include "Render/VertexBuffer.h"
#include <glm/ext/matrix_transform.hpp>
#include "Vector/nanovg.h"
#include "Vector/nanovg_backend.h"
#include "runtime_error.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>

namespace py = pybind11;

typedef py::array_t<uint8_t, py::array::c_style> ndarray_uint8;
typedef py::array_t<float, py::array::c_style> ndarray_float;

enum SpecialKeys
{
	KeyEscape = 256,
	KeyEnter = 257,
	KeyTab = 258,
	KeyBackspace = 259,
	KeyInsert = 260,
	KeyDelete = 261,
	KeyRight = 262,
	KeyLeft = 263,
	KeyDown = 264,
	KeyUp = 265,
};

typedef std::shared_ptr<SimpleText> SimpleTextPtr;

namespace pth
{
	class Context
	{
	public:
		enum RECENTER
		{
			FIT_DOCUMENT,
			ORIGINAL_SIZE
		};

		Context& operator=(const Context&) = delete;
		Context(const Context&) = delete;
		Context() = default;

		void Init(int width, int height, const std::string& name);

		void Resize(int width, int height);

		void Recenter(float w, float h, RECENTER r);
		void Recenter(float x0, float y0, float x1, float y1);

		void NewFrame();

		void Render();

		bool ShouldClose();

		int GetWidth() const;

		int GetHeight() const;

		void Point(float x, float y, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> color, float point_size) const;
		void Box(float minx, float miny, float maxx, float maxy, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> color_stroke, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> color_fill) const;

		~Context();

		GLFWwindow* m_window = nullptr;
		int m_width;
		int m_height;
		py::function mouse_button_callback;
		py::function mouse_position_callback;
		py::function keyboard_callback;

		Camera2D m_camera;
		Render::DebugRenderer m_dr;
		glm::vec2 m_world_size;
		NVGcontext* vg = nullptr;
		Render::VertexSpec m_spec;
		Render::VertexBuffer m_buff;
		Render::ProgramPtr m_program;
		Render::Uniform u_modelViewProj;
		Render::Uniform u_world_size;
		SimpleTextPtr m_text;

		Render::Renderer2D m_2drender;
		struct ImGuiContext* m_imgui;

		bool m_ctrl_c_down = false;
		bool m_ctrl_c_released = false;
		bool m_ctrl_v_down = false;
		bool m_ctrl_v_released = false;
		bool m_ctrl_x_down = false;
		bool m_ctrl_x_released = false;
	};
}

struct Vertex
{
	glm::vec2 pos;
	glm::vec2 uv;
};

void pth::Context::Init(int width, int height, const std::string& name)
{
	if (nullptr == m_window)
	{
		if (!glfwInit())
		{
			throw runtime_error("GLFW initialization failed.\nThis may happen if you try to run bimpy on a headless machine ");
		}

#if __APPLE__
		// GL 3.2 + GLSL 150
		const char* glsl_version = "#version 150";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
		// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_SRGB_CAPABLE, 1);

		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

		m_window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
	    if (!m_window)
	    {
	        glfwTerminate();
			throw runtime_error("GLFW failed to create window.\nThis may happen if you try to run bimpy on a headless machine ");
	    }

		glfwMakeContextCurrent(m_window);
		if (gl3wInit() != GL3W_OK)
		{
			throw runtime_error("GL3W initialization failed.\nThis may happen if you try to run bimpy on a headless machine ");
		}

		m_imgui = ImGui::CreateContext();
		GImGui = m_imgui;

		ImGui_ImplGlfw_InitForOpenGL(m_window, false);
		ImGui_ImplOpenGL3_Init(glsl_version);

		Render::debug_guard<> m_guard;
		m_dr.Init();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		m_width = width;
		m_height = height;

		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
		io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
		io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
		io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
		io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
		io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
		io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

		io.ConfigFlags |= 0
				| ImGuiConfigFlags_NavEnableGamepad
				| ImGuiConfigFlags_NavEnableKeyboard
				;

		glfwSetWindowUserPointer(m_window, this); // replaced m_imp.get()

		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
		{
			Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));
			ctx->Resize(width, height);
		});

		glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int, int action, int mods)
		{
			Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));

		    ImGuiContext& g = *GImGui;
			if (!(g.NavWindow != NULL))
			{
				ctx->keyboard_callback(key, action, mods);
			}
			else
			{
				ImGuiIO& io = ImGui::GetIO();
				if (action == GLFW_PRESS)
				{
					io.KeysDown[key] = true;
				}
				if (action == GLFW_RELEASE)
				{
					io.KeysDown[key] = false;
				}

				io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
				io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
				io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
				io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
			}
		});

		glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int c)
		{
		    ImGuiContext& g = *GImGui;
			if (!(g.NavWindow != NULL))
			{
				Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));
				//ctx->OnSetChar(c);
			}
			else
			{
				ImGuiIO& io = ImGui::GetIO();
				io.AddInputCharacter((unsigned short) c);
			}
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double /*xoffset*/, double yoffset)
		{
			Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));
		    ImGuiContext& g = *GImGui;
			if (!(g.NavWindow != NULL))
			{
				ctx->m_camera.Scroll(float(-yoffset));
			}
			else
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MouseWheel += (float) yoffset * 2.0f;
			}
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int /*mods*/)
		{
			Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));
			if (button == 1)
				ctx->m_camera.TogglePanning(action == GLFW_PRESS);
			if (button == 0 && ctx->mouse_button_callback)
			{
				double x, y;
				glfwGetCursorPos(window, &x, &y);
				glm::vec2 cursorposition = glm::vec2(x, y);
				auto local = glm::vec2(ctx->m_camera.GetWorldToCanvas() * glm::vec3(cursorposition, 1.0f));
				ctx->mouse_button_callback(action == GLFW_PRESS, float(x), float(y), local.x, local.y);
			}
		});
		glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double x, double y)
		{
			Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));
			if (ctx->mouse_position_callback)
			{
				glm::vec2 cursorposition = glm::vec2(x, y);
				auto local = glm::vec2(ctx->m_camera.GetWorldToCanvas() * glm::vec3(cursorposition, 1.0f));
				ctx->mouse_position_callback(float(x), float(y), local.x, local.y);
			}
		});

		vg = nvgCreateContext(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
		if (vg == nullptr)
		{
			spdlog::error("Error, Could not init nanovg.");
		}

		const char* vertex_shader_src = R"(
			uniform mat4  u_modelViewProj;

			attribute vec2 a_position;
			varying vec2 v_pos;

			void main()
			{
				v_pos = a_position.xy;
				gl_Position = u_modelViewProj * vec4(a_position, 0.0, 1.0);
			}
		)";

		const char* fragment_shader_src = R"(
			uniform vec2 u_world_size;
			varying vec2 v_pos;

			vec4 render(float d, vec4 color, float w)
			{
			    float anti = fwidth(d) * 1.0;
			    return vec4(color.rgb, color.a * (1.0-smoothstep(-anti, anti, d - w)));
			}

			vec4 gamma(vec4 c)
			{
				return vec4(pow(c.xyz, vec3(2.2)), c.a);
			}

			void render_layer(inout vec4 c, vec4 layer)
			{
			    c.rgb = mix(c.rgb, layer.rgb, layer.a);
			}

			float grid(vec2 uv, float s)
			{
			    uv += s/2.;
			    vec2 a1 = mod(uv, s);
			    return min(abs(a1.x - s / 2.), abs(a1.y - s / 2.));
			}

			void make_grid(inout vec4 c, vec2 fragCoord)
			{
			    c = vec4( pow(vec3(.12,.56,.81), vec3(2.2)),1.0);
			    float d;
			    float scale = length(fwidth(fragCoord));

				float a = 0.2;
				float s = 1.0;
				float w = 0.5;

				for(int i = 0; i < 4; ++i)
				{
					if (scale * 10. < s)
					{
						d = grid(fragCoord, s);
				        render_layer(c, render(d, gamma(vec4(.8,.8,.7, a)), w * scale));

						a *= 1.7;
						w += 0.3;
					}
					s *= 10.;
				}
			}

			void main()
			{
				vec2 q = v_pos * vec2(0.5, 0.5) + vec2(0.5);
				q *= (u_world_size + 1.);
				q -= 0.5;
				make_grid(gl_FragColor, q);
			}
		)";

		m_program = Render::MakeProgram(vertex_shader_src, fragment_shader_src);
		u_modelViewProj = m_program->GetUniform("u_modelViewProj");
		u_world_size = m_program->GetUniform("u_world_size");
		std::vector<glm::vec2> vertices = {
				{-1.0f, -1.0f},
				{ 1.0f, -1.0f},
				{ 1.0f,  1.0f},
				{-1.0f,  1.0f},
		};
		std::vector<int> indices;

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(0);
		indices.push_back(2);
		indices.push_back(3);

		m_buff.FillBuffers(vertices.data(), vertices.size(), sizeof(glm::vec2), indices.data(), indices.size(), 4);

		m_spec = Render::VertexSpecMaker().PushType<glm::vec2>("a_position");

		m_text.reset(new SimpleText);

		m_2drender.Init();
	}
}


void pth::Context::Recenter(float w, float h, RECENTER r)
{
	m_world_size = glm::vec2(w, h);
	auto size = m_world_size;
	if (r == RECENTER::FIT_DOCUMENT)
	{
		if (m_width * 1.0f / m_height > size.x * 1.0f / size.y)
		{
			m_camera.SetFOV(size.y * 1.2f / m_height);
		}
		else
		{
			m_camera.SetFOV(size.x * 1.2f / m_width);
		}
	}
	else
	{
		m_camera.SetFOV(1.0f);
	}

	auto clientArea = glm::vec2(m_width, m_height);

	clientArea = glm::ivec2(glm::vec2(clientArea) * m_camera.GetFOV());

	auto pos = glm::vec2(clientArea - size) / 2.0f;
	m_camera.SetPos(pos);
}


void pth::Context::Recenter(float x0, float y0, float x1, float y1)
{
	auto p0 = glm::vec2(x0, y0);
	auto p1 = glm::vec2(x1, y1);
	auto size = p1 - p0;

	glm::vec2 r = glm::vec2(m_width, m_height) / glm::vec2(size);

	if (m_width * 1.0f / m_height > size.x * 1.0f / size.y)
	{
		m_camera.SetFOV(size.y * 1.2f / m_height);
	}
	else
	{
		m_camera.SetFOV(size.x * 1.2f / m_width);
	}

	auto clientArea = glm::vec2(m_width, m_height);

	clientArea = glm::ivec2(glm::vec2(clientArea) * m_camera.GetFOV());

	auto pos = glm::vec2(clientArea - size) / 2.0f;
	m_camera.SetPos(pos - p0);
}


pth::Context::~Context()
{
	glfwSetWindowSizeCallback(m_window, nullptr);
	glfwTerminate();
}


void pth::Context::Render()
{
	auto size = m_world_size;

	auto transform = m_camera.GetTransform();
	glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3((size.x + 1) / 2.0f,  (size.y + 1) / 2.0f, 0.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(1.0, 1.0, 0.0f));
	model[3].x -= 0.5;
	model[3].y -= 0.5;
	// Render::DrawRect(m_dr, glm::vec2(-1.0f), glm::vec2(1.0f), transform * model);
	{
		m_program->Use();
		u_modelViewProj.ApplyValue(transform * model);
		u_world_size.ApplyValue(m_world_size);

		m_buff.Bind();
		m_spec.Enable();
		m_buff.DrawElements();
		m_buff.UnBind();
		m_spec.Disable();

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Render::View view_box(glm::vec2(m_width, m_height), 72);
	m_2drender.SetUp(view_box);
	m_2drender.Draw();

	{
		auto transform = m_camera.GetCanvasToWorld();

		glm::vec2 pos = transform * glm::vec3(-0.5, -0.5, 1);
		size = transform * glm::vec3(size + 1.0f, 0);


		float margin = size.x * 0.3;
		NVGpaint shadowPaint = nvgBoxGradient(
				vg, pos.x, pos.y, size.x, size.y, 0, margin * 0.03,
				{0, 0, 0, 1.0f}, {0, 0, 0, 0});

		nvgSave(vg);
		nvgResetScissor(vg);
		nvgBeginPath(vg);
		nvgRect(vg, pos.x - margin, pos.y - margin, size.x + 2 * margin, size.y + 2 * margin);
		nvgRect(vg, pos.x, pos.y, size.x, size.y);
		nvgPathWinding(vg, NVG_HOLE);
		nvgFillPaint(vg, shadowPaint);
		nvgFill(vg);
		nvgRestore(vg);
		nvgEndFrame(vg);
	}
	auto c2w_transform = m_camera.GetCanvasToWorld();

	m_text->EnableBlending(true);
	m_text->Render();

	glDisable(GL_FRAMEBUFFER_SRGB);
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapInterval(1);
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}


void pth::Context::NewFrame()
{
	double x, y;
	glfwGetCursorPos(m_window, &x, &y);
	glm::vec2 cursorposition = glm::vec2(x, y);
	m_camera.Move(cursorposition.x, cursorposition.y);
	m_camera.UpdateViewProjection(m_width, m_height);

	glfwMakeContextCurrent(m_window);
	Render::debug_guard<> m_guard;
	glEnable(GL_FRAMEBUFFER_SRGB);
	glViewport(0, 0, m_width, m_height);
	glClear(GL_COLOR_BUFFER_BIT);
	nvgBeginFrame(vg, m_width, m_height, 1.0f);

	GImGui = m_imgui;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();

	m_ctrl_c_released = m_ctrl_c_down && ! (ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeysDown[GLFW_KEY_C]);
	m_ctrl_c_down = ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeysDown[GLFW_KEY_C];
	m_ctrl_v_released = m_ctrl_v_down && ! (ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeysDown[GLFW_KEY_V]);
	m_ctrl_v_down = ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeysDown[GLFW_KEY_V];
	m_ctrl_x_released = m_ctrl_x_down && ! (ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeysDown[GLFW_KEY_X]);
	m_ctrl_x_down = ImGui::GetIO().KeyCtrl && ImGui::GetIO().KeysDown[GLFW_KEY_X];
}


void pth::Context::Resize(int width, int height)
{
	auto oldWindowBufferSize = glm::vec2(m_width, m_height);
	m_width = width;
	m_height = height;
	m_camera.UpdateViewProjection(m_width, m_height);
	auto size = m_world_size;

	auto oldClientArea = oldWindowBufferSize;
	auto clientArea = glm::vec2(m_width, m_height);// - glm::ivec2(0, MainMenuBar * m_window->GetPixelScale());

	auto pos = m_camera.GetPos();
	auto delta = glm::vec2((clientArea - oldClientArea) / 2.0f) * m_camera.GetFOV();
	m_camera.SetPos(pos + delta);
}


bool pth::Context::ShouldClose()
{
	return glfwWindowShouldClose(m_window) != 0;
}

int pth::Context::GetWidth() const
{
	return m_width;
}

int pth::Context::GetHeight() const
{
	return m_height;
}

void pth::Context::Point(float x, float y, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> color, float point_size) const
{
	auto transform = m_camera.GetCanvasToWorld();

	glm::vec2 point_pos_local = glm::vec2(x, y);
	glm::vec2 point_pos = transform * glm::vec3(point_pos_local, 1);

	nvgBeginPath(vg);
	nvgCircle(vg, point_pos.x, point_pos.y, point_size);
	nvgFillColor(vg, nvgRGBA(std::get<0>(color), std::get<1>(color), std::get<2>(color), std::get<3>(color)));
	nvgFill(vg);
	nvgBeginPath(vg);

	nvgCircle(vg, point_pos.x, point_pos.y, point_size * 6);
	nvgCircle(vg, point_pos.x, point_pos.y, point_size);
	nvgPathWinding(vg, NVG_HOLE);
	NVGpaint rshadowPaint = nvgBoxGradient(
			vg, point_pos.x - point_size, point_pos.y - point_size, point_size * 2, point_size * 2, point_size,
			point_size * 0.3,
			{0, 0, 0, 1.0f}, {0, 0, 0, 0});
	nvgFillPaint(vg, rshadowPaint);
	nvgFill(vg);
}

void pth::Context::Box(float minx, float miny, float maxx, float maxy, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> color_stroke, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> color_fill) const
{
	auto transform = m_camera.GetCanvasToWorld();

	glm::aabb2 box(transform * glm::vec3(minx, miny, 1), transform * glm::vec3(maxx, maxy, 1));

	nvgBeginPath(vg);
	nvgRect(vg, box);
	nvgFillColor(vg, nvgRGBA(std::get<0>(color_stroke), std::get<1>(color_fill), std::get<2>(color_fill), std::get<3>(color_fill)));
	nvgFill(vg);

	nvgBeginPath(vg);
	nvgRect(vg, box);
	nvgStrokeColor(vg, nvgRGBA(std::get<0>(color_stroke), std::get<1>(color_stroke), std::get<2>(color_stroke), std::get<3>(color_stroke)));
	nvgStroke(vg);
}


PYBIND11_MODULE(_getoolkit, m) {
	m.doc() = "getoolkit";

	py::class_<glm::vec2>(m, "vec2")
	    .def(py::init<float, float>())
	    .def(py::init<float>())
	    .def(py::init<>())
		.def_readwrite("x", &glm::vec2::x)
		.def_readwrite("y", &glm::vec2::y)
	    .def(py::self == py::self)
	    .def(py::self != py::self)
	    .def(py::self += py::self)
	    .def(py::self + py::self)
	    .def(py::self *= float{})
	    .def(py::self * float{})
	    .def(py::self *= py::self)
	    ;

	py::class_<glm::vec3>(m, "vec3")
	    .def(py::init<float, float, float>())
	    .def(py::init<float>())
	    .def(py::init<>())
		.def_readwrite("x", &glm::vec3::x)
		.def_readwrite("y", &glm::vec3::y)
		.def_readwrite("z", &glm::vec3::z)
	    .def(py::self == py::self)
	    .def(py::self != py::self)
	    .def(py::self += py::self)
	    .def(py::self + py::self)
	    .def(py::self *= float{})
	    .def(py::self * float{})
	    .def(py::self *= py::self)
	    ;

	py::class_<glm::vec4>(m, "vec4")
	    .def(py::init<float, float, float, float>())
	    .def(py::init<float>())
	    .def(py::init<>())
		.def_readwrite("x", &glm::vec4::x)
		.def_readwrite("y", &glm::vec4::y)
		.def_readwrite("z", &glm::vec4::z)
		.def_readwrite("w", &glm::vec4::w)
	    .def(py::self == py::self)
	    .def(py::self != py::self)
	    .def(py::self += py::self)
	    .def(py::self + py::self)
	    .def(py::self *= float{})
	    .def(py::self * float{})
	    .def(py::self *= py::self)
	    ;

	py::class_<Render::color>(m, "color")
	    .def(py::init<int, int, int, int>())
	    .def(py::init<int>())
	    .def(py::init<>())
		.def_readwrite("r", &Render::color::r)
		.def_readwrite("g", &Render::color::g)
		.def_readwrite("b", &Render::color::b)
		.def_readwrite("a", &Render::color::a)
//	    .def(py::self == py::self)
//	    .def(py::self != py::self)
//	    .def(py::self += py::self)
//	    .def(py::self + py::self)
//	    .def(py::self *= int{})
//	    .def(py::self * int{})
//	    .def(py::self *= py::self)
	    ;

	py::class_<glm::aabb2>(m, "aabb2")
		.def(py::init())
		.def(py::init<glm::vec2>())
		.def(py::init<glm::vec2, glm::vec2>())
		.def_readwrite("minp", &glm::aabb2::minp)
		.def_readwrite("maxp", &glm::aabb2::maxp)
		.def("size", &glm::aabb2::size)
		.def("center", &glm::aabb2::center)
		.def("set", &glm::aabb2::set)
		.def("reset", &glm::aabb2::reset)
		.def("is_positive", &glm::aabb2::is_positive)
		.def("is_negative", &glm::aabb2::is_negative)
		;

//		void PushScissors(glm::aabb2 box);
//
//		void PopScissors();
//
//		void Rect(glm::aabb2 rect, color c, glm::vec4 radius = glm::vec4(0));
//
//		void Rect(glm::aabb2 rect, TexturePtr texture, glm::aabb2 uv = glm::aabb2(glm::vec2(1.0), glm::vec2(0.0)), glm::vec4 radius = glm::vec4(0));
//
//		void Rect(glm::aabb2 rect, const glm::mat3& transform, TexturePtr texture, glm::aabb2 uv = glm::aabb2(glm::vec2(1.0), glm::vec2(0.0)));
//
//		void Text(glm::aabb2 rect, const char* text, size_t len = 0);

	py::class_<Render::Encoder>(m, "Encoder")
		.def(py::init())
		.def("push_scissors", &Render::Encoder::PushScissors)
		.def("pop_scissors", &Render::Encoder::PopScissors)
		.def("rect", [](Render::Encoder& self, glm::vec2 minp, glm::vec2 maxp, Render::color c){ self.Rect({minp, maxp}, c); })
		.def("rect", [](Render::Encoder& self, glm::vec2 minp, glm::vec2 maxp, Render::color c, glm::vec4 radius){ self.Rect({minp, maxp}, c, radius); })
		;

	py::class_<pth::Context>(m, "Context")
		.def(py::init())
		.def("init", &pth::Context::Init, "Initializes context and creates window")
		.def("new_frame", &pth::Context::NewFrame, "Starts a new frame. NewFrame must be called before any imgui functions")
		.def("render", &pth::Context::Render, "Finilizes the frame and draws all UI. Render must be called after all imgui functions")
		.def("should_close", &pth::Context::ShouldClose)
		.def("width", &pth::Context::GetWidth)
		.def("height", &pth::Context::GetHeight)
		.def("set_world_size", [](pth::Context& self, float w, float h)
			{
				self.Recenter(w, h, pth::Context::FIT_DOCUMENT);
			})
		.def("set_without_recenter", [](pth::Context& self, float w, float h)
			{
				self.m_world_size = glm::vec2(w, h);
			})
		.def("recenter", [](pth::Context& self)
			{
				self.Recenter(self.m_world_size.x, self.m_world_size.y, pth::Context::FIT_DOCUMENT);
			})
		.def("set_roi", [](pth::Context& self, float x0, float y0, float x1, float y1)
			{
				self.Recenter(x0, y0, x1, y1);
			})
		.def("__enter__", &pth::Context::NewFrame)
		.def("__exit__", [](pth::Context& self, py::object, py::object, py::object)
			{
				self.Render();
			})
		.def("set_mouse_button_callback", [](pth::Context& self, py::function f){
			self.mouse_button_callback = f;
		})
		.def("set_mouse_button_callback", [](pth::Context& self, py::function f){
			self.mouse_button_callback = f;
		})
		.def("set_mouse_position_callback", [](pth::Context& self, py::function f){
			self.mouse_position_callback = f;
		})
		.def("get_mouse_position", [](pth::Context& self){
				double x, y;
				glfwGetCursorPos(self.m_window, &x, &y);
				glm::vec2 cursorposition = glm::vec2(x, y);
				auto local = glm::vec2(self.m_camera.GetWorldToCanvas() * glm::vec3(cursorposition, 1.0f));
				return std::make_tuple(float(x), float(y), local.x, local.y);
		})
		.def("set_keyboard_callback", [](pth::Context& self, py::function f){
			self.keyboard_callback = f;
		})
		.def("text", [](pth::Context& self, const char* str, int x, int y, SimpleText::Alignment align)
		{
			self.m_text->Label(str, x, y, align);
		})
		.def("text", [](pth::Context& self, const char* str, int x, int y, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> color, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> bg_color, SimpleText::Alignment align)
		{
			self.m_text->SetColorf(SimpleText::TEXT_COLOR, std::get<0>(color) / 255.f, std::get<1>(color) / 255.f, std::get<2>(color) / 255.f, std::get<3>(color) / 255.f);
			self.m_text->SetColorf(SimpleText::BACKGROUND_COLOR, std::get<0>(bg_color) / 255.f, std::get<1>(bg_color) / 255.f, std::get<2>(bg_color) / 255.f, std::get<3>(bg_color) / 255.f);
			self.m_text->EnableBlending(true);
			self.m_text->Label(str, x, y, align);
			self.m_text->ResetFont();
		})
		.def("text_loc", [](pth::Context& self, const char* str, float x, float y, SimpleText::Alignment align)
		{
			auto transform = self.m_camera.GetCanvasToWorld();

			glm::vec2 pos_local = glm::vec2(x, y);
			glm::vec2 pos = transform * glm::vec3(pos_local, 1);

			self.m_text->Label(str, pos.x, pos.y, align);
		})
		.def("loc_2_win", [](pth::Context& self, float x, float y)
		{
			auto transform = self.m_camera.GetCanvasToWorld();
			glm::vec2 pos_local = glm::vec2(x, y);
			glm::vec2 pos = transform * glm::vec3(pos_local, 1);
			return std::tuple<float, float>(pos.x, pos.y);
		})
		.def("loc_2_win", [](pth::Context& self, glm::vec2 pos_local)
		{
			auto transform = self.m_camera.GetCanvasToWorld();
			glm::vec2 pos = transform * glm::vec3(pos_local, 1);
			return pos;
		})
		.def("win_2_loc", [](pth::Context& self, float x, float y)
		{
			auto transform = self.m_camera.GetWorldToCanvas();
			glm::vec2 pos_local = glm::vec2(x, y);
			glm::vec2 pos = transform * glm::vec3(pos_local, 1);
			return std::tuple<float, float>(pos.x, pos.y);
		})
		.def("get_scale", [] (pth::Context& self) { return 1.0 / self.m_camera.GetFOV(); })
		.def("text_loc", [](pth::Context& self, const char* str, float x, float y, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> color, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> bg_color, SimpleText::Alignment align)
		{
			auto transform = self.m_camera.GetCanvasToWorld();

			glm::vec2 pos_local = glm::vec2(x, y);
			glm::vec2 pos = transform * glm::vec3(pos_local, 1);

			self.m_text->SetColorf(SimpleText::TEXT_COLOR, std::get<0>(color) / 255.f, std::get<1>(color) / 255.f, std::get<2>(color) / 255.f, std::get<3>(color) / 255.f);
			self.m_text->SetColorf(SimpleText::BACKGROUND_COLOR, std::get<0>(bg_color) / 255.f, std::get<1>(bg_color) / 255.f, std::get<2>(bg_color) / 255.f, std::get<3>(bg_color) / 255.f);
			self.m_text->EnableBlending(true);

			self.m_text->Label(str, pos.x, pos.y, align);
			self.m_text->ResetFont();
		})
		.def("get_encoder", [](pth::Context& self){ return self.m_2drender.GetEncoder(); }, py::return_value_policy::reference)
		.def("point",  &pth::Context::Point, py::arg("x"), py::arg("y"), py::arg("color"), py::arg("radius") = 5)
		.def("get_imgui", [](pth::Context& self) { return (void*)self.m_imgui; })
		.def("box",  &pth::Context::Box)
		.def_readonly("ctrl_c",  &pth::Context::m_ctrl_c_released)
		.def_readonly("ctrl_x",  &pth::Context::m_ctrl_x_released)
		.def_readonly("ctrl_v",  &pth::Context::m_ctrl_v_released)

		.def("nvgBeginPath",  [](pth::Context& self) { nvgBeginPath(self.vg); })
		.def("nvgFillColor",  [](pth::Context& self, glm::vec4 c) { nvgFillColor(self.vg, c); })
		.def("nvgStrokeColor",  [](pth::Context& self, glm::vec4 c) { nvgStrokeColor(self.vg, c); })
		.def("nvgStrokeWidth",  [](pth::Context& self, float w) { nvgStrokeWidth(self.vg, w); })

		.def("nvgFill",  [](pth::Context& self) { nvgFill(self.vg); })
		.def("nvgStroke",  [](pth::Context& self) { nvgStroke(self.vg); })
		.def("nvgResetTransform",  [](pth::Context& self) { nvgStroke(self.vg); })
		.def("nvgRotate",  [](pth::Context& self, float a) { nvgRotate(self.vg, a); })
		.def("nvgScale",  [](pth::Context& self, glm::vec2 x) { nvgScale(self.vg, x.x, x.y); })
		.def("nvgTranslate",  [](pth::Context& self, glm::vec2 x) { nvgTranslate(self.vg, x.x, x.y); })
		.def("nvgScissor",  [](pth::Context& self, glm::aabb2 x) { nvgScissor(self.vg, x.minp.x, x.minp.y, x.size().x, x.size().y); })
		.def("nvgResetScissor",  [](pth::Context& self, glm::aabb2 x) { nvgResetScissor(self.vg); })
		.def("nvgMoveTo",  [](pth::Context& self, glm::vec2 x) { nvgMoveTo(self.vg, x.x, x.y); })
		.def("nvgLineTo",  [](pth::Context& self, glm::vec2 x) { nvgLineTo(self.vg, x.x, x.y); })
		.def("nvgQuadTo",  [](pth::Context& self, glm::vec2 c, glm::vec2 e) { nvgQuadTo(self.vg, c.x, c.y, e.x, e.y); })
		.def("nvgBezierTo",  [](pth::Context& self, glm::vec2 c1, glm::vec2 c2, glm::vec2 e) { nvgBezierTo(self.vg, c1.x, c1.y, c2.x, c2.y, e.x, e.y); })
		.def("nvgClosePath",  [](pth::Context& self) { nvgClosePath(self.vg); })
		.def("nvgPathWinding",  [](pth::Context& self, int d) { nvgPathWinding(self.vg, d); })
		.def("nvgRect",  [](pth::Context& self, glm::aabb2 d) { nvgRect(self.vg, d); })
		;

		py::enum_<SpecialKeys>(m, "SpecialKeys")
			.value("KeyEscape", KeyEscape)
			.value("KeyEnter", KeyEnter)
			.value("KeyTab", KeyTab)
			.value("KeyBackspace", KeyBackspace)
			.value("KeyInsert", KeyInsert)
			.value("KeyDelete", KeyDelete)
			.value("KeyRight", KeyRight)
			.value("KeyLeft", KeyLeft)
			.value("KeyDown", KeyDown)
			.value("KeyUp", KeyUp)
			.export_values();

		py::enum_<SimpleText::Alignment>(m, "Alignment")
			.value("Left", SimpleText::LEFT)
			.value("Center", SimpleText::CENTER)
			.value("Right", SimpleText::RIGHT)
			.export_values();



//	py::class_<pth::Image, std::shared_ptr<pth::Image> >(m, "Image")
//			.def(py::init<std::vector<ndarray_uint8>>(), "")
//			.def("grayscale_to_alpha", &pth::Image::GrayScaleToAlpha, "For grayscale images, uses values as alpha")
//			.def_readonly("width", &pth::Image::m_width)
//			.def_readonly("height", &pth::Image::m_height);
}
