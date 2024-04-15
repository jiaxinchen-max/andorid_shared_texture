#include "MyService.h"
#include <LogDefs.h>

using aidl::com::example::MyService;

ScopedAStatus aidl::com::example::MyService::sayHello(int32_t in_hi, std::string *_aidl_return) {
    *_aidl_return = "Hello, I am MyService, I had receive your request!";
    LOGD("MyService : %s : %d, return : %s", __FUNCTION__, in_hi, _aidl_return->c_str());
    return ScopedAStatus::ok();
}

ScopedAStatus aidl::com::example::MyService::createSwapchain(int32_t in_imageCount) {
    LOGD("MyService : %s : %d", __FUNCTION__, in_imageCount);
    return ScopedAStatus::ok();
}

ScopedAStatus aidl::com::example::MyService::waitFrame(int64_t *_aidl_return) {
    *_aidl_return = 999999;
    LOGD("MyService : %s, return : %lld", __FUNCTION__, *_aidl_return);
    return ScopedAStatus::ok();
}

ScopedAStatus aidl::com::example::MyService::acquireImage(int32_t *_aidl_return) {
    *_aidl_return = 0;
    LOGD("MyService : %s, return : %d", __FUNCTION__, *_aidl_return);
    return ScopedAStatus::ok();
}

ScopedAStatus aidl::com::example::MyService::releaseImage(int32_t imageIndex) {
    LOGD("MyService : %s : %d", __FUNCTION__, imageIndex);
    return ScopedAStatus::ok();
}

ScopedAStatus aidl::com::example::MyService::endFrame() {
    LOGD("MyService : %s", __FUNCTION__);
    return ScopedAStatus::ok();
}
