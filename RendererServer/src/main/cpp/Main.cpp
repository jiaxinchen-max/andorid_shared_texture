#include <android_native_app_glue.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "LogUtil.h"

bool isRunning = false;
AHardwareBuffer *hwBuffer = nullptr;

#define SOCKET_NAME     "shard_texture_socket"

void cmd_handler(struct android_app *app, int32_t cmd){
    switch (cmd) {
        case APP_CMD_START: {
            LOG_D("    APP_CMD_START");
            break;
        }
        case APP_CMD_DESTROY: {
            LOG_D("    APP_CMD_DESTROY");
            break;
        }
        case APP_CMD_INIT_WINDOW: {                         // ANativeWindow init
            LOG_D("    APP_CMD_INIT_WINDOW");
            break;
        }
        case APP_CMD_TERM_WINDOW: {                         // ANativeWindow term
            LOG_D("    APP_CMD_TERM_WINDOW");
            break;
        }
    }
}

void* SetupServer(void* obj){
    int ret;
    struct sockaddr_un serverAddr;
    int socketFd;
    int dataSocket;
    char socketName[108];

    LOG_I("Start server setup");

    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(socketFd < 0){
        LOG_E("socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    LOG_D("socket made.");

    memcpy(&socketName[0], "\0", 1);
    strcpy(&socketName[1], SOCKET_NAME);

    memset(&serverAddr, 0, sizeof(struct sockaddr_un));
    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, socketName, sizeof(serverAddr.sun_path) - 1); // max is 108

    ret = bind(socketFd, reinterpret_cast<const sockaddr *>(&serverAddr), sizeof(struct sockaddr_un));
    if(ret < 0){
        LOG_E("bind: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    LOG_D("bind made");

    // open 8 back buffers for this demo
    ret = listen(socketFd, 8);
    if(ret < 0){
        LOG_E("listen: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    LOG_I("Setup Server complete.");

    // accept
    dataSocket = accept(socketFd, nullptr, nullptr);
    if(dataSocket < 0){
        LOG_E("accept: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    ret = AHardwareBuffer_recvHandleFromUnixSocket(dataSocket, &hwBuffer);
    if(ret != 0){
        LOG_E("recvHandleFromUnixSocket: %d", ret);
        exit(EXIT_FAILURE);
    }

    AHardwareBuffer_Desc desc;
    AHardwareBuffer_describe(hwBuffer, &desc);
    LOG_D("hwbuffer: %d x %d, format: %d", desc.width, desc.height, desc.format);

    LOG_D("Close socket");

    close(dataSocket);
    close(socketFd);
    return nullptr;
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events.
 */
void android_main(struct android_app *app) {
    LOG_D( "----------------------------------------------------------------" );
    LOG_D( "    android_main()" );

    app->onAppCmd = cmd_handler;

    int32_t result;
    android_poll_source* source;

    pthread_t serverThread;
    pthread_create(&serverThread, nullptr, SetupServer, nullptr);

    for (;;) {
        while((result = ALooper_pollAll(0, nullptr, nullptr, reinterpret_cast<void**>(&source))) >= 0){
            if(source != nullptr)
                source->process(app, source);

            if(app->destroyRequested){
                LOG_D("Terminating event loop...");
                return;
            }
        }
    }
}