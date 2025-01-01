//
// Created by Jiaxin Chen on 2024/12/31.
//

#ifndef ANDORID_SHARED_TEXTURE_RENDER_H
#define ANDORID_SHARED_TEXTURE_RENDER_H

#include <sys/cdefs.h>
#include <jni.h>
#include <android/hardware_buffer.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

class Render {
public:
    __unused int RendererInit(JNIEnv *env, int *legacy_drawing, uint8_t *flip);

    __unused void RendererSetBuffer(JNIEnv *env, AHardwareBuffer *buffer);

    __unused void RendererSetWindow(JNIEnv *env, jobject surface, AHardwareBuffer *buffer);

private:
    __unused void RendererUnsetBuffer(void);

    __unused int RendererShouldRedraw(void);

    __unused int RendererRedraw(JNIEnv *env, uint8_t flip);

    __unused void RendererPrintFps(float millis);

    __unused void RendererUpdateRoot(int w, int h, void *data, uint8_t flip);

    __unused void RendererUpdateCursor(int w, int h, int xhot, int yhot, void *data);

    __unused void RendererSetCursorCoordinates(int x, int y);

    __unused GLuint CreateProgram(const char *p_vertex_source, const char *p_fragment_source);

    __unused GLuint LoadShader(GLenum shaderType, const char *pSource);

    __unused void Draw(GLuint id, float x0, float y0, float x1, float y1, uint8_t flip);

    __unused void DrawCursor(void);

private:
    EGLDisplay egl_display = EGL_NO_DISPLAY;
    EGLContext ctx = EGL_NO_CONTEXT;
    EGLSurface sfc = EGL_NO_SURFACE;
    EGLConfig cfg = 0;
    EGLNativeWindowType win = 0;
    jobject surface = NULL;
    AHardwareBuffer *buffer = NULL;
    EGLImageKHR image = NULL;
    int renderedFrames = 0;

    jmethodID Surface_release = NULL;
    jmethodID Surface_destroy = NULL;

    struct Display {
        GLuint id;
        float width, height;
    } display;
    struct Cursor {
        GLuint id;
        float x, y, width, height, xhot, yhot;
    } cursor;

    GLuint g_texture_program = 0, gv_pos = 0, gv_coords = 0;
    GLuint g_texture_program_bgra = 0, gv_pos_bgra = 0, gv_coords_bgra = 0;
};

#define AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM 5 // Stands to HAL_PIXEL_FORMAT_BGRA_8888
#endif //ANDORID_SHARED_TEXTURE_RENDER_H
