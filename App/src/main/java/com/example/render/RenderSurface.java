package com.example.render;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class RenderSurface extends SurfaceView {
    public RenderSurface(Context context) {
        super(context);
    }

    public RenderSurface(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public RenderSurface(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public RenderSurface(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    private SurfaceHolder.Callback callback;

    public native boolean startServer(String[] args);

    public native void windowChanged(Surface surface, String name);
    public native void startClient();
    public native void setNativeAssetManager(AssetManager assetManager);
}

class Render implements SurfaceHolder.Callback {
    private SurfaceHolder sh;
    private RenderSurface sv;

    public Render(RenderSurface sv, SurfaceHolder sh) {
        super();
        this.sv = sv;
        this.sh = sh;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // 启动线程
//        MyThread t = new MyThread(sh, sv);
//        new Thread(t).start();
//        this.sv.windowChanged(holder.getSurface(),"screen");
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width,
                               int height) {
        this.sv.windowChanged(holder.getSurface(),"screen");
        this.sv.startClient();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }
}

class MyThread implements Runnable {
    private SurfaceHolder sh;
    private SurfaceView sv;
    private Canvas c;
    private Paint p;
    private Bitmap image;
    // 小球的初始坐标和移动速度以及半径
    private int x = 70, y = 70, vx = 30, vy = 30, r = 70;

    public MyThread(SurfaceHolder sh, SurfaceView sv) {
        super();
        this.sh = sh;
        this.sv = sv;
    }

    public void run() {
        while (true) {
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            // 得到画布
            c = sh.lockCanvas(new Rect(0, 0, sv.getWidth(), sv.getHeight()));
            // 设置背景颜色
            c.drawColor(Color.WHITE);
            p = new Paint();
            // 先画白球
            p.setColor(Color.WHITE);
            c.drawCircle(x, y, r, p);
            // 变化坐标
            x += vx;
            y += vy;
            // 画黑球
            p.setColor(Color.YELLOW);
            c.drawCircle(x, y, r, p);
            // 解锁画布，更新提交屏幕显示内容
            sh.unlockCanvasAndPost(c);
            if (x == 70 || x + r >= sv.getWidth()) {
                vx = -vx;
            }
            if (y == 70 || y + r >= sv.getHeight()) {
                vy = -vy;
            }
//            Log.d("run","vx:"+vx+",vy:"+vy);
        }
    }
}
