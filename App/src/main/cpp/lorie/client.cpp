#include "client.h"

static bool isRunning = false;
ClientRenderer *clientRenderer = nullptr;
static AHardwareBuffer *hwBuffer = nullptr;
static int dataSocket = -1;
static AAssetManager *nativeasset= nullptr;

#define SOCKET_NAME     "shard_texture_socket"
extern "C"
JNIEXPORT void JNICALL
Java_com_example_render_RenderSurface_setNativeAssetManager(JNIEnv *env, jobject thiz,
                                                            jobject asset_manager) {
     nativeasset = AAssetManager_fromJava(env, asset_manager);
}

void Setup() {
    char socketName[108];
    struct sockaddr_un serverAddr;

    dataSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (dataSocket < 0) {
        LOG_E("socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memcpy(&socketName[0], "\0", 1);
    strcpy(&socketName[1], SOCKET_NAME);

    memset(&serverAddr, 0, sizeof(struct sockaddr_un));
    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, socketName, sizeof(serverAddr.sun_path) - 1);

    // connect
    int ret = connect(dataSocket, reinterpret_cast<const sockaddr *>(&serverAddr),
                      sizeof(struct sockaddr_un));
    if (ret < 0) {
        LOG_E("connect: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    LOG_I("Client Setup complete.");
}

void Handler(int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {                         // ANativeWindow init
            LOG_D("    APP_CMD_INIT_WINDOW");
            if (dataSocket < 0) {
                Setup();
                AHardwareBuffer_Desc hwDesc;
                hwDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
                hwDesc.width = 1024;
                hwDesc.height = 1024;
                hwDesc.layers = 1;
                hwDesc.rfu0 = 0;
                hwDesc.rfu1 = 0;
                hwDesc.stride = 0;
                hwDesc.usage =
                        AHARDWAREBUFFER_USAGE_CPU_READ_NEVER | AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER
                        | AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT |
                        AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
                int rtCode = AHardwareBuffer_allocate(&hwDesc, &hwBuffer);
                if (rtCode != 0 || !hwBuffer) {
                    LOG_E("Failed to allocate hardware buffer.");
                    exit(EXIT_FAILURE);
                }
            }
            clientRenderer = ClientRenderer::GetInstance();
            clientRenderer->m_NativeAssetManager = nativeasset;
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

            if (dataSocket > 0) {
                AHardwareBuffer_release(hwBuffer);
                close(dataSocket);
            }
            break;
        }
    }
}

void Start() {
    LOG_D("----------------------------------------------------------------");
    LOG_D("    Start()");
    for (;;) {
        if (isRunning && clientRenderer) {
            clientRenderer->Draw();
        }
        usleep(100);
    }
}