GLX vs. EGL
===========

GLX is used by default, to use EGL pass --egl to build-webkit script.


Packages required
=================

In order to compile Nix on the buildbot, the following packages were
installed:

For jhbuild, following http://trac.webkit.org/wiki/BuildingGtk:
$ sudo apt-get install gobject-introspection \
                       icon-naming-utils \
                       libgcrypt11-dev \
                       libgpg-error-dev \
                       libp11-kit-dev \
                       libtiff4-dev \
                       libcroco3-dev

For WebKitNix dependencies (automatically pulled by jhbuild)
$ sudo apt-get install gettext \
                       libpng12-dev \
                       libfreetype6-dev \
                       g++\
                       gtk-doc-tools \
                       libgnutls-dev

For WebKitNix itself
$ sudo apt-get install cmake \
                       flex bison gperf \
                       libsqlite3-dev \
                       libxslt1-dev \
                       libicu-dev \
                       libgl1-mesa-dev

NOTE: the package libgl1-mesa-dev can be replaced by another that
provides the OpenGL headers (GLES-based systems, for example).

Other dependencies like glib, cairo et al. are pulled by jhbuild.
They are compiled and installed into WebKitBuild/Dependencies.

