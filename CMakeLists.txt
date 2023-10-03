cmake_minimum_required (VERSION 3.0)

project(gvtree
        VERSION 1.7.0
        LANGUAGES CXX)

include(FeatureSummary)

                  
set(QT_MIN_VERSION "5.0.0")
set(CMAKE_CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Default build type: Debug" FORCE)
endif()

if(NOT PREFIX) 
    set(PREFIX ".")
endif()

find_package(Qt5Core ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Gui ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Network ${QT_MIN_VERSION} REQUIRED)
find_package(Qt5Widgets ${QT_MIN_VERSION} REQUIRED)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_compile_options(-Wall
                    -Wextra
                    -Wpedantic 
                    $<$<CONFIG:DEBUG>:-fsanitize=address>
                    $<$<CONFIG:DEBUG>:-fno-omit-frame-pointer>)
                    
add_compile_definitions (VERSION="${PROJECT_VERSION}"
                         $<$<CONFIG:RELEASE>:QT_NO_DEBUG_OUTPUT>)

add_link_options ("$<$<CONFIG:DEBUG>:-fsanitize=address>"
                  "$<$<CONFIG:DEBUG>:-fno-omit-frame-pointer>")

message("Buildtype: ${CMAKE_BUILD_TYPE}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRCS
    edge.cpp 
    edgeadapter.cpp 
    main.cpp 
    mainwindow.cpp 
    node.cpp 
    version.cpp 
    versionadapter.cpp 
    graphwidget.cpp
    comparetree.cpp 
    execute_cmd.cpp 
    tagpreference.cpp 
    tagprefgridlayout.cpp 
    tagtree.cpp 
    branchlist.cpp 
    mimetable.cpp 
    fromtoinfo.cpp
)

set(HDRS
    edge.h 
    edgeadapter.h 
    node.h 
    version.h 
    versionadapter.h 
    mainwindow.h 
    graphwidget.h 
    comparetree.h 
    execute_cmd.h
    tagpreference.h 
    tagprefgridlayout.h 
    tagtree.h 
    branchlist.h 
    mimetable.h 
    fromtoinfo.h 
)

set(UIS
    gvtree_preferences.ui 
    gvtree_difftool.ui 
    gvtree_help.ui 
    gvtree_license.ui 
    gvtree_comparetree.ui 
    gvtree_branchlist.ui
)

set ( RCS
   gvtree.qrc
)

set ( QTMODULES
  Qt5::Core 
  Qt5::Widgets 
  Qt5::Network
 )
 
qt5_wrap_ui(UI_HDRS ${UIS})
qt5_add_resources(RC_SRCS ${RCS})

add_executable(gvtree ${SRCS} ${HDRS} ${UI_HDRS} ${RC_SRCS})

target_compile_definitions(gvtree PUBLIC
    -DINSTALLATION_PATH=\"\\\"$${PREFIX}\\\"\" 
    -DVERSION_NAME=\"\\\"$${TARGET}$${VERSION}\\\"\"
    -DAPPLICATION_NAME=${PROJECT_NAME}
)

target_link_libraries(gvtree ${QTMODULES})

install(TARGETS gvtree
        EXPORT gvtree
        RUNTIME DESTINATION bin)
