# The Philosophy of FastDirectX

> [!IMPORTANT]
> **"Zero-GC Allocation. Direct Hardware Pipeline. Pure FFM Execution."**

FastDirectX is engineered on the principle that graphics backends on the JVM should never introduce CPU garbage collection churn or abstraction penalty when communicating with the GPU.

## Core Tenets

1. **Java 21 FFM over Legacy JNI**
   - Direct invocation of native entry points via `MethodHandle` without JNI boilerplate.
   - Slicing and passing native memory segments (`MemorySegment.ofBuffer`) with zero copying.

2. **Zero Heap Allocation in the Render Loop**
   - Reusable direct NIO off-heap buffers for vertex and index streams.
   - GPU vertex and index buffers dynamically mapped with `D3D11_MAP_WRITE_DISCARD`.

3. **Deterministic Latency & Live Resizing**
   - Clean Win32 event synchronization with `FastWindow`.
   - Swapchain resize handling (`ResizeBuffers`) executed immediately on window resize without lag or white flicker.

4. **Blueprint Consistency**
   - Standardized layout following the FastJava ecosystem:
     - Pure C ABI native engine.
     - Dynamic native loading via `FastCore`.
     - Standardized lifecycle and error handling.
