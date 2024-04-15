package com.example.ndkbinderservice;

import android.content.res.AssetManager;

public class NativeEglRender {
    public native void native_OnInit();
    public native void native_OnDraw();
    public native void native_OnDestroy();
    public native void native_SetNativeAssetManager(AssetManager assetManager);
}