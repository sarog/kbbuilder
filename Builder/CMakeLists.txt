cmake_minimum_required(VERSION 3.21)

set(APP "Builder")
project(${APP} VERSION 1.0.0.0 LANGUAGES CXX)

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
)

install(TARGETS ${APP}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
