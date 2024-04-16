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

// Can be anything if using abstract namespace
#define SOCKET_NAME "sharedServerSocket"
static int data_socket;

bool SetupClient() {
    char socket_name[108]; // 108 sun_path length max
    static struct sockaddr_un server_addr;

    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (data_socket < 0) {
        LOG_E("socket: %s", strerror(errno));
        return false;
    }

    memcpy(&socket_name[0], "\0", 1);
    strcpy(&socket_name[1], SOCKET_NAME);

    // clear for safty
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX; // Unix Domain instead of AF_INET IP domain
    strncpy(server_addr.sun_path, socket_name, sizeof(server_addr.sun_path) - 1); // 108 char max

    // Assuming only one init connection for demo
    int ret = connect(data_socket, (const struct sockaddr *) &server_addr, sizeof(struct sockaddr_un));
    if (ret < 0) {
        LOG_E("connect: %s", strerror(errno));
        return false;
    }
    LOG_I("Client Setup Complete");
    return true;
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
            break;
        }
        case APP_CMD_INIT_WINDOW: {
            LOG_D("surfaceCreated()");
            LOG_D("    APP_CMD_INIT_WINDOW");
            ServerRenderer::GetInstance()->m_GlobalApp = app;
            ServerRenderer::GetInstance()->m_NativeAssetManager = app->activity->assetManager;
            ServerRenderer::GetInstance()->Init(data_socket);
            isRunning = true;
            break;
        }
        case APP_CMD_TERM_WINDOW: {
            LOG_D("surfaceDestroyed()");
            LOG_D("    APP_CMD_TERM_WINDOW");
//            isRunning = false;
//            ServerRenderer::GetInstance()->Destroy();
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

    SetupClient();

    for (;;) {
        while((result = ALooper_pollAll(isRunning ? 0 : -1, nullptr, nullptr, reinterpret_cast<void**>(&source))) >= 0){
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