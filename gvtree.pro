TEMPLATE = app
TARGET = gvtree
DEPENDPATH += . 
INCLUDEPATH += . 
VERSION = -1.1-0
RC_ICONS = gvtree.ico
CPPFLAGS += -O3
QT += opengl widgets
INSTALLS += target documentation

isEmpty(PREFIX) {
PREFIX="."
}

DEFINES += INSTALLATION_PATH=\"\\\"$${PREFIX}\\\"\" 
DEFINES += VERSION_NAME=\"\\\"$${TARGET}$${VERSION}\\\"\"

RESOURCES += gvtree.qrc

HEADERS += \
        edge.h \
        edgeadapter.h \
        node.h \
        version.h \
        versionadapter.h \
        mainwindow.h \
        graphwidget.h \
        comparetree.h \
        execute_cmd.h \
        tagpreference.h \
        tagprefgridlayout.h \
        taglist.h \
        tagwidget.h \
        mimetable.h \
        fromtoinfo.h 

FORMS += gvtree_preferences.ui \
        gvtree_difftool.ui \
        gvtree_help.ui \
        gvtree_license.ui \
        gvtree_comparetree.ui

SOURCES += \
        edge.cpp \
        edgeadapter.cpp \
        main.cpp \
        mainwindow.cpp \
        node.cpp \
        version.cpp \
        versionadapter.cpp \
        graphwidget.cpp \
        comparetree.cpp \
        execute_cmd.cpp \
        tagpreference.cpp \
        tagprefgridlayout.cpp \
        taglist.cpp \
        tagwidget.cpp \
        mimetable.cpp \
        fromtoinfo.cpp

DISTFILES += $$SOURCEFILES \
  README \
  LICENSE \
  TODO \
  ChangeLog \
  css/gvtree.css \
  doc/GNU_GPL_v3.0.html \
  doc/gvtree-1.1-0.odt \
  doc/gvtree-1.1-0.pdf \
  images/connectorStyle0.png \
  images/connectorStyle1.png \
  images/gvt_add.png \
  images/gvt_del.png \
  images/gvt_mod.png \
  images/gvt_pointer.png \
  images/gvtree_logo_128.png \
  images/gvtree_logo_256.png \
  images/gvtree_logo_512.png \
  images/gvtree_title_512.png \
  images/gvt_ren.png \
  images/gvt_text_512.png \
  images/gvt_refresh.png \
  test/gitlog_test_1.log

TARGET.EPOCHEAPSIZE = 0x200000 0xA00000

# install
target.path = $$PREFIX/bin
source.files = $$SOURCES $$HEADERS $$RESOURCES *.pro

documentation.path = $$PREFIX/share/doc/gvtree
documentation.files = doc/gvtree-1.1-0.pdf
