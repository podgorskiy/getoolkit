#pragma once
#include <inttypes.h>

namespace Render
{
	bool ProcessGLErrorEvents(bool reportErrors = false);

	class DebugDevice
	{
	public:
		void lock();
		void unlock();

		bool enabled_high;
		bool enabled_medium;
		bool enabled_low;
		bool enabled_notify;
	};

	template<bool high=true, bool medium=true, bool low=true, bool notify=false>
	class debug_guard
	{
	public:
	    debug_guard() : m_device(DebugDevice({high, medium, low, notify}))
	    {
	    	m_device.lock();
	    }
	    explicit debug_guard(DebugDevice& m) : m_device(m)
	    {
	    	m_device.lock();
	    }
	    ~debug_guard()
	    {
	    	m_device.unlock();
	    }

	    debug_guard(const debug_guard&) = delete;
	    debug_guard& operator=(const debug_guard&) = delete;

	private:
		DebugDevice  m_device;
	};

	bool CheckExtension(const char* extension);

	inline bool has_s3tc() { return CheckExtension("GL_EXT_texture_compression_s3tc") || CheckExtension("WEBGL_compressed_texture_s3tc") || CheckExtension("GL_NV_texture_compression_s3tc"); }

	inline bool has_s3tc_srgb() { return CheckExtension("GL_EXT_texture_compression_s3tc_srgb") || CheckExtension("WEBGL_compressed_texture_s3tc_srgb"); }

	inline bool has_pvrtc() { return CheckExtension("WEBGL_compressed_texture_pvrtc") || CheckExtension("GL_IMG_texture_compression_pvrtc"); }

	inline bool has_pvrtc2() { return CheckExtension("GL_IMG_texture_compression_pvrtc2"); }

	inline bool has_astc() { return CheckExtension("GL_KHR_texture_compression_astc_hdr") || CheckExtension("GL_OES_texture_compression_astc") || CheckExtension("WEBGL_compressed_texture_astc"); }

	inline bool has_latc() { return CheckExtension("GL_NV_texture_compression_latc") || CheckExtension("GL_EXT_texture_compression_latc"); }

	inline bool has_bptc() { return CheckExtension("GL_EXT_texture_compression_bptc") || CheckExtension("GL_ARB_texture_compression_bptc"); }

	inline bool has_rgtc() { return CheckExtension("GL_EXT_texture_compression_rgtc") || CheckExtension("GL_ARB_texture_compression_rgtc"); }
}

