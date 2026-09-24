# FastDirectX

DirectX 11/12 GPU Backend for FastGraphics and FastJava powered by the Java 21+ Foreign Function & Memory (FFM) API.

## Features

- **DirectX 11 2D Rendering Pipeline**: Pure C ABI native exports (`fastdx_*`), zero JNI overhead.
- **Java 21 FFM API**: Interoperates directly with native memory segments via `java.lang.foreign`.
- **FastGraphics 2D Backend**: Full integration with `fastgraphics.backend.GraphicsBackend`.
- **Flicker-Free Resizing & Startup**: Native live resizing without white flicker or swapchain lag.

## Installation

Add JitPack and the dependency to your `pom.xml`:

```xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependencies>
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastDirectX</artifactId>
        <version>0.1.0</version>
    </dependency>
</dependencies>
```

## License

MIT License — see [LICENSE](LICENSE).
