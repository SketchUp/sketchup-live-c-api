module TT
  module Plugins
    module TraceUp

      BUILD_TYPE = ARGV[0]

      puts "Debugging TraceUp (#{BUILD_TYPE})"

      ruby_version = RUBY_VERSION.split('.').take(2).join('.')
      bin_path = File.join(__dir__, 'build', BUILD_TYPE, ruby_version)
      cext = File.join(bin_path, 'traceup.so')

      require cext

      puts "TraceUp::CEXT_VERSION: #{TT::Plugins::TraceUp::CEXT_VERSION}"

    end
  end
end
