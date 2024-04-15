#pragma once

#include "LogDefs.h"
#include <GLES3/gl3.h>

static GLint CreateGLShader(const char* source, GLenum type) {
    GLint glShader = glCreateShader(type);
    if (glShader == 0) {
        return 0;
    }
    glShaderSource(glShader, 1, &source, 0);
    glCompileShader(glShader);

    GLint status;
    glGetShaderiv(glShader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        GLint bufLength = 0;
        glGetShaderiv(glShader, GL_INFO_LOG_LENGTH, &bufLength);
        if (bufLength) {
            char info[bufLength];
            glGetShaderInfoLog(glShader, bufLength, NULL, info);
            LOGE("Shader Compile Error:\n%s", info);
        }
    }
    return glShader;
}

static GLint CreateGLProgram(GLint vertexShader, GLint fragmentShader){
    GLint program{0};
    if (!vertexShader || !fragmentShader) {
        return program;
    }

    program = glCreateProgram();
    if (!program) {
        return program;
    }
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == 0) {
        GLint bufLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
        if (bufLength) {
            char info[bufLength];
            glGetShaderInfoLog(program, bufLength, NULL, info);
            LOGE("Shader Link Error:\n%s", info);
        }
    }
    return program;
}
