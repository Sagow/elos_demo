project(oil_thermometer)

set(CONFIG_FILES
    events.json
    source.json)

set(SOURCES
    main.cpp
    ../common/common.cpp
    ../common/includes.hpp)

include_directories(../common)


foreach(val ${CONFIG_FILES})
    file(COPY ${val} DESTINATION config)
endforeach(val)

add_executable(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} elos safu json-c)
