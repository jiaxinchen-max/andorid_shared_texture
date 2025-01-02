#include "server.h"

static bool isRunning = false;
static AHardwareBuffer *hwBuffer = nullptr;
static ServerRenderer *serverRenderer = nullptr;
static int socketFd = -1;
static int dataSocket = -1;
static AAssetManager *nativeasset= nullptr;

#define SOCKET_NAME     "shard_texture_socket"
extern "C"
JNIEXPORT void JNICALL
Java_com_example_render_RenderSurface_setServerNativeAssetManager(JNIEnv *env, jobject thiz,
                                                                  jobject asset_manager) {
    nativeasset = AAssetManager_fromJava(env, asset_manager);
}
void* ServerSetup(){
    int ret;
    struct sockaddr_un serverAddr;
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

    LOG_I("SetupClient Server complete.");

    while (1){
        // accept
        dataSocket = accept(socketFd, nullptr, nullptr);
        LOG_I("accept dataSocket: %d", dataSocket);
        if(dataSocket < 0){
            LOG_E("accept: %s", strerror(errno));
            break;
        }

        while (1){
            ret = AHardwareBuffer_recvHandleFromUnixSocket(dataSocket, &hwBuffer);
            if(ret != 0){
                LOG_E("recvHandleFromUnixSocket: %d", ret);
                break;
            }
            AHardwareBuffer_Desc desc;
            AHardwareBuffer_describe(hwBuffer, &desc);
            LOG_D("recvHandleFromUnixSocket: %d x %d, layer: %d, format: %d", desc.width, desc.height, desc.layers, desc.format);
        }
    }
    LOG_D("Close dataSocket");

    close(dataSocket);
    return nullptr;
}

void ServerHandler(int32_t cmd){
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {                         // ANativeWindow init
            LOG_D("    APP_CMD_INIT_WINDOW");
            if(socketFd < 0){
//                pthread_t serverThread;
//                pthread_create(&serverThread, nullptr, ServerSetup, nullptr);
            }

            if(dataSocket > 0){
                serverRenderer = ServerRenderer::GetInstance();
                serverRenderer->m_NativeAssetManager = nativeasset;
                serverRenderer->Init(hwBuffer);
                isRunning = true;
            }
            break;
        }
        case APP_CMD_TERM_WINDOW: {                         // ANativeWindow term
            LOG_D("    APP_CMD_TERM_WINDOW");
            isRunning = false;
//            if(serverRenderer){
//                serverRenderer->Destroy();
//            }
            break;
        }
        case APP_CMD_DESTROY: {
            LOG_D("    APP_CMD_DESTROY");
            if(socketFd > 0){
                close(socketFd);
            }
        }
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events.
 */
void ServerStart() {
    LOG_D( "----------------------------------------------------------------" );
    LOG_D( "    Server.Start" );

    for (;;) {
        if(isRunning && serverRenderer){
            serverRenderer->Draw();
            usleep(100);
        }
    }
}

