GLX vs. EGL
===========

GLX is used by default, to use EGL pass --egl to build-webkit script.


Packages required
=================

WebKitNix requires some packages in the system as well as
automatically pull other dependencies using the jhbuild tool.

To compile WebKitNix all the three groups of packages below are necessary:

# These are needed for jhbuild tool to work.
$ sudo apt-get install gobject-introspection \
                       icon-naming-utils \
                       libgcrypt11-dev \
                       libgpg-error-dev \
                       libp11-kit-dev \
                       libtiff4-dev \
                       libcroco3-dev

# These are needed for the dependencies that are automatically compiled.
$ sudo apt-get install gettext \
                       libpng12-dev \
                       g++\
                       gtk-doc-tools \
                       libgnutls-dev

# These are needed for WebKitNix itself.
$ sudo apt-get install cmake \
                       flex bison gperf \
                       libsqlite3-dev \
                       libxslt1-dev \
                       libicu-dev \
                       libgl1-mesa-dev

NOTE: the package libgl1-mesa-dev can be replaced by another that
provides the OpenGL headers (GLES-based systems, for example).

Other dependencies like glib, cairo et al. are pulled by jhbuild, by
using the script:

$ Tools/Scripts/update-webkitnix-libs

They are compiled and installed into WebKitBuild/Dependencies.


Debugging
=========

Nix uses the same approach as EFL does to provide debugging support.
It is available only in DEBUG mode.

If you need to debug a crash in a WebProcess while executing the
MiniBrowser you can set the WEB_PROCESS_CMD_PREFIX environment variable.
It takes some time to load all needed symbols - so be patient.

$ WEB_PROCESS_CMD_PREFIX="/usr/bin/xterm -title WebProcess -e gdb --args" \
                                        WebKitBuild/Debug/bin/MiniBrowser

The /usr/bin/xterm is necessary or gdb will run in the current terminal,
which can get particularly confusing since it's running in the background,
and if you're also running the main process in gdb, won't work at all
(the two instances will fight over the terminal). It's necessary to pass the
full path to the xterm binary otherwise it will fail.
