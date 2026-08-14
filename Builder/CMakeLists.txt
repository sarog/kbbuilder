cmake_minimum_required(VERSION 3.21)

set(APP "BuildKB")
project(${APP} VERSION 1.0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

#include(FetchContent)
#FetchContent_Declare(
#        trace
#        GIT_REPOSITORY https://github.com/GPMueller/trace
#        GIT_TAG        b9fea96ccaee2f55f6fca7be8b05be7b204ecbe5 # <HASH or TAG>
#)
#FetchContent_MakeAvailable(trace)

#FetchContent_Declare(
#        cpptrace
#        GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
#        GIT_TAG        v1.0.4 # <HASH or TAG>
#)
#FetchContent_MakeAvailable(cpptrace)

#set(CMAKE_CXX_COMPILER_ID "Clang")

add_compile_options(-D_DEBUG) # -DUSEPACKAGES

if (BCC32)
    add_compile_options(-y -Q -k -r- -c -w-par -vi- -C8 -Od -D_DEBUG -H -n.\\Win32\\Debug\\Obj -H=.\\BuilderPCH1.h)
elseif (BCC32C)
    add_compile_options(-D_DEBUG -fpack-struct=1 -fshort-enums)
elseif (BCC32X)
    add_compile_options(-D_DEBUG -O0 -fpack-struct=1 -fshort-enums -g -fno-inline)
elseif (BCC64 OR BCC64X)
    # -DUNICODE -D_UNICODE
    add_compile_options(-D_DEBUG -O0 -fshort-enums -g -fno-inline)
endif ()

# set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} -d_DEBUG")

file(GLOB SOURCES
     Main.cpp
        "*.h"
        "*.cpp"
)

__embarcadero_language(CXX)
set_embt_target(RTL VCL DynamicRuntime) # Unicode

# add_link_options(-tV -tW -lr -lGl -lGpr -laa -lTpe) #-lL.\\Win32\\Debug\\Obj
add_link_options(-g) #-lL.\\Win32\\Debug\\Obj

add_executable(${APP} WIN32 ${SOURCES})

target_precompile_headers(${APP} PUBLIC BuilderPCH1.h)

target_link_directories(${APP} PRIVATE
        "${CMAKE_SOURCE_DIR}"
)

set(EMBT_BPI_DIR "${ROOTDIR}/lib/win32/release")

target_link_libraries(${APP} PRIVATE
        "${EMBT_BPI_DIR}/vcl.bpi"
        "${EMBT_BPI_DIR}/rtl.bpi"
        "${EMBT_BPI_DIR}/bcbsmp.bpi"
        "${EMBT_BPI_DIR}/vclx.bpi"
        "${EMBT_BPI_DIR}/vclimg.bpi"
        # cpptrace:cpptrace
)

# include_directories("${CMAKE_BINARY_DIR}/_deps/trace-src/include/")
# include_directories("${cpptrace_BINARY_DIR}/include/")

# Needed for shared library builds on windows:  copy cpptrace.dll to the same directory as the
# executable for your_target
#[[if(WIN32)
    add_custom_command(
            TARGET ${APP} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:cpptrace::cpptrace>"
            "$<TARGET_FILE_DIR:${APP}/cpptrace.dll>"
    )
endif()]]

install(TARGETS ${APP}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
