cmake_minimum_required(VERSION 3.5)

project(zebra_scanner_labeling)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets Xml REQUIRED)

set(SOURCE_FILES
    src/ConsoleMain.cpp
    src/ConsoleSampleEventListener.cpp
    src/ISO15434formatEnvelope.cpp
)

set(LIBS
    cs-client
    cs-common
)

add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    dialog.cpp
    dialog.h
    dialog.ui
    pugixml.cpp

    ${SOURCE_FILES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::Xml
    -L/usr/lib/zebra-scanner/corescanner
    ${LIBS}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    include
    /usr/include/zebra-scanner
    /usr/lib/zebra-scanner/corescanner
)
