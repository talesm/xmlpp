
add_executable(xmlpp_test
    catch
    sax_test
)

target_link_libraries(xmlpp_test
    PRIVATE xmlpp
)
