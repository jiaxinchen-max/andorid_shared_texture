#include "server.h"

#define log(prio, ...) __android_log_print(ANDROID_LOG_ ## prio, "LorieNative", __VA_ARGS__)

static int argc = 0;
static char** argv = NULL;
static int conn_fd = -1;

static bool isRunning = false;
static AHardwareBuffer *hwBuffer = NULL;
static int socketFd = -1;
static int dataSocket = -1;

#define SOCKET_NAME     "shard_texture_socket"
Bool start(JNIEnv *env, jclass clazz, jobjectArray args){
    pthread_t t;
    JavaVM* vm = NULL;
    // execv's argv array is a bit incompatible with Java's String[], so we do some converting here...
    argc = (*env)->GetArrayLength(env, args) + 1; // Leading executable path
    argv = (char**) calloc(argc, sizeof(char*));

    argv[0] = (char*) "Xlorie";
    for(int i=1; i<argc; i++) {
        jstring js = (jstring)((*env)->GetObjectArrayElement(env, args, i - 1));
        const char *pjc = (*env)->GetStringUTFChars(env, js, JNI_FALSE);
        argv[i] = (char *) calloc(strlen(pjc) + 1, sizeof(char)); //Extra char for the terminating NULL
        strcpy((char *) argv[i], pjc);
        (*env)->ReleaseStringUTFChars(env, js, pjc);
    }

    {
        cpu_set_t mask;
        long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

        for (int i = num_cpus/2; i < num_cpus; i++)
            CPU_SET(i, &mask);

        if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) == -1)
            log(ERROR, "Failed to set process affinity: %s", strerror(errno));
    }

    (*env)->GetJavaVM(env, &vm);
    lorieSetVM(vm);
    initOutput();
    pthread_create(&t, NULL, startServer, vm);
    return JNI_TRUE;
}

void* SetupServer(void* obj){
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

    ret = bind(socketFd, &serverAddr, sizeof(struct sockaddr_un));
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

    while (1){
        // accept
        dataSocket = accept(socketFd, NULL, NULL);
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
    return NULL;
}

static void* startServer(__unused void* cookie) {
    SetupServer(NULL);
}


static jclass FindClassOrDie(JNIEnv *env, const char* name) {
    jclass clazz = (*env)->FindClass(env, name);
    if (!clazz) {
        char buffer[1024] = {0};
        sprintf(buffer, "class %s not found", name);
        log(ERROR, "%s", buffer);
        (*env)->FatalError(env, buffer);
        return NULL;
    }

    return (*env)->NewGlobalRef(env, clazz);
}

static jclass FindMethodOrDie(JNIEnv *env, jclass clazz, const char* name, const char* signature, jboolean isStatic) {
    __typeof__((*env)->GetMethodID) getMethodID = isStatic ? (*env)->GetStaticMethodID : (*env)->GetMethodID;
    jmethodID method = getMethodID(env, clazz, name, signature);
    if (!method) {
        char buffer[1024] = {0};
        sprintf(buffer, "method %s %s not found", name, signature);
        log(ERROR, "%s", buffer);
        (*env)->FatalError(env, buffer);
        return NULL;
    }

    return method;
}