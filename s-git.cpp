#include "s-git.h"
#include <map>
#include <string>
#include <iostream>

extern const char *GIT_NAME = "s-git";
extern const char *GIT_DESC = "s-git"
	" is a simple but not stupid version-control system works like git.";

static std::map<std::string, Command> funcTable;

static int test(int, const char*[]);
static int help(int, const char*[]);

static void initFuncTable() {
	funcTable.emplace("test", Command{ test, "test usage function." });
	funcTable.emplace("help", Command{ help, "show this description." });

	funcTable.emplace("init", InitCommand);
}

static void usage() {
	std::cout << GIT_DESC << '\n';
	std::cout << "usage: " << GIT_NAME << " <command>\n";
	std::cout << "These are commands available:\n\n";

	for (auto &func : funcTable) {
		std::cout << '\t' << func.first << ": " << func.second.description << '\n';
	}

	std::cout << "\nUse " << GIT_NAME << " <command> --help for command specific usage.\n";
	std::cout.flush();
}

int main(int argc, const char *argv[]) {
	initFuncTable();

	if (argc < 2) {
		usage();
		return 1;
	}

	auto commandIt = funcTable.find(argv[1]);
	if (commandIt != funcTable.end()) {
		// drop argv[0] and change argv[1]
		std::string exeName(GIT_NAME);
		exeName += ' ';
		exeName += argv[1];
		argv[1] = exeName.c_str();
		return commandIt->second.function(argc - 1, argv + 1);
	} else {
		usage();
		return 1;
	}
}

int test(int, const char*[]) {
	std::cout << "This is a test command." << std::endl;
	return 0;
}

int help(int, const char*[]) {
	usage();
	return 0;
}
