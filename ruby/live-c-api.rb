module Examples
  module LiveCAPI

    unless file_loaded?(__FILE__)
      ex = SketchupExtension.new('Live C API', 'live-c-api/main')
      ex.description = 'SketchUp Ruby API example creating a cube.'
      ex.version     = '1.0.0'
      ex.copyright   = 'Trimble Inc © 2020'
      ex.creator     = 'SketchUp'
      Sketchup.register_extension(ex, true)
      file_loaded(__FILE__)
    end

  end # module LiveCAPI
end # module Examples
