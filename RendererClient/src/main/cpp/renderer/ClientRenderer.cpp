#include "ClientRenderer.h"
#include <android_native_app_glue.h>
#include <thread>

#include "LogUtil.h"
#include "ShaderUtil.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/type_ptr.hpp"

//#define RENDER_TO_SCREEN

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

ClientRenderer ClientRenderer::s_Renderer{};

ClientRenderer* ClientRenderer::GetInstance() {
    return &s_Renderer;
}

void ClientRenderer::Init(AHardwareBuffer *hwBuffer, int dataSocket) {
    DEBUG_LOG();
    if(InitEGLEnv() != 0) return;
    CreateProgram();

    glGenFramebuffers(1, &m_OutputFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_OutputFBO);
    glGenTextures(1, &m_OutputTexture);
    glBindTexture(GL_TEXTURE_2D, m_OutputTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_ImgWidth, m_ImgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_OutputTexture, 0);
    uint32_t fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(fboStatus != GL_FRAMEBUFFER_COMPLETE){
        LOG_E("func %s, line %d, OpenGL renderFramebuffer NOT COMPLETE: %d", __func__, __LINE__, fboStatus);
    }

    EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID(hwBuffer);
    EGLint eglImageAttributes[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE };
    m_NativeBufferImage = eglCreateImageKHR(m_EglDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, eglImageAttributes);
    glBindTexture(GL_TEXTURE_2D, m_OutputTexture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_NativeBufferImage);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    AHardwareBuffer_sendHandleToUnixSocket(hwBuffer, dataSocket);
}

void ClientRenderer::Destroy() {
    DEBUG_LOG();
    glDeleteTextures(1, &m_OutputTexture);
    glDeleteFramebuffers(1, &m_OutputFBO);
    eglDestroyImageKHR(m_EglDisplay, m_NativeBufferImage);
    glDeleteShader(m_VertexShader);
    glDeleteShader(m_FragShader);
    glDeleteProgram(m_Program);
    DestroyEGLEnv();
    LOG_E("Destroy success.");
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

    glClearColor(0.1f, 0.2f, 0.3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, m_ImgHeight, m_ImgHeight);
    glScissor(0, 0, m_ImgWidth, m_ImgHeight);

#ifndef RENDER_TO_SCREEN
    glBindFramebuffer(GL_FRAMEBUFFER, m_OutputFBO);
    glViewport(0, 0, m_ImgHeight, m_ImgHeight);
    glScissor(0, 0, m_ImgWidth, m_ImgHeight);
#endif
    glClearColor(0.8f, 0.2f, 0.3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    float timeSec = getTimeSec();
    glm::mat4 modeMat = glm::rotate(glm::mat4(1.f), sinf(timeSec), glm::vec3(0, 0, 1));
    glUniformMatrix4fv(glGetUniformLocation(m_Program, "modelMat"), 1, GL_FALSE, glm::value_ptr(modeMat));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#ifndef RENDER_TO_SCREEN
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

    glUseProgram(0);
    eglSwapBuffers(m_EglDisplay, m_EglSurface);
}

int ClientRenderer::InitEGLEnv() {
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

void ClientRenderer::CreateProgram() {
    std::vector<char> vsSource, fsSource;
    if(ReadShader("shaders/client.vert", vsSource) < 0){
        LOG_E("read shader error.");
        return;
    }
    if(ReadShader("shaders/client.frag", fsSource) < 0){
        LOG_E("read shader error.");
        return;
    }
    m_VertexShader = CreateGLShader(std::string(vsSource.data(), vsSource.size()).data(), GL_VERTEX_SHADER);
    m_FragShader = CreateGLShader(std::string(fsSource.data(), fsSource.size()).data(), GL_FRAGMENT_SHADER);
    m_Program = CreateGLProgram(m_VertexShader, m_FragShader);
}

int ClientRenderer::ReadShader(const char *fileName, std::vector<char> &source) const {
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