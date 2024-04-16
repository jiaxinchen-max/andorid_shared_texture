#include <android_native_app_glue.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "LogUtil.h"
#include "renderer/ServerRenderer.h"

bool isRunning = false;
AHardwareBuffer *hWbuffer = nullptr;

#define SOCKET_NAME "sharedServerSocket"

// Server to get socket data with information of SharedMem's file descriptor
void* setupServer(void* na) {
    int ret;
    struct sockaddr_un server_addr;
    int socket_fd;
    int data_socket;
    char socket_name[108]; // 108 sun_path length max

    LOG_I("Start server setup");

    // AF_UNIX for domain unix IPC and SOCK_STREAM since it works for the example
    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        LOG_E("socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    LOG_I("Socket made");

    memcpy(&socket_name[0], "\0", 1);
    strcpy(&socket_name[1], SOCKET_NAME);

    // clear for safty
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_name, sizeof(server_addr.sun_path) - 1); // 108 char max

    ret = bind(socket_fd, (const struct sockaddr *) &server_addr, sizeof(struct sockaddr_un));
    if (ret < 0) {
        LOG_E("bind: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    LOG_I("Bind made");

    // Open 8 back buffers for this demo
    ret = listen(socket_fd, 8);
    if (ret < 0) {
        LOG_E("listen: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    LOG_I("Socket listening for packages");

    // Wait for incoming connection.
    data_socket = accept(socket_fd, NULL, NULL);
    if (data_socket < 0) {
        LOG_E("accept: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    LOG_I("Accepted data");
    int count = 0;
    for (;;) {
        LOG_D("data_socket:");
        // Blocks until sent data
        ret = AHardwareBuffer_recvHandleFromUnixSocket(data_socket, &hWbuffer);
        AHardwareBuffer_Desc reportDesc;
        if(hWbuffer){
            AHardwareBuffer_describe(hWbuffer, &reportDesc);

            uint8_t buffer[4 * 4];

            void *hwPtr;
            AHardwareBuffer_lock(hWbuffer, AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN, -1, nullptr, (void**)&hwPtr);
            memcpy(buffer, hwPtr, sizeof(buffer));
            AHardwareBuffer_unlock(hWbuffer, nullptr);

            for (int i = 0; i < 4; i++) {
                LOG_E("Output: {%d,%d,%d,%d}\n", buffer[i * 4], buffer[i * 4 + 1], buffer[i * 4 + 2], buffer[i * 4 + 3]);
            }
        }
        LOG_D("data_socket: %d : hwbuffer : [%d x %d]", data_socket, hWbuffer ? reportDesc.width : 0, hWbuffer ? reportDesc.height : 0);

        usleep(10 * 1000);
        if (ret != 0) {
            LOG_E("Failed to AHardwareBuffer_recvHandleFromUnixSocket");
            if(count++ >= 1000000){
                break;
            }
            continue;
        }
        count = 0;
    }
    close(data_socket);
    close(socket_fd);

    return NULL;
}

void cmd_handler(struct android_app *app, int32_t cmd) {
    switch ( cmd ) {
        case APP_CMD_START: {
            LOG_D("onStart()");
            LOG_D("    APP_CMD_START");
            break;
        }
        case APP_CMD_RESUME: {
            LOG_D("onResume()");
            LOG_D("    APP_CMD_RESUME");
            break;
        }
        case APP_CMD_GAINED_FOCUS: {
            LOG_D("onGainedFocus()");
            LOG_D("    APP_CMD_GAINED_FOCUS");
            break;
        }
        case APP_CMD_PAUSE: {
            LOG_D("onPause()");
            LOG_D("    APP_CMD_PAUSE");
            break;
        }
        case APP_CMD_STOP: {
            LOG_D("onStop()");
            LOG_D("    APP_CMD_STOP");
            break;
        }
        case APP_CMD_DESTROY: {
            LOG_D("onDestroy()");
            LOG_D("    APP_CMD_DESTROY");
            ServerRenderer::GetInstance()->Destroy();
            break;
        }
        case APP_CMD_INIT_WINDOW: {
            LOG_D("surfaceCreated()");
            LOG_D("    APP_CMD_INIT_WINDOW");
            ServerRenderer::GetInstance()->m_GlobalApp = app;
            ServerRenderer::GetInstance()->m_NativeAssetManager = app->activity->assetManager;
            if(hWbuffer){
                ServerRenderer::GetInstance()->Init(hWbuffer);
                isRunning = true;
            }
            break;
        }
        case APP_CMD_TERM_WINDOW: {
            LOG_D("surfaceDestroyed()");
            LOG_D("    APP_CMD_TERM_WINDOW");
            isRunning = false;
            break;
        }
    }
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

    // Start server daemon on new thread
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, setupServer, (void*) nullptr);

    for (;;) {
        while((result = ALooper_pollAll(0, nullptr, nullptr, reinterpret_cast<void**>(&source))) >= 0){
            if(source != nullptr)
                source->process(app, source);

            if(app->destroyRequested){
                LOG_D("Terminating event loop...");
                return;
            }
        }
        if(isRunning){
            ServerRenderer::GetInstance()->Draw();
        }
    }
}