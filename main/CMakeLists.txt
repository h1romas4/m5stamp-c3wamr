set(INCLUDEDIRS
    ./
)

set(SRCS
    main.cpp
)

idf_component_register(
    INCLUDE_DIRS ${INCLUDEDIRS}
    SRCS ${SRCS}
    REQUIRES arduino-esp32 lcd wamr
)
