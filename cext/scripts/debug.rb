# Helper file to aid in debugging the Ruby C Extension.
#
#   sketchup.exe -RubyStartup path/to/this/debug.rb -RubyStartupArg <configuration>
#
# <configuration> should refer to a CMake configuration; Debug, Release
#
# This will load the Ruby C Extension from the binary output directory instead
# of the installed version.
#
#   <project_dir>/cext/build/<configuration>/<ruby_version>/example.{so,bundle}
module Examples
  module LiveCAPI
    module DebugLoader

      # Parse startup args:
      # "Config=${input:buildType};Path=${workspaceRoot}/ruby"
      args = ARGV[0].split(';')
      args = args.map { |arg| arg.split('=') }.to_h

      CEXT_BUILD_TYPE = args['Config']

      EXTENSION_NAME = self.name.split('::')[-2]

      puts "Debugging #{EXTENSION_NAME} (#{CEXT_BUILD_TYPE})"

      PROJECT_PATH = File.expand_path('../..', __dir__)
      PROJECT_RUBY_SOURCE_PATH = File.join(PROJECT_PATH, 'ruby')

      puts "Adding #{EXTENSION_NAME} to load path:"
      puts PROJECT_RUBY_SOURCE_PATH
      $LOAD_PATH << PROJECT_RUBY_SOURCE_PATH

      CEXT_BUILD_PATH = File.join(PROJECT_PATH, 'cext', 'build')
      CEXT_RUBY_VERSION = RUBY_VERSION.split('.').take(2).join('.') # major.minor
      CEXT_BIN_PATH = File.join(CEXT_BUILD_PATH, CEXT_BUILD_TYPE, CEXT_RUBY_VERSION)
      CEXT_FILE = File.expand_path('example', CEXT_BIN_PATH) # "example" is basename of binary

      puts "Loading C Extension: #{CEXT_FILE}"
      require CEXT_FILE

      puts "Loading Ruby Extension: #{EXTENSION_NAME}"
      ruby_files = Dir.glob("#{PROJECT_RUBY_SOURCE_PATH}/*.rb")
      ruby_files.each { |ruby_file|
        basename = File.basename(ruby_file)
        puts "> #{basename} (#{ruby_file})"
        require basename
      }

    end

    puts "#{self}::CEXT_VERSION: #{self::CEXT_VERSION}"

  end
end
