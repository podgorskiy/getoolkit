#pragma once
#include <2DEngine/Renderer2D.h>
#include "Render/Texture.h"
#include "Enums.h"


namespace UI
{
	class Block;

	class IEmitter
	{
	public:
		virtual void operator()(Render::Encoder* encoder, const Block*, float time, int flags) = 0;

		virtual ~IEmitter() = default;
	};

	class SFillEmitter : public IEmitter
	{
	public:
		explicit SFillEmitter(Render::color c);

		virtual void operator()(Render::Encoder* encoder, const Block*, float time, int flags);

	private:
		Render::color col;
	};

	class SImageEmitter : public IEmitter
	{
	public:
		SImageEmitter(Render::TexturePtr tex, ImSize::Enum size, ImPos::Enum pos, ImTransform::Enum t);

		virtual void operator()(Render::Encoder* encoder, const Block*, float time, int flags);

	private:
		glm::ivec2 image_size;
		Render::TexturePtr tex;
		UI::ImSize::Enum size;
		UI::ImPos::Enum pos;
		ImTransform::Enum t;
	};

	class STextEmitter : public IEmitter
	{
	public:
		STextEmitter(std::string text);

		virtual void operator()(Render::Encoder* encoder, const Block*, float time, int flags);

	private:
		std::string text;
	};

	struct EmitterSizeCheck
	{
		enum
		{
			DataSize = 40,
		};

		void check()
		{
			static_assert(DataSize >= sizeof(IEmitter), "");
			static_assert(DataSize >= sizeof(SImageEmitter), "");
		}
	};
}
