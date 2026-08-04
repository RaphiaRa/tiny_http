cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(tiny_http)

set(CMAKE_C_STANDARD 99)
include(CheckIPOSupported)
check_ipo_supported()

# Run gperf
find_program(GPERF gperf)
if (GPERF)
    message(STATUS "Found gperf: ${GPERF}")
    function(gperf_generate OUTPUT_FILE INPUT_FILE)
        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${GPERF} -L ANSI-C ${INPUT_FILE} > ${OUTPUT_FILE}
            DEPENDS ${INPUT_FILE}
            COMMENT "Running gperf on ${INPUT_FILE}"
        )
    endfunction(gperf_generate OUTPUT_FILE INPUT_FILE)
    gperf_generate(${CMAKE_CURRENT_SOURCE_DIR}/src/th_mime.c ${CMAKE_CURRENT_SOURCE_DIR}/src/th_mime.gperf)
    gperf_generate(${CMAKE_CURRENT_SOURCE_DIR}/src/th_method.c ${CMAKE_CURRENT_SOURCE_DIR}/src/th_method.gperf)
    gperf_generate(${CMAKE_CURRENT_SOURCE_DIR}/src/th_header_id.c ${CMAKE_CURRENT_SOURCE_DIR}/src/th_header_id.gperf)
else()
    message(FATAL_ERROR "gperf not found")
endif()

SET(TH_CORE_SRC
    src/th_server.c
    src/th_listener.c
    src/th_router.c
    src/th_mime.c
    src/th_method.c
    src/th_allocator.c
    src/th_task.c
    src/th_poll.c
    src/th_loop.c
    src/th_error.c
    src/th_socket.c
    src/th_recv.c
    src/th_send.c
    src/th_sendvec.c
    src/th_sendfile.c
    src/th_acceptor.c
    src/th_accept.c
    src/th_tcp_conn.c
    src/th_request_parser.c
    src/th_cookie_parser.c
    src/th_multipart_parser.c
    src/th_part.c
    src/th_request.c
    src/th_response.c
    src/th_conn.c
    src/th_header_id.c
    src/th_file.c
    src/th_fcache.c
    src/th_dir.c
    src/th_dir_mgr.c
    src/th_str.c
    src/th_string.c
    src/th_log.c
    src/th_http.c
    src/th_fmt.c
    src/th_date.c
    src/th_clock.c
    src/th_timer.c
    src/th_conn_tracker.c
    src/th_url_decode.c
#   SSL (compiled out via TH_WITH_SSL=0 until SSL is ported onto th_conn/th_socket)
    src/th_ssl_smem_bio.c
    src/th_ssl_context.c
    src/th_ssl_error.c
)

SET(TH_TEST_SRC
    src/th_test.c
)

add_library(tiny_http
    ${TH_CORE_SRC}
)
add_library(tiny_http::tiny_http ALIAS tiny_http)

# Extra warnings, treat warnings as error
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
   SET(TH_WOPTIONS -Wall -Wextra -Wpedantic -Werror -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wconversion)
   message(STATUS "Using Clang/GNU compiler, enabling extra warnings: ${TH_WOPTIONS}")
   target_compile_options(tiny_http PRIVATE ${TH_WOPTIONS})
endif()

# Address sanitizer (off by default - opt in, e.g. for test builds)
option(TH_ENABLE_ASAN "Build tiny_http with AddressSanitizer" OFF)
if (TH_ENABLE_ASAN)
    include(CMakePushCheckState)
    include(CheckCCompilerFlag)
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_LINK_OPTIONS -fsanitize=address)
    check_c_compiler_flag("-fsanitize=address" TH_HAVE_ASAN)
    cmake_pop_check_state()
    if (TH_HAVE_ASAN)
        target_compile_options(tiny_http PUBLIC -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(tiny_http PUBLIC -fsanitize=address -fno-omit-frame-pointer)
    else()
        message(WARNING "TH_ENABLE_ASAN is on but AddressSanitizer is not supported by this compiler")
    endif()
endif()

# Undefined behavior sanitizer (off by default - opt in, e.g. for test builds)
option(TH_ENABLE_UBSAN "Build tiny_http with UndefinedBehaviorSanitizer" OFF)
if (TH_ENABLE_UBSAN)
    include(CMakePushCheckState)
    include(CheckCCompilerFlag)
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_LINK_OPTIONS -fsanitize=undefined)
    check_c_compiler_flag("-fsanitize=undefined" TH_HAVE_UBSAN)
    cmake_pop_check_state()
    if (TH_HAVE_UBSAN)
        target_compile_options(tiny_http PUBLIC -fsanitize=undefined -fno-omit-frame-pointer)
        target_link_options(tiny_http PUBLIC -fsanitize=undefined -fno-omit-frame-pointer)
    else()
        message(WARNING "TH_ENABLE_UBSAN is on but UndefinedBehaviorSanitizer is not supported by this compiler")
    endif()
endif()

# Link time optimization
check_ipo_supported(RESULT ipo_supported)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET tiny_http PROPERTY INTERPROCEDURAL_OPTIMIZATION ${ipo_supported})
    MESSAGE(STATUS "Enabling IPO for Release build: ${ipo_supported}")
endif()

# Logging
if (NOT DEFINED TH_LOG_LEVEL)
    set(TH_LOG_LEVEL TH_LOG_LEVEL_INFO)
endif()

# OpenSSL
find_package(OpenSSL)
if (OpenSSL_FOUND)
    message(STATUS "Found OpenSSL: ${OPENSSL_INCLUDE_DIR}")
    target_link_libraries(tiny_http PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    set(TH_WITH_SSL 1)
else()
    message(STATUS "OpenSSL not found, disabling SSL support")
    set(TH_WITH_SSL 0)
endif()

# System specific configuration
include(CheckFunctionExists)
check_function_exists(sendfile TH_WITH_SENDFILE)
message(STATUS "sendfile: ${TH_WITH_SENDFILE}")
check_function_exists(sendfile TH_WITH_MMAP)
message(STATUS "mmap: ${TH_WITH_MMAP}")

target_compile_definitions(tiny_http PRIVATE TH_LOG_LEVEL=${TH_LOG_LEVEL})
target_compile_definitions(tiny_http PRIVATE TH_WITH_SSL=${TH_WITH_SSL})
target_compile_definitions(tiny_http PRIVATE TH_HAVE_MMAP=${TH_WITH_MMAP})
target_compile_definitions(tiny_http PRIVATE TH_HAVE_SENDFILE=${TH_WITH_SENDFILE})
target_include_directories(tiny_http
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    PRIVATE src
)



if (NOT TH_DISABLE_EXAMPLES)
    function(add_tiny_http_example EXAMPLE_NAME)
        add_executable(${EXAMPLE_NAME} examples/${EXAMPLE_NAME}.c)
        target_link_libraries(${EXAMPLE_NAME} PUBLIC tiny_http::tiny_http)
    endfunction(add_tiny_http_example)
    add_tiny_http_example(custom_logger)
    add_tiny_http_example(echo)
    add_tiny_http_example(file_server)
    add_tiny_http_example(file_upload)
    add_tiny_http_example(hello_world)
endif()

# Tests

if (NOT TH_DISABLE_TESTS)
    option(TH_ENABLE_COVERAGE "Instrument tiny_http with gcov coverage" OFF)
    if (TH_ENABLE_COVERAGE)
        if (NOT CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
            message(FATAL_ERROR "TH_ENABLE_COVERAGE requires Clang or GNU")
        endif()
        target_compile_options(tiny_http PUBLIC --coverage -O0 -g)
        target_link_options(tiny_http PUBLIC --coverage)
    endif()

    enable_testing()
    set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "th_test_setup();")
    set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "th_test_teardown();")

    set(TH_TESTS
        src/th_task_test.c
        src/th_timer_test.c
        src/th_loop_test.c
        src/th_poll_test.c
        src/th_socket_test.c
        src/th_recv_test.c
        src/th_send_test.c
        src/th_sendvec_test.c
        src/th_sendfile_test.c
        src/th_acceptor_test.c
        src/th_accept_test.c
        src/th_conn_test.c
        src/th_tcp_conn_test.c
        src/th_router_test.c
        src/th_allocator_test.c
        src/th_request_parser_test.c
        src/th_cookie_parser_test.c
        src/th_multipart_parser_test.c
        src/th_response_test.c
        src/th_http_test.c
        src/th_fcache_test.c
        src/th_hashmap_test.c
        src/th_str_test.c
        src/th_list_test.c
        src/th_dir_mgr_test.c
        src/th_string_test.c
    )

    create_test_sourcelist(TH_TEST_SRC_LIST th_test.c
        ${TH_TESTS}
        EXTRA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/th_test.h
    )

    add_executable(th_test ${TH_TEST_SRC_LIST} ${TH_TEST_SRC})

    target_link_libraries(th_test PUBLIC
        tiny_http::tiny_http
    )
    target_include_directories(th_test PRIVATE src)

    foreach (test_src ${TH_TESTS})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_test(NAME src/${test_name} COMMAND th_test src/${test_name})
    endforeach()

    if (TH_ENABLE_COVERAGE)
        find_program(GCOVR gcovr)
        if (GCOVR)
            add_custom_target(coverage
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/coverage
                COMMAND ${GCOVR} --root ${CMAKE_CURRENT_SOURCE_DIR}
                    --filter ${CMAKE_CURRENT_SOURCE_DIR}/src/
                    --exclude .*_test\\.c$$
                    --object-directory ${CMAKE_CURRENT_BINARY_DIR}
                    --print-summary
                    --html-details ${CMAKE_CURRENT_BINARY_DIR}/coverage/index.html
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                DEPENDS th_test
                COMMENT "Running tests and generating coverage report"
                VERBATIM
            )
        else()
            message(WARNING "TH_ENABLE_COVERAGE is on but gcovr was not found, 'coverage' target will not be available")
        endif()
    endif()
endif()

# Benchmarks

option(TH_ENABLE_BENCHMARKS "Build th_bench, a microbenchmark driver" OFF)
if (TH_ENABLE_BENCHMARKS)
    # th_bench cases don't use th_test's setup/teardown hooks
    unset(CMAKE_TESTDRIVER_BEFORE_TESTMAIN)
    unset(CMAKE_TESTDRIVER_AFTER_TESTMAIN)

    set(TH_BENCHES
        src/th_str_bench.c
        src/th_cookie_parser_bench.c
        src/th_multipart_parser_bench.c
    )

    create_test_sourcelist(TH_BENCH_SRC_LIST th_bench.c
        ${TH_BENCHES}
        EXTRA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/th_bench.h
    )

    add_executable(th_bench ${TH_BENCH_SRC_LIST})

    target_link_libraries(th_bench PUBLIC
        tiny_http::tiny_http
    )
    target_include_directories(th_bench PRIVATE src)
endif()

# Amalgamation

if (NOT TH_DISABLE_AMALGAMATION)
    set(Python3_FIND_VIRTUALENV FIRST)
    find_package(Python3)
    if (Python3_FOUND)
        message(STATUS "Found Python3: ${Python3_EXECUTABLE}")
    else()
        message(STATUS "Python3 not found, disabling amalgamation")
        SET(TH_DISABLE_AMALGAMATION 1)
    endif()
endif()

# Check whether all required python modules are available

if (NOT TH_DISABLE_AMALGAMATION)
    SET(PYTHON_REQUIRED_MODULES os re argparse networkx)
    foreach (PYTHON_MODULE ${PYTHON_REQUIRED_MODULES})
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "import ${PYTHON_MODULE}"
            RESULT_VARIABLE PYTHON_MODULE_RESULT
        )
        if (NOT PYTHON_MODULE_RESULT EQUAL 0)
            message(STATUS "Python module ${PYTHON_MODULE} not found, disabling amalgamation")
            SET(TH_DISABLE_AMALGAMATION 1)
        endif()
    endforeach()
endif()


if (NOT TH_DISABLE_AMALGAMATION)
    # run amalgamation.py with TH_CORE_SRC and TH_3RD_PARTY_SRC
   function(amalgamation_generate OUTPUT_FILE)
        # Amalgamation of th.c
        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/amalgamate.py -o ${OUTPUT_FILE} ${TH_CORE_SRC}
            COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/include/th.h ${CMAKE_CURRENT_BINARY_DIR}/th.h
            DEPENDS tiny_http
            COMMENT "Running amalgamation.py on ${OUTPUT_FILE}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
        )
    endfunction(amalgamation_generate OUTPUT_FILE)
    amalgamation_generate(${CMAKE_CURRENT_BINARY_DIR}/th.c)

    # Add amalgamation target
    # Not actually needed, but this makes sure that the amalgamation file compiles
    add_library(tiny_http_amalgamation STATIC ${CMAKE_CURRENT_BINARY_DIR}/th.c)
    target_include_directories(tiny_http_amalgamation
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
        PRIVATE src
    )
endif()
