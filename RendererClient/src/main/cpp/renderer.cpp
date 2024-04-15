#include "renderer.h"
#include "ShaderUtil.h"

using aidl::com::example::IMyService;

ClientRenderer ClientRenderer::s_Renderer{};

ClientRenderer* ClientRenderer::GetInstance() {
    return &s_Renderer;
}

void ClientRenderer::Init(IMyService *ipcService) {
    if(!ipcService){
        return;
    }
    if(InitEGLEnv() != 0) return;
    m_IpcService = ipcService;
    CreateFramebuffers();
    CreateProgram();
}

void ClientRenderer::Destroy() {
    glDeleteShader(m_VertexShader);
    glDeleteShader(m_FragShader);
    glDeleteProgram(m_Program);
    DestroyEGLEnv();
}

void ClientRenderer::Draw() {
    glUseProgram(m_Program);
    {
        float x_scale = 0.8f;
        float y_scale = 0.8f;
        GLfloat vertices[] = {
                -1.f * x_scale, -1.f * y_scale,
                1.f * x_scale, -1.f * y_scale,
                -1.f * x_scale, 1.f * y_scale,
                1.f * x_scale, 1.f * y_scale,
        };
        GLfloat colors[] = {
                1.f, 0.f, 0.f, 1.f,
                0.f, 1.f, 0.f, 1.f,
                0.f, 0.f, 1.f, 1.f,
                1.f, 1.f, 1.f, 1.f
        };
        GLfloat texCoords[] = {
                0.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f
        };
        GLuint posLoc = glGetAttribLocation(m_Program, "a_position");
        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, vertices);

        GLuint colorLoc = glGetAttribLocation(m_Program, "a_color");
        glEnableVertexAttribArray(colorLoc);
        glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 0, colors);

        GLuint texcoordLoc = glGetAttribLocation(m_Program, "a_texcoord");
        glEnableVertexAttribArray(texcoordLoc);
        glVertexAttribPointer(texcoordLoc, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
    }

    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
    glScissor(0, 0, m_ViewportWidth, m_ViewportHeight);
    glClearColor(0.1f, 0.2f, 0.3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glUseProgram(0);
    glFinish();             // its important~
}

int ClientRenderer::InitEGLEnv() {
    const EGLint confAttr[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RED_SIZE,   8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE,  8,
            EGL_ALPHA_SIZE, EGL_DONT_CARE,
            EGL_DEPTH_SIZE, EGL_DONT_CARE,
            EGL_STENCIL_SIZE, EGL_DONT_CARE,
            EGL_NONE
    };

    // EGL context attributes
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };

    const EGLint surfaceAttr[] = {
            EGL_WIDTH, m_ViewportWidth,
            EGL_HEIGHT, m_ViewportHeight,
            EGL_NONE
    };
    EGLint eglMajVers, eglMinVers;
    EGLint numConfigs;
    int resultCode = 0;
    do {
        m_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if(m_EglDisplay == EGL_NO_DISPLAY) {
            //Unable to open connection to local windowing system
            LOGE("BgRender::CreateGlesEnv Unable to open connection to local windowing system");
            resultCode = -1;
            break;
        }

        if(!eglInitialize(m_EglDisplay, &eglMajVers, &eglMinVers)) {
            // Unable to initialize EGL. Handle and recover
            LOGE("BgRender::CreateGlesEnv Unable to initialize EGL");
            resultCode = -1;
            break;
        }
        LOGD("BgRender::CreateGlesEnv EGL init with version %d.%d", eglMajVers, eglMinVers);

        if(!eglChooseConfig(m_EglDisplay, confAttr, &m_EglConfig, 1, &numConfigs)) {
            LOGE("BgRender::CreateGlesEnv some config is wrong");
            resultCode = -1;
            break;
        }

        m_EglSurface = eglCreatePbufferSurface(m_EglDisplay, m_EglConfig, surfaceAttr);
        if(m_EglSurface == EGL_NO_SURFACE) {
            switch(eglGetError()) {
                case EGL_BAD_ALLOC:
                    // Not enough resources available. Handle and recover
                    LOGE("BgRender::CreateGlesEnv Not enough resources available");
                    break;
                case EGL_BAD_CONFIG:
                    // Verify that provided EGLConfig is valid
                    LOGE("BgRender::CreateGlesEnv provided EGLConfig is invalid");
                    break;
                case EGL_BAD_PARAMETER:
                    // Verify that the EGL_WIDTH and EGL_HEIGHT are
                    // non-negative values
                    LOGE("BgRender::CreateGlesEnv provided EGL_WIDTH and EGL_HEIGHT is invalid");
                    break;
                case EGL_BAD_MATCH:
                    // Check window and EGLConfig attributes to determine
                    // compatibility and pbuffer-texture parameters
                    LOGE("BgRender::CreateGlesEnv Check window and EGLConfig attributes");
                    break;
            }
        }

        m_EglContext = eglCreateContext(m_EglDisplay, m_EglConfig, EGL_NO_CONTEXT, ctxAttr);
        if(m_EglContext == EGL_NO_CONTEXT) {
            EGLint error = eglGetError();
            if(error == EGL_BAD_CONFIG) {
                // Handle error and recover
                LOGE("BgRender::CreateGlesEnv EGL_BAD_CONFIG");
                resultCode = -1;
                break;
            }
        }

        if(!eglMakeCurrent(m_EglDisplay, m_EglSurface, m_EglSurface, m_EglContext)) {
            LOGE("BgRender::CreateGlesEnv MakeCurrent failed");
            resultCode = -1;
            break;
        }
        LOGE("BgRender::CreateGlesEnv initialize success!");
    } while (false);

    if (resultCode != 0){
        LOGE("BgRender::CreateGlesEnv fail");
    }
    return resultCode;
}

void ClientRenderer::DestroyEGLEnv() {
    if (m_EglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(m_EglDisplay, m_EglContext);
        eglDestroySurface(m_EglDisplay, m_EglSurface);
        eglReleaseThread();
        eglTerminate(m_EglDisplay);
    }
    m_EglDisplay = EGL_NO_DISPLAY;
    m_EglSurface = EGL_NO_SURFACE;
    m_EglContext = EGL_NO_CONTEXT;
}

void ClientRenderer::CreateFramebuffers() {
    AHardwareBuffer *buffer = nullptr;
    AHardwareBuffer_Desc desc = {
            static_cast<uint32_t>(600),
            static_cast<uint32_t>(600),
            1,
            AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
            AHARDWAREBUFFER_USAGE_CPU_READ_NEVER | AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER |
            AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT,
            0,
            0,
            0};
    int errCode = AHardwareBuffer_allocate(&desc, &buffer);
    if(errCode != 0){
        LOGE("allocate hw buffer fail.");
        return;
    }
    m_IpcService->createSwapchain(IMAGE_COUNT);
}

void ClientRenderer::CreateProgram() {
    std::vector<char> vsSource, fsSource;
    if(ReadShader("shaders/lut.vert", vsSource) < 0){
        LOGE("read shader error.");
        return;
    }
    if(ReadShader("shaders/lut.frag", fsSource) < 0){
        LOGE("read shader error.");
        return;
    }
    m_VertexShader = CreateGLShader(std::string(vsSource.data(), vsSource.size()).c_str(), GL_VERTEX_SHADER);
    m_FragShader = CreateGLShader(std::string(fsSource.data(), fsSource.size()).c_str(), GL_FRAGMENT_SHADER);
    m_Program = CreateGLProgram(m_VertexShader, m_FragShader);
}

int ClientRenderer::ReadShader(const char *fileName, std::vector<char> &source) const {
    AAsset* file = AAssetManager_open(m_NativeAssetManager, fileName, AASSET_MODE_BUFFER);
    if(!file){
        LOGE("Can not open file: %s", fileName);
        return -1;
    }
    size_t fileLength = AAsset_getLength(file);
    source.resize(fileLength);
    int rt = AAsset_read(file, source.data(), fileLength);
    AAsset_close(file);
    return rt;
}