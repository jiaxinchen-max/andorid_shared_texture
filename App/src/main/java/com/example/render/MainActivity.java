package com.example.render;

import android.os.Bundle;
import android.view.SurfaceHolder;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    private RenderSurface sv;
    private SurfaceHolder sh;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //得到控制器
        sv=findViewById(R.id.render_surface);
        sh = sv.getHolder();
        sh.addCallback(new Render(sv,sh));
    }
}