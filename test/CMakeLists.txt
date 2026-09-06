find_package(Catch2 3 CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(ifmsvg_tests
    main_test.cpp
    path_utils_test.cpp
    bmp_test.cpp
    svg_detect_test.cpp
    svgz_test.cpp
    config_test.cpp
    compositor_test.cpp
    font_db_test.cpp
    svg_render_test.cpp
    ifmsvg_export_test.cpp
    ${MODULE_SOURCES}
)
add_dependencies(ifmsvg_tests ifmsvg)

add_custom_command(TARGET ifmsvg_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ifmsvg>
        $<TARGET_FILE_DIR:ifmsvg_tests>/ifmsvg.sph
)
target_include_directories(ifmsvg_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${RESVG_INCLUDE_DIR}
)
target_link_libraries(ifmsvg_tests PRIVATE
    Catch2::Catch2WithMain
    ZLIB::ZLIB
    resvg
)

include(CTest)
include(Catch)
catch_discover_tests(ifmsvg_tests)
