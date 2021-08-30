# CMake Quick-Start

## Windows

### Configure Project

While configuring the path to the extracted SketchUp C API package needs to be provided via `SketchUpAPI_DIR`.

```
git clone https://github.com/SketchUp/sketchup-live-c-api.git
cd sketchup-live-c-api
git submodule update --init --recursive

mkdir build
cd build

cmake .. -DSketchUpAPI_DIR=C:\Users\Thomas\SLAPI\SDK_WIN_x64_2020-2-172
```

### Build Debug
```
cmake --build . --config Debug
```

### Build Release
```
cmake --build . --config Release
```

### Build via Visual Studio

Open the `build/Example.sln` solution and build from Visual Studio. (Assuming the Visual Studio generator is the default on your system.)

## macOS

### Configure Project

While configuring the path to the extracted SketchUp C API package needs to be provided via `SketchUpAPI_DIR`.

Additionally the path to a SketchUp application binary containing all the symbols needed by the project must be provided via `SketchUpAPI_BUNDLE_LOADER`.

```
git clone https://github.com/SketchUp/sketchup-live-c-api.git
cd sketchup-live-c-api
git submodule update --init --recursive

mkdir build
cd build

cmake .. -DSketchUpAPI_DIR=/Users/thomas/SLAPI/SDK_Mac_2020-2-171 -DSketchUpAPI_BUNDLE_LOADER="/Applications/SketchUp 2020/SketchUp.app/Contents/MacOS/SketchUp"
```

### Build Debug
```
cmake --build . --config Debug
```

### Build Release
```
cmake --build . --config Release
```

### Build via XCode

Add `-G Xcode` to the command while configuring the project.

Then Open the `build/Example.xcodeproj` project and build from Xcode.
