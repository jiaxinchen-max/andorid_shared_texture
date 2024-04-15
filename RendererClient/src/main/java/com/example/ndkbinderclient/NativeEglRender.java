package com.example.ndkbinderclient;
import android.content.res.AssetManager;
import android.os.IBinder;

public class NativeEglRender {
    public native void onServiceConnected(IBinder binder);
    public native void onServiceDisconnected();
    public native String talkToService();

    public native void native_OnInit();
    public native void native_OnDraw();
    public native void native_OnDestroy();
    public native void native_SetNativeAssetManager(AssetManager assetManager);
}