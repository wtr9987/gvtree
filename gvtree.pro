TEMPLATE = app
TARGET = gvtree
DEPENDPATH += . 
INCLUDEPATH += . 
VERSION = -1.2-0
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
        branchlist.h \
        mimetable.h \
        fromtoinfo.h 

FORMS += gvtree_preferences.ui \
        gvtree_difftool.ui \
        gvtree_help.ui \
        gvtree_license.ui \
        gvtree_comparetree.ui \
        gvtree_branchlist.ui

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
        branchlist.cpp \
        mimetable.cpp \
        fromtoinfo.cpp

DISTFILES += $$SOURCEFILES \
  README \
  LICENSE \
  ChangeLog \
  css/gvtree.css \
  doc/GNU_GPL_v3.0.html \
  doc/gvtree-1.2-0.odt \
  doc/gvtree-1.2-0.pdf \
  doc/doc_test_repository.sh \
  doc/images/f1.png \
  doc/images/f2.png \
  doc/images/f3.png \
  doc/images/f4.png \
  doc/images/f5.png \
  doc/images/f6.png \
  doc/images/f7.png \
  doc/images/f8.png \
  doc/images/f10.png \
  doc/images/f11.png \
  doc/images/f12.png \
  doc/images/f13.png \
  doc/images/f14.png \
  doc/images/f15.png \
  doc/images/f16.png \
  doc/images/f17.png \
  doc/images/f18.png \
  doc/images/f20.png \
  doc/images/f21.png \
  doc/images/f22.png \
  doc/images/f23.png \
  doc/images/f24.png \
  doc/images/f25.png \
  doc/images/f27.png \
  doc/images/f28.png \
  doc/images/f29.png \
  doc/images/f30.png \
  doc/images/f31.png \
  doc/images/f32.png \
  doc/images/f33.png \
  doc/images/f34.png \
  doc/images/f35.png \
  doc/images/f36.png \
  doc/images/f37.png \
  doc/images/f38.png \
  doc/images/f39.png \
  doc/images/f40.png \
  doc/images/f41.png \
  doc/images/f42.png \
  doc/images/f43.png \
  doc/images/f44.png \
  doc/images/f45.png \
  doc/images/f46.png \
  doc/images/f47.png \
  doc/images/f48.png \
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
documentation.files = doc/gvtree-1.2-0.pdf
