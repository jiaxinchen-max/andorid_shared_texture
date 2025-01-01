#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "LogUtil.h"
#include "renderer/ClientRenderer.h"

#ifndef APP_CMD_INIT_WINDOW
#define APP_CMD_INIT_WINDOW 1
#endif

#ifndef APP_CMD_TERM_WINDOW
#define APP_CMD_TERM_WINDOW 2
#endif

#ifndef APP_CMD_DESTROY
#define APP_CMD_DESTROY 3
#endif


void Setup();

void Handler(int32_t cmd);

void Start();