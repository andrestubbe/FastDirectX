# FastDirectX Reference

## 1. Core Classes

### `fastdirectx.DirectXBackend`
Implements `fastgraphics.backend.GraphicsBackend`.
- `initialize(long hwnd, int width, int height)`: Initializes Direct3D 11 device, context, swap chain, and shaders.
- `beginFrame(int width, int height)`: Prepares the render target and clears depth/stencil.
- `clear(float r, float g, float b, float a)`: Clears the render target view to RGBA color.
- `drawTriangles(FloatBuffer vertices, IntBuffer indices, int indexCount, TextureHandle texture, float[] matrix)`: Streams vertex/index data into dynamic GPU buffers and dispatches indexed draw calls.
- `createTexture(int width, int height, ByteBuffer rgbaPixels)`: Creates a 2D texture and shader resource view from RGBA pixels.
- `destroyTexture(TextureHandle handle)`: Releases texture and shader resource view resources.
- `resize(int width, int height)`: Releases render targets, resizes swap chain buffers, and re-creates views.
- `present()`: Presents the swapchain buffer to the window (`IDXGISwapChain::Present(1, 0)`).
- `close()`: Releases all COM interfaces and destroys backend state.

### `fastdirectx.D3D11Texture`
Represents an instantiated DirectX 11 texture on the GPU holding an native handle (`nativeHandle`).

## 2. Native Functions (C ABI)

| Function | Signature | Description |
|:---|:---|:---|
| `fastdx_create` | `() -> void*` | Allocates native DirectX state struct. |
| `fastdx_init` | `(void* state, void* hwnd, int w, int h) -> int` | Initializes D3D11 device and swapchain. |
| `fastdx_begin_frame` | `(void* state, int w, int h) -> void` | Binds viewport and render target view. |
| `fastdx_clear` | `(void* state, float r, float g, float b, float a) -> void` | Clears current render target. |
| `fastdx_draw_triangles` | `(void* state, void* vertices, int vCount, void* indices, int iCount, void* tex, void* matrix) -> void` | Dispatches indexed draw call. |
| `fastdx_create_texture` | `(void* state, int w, int h, void* rgba) -> void*` | Allocates GPU texture and SRV. |
| `fastdx_destroy_texture` | `(void* state, void* tex) -> void` | Releases GPU texture. |
| `fastdx_resize` | `(void* state, int w, int h) -> void` | Resizes swapchain buffers. |
| `fastdx_present` | `(void* state) -> void` | Swaps buffer to display. |
| `fastdx_destroy` | `(void* state) -> void` | Tears down backend and frees memory. |
