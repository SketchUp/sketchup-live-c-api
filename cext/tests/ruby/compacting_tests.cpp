#include <iostream>

#include <ruby.h>

int main(int argc, char* argv[])
{
  std::cout << "Ruby Compacting Test\n";

  // Initialize Ruby
  // int argc = 0;
  // char** argv = NULL;
  ruby_sysinit(&argc, &argv); // Without we crash at ruby_init() on Windows.
  {
    // This macro supposedly requires its own scope after Ruby 1.9.
    RUBY_INIT_STACK; // TODO: Is this still valid?
    ruby_init();

    // ruby_options(argc, argv);

    // Because using the -e option overrides the script name argument given to
    // ruby_options. This is returned by `$0` in Ruby.
    // ruby_script("SketchUp");

    ruby_show_version();

  }
  return ruby_cleanup(0);
}
