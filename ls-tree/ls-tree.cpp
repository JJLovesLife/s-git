#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"
#include <fstream>

namespace fs = std::filesystem;

int lsTreeMain(int argc, const char* argv[]);
Command LsTreeCommand{
	lsTreeMain	,
	"List the contents of a tree object"
};

void coutTree(const std::string& sha1, fs::path dir) {
	if (!sha1Exist(sha1)) {
		std::cerr << red << "Error: " << GIT_NAME << " commit file doesn't exist" << Reset << std::endl;
		return;
	}
	fs::path treePath = sha1_to_path(sha1);
	std::ifstream file(treePath, std::ios::in | std::ios::binary);

	std::string type;
	std::getline(file, type, ' ');

	if (file.peek() < '0' || file.peek() > '9') {
		std::cerr << red << "Error: " << GIT_NAME << sha1 << " this file is corrupted, read tree failed" << Reset << std::endl;
		return;
	}
	size_t size;
	file >> size;

	char nullChar;
	file >> nullChar;
	if (nullChar != '\0') {
		std::cerr << red << "Error: " << GIT_NAME << sha1 << " this file is corrupted, read tree failed" << Reset << std::endl;
		return;
	}

	if (type == "blob") {
		std::cerr << red << "Error: want tree object, get blob object." << Reset << std::endl;
		return;
	}
	else {
		std::cout << green << "tree " << yellow << sha1 << Reset << ' ' << dir.generic_string() << std::endl;
		while (file.peek() != decltype(file)::traits_type::eof()) {
			object tmp;
			std::getline(file, tmp.object_type, ' ');
			std::getline(file, tmp.sha1, ' ');

			std::string tmpPath;
			std::getline(file, tmpPath);
			fs::path path = dir / tmpPath;

			if (tmp.object_type == "blob") {
				std::cout << "     " << blod_blue << "-blob " << yellow << tmp.sha1 << Reset << ' ' << path.generic_string() << std::endl;
			}
			else if (tmp.object_type == "tree") {
				coutTree(tmp.sha1, path);
			}
			else {
				std::cerr << red << "Error:" << GIT_NAME << " " << sha1 << " Tree object  raw data damaged" << Reset << std::endl;
			}
		}
	}
}

int lsTreeMain(int argc, const char* argv[]) {
	cmdline::parser argParser;
	argParser.add<std::string>("commit", 'c', "use this commit instead of the one HEAD is on", false);
	argParser.parse_check(argc, argv);

	std::string commitSha1;
	if (argParser.exist("commit")) {
		commitSha1 = argParser.get<std::string>("commit");
	}
	else {
		commitSha1 = readMain();
	}
	commit currCommit;
	if (!readCommit(commitSha1, currCommit)) {
		std::cerr << red << "Error: " << commitSha1 << " is not a commit object." << Reset << std::endl;
	}

	coutTree(currCommit.tree, fs::path("/"));

	return 0;
}
