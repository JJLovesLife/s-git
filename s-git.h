#pragma once

#include <optional>
#include <filesystem>
#include "3rdParty/cmdline.h"

extern const char *GIT_NAME;
extern const char *GIT_DESC;
extern std::optional<std::filesystem::path> GIT_DIR;
extern std::optional<std::filesystem::path> ROOT_DIR;
extern std::filesystem::path CWD;


struct Command {
	int(*function)(int, const char*[]);
	const char* description;
};

extern Command InitCommand;
extern Command StatusCommand;
extern Command CommitCommand;
extern Command TagCommand;

extern Command TestCommand;
