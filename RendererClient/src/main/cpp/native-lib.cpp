#include <jni.h>
#include <aidl/com/example/IMyService.h>
#include <android/binder_ibinder_jni.h>
#include <android/asset_manager_jni.h>
#include <LogDefs.h>

#include "renderer.h"

using aidl::com::example::IMyService;
using ndk::ScopedAStatus;
using namespace std;

std::shared_ptr<IMyService> g_spMyService;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderclient_NativeEglRender_onServiceConnected(JNIEnv *env, jobject thiz, jobject binder) {
    AIBinder* pBinder = AIBinder_fromJavaBinder(env, binder);

    const ::ndk::SpAIBinder spBinder(pBinder);
    g_spMyService = IMyService::fromBinder(spBinder);

    LOGD("[App] [cpp] onServiceConnected");
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderclient_NativeEglRender_onServiceDisconnected(JNIEnv *env, jobject thiz) {
    g_spMyService = nullptr;

    LOGD("[App] [cpp] onServiceDisconnected");
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_ndkbinderclient_NativeEglRender_talkToService(JNIEnv *env, jobject thiz) {
    std::string resp;
    ScopedAStatus basicTypesResult = g_spMyService->sayHello(1, &resp);

    if(basicTypesResult.isOk())
    {
        LOGD("[App] [cpp] IMyService.basicTypes - Succeeded");
    }
    else
    {
        LOGE("[App] [cpp] IMyService.basicTypes - Failed");
    }

    LOGD("[App] [cpp] IMyService return: %s", resp.c_str());

    return env->NewStringUTF(resp.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderclient_NativeEglRender_native_1OnInit(JNIEnv *env, jobject thiz) {
    LOGD("native_OnInit...");
    ClientRenderer::GetInstance()->Init(g_spMyService.get());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderclient_NativeEglRender_native_1OnDraw(JNIEnv *env, jobject thiz) {
    LOGD("native_OnDraw...");
    ClientRenderer::GetInstance()->Draw();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderclient_NativeEglRender_native_1OnDestroy(JNIEnv *env, jobject thiz) {
    LOGD("native_OnDestroy...");
    ClientRenderer::GetInstance()->Destroy();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderclient_NativeEglRender_native_1SetNativeAssetManager(JNIEnv *env, jobject thiz, jobject asset_manager) {
    ClientRenderer::GetInstance()->m_NativeAssetManager = AAssetManager_fromJava(env, asset_manager);
}
