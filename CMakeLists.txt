cmake_minimum_required(VERSION 3.18)
include(FetchContent)

# Host CPU
if(NOT CMAKE_HOST_SYSTEM_PROCESSOR)
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(CMAKE_HOST_SYSTEM_PROCESSOR "${ARCH}")
endif()
if(NOT CMAKE_HOST_SYSTEM_PROCESSOR)
    message(WARNING "host system processor not detected.")
    set(CMAKE_HOST_SYSTEM_PROCESSOR "x86_64")
endif()
string(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} CMAKE_HOST_SYSTEM_NAME_LOWER)

# Zig
set(ZIG_VERSION "0.13.0")
set(ZIG_DIR "${CMAKE_SOURCE_DIR}/vendor/zig")
set(ZIG_DOWNLOAD "http://ziglang.org/download")
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
    set(ZIG_DOWNLOAD "${ZIG_DOWNLOAD}/${ZIG_VERSION}/zig-macos-${CMAKE_HOST_SYSTEM_PROCESSOR}-${ZIG_VERSION}.tar.xz")
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    set(ZIG_DOWNLOAD "${ZIG_DOWNLOAD}/${ZIG_VERSION}/zig-linux-${CMAKE_HOST_SYSTEM_PROCESSOR}-${ZIG_VERSION}.tar.xz")
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    set(ZIG_DOWNLOAD "${ZIG_DOWNLOAD}/${ZIG_VERSION}/zig-windows-${CMAKE_HOST_SYSTEM_PROCESSOR}-${ZIG_VERSION}.zip")
endif()
if(NOT EXISTS ${ZIG_DIR})
    FetchContent_Populate(dep_zig URL ${ZIG_DOWNLOAD} SOURCE_DIR ${ZIG_DIR})
endif()
find_program(ZIGC zig PATHS "${CMAKE_SOURCE_DIR}/vendor/zig" REQUIRED NO_DEFAULT_PATH)

# Compiler
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain.cmake")

# Project
project(game C)
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/vendor")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})

# Font
set(FONT_VERSION "0.2.1")
set(FONT_DIR "${CMAKE_SOURCE_DIR}/vendor/font")
set(FONT_DOWNLOAD "https://github.com/gamelly/gly-type/archive/refs/tags/${FONT_VERSION}.tar.gz")
if(NOT EXISTS ${FONT_DIR})
    FetchContent_Populate(dep_font URL ${FONT_DOWNLOAD} SOURCE_DIR ${FONT_DIR})
endif()

# Color
set(COLOR_DIR "${CMAKE_SOURCE_DIR}/vendor/color")
set(COLOR_DOWNLOAD "https://github.com/rodrigodornelles/rgb-to-xterm256/archive/refs/heads/main.zip")
if(NOT EXISTS "${COLOR_DIR}")
    FetchContent_Populate(lib_color URL "${COLOR_DOWNLOAD}" SOURCE_DIR "${COLOR_DIR}")
endif()

# lua
set(LUA_VERSION "5.4.7")
set(LUA_DIR "${CMAKE_SOURCE_DIR}/vendor/lua")
set(LUA_DOWNLOAD "https://github.com/lua/lua/archive/refs/tags/v${LUA_VERSION}.tar.gz")
if(NOT EXISTS "${LUA_DIR}/lua")
    FetchContent_Populate(dep_lua URL "${LUA_DOWNLOAD}" SOURCE_DIR ${LUA_DIR})
    execute_process(
        WORKING_DIRECTORY ${LUA_DIR}
        COMMAND ${ZIGC} cc -DMAKE_LUA onelua.c -o lua
            -target ${CMAKE_HOST_SYSTEM_PROCESSOR}-${CMAKE_HOST_SYSTEM_NAME_LOWER}
    )
endif()
file(GLOB lua_files "${LUA_DIR}/*.c")
list(REMOVE_ITEM lua_files "${LUA_DIR}/lua.c")
list(REMOVE_ITEM lua_files "${LUA_DIR}/onelua.c")
add_library(lua-static STATIC ${lua_files})
find_program(LUAC lua PATHS ${LUA_DIR} REQUIRED NO_DEFAULT_PATH)

# Engine
set(ENGINE_VERSION "0.0.18")
set(ENGINE_DIR "${CMAKE_SOURCE_DIR}/vendor/engine")
set(ENGINE_CLI "${ENGINE_DIR}/src/cli/main.lua")
set(ENGINE_DOWNLOAD "https://github.com/gamelly/gly-engine/archive/refs/tags/${ENGINE_VERSION}.tar.gz")
set(ENGINE_HEADER "${ENGINE_DIR}/bytecode.h")
set(ENGINE_LOVE_DIR1 "${ENGINE_DIR}/love")
set(ENGINE_LOVE_DIR2 "${CMAKE_SOURCE_DIR}/vendor/love")
set(ENGINE_LOVE_DOWNLOAD "https://github.com/gamelly/love-engine/archive/refs/tags/${ENGINE_VERSION}.tar.gz")
if(NOT EXISTS ${ENGINE_HEADER})
    FetchContent_Populate(dep_engine URL ${ENGINE_DOWNLOAD} SOURCE_DIR ${ENGINE_DIR})
    file(MAKE_DIRECTORY "${ENGINE_DIR}/lite")
    file(MAKE_DIRECTORY "${ENGINE_DIR}/love")
    file(MAKE_DIRECTORY "${ENGINE_DIR}/native")
    file(MAKE_DIRECTORY "${ENGINE_DIR}/samples/template")
    execute_process(
        WORKING_DIRECTORY ${ENGINE_DIR}
        COMMAND ${LUAC} ${ENGINE_CLI} build --core lite --bundler --dist ${ENGINE_DIR}/lite/
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        WORKING_DIRECTORY ${ENGINE_DIR}
        COMMAND ${LUAC} ${ENGINE_CLI} build --core native --bundler --dist ${ENGINE_DIR}/native/
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        WORKING_DIRECTORY ${ENGINE_DIR}
        COMMAND ${LUAC} ${ENGINE_CLI} tool-template-fill ${ENGINE_DIR}/samples/template/game.lua 204800
    )
    if (NOT DEFINED ENGINE)
        set(ENGINE "@lite")
    endif()
endif()
if(DEFINED ENGINE)
    if (ENGINE STREQUAL "@love" AND NOT EXISTS "${ENGINE_LOVE_DIR1}/main.lua")
        message(WARNING "currently LOVE2D is experimental.")
        FetchContent_Populate(dep_love URL ${ENGINE_LOVE_DOWNLOAD} SOURCE_DIR ${ENGINE_LOVE_DIR2})
        file(COPY "${ENGINE_LOVE_DIR2}/src/main.lua" DESTINATION "${ENGINE_LOVE_DIR1}")
    endif()
    if (ENGINE MATCHES "^\\@")
        string(SUBSTRING "${ENGINE}" 1 -1 ENGINE)
        set(ENGINE "${ENGINE_DIR}/${ENGINE}/main.lua")
    endif()
    if(NOT EXISTS ${ENGINE})
        message(FATAL_ERROR "engine not found: ${ENGINE}")
    endif()
    get_filename_component(ENGINE "${ENGINE}" ABSOLUTE)
    execute_process(
        WORKING_DIRECTORY ${ENGINE_DIR}
        COMMAND ${LUAC} ${ENGINE_CLI} fs-xxd-i ${ENGINE} ${ENGINE_HEADER} --const --name engine_bytecode_lua
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

# Game
string(LENGTH "${GAME}" GAME_LENGTH)
set(GAME_HEADER "${CMAKE_SOURCE_DIR}/vendor/game/bytecode.h")
if (DEFINED GAME AND GAME_LENGTH AND GAME MATCHES "^\\@")
    string(SUBSTRING "${GAME}" 1 -1 GAME)
    set(GAME "${ENGINE_DIR}/samples/${GAME}/game.lua")
endif()
if ((NOT DEFINED GAME AND NOT EXISTS ${GAME_HEADER}) OR (DEFINED GAME AND NOT GAME_LENGTH))
    file(WRITE ${GAME_HEADER} "unsigned char* game_bytecode_lua = (unsigned char*) 0u;\nunsigned int game_bytecode_lua_len = 0;")
endif()
if(DEFINED GAME AND GAME_LENGTH)
    if(NOT EXISTS ${GAME})
        message(FATAL_ERROR "game not found: ${GAME}")
    endif()
    get_filename_component(GAME "${GAME}" ABSOLUTE)
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/game")
    execute_process(
        WORKING_DIRECTORY ${ENGINE_DIR}
        COMMAND ${LUAC} src/cli/main.lua fs-xxd-i ${GAME} ${GAME_HEADER} --const --name game_bytecode_lua
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

# Lua CJson
set(LUACJSON_VERSION "2.1.0.9")
set(LUACJSON_DIR "${CMAKE_SOURCE_DIR}/vendor/lua_cjson")
set(LUACJSON_FILE "${LUACJSON_DIR}/lua_cjson.c")
set(LUACJSON_DOWNLOAD "https://github.com/openresty/lua-cjson/archive/refs/tags/${LUACJSON_VERSION}.tar.gz")
if(NOT EXISTS ${LUACJSON_FILE})
    FetchContent_Populate(dep_cjson_lua URL ${LUACJSON_DOWNLOAD} SOURCE_DIR ${LUACJSON_DIR})
    execute_process(
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/engine"
        COMMAND ${LUAC} ${ENGINE_CLI} fs-replace ${LUACJSON_FILE} ${LUACJSON_FILE}
            --format "<lua.h>" --replace "\"lua/lua.h\""
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/engine"
        COMMAND ${LUAC} ${ENGINE_CLI} fs-replace ${LUACJSON_FILE} ${LUACJSON_FILE}
            --format "<lauxlib.h>" --replace "\"lua/lauxlib.h\""
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()
add_library(lua_cjson-static STATIC "${LUACJSON_DIR}/lua_cjson.c;${LUACJSON_DIR}/strbuf.c;${LUACJSON_DIR}/fpconv.c")

# Bear SSL
set(BEARSSL_VERSION "0.6")
set(BEARSSL_DIR "${CMAKE_SOURCE_DIR}/vendor/bearssl")
set(USE_LIBIDN2 OFF CACHE BOOL "" FORCE)
set(BEARSSL_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/vendor/bearssl/inc")
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vendor/bearssl/build;${CMAKE_PREFIX_PATH}")
FetchContent_Declare(
    dep_bearssl
    GIT_REPOSITORY https://www.bearssl.org/git/BearSSL
    GIT_TAG "v${BEARSSL_VERSION}"
    SOURCE_DIR ${BEARSSL_DIR}
)
FetchContent_MakeAvailable(dep_bearssl)
if (NOT EXISTS "${BEARSSL_DIR}/build/libbearssl.a")
    execute_process(
        COMMAND ${CMAKE_MAKE_PROGRAM} build/libbearssl.a -j ${J}
        WORKING_DIRECTORY ${BEARSSL_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

# Curl
set(CURL_VERSION "8_9_1")
set(CURL_DIR "${CMAKE_SOURCE_DIR}/vendor/curl")
set(HTTP_ONLY ON CACHE BOOL "" FORCE)
set(PICKY_COMPILER OFF CACHE BOOL "" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_STATIC_CURL OFF CACHE BOOL "" FORCE)
set(ENABLE_ARES OFF CACHE BOOL "" FORCE)
set(CURL_DISABLE_TESTS ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(BUILD_LIBCURL_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_MISC_DOCS OFF CACHE BOOL "" FORCE)
set(ENABLE_CURL_MANUAL OFF CACHE BOOL "" FORCE)
set(CURL_USE_OPENSSL OFF CACHE BOOL "" FORCE)
set(CURL_USE_BEARSSL ON CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBSSH2 OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    dep_curl
    GIT_REPOSITORY https://github.com/curl/curl
    GIT_TAG curl-8_9_1
    SOURCE_DIR ${CURL_DIR}
)
FetchContent_MakeAvailable(dep_curl)

target_link_libraries(${PROJECT_NAME} CURL::libcurl)
target_link_libraries(${PROJECT_NAME} lua-static)
target_link_libraries(${PROJECT_NAME} lua_cjson-static)
target_link_libraries(${PROJECT_NAME} m)
