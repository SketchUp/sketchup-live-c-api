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

    CEXT_BUILD_TYPE = ARGV[0]

    puts "Debugging #{self.name.split('::').last} (#{CEXT_BUILD_TYPE})"

    ruby_version = RUBY_VERSION.split('.').take(2).join('.')
    bin_path = File.join(__dir__, '..', 'build', CEXT_BUILD_TYPE, ruby_version)
    cext = File.expand_path('example', bin_path)

    puts "Loading: #{cext}"

    require cext

    puts "#{self}::CEXT_VERSION: #{self::CEXT_VERSION}"

  end
end
