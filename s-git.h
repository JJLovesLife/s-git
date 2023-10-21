#pragma once

#include <optional>
#include <filesystem>
#include "3rdParty/cmdline.h"


static constexpr const char* red("\033[0;31m");
static constexpr const char* blod_red("\033[1;31m");

static constexpr const char* green("\033[0;32m");
static constexpr const char* blod_green("\033[1;32m");


static constexpr const char* yellow("\033[0;33m");
static constexpr const char* blod_yellow("\033[1;33m");

static constexpr const char* blue("\033[0;34m");
static constexpr const char* blod_blue("\033[1;34m");


static constexpr const char* cyan("\033[0;36m");
static constexpr const char* bold_cyan("\033[1;36m");

static constexpr const char* white("\033[1;37m");
static constexpr const char* Reset("\033[0m");

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
extern Command CheckoutCommand;
extern Command LogCommand;
extern Command BranchCommand;
extern Command LsTreeCommand;
extern Command MergeCommand;
