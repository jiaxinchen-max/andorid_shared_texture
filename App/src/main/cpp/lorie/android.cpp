#include "client.h"

extern "C" void initClient(){
    Handler(APP_CMD_INIT_WINDOW);
    pthread_t t;
    pthread_create(&t, NULL, reinterpret_cast<void *(*)(void *)>(Start), NULL);

}
extern "C" {
#include "server.h"
}
extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_render_RenderSurface_startServer(JNIEnv *env, jclass clazz, jobjectArray args) {
    return start(env, clazz, args);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_render_RenderSurface_windowChanged(JNIEnv *env, jobject thiz, jobject surface,
                                                    jstring jname) {
    lorieChangeWindow(NULL, surface ? env->NewGlobalRef( surface) : NULL);
}
extern "C" JNIEXPORT void JNICALL
Java_com_example_render_RenderSurface_startClient(JNIEnv *env, jobject thiz) {
    initClient();
}
