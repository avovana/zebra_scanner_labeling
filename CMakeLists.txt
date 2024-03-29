cmake_minimum_required(VERSION 3.5)

project(zebra_scanner_labeling)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(Qt5 COMPONENTS Widgets Xml SerialPort Core REQUIRED)

set(SOURCE_FILES
    src/ConsoleMain.cpp
    src/ConsoleSampleEventListener.cpp
    src/ISO15434formatEnvelope.cpp
)

set(LIBS
    cs-client
    cs-common
)

include_directories(${CMAKE_BINARY_DIR})
#find_package(Git)
#add_custom_target(version
#  ${CMAKE_COMMAND} -D SRC=${CMAKE_SOURCE_DIR}/foobar_version.h.in
#                   -D DST=${CMAKE_BINARY_DIR}/foobar_version.h
#                   -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
#                   -P ${CMAKE_SOURCE_DIR}/GenerateVersionHeader.cmake
#  )


add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    dialog.cpp
    dialog.h
    dialog.ui
    output.h
    output.cpp
    output.ui
    position.h
    pugixml.cpp
    sender.h
    sender.cpp
    foobar_version.h.in

    ${SOURCE_FILES}
)

#add_dependencies(${PROJECT_NAME} version)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::Xml
    Qt5::SerialPort
    Qt5::Core
    -L/usr/lib/zebra-scanner/corescanner
    ${LIBS}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    include
    /usr/include/zebra-scanner
    /usr/lib/zebra-scanner/corescanner
)
