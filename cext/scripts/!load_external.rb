# Helper file for loading extensions from external locations.
# Useful for extension development so the extensions can be loaded from the
# source repositories directly.

# TODO: Copy this file to the SketchUp Plugins directory.
#
# Standard locations for SketchUp 2019 are:
# * %APPDATA%\SketchUp\SketchUp 2019\SketchUp\Plugins
# * ~/Library/Application Support/SketchUp 2019/SketchUp/Plugins
#
# You get get the for your machine and SketchUp installation by typing
# `Sketchup.find_support_file('Plugins')` into the Ruby Console.

# Show the Ruby Console to catch any loading errors.
SKETCHUP_CONSOLE.show

# List of directories to add to the load path. Any .rb file in these directories
# will be loaded by SketchUp.
# TODO: Add paths here:
paths = [
  '<TODO_PathToRepository>/sketchup-live-c-api/ruby',
]

# When debugging Ruby C Extensions and using `-RubyStartup` to load a helper
# file that bootstrap loading of the binary from the build directory that file
# will be loaded after the files in the Plugins directory.
#
# Because of that the path to the extension being debugged is passed along
# so it can be ignored by this file. The extension will be loaded by the
# helper script instead.
ignore_path = nil
if ARGV.size == 1 && ARGV[0].is_a?(String)
  # Parse startup args from `-RubyStartup`:
  # "Config=${input:buildType};Path=${workspaceRoot}/ruby"
  args = ARGV[0].split(';')
  args = args.map { |arg| arg.split('=') }.to_h

  ignore_path = File.expand_path(args['Path']) if args.key?('Path')
end

ruby_files = []
paths.each { |path|
  # Check if path should be ignored because it is being debugged.
  if ignore_path && File.identical?(path, ignore_path)
    puts "Ignoring path: #{ignore_path}"
  end
  next if ignore_path && File.identical?(path, ignore_path)

  # Add to load path...
  $LOAD_PATH << path

  # Collect the Ruby files in the directory.
  # Deferring the loading for later, first ensuring all paths are added to the
  # load path..
  ruby_files_filter = File.join(path, '*.rb')
  ruby_files_basenames = Dir.glob(ruby_files_filter).map { |filename|
    File.basename(filename, '.*')
  }
  ruby_files.concat(ruby_files_basenames)
}

ruby_files.each { |ruby_file|
  # Load the Ruby files discovered.
  # Using Sketchup.require simulates what SketchUp does upon startup and will
  # not raise errors.
  Sketchup.require ruby_file
}
