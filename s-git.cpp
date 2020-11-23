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
}

static void usage() {
	std::cout << GIT_DESC << '\n';
	std::cout << "usage: " << GIT_NAME << " <command>\n";
	std::cout << "These are commands available:\n\n";

	for (auto &func : funcTable) {
		std::cout << '\t' << func.first << ": " << func.second.description << '\n';
	}

	std::cout << "\nUse " << GIT_NAME << " <command> --help for more information.\n";
	std::cout.flush();
}

int main(int argc, const char *argv[]) {
	initFuncTable();

	if (argc >= 2 && funcTable.count(argv[1]) != 0) {
		return funcTable[argv[1]].function(argc, argv);
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
