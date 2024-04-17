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

    void Init(AHardwareBuffer *hwBuffer, int dataSocket);
    void Destroy();
    void Draw();

    AAssetManager *m_NativeAssetManager;
    struct android_app *m_GlobalApp;
private:
    ClientRenderer() = default;

    int InitEGLEnv();
    void DestroyEGLEnv();

    void CreateProgram();
    int ReadShader(const char *fileName, std::vector<char> &source) const;

    static ClientRenderer s_Renderer;

    EGLDisplay m_EglDisplay = EGL_NO_DISPLAY;
    EGLSurface m_EglSurface = EGL_NO_SURFACE;
    EGLContext m_EglContext = EGL_NO_CONTEXT;
    EGLConfig m_EglConfig;

    GLint m_VertexShader;
    GLint m_FragShader;
    GLuint m_Program;
    uint32_t m_ImgWidth = 1024;
    uint32_t m_ImgHeight = 1024;

    GLuint m_OutputFBO;
    GLuint m_OutputTexture;
    EGLImageKHR m_NativeBufferImage;
};