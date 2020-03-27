module Examples
  module LiveCAPI

    CEXT_BUILD_TYPE = ARGV[0]

    puts "Debugging #{self.name.split('::').last} (#{CEXT_BUILD_TYPE})"

    ruby_version = RUBY_VERSION.split('.').take(2).join('.')
    bin_path = File.join(__dir__, 'build', CEXT_BUILD_TYPE, ruby_version)
    cext = File.join(bin_path, 'example')

    require cext

    puts "#{self}::CEXT_VERSION: #{self::CEXT_VERSION}"

  end
end
