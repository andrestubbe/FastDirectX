> [!WARNING]
> **🚧 WORK IN PROGRESS (WIP) — Active Engine Development & Calibration**
> 
> * **Active Calibration:** The 2D rendering pipeline, analytical HLSL shaders (anti-aliasing, stroke coverage, corner radii), and color spaces are currently undergoing active calibration against Java2D references.
> * **FastGraphics Migration:** High-level 2D drawing primitives, shape APIs, and canvas abstractions will progressively migrate to **[FastGraphics](https://github.com/andrestubbe/FastGraphics)** as the unified drawing layer across backends, while FastDirectX serves as the dedicated native DirectX hardware backend and swapchain runtime.

# FastDirectX 0.1.0 — High-Performance Native DirectX 11 2D Rendering & Swapchain Engine for Java

[![Status](https://img.shields.io/badge/status-0.1.0-brightgreen.svg)](https://github.com/andrestubbe/FastDirectX/releases/tag/0.1.0)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Java](https://img.shields.io/badge/Java-21+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010+-lightgrey.svg)]()
[![JitPack](https://img.shields.io/badge/JitPack-0.1.0-green.svg)](https://jitpack.io/#andrestubbe/FastDirectX)

---

**⚡ Ultra-fast native DirectX 11 2D batch rendering and Win32 zero-latency hardware swapchain engine for Java, designed to power FastGraphics and FastUI.** Built for maximum framerates, zero JVM Garbage Collection overhead, and seamless live window resizing via the modern Java 21+ Foreign Function & Memory (FFM) API.

FastDirectX provides a low-overhead GPU-accelerated 2D pipeline (instanced shapes, quad batching, texture rendering, and smooth zoom/transforms) with native C ABI hardware execution and zero legacy JNI marshalling cost.

---

## Quick Start — Example

```java
import fastdirectx.DirectXBackend;
import fastgraphics.g2d.FastGraphics2D;
import fastwindow.FastNativeWindow;
import fastwindow.FastWindow;

public class Demo {
    public static void main(String[] args) {
        int width = 1280;
        int height = 720;

        try (FastNativeWindow window = FastWindow.create("FastDirectX Demo", width, height);
             DirectXBackend backend = new DirectXBackend()) {

            long hwnd = window.getHWND();
            backend.initialize(hwnd, width, height);

            FastGraphics2D g2d = new FastGraphics2D(backend, width, height);

            // Render first frame before showing window to eliminate white flash
            g2d.begin();
            g2d.clear(0.08f, 0.08f, 0.08f, 1.0f);
            g2d.setColor(1.0f, 0.5f, 0.0f, 1.0f);
            g2d.fillRect(100, 100, 400, 300);
            g2d.end();

            window.setVisible(true);

            while (window.pollEvents()) {
                g2d.begin();
                g2d.clear(0.08f, 0.08f, 0.08f, 1.0f);
                g2d.setColor(0.2f, 0.7f, 1.0f, 1.0f);
                g2d.fillRect(150, 150, 500, 350);
                g2d.end();
            }
        }
    }
}
```

---

## Table of Contents

- [Why FastDirectX?](#why-fastdirectx)
- [Key Features](#key-features)
- [Architecture Overview](#architecture-overview)
- [Real-World Use Cases](#real-world-use-cases)
- [Installation](#installation)
- [Documentation](#documentation)
- [Platform Support](#platform-support)
- [License](#license)
- [Related Projects](#related-projects)

---

## Why FastDirectX?

Standard Java GUI toolkits and rendering engines suffer from thread synchronization bottlenecks, laggy window resizing, and high CPU rasterization overhead:

1. **CPU Rasterization & EDT Stalls**: Java2D and Swing rely on CPU rasterization and synchronize rendering on the single Event Dispatch Thread (EDT), creating stutter under heavy UI loads.
2. **Laggy Window Resizing**: Standard Java windows freeze or display white flickering artifacts during interactive window border dragging (`WM_SIZE` / `WM_SIZING`).
3. **High Heap Allocation Churn**: Constructing `Shape`, `Path2D`, and scene graph objects in the render loop causes constant garbage collection pauses.
4. **Opaque & Obsolete Backends**: JavaFX (Prism) relies on legacy Direct3D 9/11 wrappers with hidden memory copies and no direct off-heap or FFM access.

**FastDirectX** eliminates these bottlenecks by coupling a dedicated DirectX 11 pipeline with native Win32 windowing and Java 21+ FFM:

- **Pure C ABI & Java 21 FFM**: Bypasses legacy JNI marshalling by using direct foreign function addresses and memory segment off-heap buffers.
- **Native Win32 Message Loop**: Latency-free live window resize (`WM_SIZE` / `WM_SIZING`) without Java thread blocking or white flashes.
- **Hardware Swapchain Integration**: Direct DXGI swapchain presentation (`DXGI_SWAP_EFFECT_DISCARD` / `FLIP`) with zero CPU-to-GPU copy bottlenecks.
- **Zero-GC Architecture**: Off-heap vertex generation and direct native buffer exchanges (**0 bytes GC pressure**).
- **FastGraphics 2D Backend**: Out-of-the-box drop-in backend implementation for `fastgraphics.backend.GraphicsBackend`.

| Feature | Java2D / Swing | JavaFX (Prism) | FastDirectX |
|:---|:---|:---|:---|
| **Graphics Backend** | GDI / Software CPU rasterizer | Legacy Direct3D 9/11 / OpenGL | Modern DirectX 11 Native Pipeline |
| **Interoperability** | Legacy JNI | Internal native wrappers | **Java 21+ FFM (`java.lang.foreign`)** |
| **Interactive Window Resize** | ❌ White flashes & EDT freeze | ⚠️ Stutter on continuous resize | ✅ Flicker-free live native resize loop |
| **2D Shape Batching** | ❌ Immediate mode draw calls | ⚠️ Limited quad batching | ✅ Instanced GPU quad batching (1 draw call) |
| **Render Loop Allocations** | High (`Graphics2D`, `Shape` churn) | High (Scene graph node churn) | **0 bytes** (Off-heap vertex buffers) |
| **Framerate Cap** | 30–60 FPS (EDT bound) | 60 FPS (VSync bound) | **500–2000+ FPS** (Uncapped swapchain) |
| **GPU Memory Access** | ❌ Opaque / No direct access | ❌ Private internal pipeline | ✅ Direct off-heap & native memory segments |

---

## Key Features

- ⚡ **DirectX 11 2D Render Engine**: High-performance HLSL shaders for real-time shapes, textured quads, and analytical anti-aliasing.
- 🚀 **Java 21+ FFM Architecture**: Driven by `java.lang.foreign` with pure C ABI native exports (`fastdx_*`), zero JNI overhead.
- 🪟 **Native Win32 Windowing**: Native message handling with crisp DPI awareness and direct frame presents via **FastWindow**.
- 🎨 **FastGraphics Integration**: Full compliance with `fastgraphics.backend.GraphicsBackend` for universal 2D drawing.
- 📦 **FastJava Ecosystem Ready**: Interoperates seamlessly with **FastWindow**, **FastGraphics**, **FastUI**, and **FastCore**.

---

## Architecture Overview

1. ✅ **Pure C ABI Native Engine**: Native library exposing `fastdx_*` functions loaded via `FastCore.lookupFunction`.
2. ✅ **DirectX 11 Device & DXGI SwapChain**: Dynamic device creation, render target view management, and resize handling.
3. ✅ **Off-Heap Vertex Buffers**: Dynamic D3D11 vertex buffers filled directly from Java NIO buffers via `MemorySegment.ofBuffer()`.
4. ✅ **D3D11 Texture Pipeline**: Hardware texture uploads (`fastdx_create_texture`), sampling, and shader resource views.
5. 🔄 **FastUI Integration**: Powering lightweight desktop UI components with high-frequency rendering.

---

## Real-World Use Cases

- 🖥️ **High-FPS UI Frameworks ([FastUI](https://github.com/andrestubbe/FastUI))**: Power complex desktop dashboards, rich vector controls, and live animations with zero GC pauses.
- 🖼️ **Real-Time Image & Video Viewports**: Seamlessly render, pan, and smoothly zoom 4K/8K bitmaps streamed from **[FastImage](https://github.com/andrestubbe/FastImage)** and **[FastScreen](https://github.com/andrestubbe/FastScreen)**.
- 🎮 **2D Game Engines & Particle Canvas**: Render tens of thousands of batch-instanced sprites, lines, and HUD shapes at 1000+ FPS.
- 📊 **Scientific & Financial Charting**: Real-time high-frequency candlestick, waveform, and scatter data visualization with instant window resizing.

---

## Installation

### Option 1: Maven (Recommended via JitPack)

Add the JitPack repository and the dependency stack to your `pom.xml`:

```xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependencies>
    <!-- FastDirectX 11 GPU Backend -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastDirectX</artifactId>
        <version>0.1.0</version>
    </dependency>

    <!-- FastGraphics Unified 2D API -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastGraphics</artifactId>
        <version>0.2.0</version>
    </dependency>

    <!-- FastWindow Native Win32 Window Engine -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastWindow</artifactId>
        <version>0.1.3</version>
    </dependency>

    <!-- FastCore Unified FFM Loader -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastCore</artifactId>
        <version>0.1.1</version>
    </dependency>
</dependencies>
```

### Option 2: Gradle (via JitPack)

```groovy
repositories {
    maven { url 'https://jitpack.io' }
}

dependencies {
    implementation 'com.github.andrestubbe:FastDirectX:0.1.0'
    implementation 'com.github.andrestubbe:FastGraphics:0.2.0'
    implementation 'com.github.andrestubbe:FastWindow:0.1.3'
    implementation 'com.github.andrestubbe:FastCore:0.1.1'
}
```

---

## Documentation

- **[CHANGELOG.md](docs/CHANGELOG.md)**: Release notes and version history.
- **[REFERENCE.md](docs/REFERENCE.md)**: API contract, FFM bindings, and methods.
- **[PHILOSOPHY.md](docs/PHILOSOPHY.md)**: Design principles and Zero-GC FFM architecture.
- **[COMPILE.md](docs/COMPILE.md)**: Native C++ compilation guide.
- **[ROADMAP.md](docs/ROADMAP.md)**: Detailed feature roadmap.

---

## Platform Support

| Platform | Architecture | Status | Driver / Subsystem |
|:---|:---:|:---:|:---|
| **Windows 10 / 11** | x64 | ✅ Fully Supported | Native Win32 + Direct3D 11 (d3d11.dll) |
| **Linux** | x64 / AArch64 | ❌ Not Supported | Use [FastVulkan](https://github.com/andrestubbe/FastVulkan) for cross-platform |
| **macOS** | Apple Silicon / x64 | ❌ Not Supported | Use [FastVulkan](https://github.com/andrestubbe/FastVulkan) via MoltenVK |

---

## License

MIT License — Free for commercial and personal use. See [LICENSE](LICENSE) for details.

---

## Related Projects

- [FastVulkan](https://github.com/andrestubbe/FastVulkan) — Modern Vulkan 1.3 2D Render Engine & Backend
- [FastWindow](https://github.com/andrestubbe/FastWindow) — Ultra-Fast Win32 Native Window Engine
- [FastGraphics](https://github.com/andrestubbe/FastGraphics) — Hardware-accelerated 2D graphics layer
- [FastImage](https://github.com/andrestubbe/FastImage) — Native SIMD image processing engine
- [FastCore](https://github.com/andrestubbe/FastCore) — Native FFM loader and platform abstraction

---

Part of the FastJava Ecosystem — Making the JVM faster. Small package. Maximum speed. Zero bloat. ⚡
