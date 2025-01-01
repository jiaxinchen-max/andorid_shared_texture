cmake_minimum_required(VERSION 3.4.1)

set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
#add_library( app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)

include_directories(inc)

add_library(Server
        SHARED
#        Main.cpp
        renderer/ServerRenderer.cpp
        renderer/renderer.c
        renderer/initOutput.c
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
target_link_libraries(Server
#        app-glue
        log
        EGL
        GLESv3
        android
)