# Building FastDirectX from Source

## Prerequisites

- **Windows 10 / 11 (x64)**
- **JDK 21+** (e.g. Eclipse Temurin or Microsoft OpenJDK, with `JAVA_HOME` set)
- **Maven 3.9+**
- **Visual Studio 2019 / 2022** (Desktop development with C++, MSVC toolchain)
- Windows SDK with DirectX 11 headers and libraries (`d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`)

## Quick Build

```cmd
:: 1. Compile native FastDirectX.dll
compile.bat

:: 2. Compile Java classes and test demo
mvn clean test-compile
```

## Running the Interactive Demo

```cmd
run-demo.bat
```

## Native Architecture & FFM Linkage

FastDirectX uses pure C ABI exports (`fastdx_*`) defined in `native/DirectXBackend.h` and implemented in `native/DirectXBackend.cpp`.
Java binds directly to these entry points using Java 21's `Linker.nativeLinker()` and `SymbolLookup` via `FastCore.lookupFunction()`. No intermediate JNI wrapper classes or `.def` files are required.
