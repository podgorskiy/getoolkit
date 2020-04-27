#pragma once


// Requires EXT_texture_compression_s3tc or WEBGL_compressed_texture_s3tc or NV_texture_compression_s3tc https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_compression_s3tc.txt
// GL_EXT_texture_compression_dxt1
#ifndef COMPRESSED_RGB_S3TC_DXT1_EXT
#define COMPRESSED_RGB_S3TC_DXT1_EXT                      0x83F0
#endif

#ifndef COMPRESSED_RGBA_S3TC_DXT1_EXT
#define COMPRESSED_RGBA_S3TC_DXT1_EXT                     0x83F1
#endif

#ifndef COMPRESSED_RGBA_S3TC_DXT3_EXT
#define COMPRESSED_RGBA_S3TC_DXT3_EXT                     0x83F2
#endif

#ifndef COMPRESSED_RGBA_S3TC_DXT5_EXT
#define COMPRESSED_RGBA_S3TC_DXT5_EXT                     0x83F3
#endif

// Requires EXT_texture_compression_s3tc_srgb https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_compression_s3tc_srgb.txt
// WEBGL_compressed_texture_s3tc_srgb https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_s3tc_srgb/
#define COMPRESSED_SRGB_S3TC_DXT1_EXT                     0x8C4C
#define COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT               0x8C4D
#define COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT               0x8C4E
#define COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT               0x8C4F

// Requires WEBGL_compressed_texture_pvrtc or GL_IMG_texture_compression_pvrtc https://www.khronos.org/registry/OpenGL/extensions/IMG/IMG_texture_compression_pvrtc.txt
// https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_pvrtc/
#ifndef COMPRESSED_RGB_PVRTC_4BPPV1_IMG
#define COMPRESSED_RGB_PVRTC_4BPPV1_IMG                   0x8C00
#endif
#ifndef COMPRESSED_RGB_PVRTC_2BPPV1_IMG
#define COMPRESSED_RGB_PVRTC_2BPPV1_IMG                   0x8C01
#endif
#ifndef COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
#define COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                  0x8C02
#endif
#ifndef COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
#define COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                  0x8C03
#endif


// Requires GL_IMG_texture_compression_pvrtc2 https://www.khronos.org/registry/OpenGL/extensions/IMG/IMG_texture_compression_pvrtc2.txt
#define COMPRESSED_RGBA_PVRTC_2BPPV2_IMG                  0x9137
#define COMPRESSED_RGBA_PVRTC_4BPPV2_IMG                  0x9138
/*
 * // Upload a 4bpp PVRTC2 texture
 * GLuint bitsPerPixel = 4;
 * glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG, 1024, 1024, 0, (width*height*bitsPerPixel)/8, pixelData);
 * // Upload a 2bpp PVRTC2 texture
 * GLuint bitsPerPixel = 2;
 * glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG, 1024, 1024, 0, (width*height*bitsPerPixel)/8, pixelData);
 */

// Regires GL_OES_texture_compression_astc https://www.khronos.org/registry/OpenGL/extensions/OES/OES_texture_compression_astc.txt
// https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_texture_compression_astc_hdr.txt
//    GL_KHR_texture_compression_astc_hdr
//    GL_KHR_texture_compression_astc_ldr
//    WEBGL_compressed_texture_astc
// https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_astc/

#define COMPRESSED_RGBA_ASTC_4x4_KHR                      0x93B0
#define COMPRESSED_RGBA_ASTC_5x4_KHR                      0x93B1
#define COMPRESSED_RGBA_ASTC_5x5_KHR                      0x93B2
#define COMPRESSED_RGBA_ASTC_6x5_KHR                      0x93B3
#define COMPRESSED_RGBA_ASTC_6x6_KHR                      0x93B4
#define COMPRESSED_RGBA_ASTC_8x5_KHR                      0x93B5
#define COMPRESSED_RGBA_ASTC_8x6_KHR                      0x93B6
#define COMPRESSED_RGBA_ASTC_8x8_KHR                      0x93B7
#define COMPRESSED_RGBA_ASTC_10x5_KHR                     0x93B8
#define COMPRESSED_RGBA_ASTC_10x6_KHR                     0x93B9
#define COMPRESSED_RGBA_ASTC_10x8_KHR                     0x93BA
#define COMPRESSED_RGBA_ASTC_10x10_KHR                    0x93BB
#define COMPRESSED_RGBA_ASTC_12x10_KHR                    0x93BC
#define COMPRESSED_RGBA_ASTC_12x12_KHR                    0x93BD

#define COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR              0x93D0
#define COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR              0x93D1
#define COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR              0x93D2
#define COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR              0x93D3
#define COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR              0x93D4
#define COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR              0x93D5
#define COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR              0x93D6
#define COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR              0x93D7
#define COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR             0x93D8
#define COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR             0x93D9
#define COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR             0x93DA
#define COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR            0x93DB
#define COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR            0x93DC
#define COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR            0x93DD


// WEBGL_compressed_texture_etc https://www.khronos.org/registry/webgl/extensions/WEBGL_compressed_texture_etc/
#define COMPRESSED_R11_EAC                                0x9270
#define COMPRESSED_SIGNED_R11_EAC                         0x9271
#define COMPRESSED_RG11_EAC                               0x9272
#define COMPRESSED_SIGNED_RG11_EAC                        0x9273
#define COMPRESSED_RGB8_ETC2                              0x9274
#define COMPRESSED_SRGB8_ETC2                             0x9275
#define COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2          0x9276
#define COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2         0x9277
#define COMPRESSED_RGBA8_ETC2_EAC                         0x9278
#define COMPRESSED_SRGB8_ALPHA8_ETC2_EAC                  0x9279

// Requires NV_texture_compression_latc  https://www.khronos.org/registry/OpenGL/extensions/NV/NV_texture_compression_latc.txt
// GL_EXT_texture_compression_latc https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_compression_latc.txt
#define COMPRESSED_LUMINANCE_LATC1_NV                     0x8C70
#define COMPRESSED_SIGNED_LUMINANCE_LATC1_NV              0x8C71
#define COMPRESSED_LUMINANCE_ALPHA_LATC2_NV               0x8C72
#define COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_NV        0x8C73

// GL_ARB_texture_compression_rgtc GL_EXT_texture_compression_rgtc https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_compression_rgtc.txt
#define COMPRESSED_RED_RGTC1                              0x8DBB
#define COMPRESSED_SIGNED_RED_RGTC1                       0x8DBC
#define COMPRESSED_RG_RGTC2                               0x8DBD
#define COMPRESSED_SIGNED_RG_RGTC2                        0x8DBE


// Requires GL_OES_compressed_ETC1_RGB8_texture https://www.khronos.org/registry/OpenGL/extensions/OES/OES_compressed_ETC1_RGB8_texture.txte2D
// WEBGL_compressed_texture_etc1
#define ETC1_RGB8_OES                                     0x8D64

// Requires GL_EXT_texture_compression_bptc
// https://www.khronos.org/registry/webgl/extensions/EXT_texture_compression_bptc/
#define COMPRESSED_RGBA_BPTC_UNORM_EXT                    0x8E8C
#define COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT              0x8E8D
#define COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT              0x8E8E
#define COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT            0x8E8F
