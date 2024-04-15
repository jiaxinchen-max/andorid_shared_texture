#include <jni.h>
#include "MyService.h"
#include "LogDefs.h"

#include <android/binder_ibinder_jni.h>

using aidl::com::example::MyService;
using namespace std;

extern "C" JNIEXPORT jobject JNICALL
Java_com_example_ndkbinderservice_MyService_createServiceBinder(
        JNIEnv* env,
        jobject /* this */)
{
    static MyService myService;
    return env->NewGlobalRef(AIBinder_toJavaBinder(env, myService.asBinder().get()));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderservice_NativeEglRender_native_1OnInit(JNIEnv *env, jobject thiz) {
    LOGD("%s", "native_OnInit");
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderservice_NativeEglRender_native_1OnDraw(JNIEnv *env, jobject thiz) {
    LOGD("%s", "native_OnDraw");
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderservice_NativeEglRender_native_1OnDestroy(JNIEnv *env, jobject thiz) {
    LOGD("%s", "native_OnDestroy");
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderservice_NativeEglRender_native_1SetNativeAssetManager(JNIEnv *env,
                                                                                jobject thiz,
                                                                                jobject asset_manager) {
    LOGD("%s", "native_SetNativeAssetManager");
}