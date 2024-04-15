#include <jni.h>
#include <aidl/com/example/IMyService.h>
#include <android/binder_ibinder_jni.h>
#include <android/asset_manager_jni.h>
#include <LogDefs.h>

#include "renderer.h"

using aidl::com::example::IMyService;
using aidl::com::example::ComplexType;
using ndk::ScopedAStatus;
using namespace std;

std::shared_ptr<IMyService> g_spMyService;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderclient_NativeEglRender_onServiceConnected(JNIEnv *env, jobject thiz,
                                                                    jobject binder) {
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
    ScopedAStatus basicTypesResult = g_spMyService->basicTypes(2021, 65535000,
                                                               true, 3.14f, 3.141592653589793238, "Hello, World!");

    if(basicTypesResult.isOk())
    {
        LOGD("[App] [cpp] IMyService.basicTypes - Succeeded");
    }
    else
    {
        LOGE("[App] [cpp] IMyService.basicTypes - Failed");
    }

    ComplexType ct(2021, 65535000, true, 3.14f,3.141592653589793238,
                   "Hello, World!");

    std::string sReturnedString;

    ScopedAStatus complexTypeResult = g_spMyService->complexType(ct, &sReturnedString);

    if(complexTypeResult.isOk())
    {
        LOGD("[App] [cpp] IMyService.complexType - Succeeded");
    }
    else
    {
        LOGE("[App] [cpp] IMyService.complexType - Failed");
    }

    ComplexType returnedComplexObject;

    ScopedAStatus returnComplexTypeResult = g_spMyService->returnComplexType(2021, 65535000, true, 3.14f, 3.141592653589793238,
                                                                             "Hello, World!", &returnedComplexObject);

    if(returnComplexTypeResult.isOk())
    {
        LOGD("[App] [cpp] IMyService.complexType - Succeeded");
    }
    else
    {
        LOGE("[App] [cpp] IMyService.complexType - Failed");
    }

    std::string sRet;
    returnedComplexObject.toString(&sRet);

    return env->NewStringUTF(sRet.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndkbinderclient_NativeEglRender_native_1OnInit(JNIEnv *env, jobject thiz) {
    LOGD("native_OnInit...");
    ClientRenderer::GetInstance()->Init();
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
