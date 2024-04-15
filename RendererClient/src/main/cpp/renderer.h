#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <android/asset_manager.h>
#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <android/hardware_buffer.h>

class ClientRenderer{
public:
    static ClientRenderer* GetInstance();

    void Init();
    void Destroy();
    void Draw();
    void SetImageData(uint32_t width, uint32_t height, uint8_t *buf);

    AAssetManager *m_NativeAssetManager;
private:
    ClientRenderer() = default;

    int InitEGLEnv();
    void DestroyEGLEnv();

    void CreateProgram();
    int ReadShader(const char *fileName, std::vector<char> &source) const;

    static ClientRenderer s_Renderer;

    int m_ViewportWidth = 1920;
    int m_ViewportHeight = 1080;
    EGLDisplay m_EglDisplay = EGL_NO_DISPLAY;
    EGLSurface m_EglSurface = EGL_NO_SURFACE;
    EGLContext m_EglContext = EGL_NO_CONTEXT;
    EGLConfig m_EglConfig;

    GLint m_VertexShader;
    GLint m_FragShader;
    GLuint m_Program;
};