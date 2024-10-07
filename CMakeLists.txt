cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(tiny_http)

set(CMAKE_C_STANDARD 99)
include(CheckIPOSupported)
check_ipo_supported()

# Extra warnings, treat warnings as error
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
   SET(TH_WARNINGS -Wall -Wextra -Wpedantic -Werror)
   message(STATUS "Using Clang/GNU compiler, enabling extra warnings: ${TH_WARNINGS}")
   add_compile_options(${TH_WARNINGS})
endif()

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
    src/th_acceptor.c
    src/th_runner.c
    src/th_io_task.c
    src/th_io_composite.c
    src/th_io_op_posix.c
    src/th_io_op_bsd.c
    src/th_io_op_linux.c
    src/th_allocator.c
    src/th_task.c
    src/th_kqueue_service.c
    src/th_poll_service.c
    src/th_error.c
    src/th_socket.c
    src/th_tcp_socket.c
    src/th_request_parser.c
    src/th_request.c
    src/th_response.c
    src/th_context.c
    src/th_client.c
    src/th_header_id.c
    src/th_file.c
    src/th_fcache.c
    src/th_dir.c
    src/th_dir_mgr.c
    src/th_client_acceptor.c
    src/th_string.c
    src/th_heap_string.c
    src/th_log.c
    src/th_exchange.c
    src/th_fmt.c
    src/th_date.c
    src/th_io_op.c
    src/th_timer.c
    src/th_client_tracker.c
    src/th_url_decode.c
    src/th_path.c
#   SSL
    src/th_ssl_smem_bio.c
    src/th_ssl_context.c
    src/th_ssl_socket.c
    src/th_ssl_error.c
)

SET(TH_TEST_SRC
    src/th_mock_service.c
    src/th_mock_syscall.c
    src/th_io_op_mock.c
    src/th_test.c
)

add_library(tiny_http 
    ${TH_CORE_SRC}
)
add_library(tiny_http::tiny_http ALIAS tiny_http)

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

add_library(tiny_http::tiny_http ALIAS tiny_http)

# Examples

if (NOT TH_DISABLE_EXAMPLES)
    # Hello world example
    add_executable(hello_world
        examples/hello_world.c
    )
    target_link_libraries(hello_world PUBLIC 
        tiny_http::tiny_http
    )
    # Echo server example
    add_executable(echo
        examples/echo.c
    )
    target_link_libraries(echo PUBLIC 
        tiny_http::tiny_http
    )
    # File server example
    add_executable(file_server
        examples/file_server.c
    )
    target_link_libraries(file_server PUBLIC 
        tiny_http::tiny_http
    )
    # Custom Logger example
    add_executable(custom_logger
        examples/custom_logger.c
    )
    target_link_libraries(custom_logger PUBLIC 
        tiny_http::tiny_http
    )
endif()

# Tests

if (NOT TH_DISABLE_TESTS)
    include(CMakePushCheckState)
    include(CheckCCompilerFlag)
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_LINK_OPTIONS -fsanitize=address)
    check_c_compiler_flag("-fsanitize=address" TH_HAVE_ASAN)
    cmake_pop_check_state()

    add_library(tiny_http_test
        ${TH_CORE_SRC}
        ${TH_TEST_SRC}
    )
    if (OpenSSL_FOUND)
        target_link_libraries(tiny_http_test PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
    if (TH_HAVE_ASAN)
        target_compile_options(tiny_http_test PUBLIC -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(tiny_http_test PUBLIC -fsanitize=address -fno-omit-frame-pointer)
    else()
        message(WARNING "Address sanitizer not found, tests will not be run with ASAN")
    endif()
    target_compile_definitions(tiny_http_test PRIVATE TH_CONFIG_OS_MOCK=1)
    target_compile_definitions(tiny_http_test PRIVATE TH_LOG_LEVEL=${TH_LOG_LEVEL})
    target_compile_definitions(tiny_http_test PRIVATE TH_WITH_SSL=${TH_WITH_SSL})
    target_include_directories(tiny_http_test 
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
        PRIVATE src
    )
    enable_testing()
    set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "th_test_setup();")
    set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "th_test_teardown();")

    set(TH_TESTS 
        src/th_io_task_test.c
        src/th_task_test.c
        src/th_router_test.c
        src/th_allocator_test.c
        src/th_request_parser_test.c
        src/th_response_test.c
        src/th_fcache_test.c
        src/th_hashmap_test.c
        src/th_string_test.c
        src/th_list_test.c
        src/th_dir_mgr_test.c
        src/th_tcp_socket_test.c
        src/th_heap_string_test.c
    )

    create_test_sourcelist(TH_TEST_SRC_LIST th_test.c
        ${TH_TESTS}
        EXTRA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/th_test.h
    )

    add_executable(th_test ${TH_TEST_SRC_LIST})

    target_link_libraries(th_test PUBLIC 
        tiny_http_test
    )

    foreach (test_src ${TH_TESTS})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_test(NAME src/${test_name} COMMAND th_test src/${test_name})
    endforeach()
endif()

# Amalgamation

if (NOT TH_DISABLE_AMALGAMATION)
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
