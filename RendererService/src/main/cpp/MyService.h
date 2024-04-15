#pragma once

#include <aidl/com/example/BnMyService.h>

using ndk::ScopedAStatus;

namespace aidl {
namespace com {
namespace example {

class MyService : public BnMyService
{
public:
    ScopedAStatus sayHello(int32_t in_hi, std::string *_aidl_return) override;
    ScopedAStatus createSwapchain(int32_t in_imageCount) override;
    ScopedAStatus waitFrame(int64_t *_aidl_return) override;
    ScopedAStatus acquireImage(int32_t *_aidl_return) override;
    ScopedAStatus releaseImage(int32_t imageIndex) override;
    ScopedAStatus endFrame() override;
};

} // namespace example
} // namespace com
} // namespace aidl
