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


    # Examples::LiveCAPI.num_faces_in_active_path
    def self.num_faces_in_active_path
      num_faces = active_entities_num_faces
      puts "Number of faces in active entitites: #{num_faces}"
    end

    # Examples::LiveCAPI.active_path
    def self.active_path
      pids = active_path_pids
      p pids
      Sketchup.active_model.find_entity_by_persistent_id(pids)
    end

    unless file_loaded?(__FILE__)
      menu = UI.menu('Plugins')
      sub_menu = menu.add_submenu('Live C API Examples')
      sub_menu.add_item('Greyscale') {
        self.blend_to_greyscale
      }

      sub_menu.add_separator

      sub_menu.add_item('Number of Faces in Active Entities') {
        self.num_faces_in_active_path
      }
      sub_menu.add_item('Active Path') {
        self.active_path
      }
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
