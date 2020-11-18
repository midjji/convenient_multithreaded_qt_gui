cmake_minimum_required(VERSION 3.16)
project(mtgui CXX)
# changes means you need to rerun cmake to update


#qt5
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5 5.9 REQUIRED Core Gui Widgets )

add_library(mtgui "mtgui.h" "mtgui.cpp" ) # must be after the qt stuff is set!

target_compile_features(mtgui PUBLIC cxx_std_17)
#target_configure_warnings(jkqtplotter)
# We need this directory, and users of our library will need it too
target_include_directories(mtgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mtgui PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets  )
FIND_PACKAGE(Threads REQUIRED )
target_link_libraries(mtgui PRIVATE Threads::Threads)
get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
    add_executable(main main.cpp)
    target_link_libraries(main mtgui)
    target_link_libraries(main Qt5::Core Qt5::Gui Qt5::Widgets)
endif()




