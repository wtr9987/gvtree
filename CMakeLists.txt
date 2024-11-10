cmake_minimum_required (VERSION 3.16.0)

project(gvtree
        VERSION 1.9.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

option(USE_QT6 "Enable this if you want to use Qt6, instead of Qt5" no)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Default build type: Debug" FORCE)
endif()

if(NOT CMAKE_INSTALL_PREFIX) 
    set(CMAKE_INSTALL_PREFIX ".")
endif()

if (NOT DEFINED USE_QT6)
  find_package(Qt6 COMPONENTS Widgets)
  if (NOT Qt6_FOUND)
      find_package(Qt5 5.15 COMPONENTS Core REQUIRED)
  endif(NOT Qt6_FOUND)
endif(NOT DEFINED USE_QT6)

if(USE_QT6)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Network)
else(USE_QT6)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif(USE_QT6)

include(FeatureSummary)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_compile_options(-Wall
                    -Wextra
                    -Wpedantic 
                    $<$<CONFIG:DEBUG>:-fsanitize=address>
                    $<$<CONFIG:DEBUG>:-fno-omit-frame-pointer>)
                    
add_compile_definitions(VERSION="${VERSION}"
                         $<$<CONFIG:RELEASE>:QT_NO_DEBUG_OUTPUT>)

add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)

add_link_options ("$<$<CONFIG:DEBUG>:-fsanitize=address>"
                  "$<$<CONFIG:DEBUG>:-fno-omit-frame-pointer>")

message("Buildtype: ${CMAKE_BUILD_TYPE}")

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
    tagpreflist.cpp 
    tagtree.cpp 
    branchtable.cpp 
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
    tagpreflist.h 
    tagtree.h 
    branchtable.h 
    mimetable.h 
    fromtoinfo.h 
)

set(UIS
    gvtree_preferences.ui 
    gvtree_difftool.ui 
    gvtree_help.ui 
    gvtree_license.ui 
    gvtree_comparetree.ui 
    gvtree_branchtable.ui
)

set(RCS
   gvtree.qrc
)

if(USE_QT6)
  set(QTMODULES
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Network
  )
  qt6_wrap_ui(UI_HDRS ${UIS})
  qt6_add_resources(RC_SRCS ${RCS})
else(USE_QT6)
  set(QTMODULES
    Qt5::Core 
    Qt5::Widgets 
  )
  qt5_wrap_ui(UI_HDRS ${UIS})
  qt5_add_resources(RC_SRCS ${RCS})
endif(USE_QT6)

add_executable(gvtree ${SRCS} ${HDRS} ${UI_HDRS} ${RC_SRCS})

target_compile_definitions(gvtree PUBLIC
        -DINSTALLATION_PATH=\"${CMAKE_INSTALL_PREFIX}\" 
        -DVERSION_NAME=\"${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}-${PROJECT_VERSION_PATCH}\"
        -DAPPLICATION_NAME=\"${PROJECT_NAME}\"
)

target_link_libraries(gvtree ${QTMODULES})

install(TARGETS gvtree
        EXPORT gvtree
        RUNTIME DESTINATION bin)

install(FILES doc/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}-${PROJECT_VERSION_PATCH}.pdf DESTINATION share/doc/${PROJECT_NAME})
