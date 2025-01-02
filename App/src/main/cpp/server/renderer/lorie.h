#pragma once
#include <android/hardware_buffer.h>
#include <jni.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef Bool
#define Bool int
#endif
void initOutput();
void lorieSetVM(JavaVM *vm);
Bool lorieChangeWindow(__unused void* pClient, void *closure);
Bool lorieChangeBuffer(AHardwareBuffer* buffer);