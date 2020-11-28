#include "s-git.h"
#include <map>
#include <string>
#include <iostream>

#include "object/object.h"
/*temp*/
#include <filesystem>

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
	funcTable.emplace("status", StatusCommand);
	funcTable.emplace("commit", CommitCommand);
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
	std::vector<object> path;
	readTree("9ea16e1d6cbc67d41da68b7686afd8be150ae7be", path);

	/*char* buffer;
	int size = readFile(fs::path(".\\ss\\a.txt"), buffer);
	hash_object("blob", buffer, size);*/
	/*
	*/
	/*std::string sha1 = "08c8644401fef5323d1071a15e3c101bea50a657";
	std::vector<object> path;
	readTree(sha1, path);*/
	/*char buf[20];
	for (int i = 0; i <= 19; ++i) {
		buf[i]= i + 'a';
	}
	std::string data(buf+5,5);
	std::cout << data;*/
	/*char* buf = new char[20];
	buf[0] = '\0'; 

	for (int i = 1; i <= 19; ++i) {
		buf[i] = i + 'a';
	}
	buf[5] = '\0';
	buf[10] = '\0';
	hash_object("blob", buf , 20);
	/**/

	/*namespace fs = std::filesystem;
	fs::path p1("C:\\temp");
	p1 /= "user";
	p1 /= "data";
	std::cout << p1 << "\n";
	std::cout << "This is a test command." << std::endl;
	return 0;*/
	return 0;
}

int help(int, const char*[]) {
	usage();
	return 0;
}
