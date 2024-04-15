// IMyService.aidl
package com.example;

//import android.hardware.HardwareBuffer;

interface IMyService {
    String sayHello(int hi);
    void createSwapchain(int imageCount);
    long waitFrame();
    int acquireImage();
    void releaseImage(int imageIndex);
    void endFrame();
}
