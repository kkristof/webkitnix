# -------------------------------------------------------------------
# Target file for the QtWebKitWidgets dynamic library
#
# See 'Tools/qmake/README' for an overview of the build system
# -------------------------------------------------------------------

# Use Qt5's module system
load(qt_build_config)

TEMPLATE = lib
TARGET = QtWebKitWidgets

# This is relied upon by our export macros and seems not to be properly
# defined by the logic in qt_module.prf as it should
DEFINES += QT_BUILD_WEBKITWIDGETS_LIB

WEBKIT_DESTDIR = $${ROOT_BUILD_DIR}/lib

SOURCE_DIR = $${ROOT_WEBKIT_DIR}/Source/WebKit

INCLUDEPATH += \
    $$SOURCE_DIR/qt/Api \
    $$SOURCE_DIR/qt/WidgetApi \
    $$SOURCE_DIR/qt/WebCoreSupport \
    $$SOURCE_DIR/qt/WidgetSupport \
    $$ROOT_WEBKIT_DIR/Source/WTF/wtf/qt

have?(qtsensors):if(enable?(DEVICE_ORIENTATION)|enable?(ORIENTATION_EVENTS)): QT += sensors

have?(qtlocation):enable?(GEOLOCATION): QT += location

use?(QT_MULTIMEDIA): QT *= multimediawidgets

contains(CONFIG, texmap): DEFINES += WTF_USE_TEXTURE_MAPPER=1

use?(PLUGIN_BACKEND_XLIB): PKGCONFIG += x11

QT += network widgets widgets-private
have?(QTQUICK): QT += quick
have?(QTPRINTSUPPORT): QT += printsupport

use?(TEXTURE_MAPPER_GL)|enable?(WEBGL) {
    QT *= opengl
}
QT += webkit

use?(3D_GRAPHICS): WEBKIT += ANGLE

WEBKIT += javascriptcore wtf webcore

MODULE = webkitwidgets

# We want the QtWebKit API forwarding includes to live in the root build dir.
MODULE_BASE_DIR = $$_PRO_FILE_PWD_
MODULE_BASE_OUTDIR = $$ROOT_BUILD_DIR

# This is the canonical list of dependencies for the public API of
# the QtWebKitWidgets library, and will end up in the library's prl file.
QT_API_DEPENDS = core gui widgets network webkit

# ---------------- Custom developer-build handling -------------------
#
# The assumption for Qt developer builds is that the module file
# will be put into qtbase/mkspecs/modules, and the libraries into
# qtbase/lib, with rpath/install_name set to the Qt lib dir.
#
# For WebKit we don't want that behavior for the libraries, as we want
# them to be self-contained in the WebKit build dir.
#
!production_build: CONFIG += force_independent

BASE_TARGET = $$TARGET

load(qt_module)

# Allow doing a debug-only build of WebKit (not supported by Qt)
macx:!debug_and_release:debug: TARGET = $$BASE_TARGET

# Make sure the install_name of the QtWebKit library point to webkit
force_independent:macx {
    # We do our own absolute path so that we can trick qmake into
    # using the webkit build path instead of the Qt install path.
    CONFIG -= absolute_library_soname
    QMAKE_LFLAGS_SONAME = $$QMAKE_LFLAGS_SONAME$$WEBKIT_DESTDIR/

    !debug_and_release|build_pass {
        # We also have to make sure the install_name is correct when
        # the library is installed.
        change_install_name.depends = install_target

        # The install rules generated by qmake for frameworks are busted in
        # that both the debug and the release makefile copy QtWebKit.framework
        # into the install dir, so whatever changes we did to the release library
        # will get overwritten when the debug library is installed. We work around
        # that by running install_name on both, for both configs.
        change_install_name.commands = framework_dir=\$\$(dirname $(TARGETD)); \
            for file in \$\$(ls $$[QT_INSTALL_LIBS]/\$\$framework_dir/$$BASE_TARGET*); do \
                install_name_tool -id \$\$file \$\$file; \
            done
        default_install_target.target = install
        default_install_target.depends += change_install_name

        QMAKE_EXTRA_TARGETS += change_install_name default_install_target
    }
}

SOURCES += \
    $$PWD/WebKit/qt/WidgetApi/qgraphicswebview.cpp \
    $$PWD/WebKit/qt/WidgetApi/qwebframe.cpp \
    $$PWD/WebKit/qt/WidgetApi/qwebpage.cpp \
    $$PWD/WebKit/qt/WidgetApi/qwebview.cpp \
    $$PWD/WebKit/qt/WidgetApi/qwebinspector.cpp \
    $$PWD/WebKit/qt/WidgetSupport/QtFallbackWebPopup.cpp \
    $$PWD/WebKit/qt/WidgetSupport/QtWebComboBox.cpp \
    $$PWD/WebKit/qt/WidgetSupport/QWebUndoCommand.cpp \
    $$PWD/WebKit/qt/WidgetSupport/DefaultFullScreenVideoHandler.cpp \
    $$PWD/WebKit/qt/WidgetSupport/InitWebKitQt.cpp \
    $$PWD/WebKit/qt/WidgetSupport/InspectorClientWebPage.cpp \
    $$PWD/WebKit/qt/WidgetSupport/PageClientQt.cpp \
    $$PWD/WebKit/qt/WidgetSupport/QStyleFacadeImp.cpp \
    $$PWD/WebKit/qt/WidgetSupport/QGraphicsWidgetPluginImpl.cpp \
    $$PWD/WebKit/qt/WidgetSupport/QWidgetPluginImpl.cpp

HEADERS += \
    $$PWD/WebKit/qt/WidgetApi/qgraphicswebview.h \
    $$PWD/WebKit/qt/WidgetApi/qwebframe.h \
    $$PWD/WebKit/qt/WidgetApi/qwebframe_p.h \
    $$PWD/WebKit/qt/WidgetApi/qwebpage.h \
    $$PWD/WebKit/qt/WidgetApi/qwebpage_p.h \
    $$PWD/WebKit/qt/WidgetApi/qwebview.h \
    $$PWD/WebKit/qt/WidgetApi/qwebinspector.h \
    $$PWD/WebKit/qt/WidgetApi/qwebinspector_p.h \
    $$PWD/WebKit/qt/Api/qwebkitplatformplugin.h \
    $$PWD/WebKit/qt/WidgetSupport/InitWebKitQt.h \
    $$PWD/WebKit/qt/WidgetSupport/InspectorClientWebPage.h \
    $$PWD/WebKit/qt/WidgetSupport/DefaultFullScreenVideoHandler.h \
    $$PWD/WebKit/qt/WidgetSupport/QtFallbackWebPopup.h \
    $$PWD/WebKit/qt/WidgetSupport/QtWebComboBox.h \
    $$PWD/WebKit/qt/WidgetSupport/QWebUndoCommand.h \
    $$PWD/WebKit/qt/WidgetSupport/PageClientQt.h \
    $$PWD/WebKit/qt/WidgetSupport/QGraphicsWidgetPluginImpl.h \
    $$PWD/WebKit/qt/WidgetSupport/QStyleFacadeImp.h \
    $$PWD/WebKit/qt/WidgetSupport/QWidgetPluginImpl.h \

contains(QT_CONFIG, accessibility) {
    SOURCES += $$PWD/WebKit/qt/WidgetApi/qwebviewaccessible.cpp
    HEADERS += $$PWD/WebKit/qt/WidgetApi/qwebviewaccessible_p.h 
}

INCLUDEPATH += \
    $$PWD/qt/Api \
    $$PWD/qt/WebCoreSupport

enable?(VIDEO) {
    !use?(QTKIT):!use?(GSTREAMER):use?(QT_MULTIMEDIA) {
        HEADERS += $$PWD/WebKit/qt/WidgetSupport/FullScreenVideoWidget.h
        SOURCES += $$PWD/WebKit/qt/WidgetSupport/FullScreenVideoWidget.cpp
    }
}

