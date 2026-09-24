package fastdirectx;

import fastcore.FastCore;
import fastgraphics.backend.BackendTexture;
import fastgraphics.backend.BlendMode;
import fastgraphics.backend.GraphicsBackend;
import fastgraphics.backend.TriangleBatch;

import java.lang.foreign.Arena;
import java.lang.foreign.FunctionDescriptor;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;
import java.lang.invoke.MethodHandle;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;

/**
 * <h1>DirectXBackend — High-Performance D3D11 GPU Backend (Java 21+ FFM)</h1>
 *
 * <p>Implements {@link GraphicsBackend} utilizing modern Java 21+ Foreign Function &amp; Memory (FFM)
 * direct C ABI downcalls via {@link FastCore#lookupFunction}, eliminating legacy JNI marshaling overhead.</p>
 */
public class DirectXBackend implements GraphicsBackend {

    private static final MethodHandle MH_CREATE;
    private static final MethodHandle MH_RESIZE;
    private static final MethodHandle MH_BEGIN_FRAME;
    private static final MethodHandle MH_CLEAR;
    private static final MethodHandle MH_SET_VIEWPORT;
    private static final MethodHandle MH_SET_PROJECTION;
    private static final MethodHandle MH_SET_BLEND_MODE;
    private static final MethodHandle MH_DRAW_TRIANGLES;
    private static final MethodHandle MH_CREATE_TEXTURE;
    private static final MethodHandle MH_UPDATE_TEXTURE;
    private static final MethodHandle MH_DESTROY_TEXTURE;
    private static final MethodHandle MH_END_FRAME;
    private static final MethodHandle MH_PRESENT;
    private static final MethodHandle MH_DESTROY;

    static {
        try {
            MH_CREATE = FastCore.lookupFunction("FastDirectX", "fastdx_create",
                    FunctionDescriptor.of(ValueLayout.JAVA_LONG, ValueLayout.JAVA_LONG, ValueLayout.JAVA_INT, ValueLayout.JAVA_INT),
                    DirectXBackend.class, Arena.global());

            MH_RESIZE = FastCore.lookupFunction("FastDirectX", "fastdx_resize",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG, ValueLayout.JAVA_INT, ValueLayout.JAVA_INT),
                    DirectXBackend.class, Arena.global());

            MH_BEGIN_FRAME = FastCore.lookupFunction("FastDirectX", "fastdx_begin_frame",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG),
                    DirectXBackend.class, Arena.global());

            MH_CLEAR = FastCore.lookupFunction("FastDirectX", "fastdx_clear",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG, ValueLayout.JAVA_FLOAT, ValueLayout.JAVA_FLOAT, ValueLayout.JAVA_FLOAT, ValueLayout.JAVA_FLOAT),
                    DirectXBackend.class, Arena.global());

            MH_SET_VIEWPORT = FastCore.lookupFunction("FastDirectX", "fastdx_set_viewport",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG, ValueLayout.JAVA_INT, ValueLayout.JAVA_INT, ValueLayout.JAVA_INT, ValueLayout.JAVA_INT),
                    DirectXBackend.class, Arena.global());

            MH_SET_PROJECTION = FastCore.lookupFunction("FastDirectX", "fastdx_set_projection",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG, ValueLayout.ADDRESS),
                    DirectXBackend.class, Arena.global());

            MH_SET_BLEND_MODE = FastCore.lookupFunction("FastDirectX", "fastdx_set_blend_mode",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG, ValueLayout.JAVA_INT),
                    DirectXBackend.class, Arena.global());

            MH_DRAW_TRIANGLES = FastCore.lookupFunction("FastDirectX", "fastdx_draw_triangles",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG, ValueLayout.ADDRESS, ValueLayout.JAVA_INT, ValueLayout.ADDRESS, ValueLayout.JAVA_INT, ValueLayout.JAVA_LONG),
                    DirectXBackend.class, Arena.global());

            MH_CREATE_TEXTURE = FastCore.lookupFunction("FastDirectX", "fastdx_create_texture",
                    FunctionDescriptor.of(ValueLayout.JAVA_LONG, ValueLayout.JAVA_LONG, ValueLayout.JAVA_INT, ValueLayout.JAVA_INT, ValueLayout.ADDRESS),
                    DirectXBackend.class, Arena.global());

            MH_UPDATE_TEXTURE = FastCore.lookupFunction("FastDirectX", "fastdx_update_texture",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG, ValueLayout.JAVA_LONG, ValueLayout.ADDRESS),
                    DirectXBackend.class, Arena.global());

            MH_DESTROY_TEXTURE = FastCore.lookupFunction("FastDirectX", "fastdx_destroy_texture",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG, ValueLayout.JAVA_LONG),
                    DirectXBackend.class, Arena.global());

            MH_END_FRAME = FastCore.lookupFunction("FastDirectX", "fastdx_end_frame",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG),
                    DirectXBackend.class, Arena.global());

            MH_PRESENT = FastCore.lookupFunction("FastDirectX", "fastdx_present",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG),
                    DirectXBackend.class, Arena.global());

            MH_DESTROY = FastCore.lookupFunction("FastDirectX", "fastdx_destroy",
                    FunctionDescriptor.ofVoid(ValueLayout.JAVA_LONG),
                    DirectXBackend.class, Arena.global());

        } catch (Throwable t) {
            throw new ExceptionInInitializerError(t);
        }
    }

    private long handle;
    private final Arena frameArena = Arena.ofAuto();
    private final MemorySegment projSegment = frameArena.allocateArray(ValueLayout.JAVA_FLOAT, 16);

    @Override
    public void initialize(long hwnd, int width, int height) {
        try {
            this.handle = (long) MH_CREATE.invokeExact(hwnd, width, height);
            if (this.handle == 0) {
                throw new RuntimeException("Failed to initialize DirectX 11 backend for HWND " + hwnd);
            }
        } catch (RuntimeException e) {
            throw e;
        } catch (Throwable t) {
            throw new RuntimeException("Error initializing DirectX 11 backend", t);
        }
    }

    @Override
    public void resize(int width, int height) {
        if (handle != 0) {
            try {
                MH_RESIZE.invokeExact(handle, width, height);
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }

    @Override
    public void beginFrame() {
        if (handle != 0) {
            try {
                MH_BEGIN_FRAME.invokeExact(handle);
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }

    @Override
    public void clear(float r, float g, float b, float a) {
        if (handle != 0) {
            try {
                MH_CLEAR.invokeExact(handle, r, g, b, a);
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }

    @Override
    public void setViewport(int x, int y, int width, int height) {
        if (handle != 0) {
            try {
                MH_SET_VIEWPORT.invokeExact(handle, x, y, width, height);
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }

    @Override
    public void setProjectionMatrix(float[] matrix16) {
        if (handle != 0 && matrix16 != null && matrix16.length >= 16) {
            try {
                MemorySegment.copy(matrix16, 0, projSegment, ValueLayout.JAVA_FLOAT, 0, 16);
                MH_SET_PROJECTION.invokeExact(handle, projSegment);
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }

    @Override
    public void setBlendMode(BlendMode mode) {
        if (handle != 0) {
            try {
                MH_SET_BLEND_MODE.invokeExact(handle, mode != null ? mode.ordinal() : 0);
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }

    @Override
    public void drawTriangles(TriangleBatch batch) {
        if (handle != 0 && batch != null && batch.getVertexCount() > 0 && batch.getIndexCount() > 0) {
            try {
                FloatBuffer vb = batch.getVertexBuffer().duplicate().position(0);
                IntBuffer ib = batch.getIndexBuffer().duplicate().position(0);
                MemorySegment vbSeg = MemorySegment.ofBuffer(vb);
                MemorySegment ibSeg = MemorySegment.ofBuffer(ib);
                long texHandle = batch.getTexture() != null ? batch.getTexture().getHandle() : 0;
                MH_DRAW_TRIANGLES.invokeExact(handle, vbSeg, batch.getVertexCount(), ibSeg, batch.getIndexCount(), texHandle);
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }

    @Override
    public BackendTexture createTexture(int width, int height, ByteBuffer rgbaPixels) {
        if (handle == 0) return null;
        try {
            MemorySegment pixSeg = rgbaPixels != null ? MemorySegment.ofBuffer(rgbaPixels) : MemorySegment.NULL;
            long texHandle = (long) MH_CREATE_TEXTURE.invokeExact(handle, width, height, pixSeg);
            if (texHandle == 0) return null;
            return new D3D11Texture(texHandle, width, height);
        } catch (Throwable t) {
            throw new RuntimeException(t);
        }
    }

    @Override
    public void updateTexture(BackendTexture texture, ByteBuffer rgbaPixels) {
        if (handle != 0 && texture != null && rgbaPixels != null) {
            try {
                MemorySegment pixSeg = MemorySegment.ofBuffer(rgbaPixels);
                MH_UPDATE_TEXTURE.invokeExact(handle, texture.getHandle(), pixSeg);
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }

    @Override
    public void destroyTexture(BackendTexture texture) {
        if (handle != 0 && texture != null) {
            try {
                MH_DESTROY_TEXTURE.invokeExact(handle, texture.getHandle());
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }

    @Override
    public void endFrame() {
        if (handle != 0) {
            try {
                MH_END_FRAME.invokeExact(handle);
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }

    @Override
    public void present() {
        if (handle != 0) {
            try {
                MH_PRESENT.invokeExact(handle);
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }

    @Override
    public void close() {
        if (handle != 0) {
            try {
                MH_DESTROY.invokeExact(handle);
                handle = 0;
            } catch (Throwable t) {
                throw new RuntimeException(t);
            }
        }
    }
}
