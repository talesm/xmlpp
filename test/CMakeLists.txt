
add_executable(xmlpp_test
    catch
    sax_test
)

target_link_libraries(xmlpp_test
    PRIVATE xmlpp
)

add_test(unit_test xmlpp_test)
