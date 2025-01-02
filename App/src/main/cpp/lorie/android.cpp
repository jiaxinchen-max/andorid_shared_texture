#include "client.h"
#include "server.h"
extern "C" {
//#include "server.h"
}
extern "C" void initClient(){
    ClientHandler(APP_CMD_INIT_WINDOW);
    pthread_t t;
    pthread_create(&t, NULL, reinterpret_cast<void *(*)(void *)>(ClientStart), NULL);
}
extern "C" void initServer(){
    pthread_t t1;
    pthread_create(&t1, NULL, reinterpret_cast<void *(*)(void *)>(ServerSetup), NULL);
    ServerHandler(APP_CMD_INIT_WINDOW);
    pthread_t t2;
    pthread_create(&t2, NULL, reinterpret_cast<void *(*)(void *)>(ServerStart), NULL);
}
extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_render_RenderSurface_startServer(JNIEnv *env, jclass clazz, jobjectArray args) {
//    return start(env, clazz, args);
    initServer();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_render_RenderSurface_windowChanged(JNIEnv *env, jobject thiz, jobject surface,
                                                    jstring jname) {
//    lorieChangeWindow(NULL, surface ? env->NewGlobalRef( surface) : NULL);
}
extern "C" JNIEXPORT void JNICALL
Java_com_example_render_RenderSurface_startClient(JNIEnv *env, jobject thiz) {
    initClient();
}
