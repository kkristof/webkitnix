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

