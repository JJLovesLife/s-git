#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"
#include <unordered_set>
#include <iostream>
#include <string>
#include <vector>
#include <functional>

int status(int argc, const char* argv[]);

Command StatusCommand{
	status,
	"Show the working tree status"
};


namespace fs = std::filesystem;

struct index{
	std::string object_type;
	fs::path path;
	char sha1[20];
	int flag ;
	std::time_t  changetime;
	int size;
};


int
status(int argc, const char* argv[]) {
	cmdline::parser argParser;
	argParser.parse_check(argc, argv);
	if (!GIT_DIR.has_value()) {
		std::cout << red << "fatal: not a " << GIT_NAME << " repository(or any of the parent directories)" << Reset << std::endl;
		return 1;
	}

	std::set<fs::path> Pathes;

	Pathes = paths(ROOT_DIR.value());
	std::string commitSha1 = readMain();
	
	if (commitSha1.length()==0) {
		std::cout << "No commits yet  \n";
		std::cout << red << "Untracked files:\n";
		for (auto& path : Pathes) {
			std::cout << '\t' << fs::relative(path).generic_string() << '\n';
		}
		std::cout << Reset;
	}
	else {
		std::vector<object> entries; // objects in current commit
		std::unordered_set<fs::path, opt_path_hash> entries_path_set;
		std::unordered_map<fs::path, std::string, opt_path_hash>  entries_map;
		commit parentCommit;
		if (!readCommit(commitSha1, parentCommit)) {
			std::cout << "No commits yet  \n";
			std::cout << red << "Untracked files:\n";
			for (auto& path : Pathes) {
				std::cout << '\t' << fs::relative(path).generic_string() << '\n';
			}
			std::cout << Reset;
			return 0;
		}

		std::string branchName = readBranchName();
		if (branchName.length() != 0) {
			std::cout << "On branch " << readBranchName() << std::endl;
		}
		else {
			std::cout << "Head detached at " << commitSha1 << std::endl;
		}

		bool clean = true;

		std::string sha1 = parentCommit.tree;
		readTree(ROOT_DIR.value(), sha1, entries);
		for (auto& e : entries) {
			entries_path_set.insert(e.path);
			entries_map[e.path] = e.sha1;
		}

		std::cout << blod_green << "changed files: \n";
		for (auto &p:Pathes) {
			if (entries_path_set.count(p) != 0) {
				std::string sha1_1 = hash_blob_path(p, false);
				std::string sha1_2= entries_map.find(p)->second;
				if (sha1_1 != sha1_2) {
					std::cout << '\t' << fs::relative(p).generic_string() << '\n';
					clean = false;
				}
			}
		}
		std::cout << Reset;
		std::cout << green << "new files: \n";
		for (auto& p : Pathes){
			if (entries_path_set.count(p)==0) {
				std::cout << '\t' << fs::relative(p).generic_string() << '\n';
				clean = false;
			}
		}
		std::cout << Reset;
		std::cout << cyan << "delete files: \n";
		for (auto& e : entries) {
			if (e.object_type != "blob") continue;
			auto &p = e.path;
			if (Pathes.count(p) == 0) {
				std::cout << '\t' << fs::relative(p).generic_string() << '\n';
				clean = false;
			}
		}
		std::cout << Reset;

		if (clean) {
			std::cout << "nothing to commit, working tree clean\n";
		}
	}
	return 0;
}
