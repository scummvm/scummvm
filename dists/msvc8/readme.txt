The Visual Studio project files can now be created automatically from the GCC
files using the create_msvc tool inside the /tools/create_msvc folder.
To create the default project files, build create_msvc, copy it inside this
folder and run it like this:

create_msvc ..\.. --msvc-version 8

Run the tool with no parameters to check the possible command-line options
