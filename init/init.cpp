#include "../s-git.h"
#include "../cache/file.h"

#include <iostream>

namespace fs = std::filesystem;

int initMain(int argc, const char* argv[]);
Command InitCommand{
	initMain,
	"init a new empty s-git repository"
};

int initMain(int argc, const char* argv[]) {
	cmdline::parser argParser;
	argParser.parse_check(argc, argv);

	if (GIT_DIR.has_value() && ROOT_DIR.value() == CWD) {
		std::cerr << "Error: " << GIT_NAME << " repository already exists" << std::endl;
		return 1;
	}

	fs::path gitDir(std::string{ '.' } + GIT_NAME);

	if (!fs::create_directories(gitDir)) {
		std::cerr << "Error: failed to create " << gitDir << std::endl;
		return 1;
	}
	else {
		for (auto path : { "objects","refs","refs/heads" }) {
			fs::path dir = (gitDir / path);
			//std::cout << gitDir << std::endl;
			fs::create_directories(dir);
		}
		const char *HeadContent = "ref: refs/heads/main";
		write_file(gitDir / "HEAD", HeadContent, std::char_traits<char>::length(HeadContent));
	}

	std::cout << "Initialized empty " << GIT_NAME << " repository in "
		<< gitDir << std::endl;
	return 0;
}
