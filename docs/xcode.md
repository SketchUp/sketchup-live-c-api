# Xcode

The base for this is a [Ruby C Extension](https://github.com/SketchUp/ruby-c-extension-examples).

## Include Headers

* Add `<SDK_PATH>` to `<TARGET> / Build Settings / Search Paths / Framework Search Paths`

![](images/xcode-framework-search-path.png)

## Linking

**Important!** Do *not* link against `SketchUpAPI.framework`.

* Add `/Application/SketchUp 2019/SketchUp.app/Contents/MacOS/SketchUp` to `<PROJECT> / Build Settings / Linking / Bundle Loader`.

![](images/xcode-bundle-loader.png)


## Next

* [Using the Live C API](using-live-c-api.md)

