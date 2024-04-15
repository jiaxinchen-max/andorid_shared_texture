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

#include <aidl/com/example/IMyService.h>

#define IMAGE_COUNT 3

class ClientRenderer{
public:
    static ClientRenderer* GetInstance();

    void Init(aidl::com::example::IMyService *ipcService);
    void Destroy();
    void Draw();

    AAssetManager *m_NativeAssetManager;
private:
    ClientRenderer() = default;

    int InitEGLEnv();
    void DestroyEGLEnv();

    void CreateFramebuffers();
    void CreateProgram();
    int ReadShader(const char *fileName, std::vector<char> &source) const;

    static ClientRenderer s_Renderer;

    aidl::com::example::IMyService *m_IpcService;

    int m_ViewportWidth = 1920;
    int m_ViewportHeight = 1080;
    EGLDisplay m_EglDisplay = EGL_NO_DISPLAY;
    EGLSurface m_EglSurface = EGL_NO_SURFACE;
    EGLContext m_EglContext = EGL_NO_CONTEXT;
    EGLConfig m_EglConfig;

    GLuint m_Framebuffer[IMAGE_COUNT];
    GLuint m_EGLImage[IMAGE_COUNT];

    GLint m_VertexShader;
    GLint m_FragShader;
    GLuint m_Program;
};