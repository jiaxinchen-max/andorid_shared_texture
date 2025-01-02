#include <jni.h>
#include "renderer.h"
#include "lorie.h"

typedef struct {

    Bool cursorMoved;
    int timerFd;

    struct {
        AHardwareBuffer* buffer;
        Bool locked;
        Bool legacyDrawing;
        uint8_t flip;
        uint32_t width, height;
    } root;

    JavaVM* vm;
    JNIEnv* env;
    Bool dri3;
} lorieScreenInfo, *lorieScreenInfoPtr;

static lorieScreenInfo lorieScreen = { .root.width = 1280, .root.height = 1024, .dri3 = TRUE };
static lorieScreenInfoPtr pvfb = &lorieScreen;
static char *xstartup = NULL;

static jobject currentSurface=NULL;

static Bool TrueNoop() { return TRUE; }
static Bool FalseNoop() { return FALSE; }
static void VoidNoop() {}



void initOutput() {
    renderer_init(pvfb->env, &pvfb->root.legacyDrawing, &pvfb->root.flip);
}
void lorieSetVM(JavaVM* vm) {
    pvfb->vm = vm;
    (*vm)->AttachCurrentThread(vm, &pvfb->env, NULL);
}
Bool lorieChangeWindow(__unused void* pClient, void *closure) {
    jobject surface = (jobject) closure;
    currentSurface = surface;
    renderer_set_window(pvfb->env, surface, pvfb->root.buffer);
//    lorieSetCursor(NULL, NULL, CursorForDevice(GetMaster(lorieMouse, MASTER_POINTER)), -1, -1);

//    if (pvfb->root.legacyDrawing) {
//        renderer_update_root(pScreenPtr->width, pScreenPtr->height, ((PixmapPtr) pScreenPtr->devPrivate)->devPrivate.ptr, pvfb->root.flip);
//        renderer_redraw(pvfb->env, pvfb->root.flip);
//    }

    return TRUE;
}
Bool lorieChangeBuffer(AHardwareBuffer* buffer){
    renderer_set_window(pvfb->env, currentSurface, buffer);
}