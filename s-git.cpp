#include "s-git.h"
#include <map>
#include <string>
#include <iostream>

const char *GIT_NAME = "s-git";
const char *GIT_DESC = "s-git"
	" is a simple but not stupid version-control system works like git.";
std::optional<std::filesystem::path> GIT_DIR{};
std::optional<std::filesystem::path> ROOT_DIR{};
std::filesystem::path CWD{};

static std::map<std::string, Command> funcTable;

static int help(int, const char*[]);
static int version(int, const char*[]);

static void initFuncTable() {
	funcTable.emplace("help", Command{ help, "show this description." });
	funcTable.emplace("version", Command{ version, "version info." });

	funcTable.emplace("init", InitCommand);
	funcTable.emplace("status", StatusCommand);
	funcTable.emplace("commit", CommitCommand);
	funcTable.emplace("checkout", CheckoutCommand);
	funcTable.emplace("tag", TagCommand);
	funcTable.emplace("log", LogCommand);
	funcTable.emplace("branch", BranchCommand);
	funcTable.emplace("ls-tree", LsTreeCommand);
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

static void setupDir() {
	CWD = std::filesystem::current_path();

	auto root = CWD.root_path();
	auto path = CWD;
	while (path != root) {
		auto git = path / (std::string{ '.' } + GIT_NAME);
		if (std::filesystem::exists(git) && std::filesystem::is_directory(git)) {
			GIT_DIR = git;
			ROOT_DIR = path;
			return;
		}
		path = path.parent_path();
	}
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

		setupDir();
		return commandIt->second.function(argc - 1, argv + 1);
	} else {
		std::cout << GIT_NAME << ": '" << argv[1] << "' is not a "
			<< GIT_NAME << " command.\n";
		usage();
		return 1;
	}
}

int version(int, const char*[]) {
	std::cout << GIT_NAME << " versoin 0.6.1" << std::endl;
	return 0;
}

int help(int, const char*[]) {
	usage();
	return 0;
}
