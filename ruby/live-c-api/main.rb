require 'sketchup.rb'

require 'tempfile'

module Examples
  module LiveCAPI

    # Load guard for the Ruby C Extension, allowing the binary to be loaded
    # from the build directory.
    if defined?(CEXT_VERSION)
      # This means the Ruby C Extension was loaded via the
      # <project_dir>/cext/scripts/debug.rb helper for purposes of debugging.
      puts "#{self.name} C Extension already loaded"
    else
      # This binary is not checked in. Use CMake to "install" it to this
      # location before running/packaging the SketchUp extension.
      # (VSCode) `<Command Palette>` » CMake: Install
      require 'live-c-api/cext/2.5/example'
    end

    class Failure < StandardError; end

    def self.blend_to_greyscale
      model = Sketchup.active_model
      faces = model.selection.grep(Sketchup::Face)
      raise Failure, 'Select exactly one face.' if faces.size != 1

      prompts = ['Blend Ratio:']
      defaults = [0.5]
      results = UI.inputbox(prompts, defaults, "Blend to Greyscale")
      return if results == false

      amount = results[0]

      face = faces.first

      path = model.active_path || []
      path << face
      face_pid_path = Sketchup::InstancePath.new(path).persistent_id_path

      image_rep = Tempfile.create(['greyscale-', '.png']) do |file|
        result = grey_scale(face_pid_path, amount, file.path)
        raise Failure, 'Failed to generate texture.' unless result

        puts file.path
        puts File.exist?(file.path)
        file.close

        Sketchup::ImageRep.new(file.path)
      end

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

    def self.add_selection_test_with_param(toolbar, method_name)
      cmd = UI::Command.new("Selection #{method_name}") {
        model = Sketchup.active_model
        faces = model.active_entities.grep(Sketchup::Face)
        puts "Live C API Selection: #{method_name} (#{faces.size} faces)"
        p Examples::LiveCAPI::Selection.send(method_name, faces)
      }
      cmd.tooltip = cmd.menu_text
      toolbar.add_item(cmd)
    end

    def self.add_selection_test(toolbar, method_name)
      cmd = UI::Command.new("Selection #{method_name}") {
        puts "Live C API Selection: #{method_name}"
        p Examples::LiveCAPI::Selection.send(method_name)
      }
      cmd.tooltip = cmd.menu_text
      toolbar.add_item(cmd)
    end

    unless file_loaded?(__FILE__)
      menu = UI.menu('Plugins')
      sub_menu = menu.add_submenu('Live C API Examples')
      sub_menu.add_item('Greyscale') {
        self.blend_to_greyscale
      }

      toolbar = UI::Toolbar.new("Live C API - Selection")
      add_selection_test_with_param(toolbar, :add)
      add_selection_test_with_param(toolbar, :remove)
      add_selection_test_with_param(toolbar, :toggle)
      add_selection_test(toolbar, :invert)
      add_selection_test(toolbar, :clear)
      add_selection_test(toolbar, :size)
      add_selection_test(toolbar, :to_a)
      add_selection_test(toolbar, :curve?)
      add_selection_test(toolbar, :surface?)
      add_selection_test(toolbar, :single_object?)

      file_loaded(__FILE__)
    end

    # Examples::LiveCAPI.reload
    def self.reload
      original_verbose = $VERBOSE
      $VERBOSE = nil
      load __FILE__
      pattern = File.join(__dir__, '**/*.rb')
      Dir.glob(pattern).each { |file| load file }.size
    ensure
      $VERBOSE = original_verbose
    end

  end # module LiveCAPI
end # module Examples
