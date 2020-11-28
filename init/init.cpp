#include "../s-git.h"
#include "../cache/file.h"
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
	}else{

		for (auto path:{"objects","refs","refs\\head"}){
			fs::path dir = (gitDir/path);
			//std::cout << gitDir << std::endl;
			fs::create_directories(dir);
		}
		write_file(gitDir / "HEAD", (char *)"ref: refs/heads/main",strlen("ref: refs/heads/main"));
	}

	std::cout << "Initialized empty " << GIT_NAME << " repository in "
		<< gitDir << std::endl;
	return 0;

	// to do 在.s-git 文件夹下 创立 objects  
}
