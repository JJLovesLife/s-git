#include "../s-git.h"

int initMain(int argc, const char* argv[]);
extern Command InitCommand{
	initMain,
	"init a new empty s-git repository"
};

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int initMain(int argc, const char* argv[]) {
	cmdline::parser argParser;
	argParser.parse_check(argc, argv);
	fs::path gitDir(std::string{ '.' } + GIT_NAME);

	if (fs::exists(gitDir)) {
		std::cerr << "Error: " << GIT_NAME << " repository already exists" << std::endl;
		return 1;
	}

	if (!fs::create_directories(gitDir)) {
		std::cerr << "Error: failed to create " << gitDir << std::endl;
		return 1;
	}

	std::cout << "Initialized empty " << GIT_NAME << " repository in "
		<< gitDir << std::endl;
	return 0;
}
