#include <iostream>

#include <ruby.h>

int main(int argc, char* argv[])
{
  std::cout << "Ruby Compacting Test\n";

  // Initialize Ruby
  // int argc = 0;
  // char** argv = NULL;

  std::cout << "ruby_sysinit...\n";
  ruby_sysinit(&argc, &argv); // Without we crash at ruby_init() on Windows.
  std::cout << "ruby_sysinit (done)\n";

  {
    // This macro supposedly requires its own scope after Ruby 1.9.
    RUBY_INIT_STACK; // TODO: Is this still valid?

    std::cout << "ruby_init...\n";
    ruby_init();
    std::cout << "ruby_init (done)\n";

    // ruby_options(argc, argv);

    // Because using the -e option overrides the script name argument given to
    // ruby_options. This is returned by `$0` in Ruby.
    // ruby_script("SketchUp");

    ruby_show_version();

    std::cout << "ruby_init_loadpath...\n";
    ruby_init_loadpath();
    std::cout << "ruby_init_loadpath (done)\n";

    std::cout << "ruby_incpush...\n";
    ruby_incpush("C:/Users/Thomas/SourceTree/sketchup-live-c-api/cext/build/Debug/cext/tests/ruby/Debug");
    ruby_incpush("C:/Users/Thomas/SourceTree/sketchup-live-c-api/cext/tests/ruby");
    std::cout << "ruby_incpush (done)\n";

    // std::cout << "rb_require compacter ...\n";
    // VALUE result = rb_require("compacter");
    // VALUE result = rb_require("C:/Users/Thomas/SourceTree/sketchup-live-c-api/cext/build/Debug/cext/tests/ruby/Debug/compacter.so");
    // std::cout << "rb_require: " << result << "\n";

    std::cout << "rb_require compact_fuzzer ...\n";
    VALUE result = rb_require("compact_fuzzer");
    std::cout << "rb_require: " << result << "\n";
  }
  return ruby_cleanup(0);
}
