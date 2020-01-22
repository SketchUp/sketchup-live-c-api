require 'sketchup.rb'

require 'tempfile'

require_relative '../../cext/build/Debug/2.5/example'

module Examples
  module LiveCAPI

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
