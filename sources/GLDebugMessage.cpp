#include "GLDebugMessage.h"
#include <assert.h>
#include <GL/gl3w.h>
#include <spdlog/spdlog.h>

#include <set>


using namespace Render;

#define CASE(ENUM)  case GL_DEBUG_##ENUM: res = #ENUM


void APIENTRY DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                          const void* userParam)
{
	DebugDevice* dd = (DebugDevice*) userParam;
	const char* res= "";
	switch (source)
	{
		CASE(SOURCE_API); break;
		CASE(SOURCE_WINDOW_SYSTEM); break;
		CASE(SOURCE_SHADER_COMPILER); break;
		CASE(SOURCE_THIRD_PARTY); break;
		CASE(SOURCE_APPLICATION); break;
		CASE(SOURCE_OTHER); break;
	}
	const char* sourceStr = res;

	switch (severity)
	{
		CASE(SEVERITY_HIGH); if (!dd->enabled_high) return; break;
		CASE(SEVERITY_MEDIUM); if (!dd->enabled_medium) return; break;
		CASE(SEVERITY_LOW); if (!dd->enabled_low) return; break;
		CASE(SEVERITY_NOTIFICATION); if (!dd->enabled_notify) return; break;
	}
	const char* severityStr = res;

	switch (type)
	{
		CASE(TYPE_ERROR);
		CASE(TYPE_DEPRECATED_BEHAVIOR);
		CASE(TYPE_PORTABILITY);
		CASE(TYPE_PERFORMANCE);
		CASE(TYPE_MARKER);
		CASE(TYPE_OTHER);
	}
	const char* typeStr = res;

	spdlog::warn("[{}:{}:{}] {}", sourceStr, severityStr, typeStr, message);
}

static const char* GetErrorString(uint32_t err)
{
	switch (err)
	{
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";

		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";

		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";

		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";

		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";

		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";

		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";

		case GL_CONTEXT_LOST:
			return "GL_CONTEXT_LOST";

		default:
			return "Unknown error";
	}
}


bool Render::ProcessGLErrorEvents(bool reportErrors)
{
	bool errors = false;
	GLenum err = GL_NO_ERROR;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		errors = true;
		if (reportErrors)
		{
			auto str = GetErrorString(err);
			spdlog::warn("{}", str);
		}
	}
	return errors;
}


void DebugDevice::lock()
{
	glDebugMessageCallback(&DebugMessageCallback, this);
	glEnable(GL_DEBUG_OUTPUT);
	ProcessGLErrorEvents();
}

void DebugDevice::unlock()
{
	glDisable(GL_DEBUG_OUTPUT);
	bool errors = ProcessGLErrorEvents(true);
	glDebugMessageCallback(nullptr, nullptr);
}


static std::set<std::string> InitExtensionList()
{
	std::set<std::string> extensions;
	GLint n = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);

	spdlog::info("Available OpenGL extensions:");

	for (GLint i = 0; i < n; i++)
	{
		const char* extension = (const char*) glGetStringi(GL_EXTENSIONS, i);
		extensions.insert(extension);
		spdlog::info("\t- {}", extension);
	}
	return extensions;
}

bool Render::CheckExtension(const char* extension)
{
	static std::set<std::string> extensions = InitExtensionList();
	return extensions.find(extension) != extensions.end();
}

static std::set<std::string> GetSupportedTextureCompressionFormats()
{
	std::set<std::string> extensions;
	GLint n = 0;
	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &n);

	spdlog::info("Available {} Texture compression formats:", n);
	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &n);
	GLint* fs = new GLint[n];
	glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, fs);
	for (int i = 0; i < n; i++)
	{
		// _compressed_texure_formats.push_back(*(fs + i));
		spdlog::info("{:x}", *(fs + i));
	}

//	const char* extension = (const char*) glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, i);
//	spdlog::info("\t- {}", extension);
	return extensions;
}

#include <doctest.h>

TEST_CASE("[Render] Extensions")
{
	SUBCASE("Basic")
	{
		CHECK_EQ(CheckExtension("Does not exists"), false);
		GetSupportedTextureCompressionFormats();
	}

	SUBCASE("Check compression formats")
	{
		spdlog::info("has_s3tc: {}", has_s3tc());
		spdlog::info("has_s3tc_srgb: {}", has_s3tc_srgb());
		spdlog::info("has_pvrtc: {}", has_pvrtc());
		spdlog::info("has_pvrtc2: {}", has_pvrtc2());
		spdlog::info("has_astc: {}", has_astc());
		spdlog::info("has_latc: {}", has_latc());
		spdlog::info("has_bptc: {}", has_bptc());
		spdlog::info("has_rgtc: {}", has_rgtc());

		CHECK(has_s3tc());

		CHECK(CheckExtension("GL_EXT_texture_sRGB"));
	}
}
