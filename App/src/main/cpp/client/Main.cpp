#include <android_native_app_glue.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "LogUtil.h"
#include "renderer/ClientRenderer.h"

bool isRunning = false;
ClientRenderer *clientRenderer = nullptr;
AHardwareBuffer *hwBuffer = nullptr;
int dataSocket = -1;

#define SOCKET_NAME     "shard_texture_socket"

void SetupClient(){
    char socketName[108];
    struct sockaddr_un serverAddr;

    dataSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(dataSocket < 0){
        LOG_E("socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memcpy(&socketName[0], "\0", 1);
    strcpy(&socketName[1], SOCKET_NAME);

    memset(&serverAddr, 0, sizeof(struct sockaddr_un));
    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, socketName, sizeof(serverAddr.sun_path) - 1);

    // connect
    int ret = connect(dataSocket, reinterpret_cast<const sockaddr *>(&serverAddr), sizeof(struct sockaddr_un));
    if(ret < 0){
        LOG_E("connect: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    LOG_I("Client ClientSetup complete.");
}

void cmd_handler(struct android_app *app, int32_t cmd){
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {                         // ANativeWindow init
            LOG_D("    APP_CMD_INIT_WINDOW");
            if(dataSocket < 0){
                SetupClient();
                AHardwareBuffer_Desc hwDesc;
                hwDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
                hwDesc.width = 1024;
                hwDesc.height = 1024;
                hwDesc.layers = 1;
                hwDesc.rfu0 = 0;
                hwDesc.rfu1 = 0;
                hwDesc.stride = 0;
                hwDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER | AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER
                               | AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
                int rtCode = AHardwareBuffer_allocate(&hwDesc, &hwBuffer);
                if(rtCode != 0 || !hwBuffer){
                    LOG_E("Failed to allocate hardware buffer.");
                    exit(EXIT_FAILURE);
                }
            }
            clientRenderer = ClientRenderer::GetInstance();
            clientRenderer->m_GlobalApp = app;
            clientRenderer->m_NativeAssetManager = app->activity->assetManager;
            clientRenderer->Init(hwBuffer, dataSocket);

            isRunning = true;
            break;
        }
        case APP_CMD_TERM_WINDOW: {                         // ANativeWindow term
            LOG_D("    APP_CMD_TERM_WINDOW");
            break;
        }
        case APP_CMD_DESTROY: {
            LOG_D("    APP_CMD_DESTROY");

            isRunning = false;
            clientRenderer->Destroy();

            if(dataSocket > 0){
                AHardwareBuffer_release(hwBuffer);
                close(dataSocket);
            }
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

    for (;;) {
        while((result = ALooper_pollAll(isRunning ? 0 : -1, nullptr, nullptr, reinterpret_cast<void**>(&source))) >= 0){
            if(source != nullptr)
                source->process(app, source);

            if(app->destroyRequested){
                LOG_D("Terminating event loop...");
                return;
            }
        }

        if(isRunning && clientRenderer){
            clientRenderer->Draw();
        }
    }
}