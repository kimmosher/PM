cmake_minimum_required(VERSION 3.16)
project(PM_Qt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(pm_qt
    main.cpp
    MainWindow.hpp
    MainWindow.cpp
)

qt_add_resources(pm_qt "resources"
    PREFIX "/"
    FILES icons/pm_icon.png
)

target_link_libraries(pm_qt PRIVATE Qt6::Widgets)
