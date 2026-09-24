#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <stdint.h>

#ifdef _WIN32
#define FASTDX_API __declspec(dllexport)
#else
#define FASTDX_API
#endif

extern "C" {

FASTDX_API int64_t fastdx_create(int64_t hwnd, int32_t w, int32_t h);
FASTDX_API void    fastdx_resize(int64_t handle, int32_t w, int32_t h);

FASTDX_API void    fastdx_begin_frame(int64_t handle);
FASTDX_API void    fastdx_clear(int64_t handle, float r, float g, float b, float a);

FASTDX_API void    fastdx_set_viewport(int64_t handle, int32_t x, int32_t y, int32_t w, int32_t h);
FASTDX_API void    fastdx_set_projection(int64_t handle, const float* matrix16);
FASTDX_API void    fastdx_set_blend_mode(int64_t handle, int32_t mode);

FASTDX_API void    fastdx_draw_triangles(int64_t handle,
                                         const float* vertexData, int32_t vertexCount,
                                         const int32_t* indexData, int32_t indexCount,
                                         int64_t textureHandle);

FASTDX_API int64_t fastdx_create_texture(int64_t handle, int32_t w, int32_t h, const void* rgbaPixels);
FASTDX_API void    fastdx_update_texture(int64_t handle, int64_t texHandle, const void* rgbaPixels);
FASTDX_API void    fastdx_destroy_texture(int64_t handle, int64_t texHandle);

FASTDX_API void    fastdx_end_frame(int64_t handle);
FASTDX_API void    fastdx_present(int64_t handle);

FASTDX_API void    fastdx_destroy(int64_t handle);

}
