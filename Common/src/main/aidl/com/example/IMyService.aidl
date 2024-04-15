// IMyService.aidl
package com.example;

interface IMyService {
    String sayHello(int hi);
    void createSwapchain(int imageCount);
    long waitFrame();
    int acquireSwapchain();
    void releaseSwapchain();
    void endFrame();
}
