add_executable(math-reader
    "math-reader"
)

configure_file(config.h.in config.h)

target_include_directories(math-reader
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(math-reader
    PRIVATE xmlpp
)