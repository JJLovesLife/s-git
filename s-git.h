#pragma once

#include "3rdParty/cmdline.h"

extern const char *GIT_NAME;
extern const char *GIT_DESC;

struct Command {
	int(*function)(int, const char*[]);
	const char* description;
};

extern Command InitCommand;
