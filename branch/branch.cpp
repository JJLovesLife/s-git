#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"

namespace fs = std::filesystem;

int branchMain(int argc, const char* argv[]);
Command BranchCommand{
	branchMain	,
	"List, create, or delete branches"
};


int branchMain(int argc, const char* argv[]) {
	cmdline::parser argParser;
	argParser.add("delete", 'd', "delete branch");
	argParser.add("list", 'l', "list branch names");
	argParser.footer("<branchname> ...");
	argParser.parse_check(argc, argv);

	if (!GIT_DIR.has_value()) {
		std::cout << red << "fatal: not a " << GIT_NAME << " repository(or any of the parent directories)" << Reset << std::endl;
		return 1;
	}

	fs::path branchDir = GIT_DIR.value();
	branchDir /= "refs";
	branchDir /= "heads";

	if (!fs::exists(branchDir)) {
		if (!fs::create_directories(branchDir)) {
			std::cerr << red  << "Error: unable to create branch directory" << Reset << std::endl;
			return 1;
		}
	}

	if (argParser.exist("list")) {
		for (auto &it : fs::directory_iterator(branchDir)) {
			std::cout << it.path().filename().generic_string() << std::endl;
		}
		return 0;
	}


	if (argParser.rest().size() == 0) {
		std::cout << argParser.usage();
		return 0;
	}

	if (argParser.exist("delete")) {
		std::string currBranch = readBranchName();

		bool error = false;
		for (auto branchName : argParser.rest()) {
			if (branchName == currBranch) {
				std::cout << "On branch " << currBranch << " now, unable to delete." << std::endl;
				continue;
			}

			fs::path branchPath = branchDir / branchName;
			if (fs::is_regular_file(branchPath)) {
				if (fs::remove(branchPath) != 1) {
					error = true;
				}
				else {
					std::cout << "Deleted branch '" << branchName << '\'' << std::endl;
				}
			}
		}
		return error ? 1 : 0;
	}
	else {
		std::string sha1 = readMain();
		commit dummy;
		if (!checkSha1(sha1) || !readCommit(sha1, dummy)) {
			std::cerr << red << "Error: failed to resolve 'HEAD' as a valid commit" << Reset << std::endl;
			return 1;
		}

		bool error = false;
		for (auto branchName : argParser.rest()) {
			fs::path branchPath = branchDir / branchName;
			if (fs::exists(branchPath)) {
				std::cerr << red << "Error: branch '" << branchName << "' already exists" << Reset << std::endl;
				error = true;
			}
			else {
				write_file(branchPath, sha1.data(), sha1.length());
				std::cout << "Added branch '" << blod_blue << branchName << Reset << '\'' << std::endl;
			}
		}
		return error ? 1 : 0;
	}

	return 0;
}
