cmake_minimum_required(VERSION 3.16)
project(PM_Qt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(pm_qt
    main.cpp
    MainWindow.hpp
    MainWindow.cpp
    TextAdventureWidget.cpp
    TextAdventureWidget.hpp
    TutorialDialog.hpp
    TutorialDialog.cpp
    MinimapWidget.hpp
    TriviaWidget.hpp
    TriviaWidget.cpp
    MinimapWidget.cpp
    DoodlePadWidget.hpp
    DoodlePadWidget.cpp
    SolitaireWidget.h
    SolitaireWidget.cpp
    SolitaireGame.h
    SolitaireGame.cpp
    Card.h
    Pile.h
    resources.qrc
)

target_link_libraries(pm_qt PRIVATE Qt6::Widgets Qt6::Network)
