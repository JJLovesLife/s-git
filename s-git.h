#pragma once

extern const char *GIT_NAME;
extern const char *GIT_DESC;

struct Command {
	int(*function)(int, const char*[]);
	const char* description;
};
