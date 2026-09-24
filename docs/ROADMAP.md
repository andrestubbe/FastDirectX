# FastDirectX Roadmap 🗺️

**Vision:** To provide the fastest, zero-overhead DirectX 11/12 GPU backend on the JVM for real-time 2D graphics, vector UI rendering, and hardware swapchain presentations.

## 🟢 v0.1.0: Java 21 FFM & FastGraphics Integration (Current)
- [x] **Pure C ABI Native Engine**: Implemented `fastdx_*` exports in Direct3D 11.
- [x] **Java 21 FFM Loader**: Native symbol linking via `FastCore.lookupFunction()`.
- [x] **FastGraphics2D Integration**: Triangle batching, texture sampling, and matrix transformations.
- [x] **Flicker-Free Window Launch**: Render before window display and live resize support.

## 🟡 v0.2.0: Pipeline Optimizations
- [ ] **Instanced Quad Batching**: Dedicated instanced drawing path for untextured/textured quads.
- [ ] **Multi-Texture Batching**: Texture array or descriptor table binding in single draw calls.
- [ ] **DirectX 12 Compute Pipeline**: Optional Direct3D 12 backend for compute-assisted rendering.

## 🔴 v1.0.0: Production Hardening
- [ ] **Full Stress Benchmarks**: Continuous long-running window resize and VSync frame timing audit.
- [ ] **HDR & Advanced Color Spaces**: ScRGB / HDR10 swapchain presentation support.
