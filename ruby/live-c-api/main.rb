require 'sketchup.rb'

require 'tempfile'

module Examples
  module LiveCAPI

    # Load guard for the Ruby C Extension, allowing the binary to be loaded
    # from the build directory.
    #
    # Parse startup args:
    # "Config=${input:buildType};Path=${workspaceRoot}/ruby"
    args = (ARGV[0] || '').split(';')
    args = args.map { |arg| arg.split('=') }.to_h
    if args['Id'] == 'LiveCAPI'
      # This means the Ruby C Extension will be loaded via the
      # <project_dir>/cext/scripts/debug.rb helper for purposes of debugging.
      puts "#{self.name} C Extension deferred loading for debugging..."
    elsif defined?(CEXT_VERSION)
      # This means the Ruby C Extension has already been loaded by other means.
      puts "#{self.name} C Extension already loaded"
    else
      # This binary is not checked in. Use CMake to "install" it to this
      # location before running/packaging the SketchUp extension.
      # (VSCode) `<Command Palette>` » CMake: Install
      require 'live-c-api/cext/2.5/example'
    end

    class Failure < StandardError; end

    MSG_SELECT_TEXTURED_FACE = 'Select exactly one textured face.'
    MSG_EXPECTED_BLEND_RATIO = 'Blend Ratio must be between 0.0 and 1.0.'

    def self.blend_to_greyscale
      model = Sketchup.active_model
      faces = model.selection.grep(Sketchup::Face)
      raise Failure, MSG_SELECT_TEXTURED_FACE if faces.size != 1

      face = faces.first
      texture = face&.material&.texture
      raise Failure, MSG_SELECT_TEXTURED_FACE if texture.nil?

      prompts = ['Blend Ratio:']
      defaults = [0.5]
      results = UI.inputbox(prompts, defaults, "Blend to Greyscale")
      return if results == false

      amount = results[0]
      raise Failure, MSG_EXPECTED_BLEND_RATIO unless (0.0..1.0).include?(amount)

      image_rep = grey_scale(texture.image_rep, amount)

      model.start_operation('Greyscale Blend', true)

      material = face.material
      width = material.texture.width
      height = material.texture.height

      material.texture = image_rep
      material.texture.size = [width, height]

      model.commit_operation
    rescue Failure => error
      UI.messagebox(error.message)
    end

    unless file_loaded?(__FILE__)
      menu = UI.menu('Plugins')
      sub_menu = menu.add_submenu('Live C API Examples')
      sub_menu.add_item('Greyscale') {
        self.blend_to_greyscale
      }
      file_loaded(__FILE__)
    end

    # Examples::LiveCAPI.reload
    def self.reload
      # rubocop:disable SketchupSuggestions/FileEncoding
      original_verbose = $VERBOSE
      $VERBOSE = nil
      load __FILE__
      pattern = File.join(__dir__, '**/*.rb')
      # rubocop:enable SketchupSuggestions/FileEncoding
      Dir.glob(pattern).each { |file| load file }.size
    ensure
      $VERBOSE = original_verbose
    end

  end # module LiveCAPI
end # module Examples
