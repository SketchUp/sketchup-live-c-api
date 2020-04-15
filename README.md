# SketchUp Live C API Examples

The [SketchUp C API](https://extensions.sketchup.com/sketchup-sdk) is mainly used for the following:

* Read/Write `.skp` files in applications other than SketchUp.
* Create Importers/Exporters for SketchUp.

A third usage, officially supported since SketchUp 2019.2 it allows _read only_ access to the open SketchUp model via [Ruby C extensions](https://github.com/SketchUp/ruby-c-extension-examples).

This is a guide demonstrating how to setup and build your own Ruby C extension that uses the SketchUp C API to read from the active model.

## General Instructions

`<SDK_PATH>` is the location where the SDK package is extracted.

### Windows

* Add `<SDK_PATH>\headers` to the include paths.
* Link against `<SDK_PATH>\binaries\sketchup\x64\sketchup.lib` (Do **not** link against `SketchUpAPI.lib`!)

### macOS

* Add `<SDK_PATH>` to framework search paths.
* Set `/Application/SketchUp 2019/SketchUp.app/Contents/MacOS/SketchUp` as the bundle loader. (Do **not** link against `SketchUpAPI.framework`!)


## Editor Instructions

The code in this repository will demonstrate a [CMake](https://cmake.org/) project for a Windows & Mac builds. This CMake project can be used to create a [Visual Studio solution](https://visualstudio.microsoft.com/), [Xcode project](https://developer.apple.com/xcode/).


For those who prefer not to use CMake there is documentation for setting up the following IDEs/editors directly:

* [Visual Studio](docs/visual-studio.md)
* [XCode](docs/xcode.md)

Visual Studio and Xcode is the main IDEs we work with. We welcome pull-requests if you want to contribute documentation for setting up your favorite editor.

## See Also

* [Using the Live C API](docs/using-live-c-api.md)
