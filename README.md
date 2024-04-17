# andorid_shared_texture

## 前言

图片流交换是非常常见的需求，为了保证程序的健壮性，可以将这些步骤分为各个进程分别处理，防止中间步骤错误导致主进程crash从而降低用户体验。比如人脸识别、场景识别、直播等场景，这些图片数据可能要经过很多个步骤流转，在各步骤中执行格式转换、图层叠加、图片畸变、颜色矫正等，比如直播中常见的特效和相机图片叠加。

该工程是Android平台中，使用Socket+HardwareBuffer来做的一个案例。案例分为RendererClient客户端和RendererServer服务端，客户端和服务的渲染都是用OpenGLES。

## 编译和运行

- 设备需要支持Socket，并且拥有权限；
- 需要Android 8.0（API 级别 26）及以上版本；

1. 安装两个APK。

```shell
 adb install -t -f RendererServer-debug.apk
 adb install -t -f RendererClient-debug.apk
```

2. 先运行`RendererServer`。

```shell
adb shell am start -n com.example.RendererServer/android.app.NativeActivity
```

这个时候，服务端因为没开始渲染，应该是一个黑屏状态。

3. 再运行`RendererClient`

```shell
adb shell am start -n com.example.RendererClient/android.app.NativeActivity
```

4. 切换到RendererServer界面。

![结果](https://foruda.gitee.com/images/1713325929435843472/f80eeb5b_1717396.png)