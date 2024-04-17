#include "ServerRenderer.h"
#include <android_native_app_glue.h>
#include <thread>

#include "LogUtil.h"
#include "ShaderUtil.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

static const char *egl_error_str(EGLint ret){
    switch (ret) {
        case EGL_SUCCESS: return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
        case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
        case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
        case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
        case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
        case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
        case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
        case EGL_CONTEXT_LOST: return "EGL_CONTEXT_LOST";
        default: return "EGL_<UNKNOWN>";
    }
}
#define GL_CHECK(...) if(glGetError() != 0) LOG_E("CHECK_GL_ERROR %s glGetError = %s[%d], line = %d, ",  __FUNCTION__, egl_error_str(glGetError()), glGetError(), __LINE__)

ServerRenderer ServerRenderer::s_Renderer{};

ServerRenderer* ServerRenderer::GetInstance() {
    return &s_Renderer;
}

void ServerRenderer::Init(AHardwareBuffer *hwBuffer) {
    DEBUG_LOG();

    if(InitEGLEnv() != 0) return;
    CreateProgram();

    glGenTextures(1, &m_InputTexture);
    if(hwBuffer && m_NativeBufferImage == nullptr){
        EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID(hwBuffer);
        EGLint eglImageAttributes[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE };
        m_NativeBufferImage = eglCreateImageKHR(m_EglDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, eglImageAttributes);
        glBindTexture(GL_TEXTURE_2D, m_InputTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_NativeBufferImage);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

//    size_t len = 0;
//    uint8_t *imageData = ReaderImage("textures/awesomeface.png", &len);
//    if(imageData){
//        int width, height, channel;
//        uint8_t *chs = stbi_load_from_memory(imageData, len, &width, &height, &channel, STBI_rgb_alpha);
//        glBindTexture(GL_TEXTURE_2D, m_InputTexture);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, chs);
//        glBindTexture(GL_TEXTURE_2D, 0);
//        stbi_image_free(chs);
//    }
}

void ServerRenderer::Destroy() {
    DEBUG_LOG();
    glDeleteTextures(1, &m_InputTexture);
    eglDestroyImageKHR(m_EglDisplay, m_NativeBufferImage);
    glDeleteShader(m_VertexShader);
    glDeleteShader(m_FragShader);
    glDeleteProgram(m_Program);
    DestroyEGLEnv();
    LOG_E("Destroy success.");
}

void ServerRenderer::Draw() {
//    BEGIN_TIME(__FUNCTION__);
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

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(glGetUniformLocation(m_Program, "tex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_InputTexture);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //GL_CHECK();
    //glFinish();             // its important!
    eglSwapBuffers(m_EglDisplay, m_EglSurface);
    glUseProgram(0);
//    END_TIME(__FUNCTION__)
}

int ServerRenderer::InitEGLEnv() {
    const EGLint confAttr[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
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

    EGLint eglMajVers, eglMinVers;
    EGLint numConfigs;
    int resultCode = 0;
    do {
        m_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if(m_EglDisplay == EGL_NO_DISPLAY) {
            //Unable to open connection to local windowing system
            LOG_E("BgRender::CreateGlesEnv Unable to open connection to local windowing system");
            resultCode = -1;
            break;
        }

        if(!eglInitialize(m_EglDisplay, &eglMajVers, &eglMinVers)) {
            // Unable to initialize EGL. Handle and recover
            LOG_E("BgRender::CreateGlesEnv Unable to initialize EGL");
            resultCode = -1;
            break;
        }
        LOG_I("BgRender::CreateGlesEnv EGL init with version %d.%d", eglMajVers, eglMinVers);

        if(!eglChooseConfig(m_EglDisplay, confAttr, &m_EglConfig, 1, &numConfigs)) {
            LOG_E("BgRender::CreateGlesEnv some config is wrong");
            resultCode = -1;
            break;
        }

        m_EglSurface = eglCreateWindowSurface(m_EglDisplay, m_EglConfig, m_GlobalApp->window, nullptr);
        if(m_EglSurface == EGL_NO_SURFACE) {
            switch(eglGetError()) {
                case EGL_BAD_ALLOC:
                    // Not enough resources available. Handle and recover
                    LOG_E("BgRender::CreateGlesEnv Not enough resources available");
                    break;
                case EGL_BAD_CONFIG:
                    // Verify that provided EGLConfig is valid
                    LOG_E("BgRender::CreateGlesEnv provided EGLConfig is invalid");
                    break;
                case EGL_BAD_PARAMETER:
                    // Verify that the EGL_WIDTH and EGL_HEIGHT are
                    // non-negative values
                    LOG_E("BgRender::CreateGlesEnv provided EGL_WIDTH and EGL_HEIGHT is invalid");
                    break;
                case EGL_BAD_MATCH:
                    // Check window and EGLConfig attributes to determine
                    // compatibility and pbuffer-texture parameters
                    LOG_E("BgRender::CreateGlesEnv Check window and EGLConfig attributes");
                    break;
            }
        }

        m_EglContext = eglCreateContext(m_EglDisplay, m_EglConfig, EGL_NO_CONTEXT, ctxAttr);
        if(m_EglContext == EGL_NO_CONTEXT) {
            EGLint error = eglGetError();
            if(error == EGL_BAD_CONFIG) {
                // Handle error and recover
                LOG_E("BgRender::CreateGlesEnv EGL_BAD_CONFIG");
                resultCode = -1;
                break;
            }
        }

        if(!eglMakeCurrent(m_EglDisplay, m_EglSurface, m_EglSurface, m_EglContext)) {
            LOG_E("BgRender::CreateGlesEnv MakeCurrent failed");
            resultCode = -1;
            break;
        }
        LOG_E("BgRender::CreateGlesEnv initialize success!");
    } while (false);

    if (resultCode != 0){
        LOG_E("BgRender::CreateGlesEnv fail");
    }
    return resultCode;
}

void ServerRenderer::DestroyEGLEnv() {
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

void ServerRenderer::CreateProgram() {
    std::vector<char> vsSource, fsSource;
    if(ReadShader("shaders/server.vert", vsSource) < 0){
        LOG_E("read shader error.");
        return;
    }
    if(ReadShader("shaders/server.frag", fsSource) < 0){
        LOG_E("read shader error.");
        return;
    }
    m_VertexShader = CreateGLShader(std::string(vsSource.data(), vsSource.size()).data(), GL_VERTEX_SHADER);
    m_FragShader = CreateGLShader(std::string(fsSource.data(), fsSource.size()).data(), GL_FRAGMENT_SHADER);
    m_Program = CreateGLProgram(m_VertexShader, m_FragShader);
}

int ServerRenderer::ReadShader(const char *fileName, std::vector<char> &source) const {
    AAsset* file = AAssetManager_open(m_NativeAssetManager, fileName, AASSET_MODE_BUFFER);
    if(!file){
        LOG_E("Can not open file: %s", fileName);
        return -1;
    }
    size_t fileLength = AAsset_getLength(file);
    source.resize(fileLength);
    int rt = AAsset_read(file, source.data(), fileLength);
    AAsset_close(file);
    return rt;
}

uint8_t* ServerRenderer::ReaderImage(const char *fileName, size_t *outFileLength){
    AAsset* file = AAssetManager_open(m_NativeAssetManager, fileName, AASSET_MODE_BUFFER);
    if(!file){
        LOG_E("Can not open file: %s", fileName);
        return nullptr;
    }
    long length = AAsset_getLength(file);
    uint8_t *data = static_cast<uint8_t *>(malloc(length));
    AAsset_read(file, data, length);
    AAsset_close(file);

    *outFileLength = length;
    return data;
}