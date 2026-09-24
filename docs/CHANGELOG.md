# Changelog

All notable changes to FastDirectX will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [0.1.0] - 2026-09-24

### Added
- Complete migration to Java 21+ Foreign Function & Memory (FFM) API via `FastCore`.
- Pure C ABI native exports (`fastdx_*`) in `FastDirectX.dll`.
- Native Direct3D 11 rendering pipeline with dynamic vertex and index buffer streaming.
- `RSSetState` with `D3D11_CULL_NONE` rasterizer state to support arbitrary triangle winding order.
- Row-major HLSL projection matrix transformation with aspect-ratio preservation.
- Full integration with `fastgraphics.backend.GraphicsBackend` and `FastGraphics2D`.
- Flicker-free native window launch and live resizing support with `FastWindow`.
