cmake_minimum_required(VERSION 3.4.1)

set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library( app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)

include_directories(inc)
add_library(ClientIPC
        SHARED
        Main.cpp
        renderer/ClientRenderer.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_subdirectory(glm)
target_include_directories(ClientIPC PUBLIC glm)

target_link_libraries(ClientIPC
                        app-glue
                        log
                        EGL
                        GLESv3
                        android
                        glm
)