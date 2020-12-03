#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"
#include <iostream>
#include <unordered_map>

namespace fs = std::filesystem;

int commitMain(int argc, const char* argv[]);
Command CommitCommand{
	commitMain,
	"Record changes to the repository"
};


std::string writeCommit(std::string treeSha1, std::string parent, std::string message) {
	std::stringstream buffer;
	buffer << treeSha1 << ' ' << parent << ' ' << message;

	auto str = buffer.str();
	std::string sha1 = hash_object("commit", str, true);

	return sha1;
}

object
fileIndex(fs::path path) {
	object thisBlob;
	thisBlob.object_type = "blob";
	thisBlob.path = path;
	thisBlob.sha1 = hash_blob_path(path, true);

	return thisBlob;
}

static bool cmp(const object& a, const object& b) {
	return a.sha1 < b.sha1;
}

std::string write_tree(std::vector<object> index) {
	std::stringstream buffer;
	for (auto &v : index) {
		buffer << v.object_type << ' ' << v.sha1 << ' ' << v.path.filename().u8string() << '\n';
	}

	auto str = buffer.str();
	std::string sha1 = hash_object("tree", str, true);

	return sha1;
}

std::optional<object>
build_tree(fs::path path) {
	fs::directory_entry entry(path);
	if (entry.status().type() != fs::file_type::directory) {
		std::cerr << "Error: " << GIT_NAME << " is not a directory" << std::endl;
		return object{};
	}

	fs::path ignore = path / ".s-gitignore";
	if (fs::exists(ignore) && fs::file_size(ignore) == 0) {
		// ignore this directory if there exists file ".s-gitignore" with empty content
		return {};
	}

	fs::directory_iterator list(path);
	std::vector<object> v;
	for (auto& it : list) {
		if (it.path() == GIT_DIR.value()) {
			continue;
		}

		if (fs::is_directory(it.status())) {
			auto obj = build_tree(it);
			if (obj.has_value()) {
				v.push_back(obj.value());
			}
		}
		else if (fs::is_regular_file(it)) {
			v.push_back(fileIndex(it.path()));
		}
		else {
			std::cerr << red  <<"Warning: " << fs::relative(it.path()) << " is neither a directory or a regular file"<<Reset << std::endl;
		}
	}
	
	if (v.size() == 0) {
		return {};
	}

	object thisTree;
	
	sort(v.begin(), v.end(), cmp);
	thisTree.object_type = "tree";
	thisTree.sha1 = write_tree(v);
	thisTree.path = path;
	return thisTree;
}


int commitMain(int argc, const char* argv[]) {
	cmdline::parser argParser;
	argParser.add<std::string>("message", 'm', "commit message", true, "");
	argParser.parse_check(argc, argv);
	if (!GIT_DIR.has_value()) {
		std::cout << red << "fatal: not a " << GIT_NAME << " repository(or any of the parent directories)" << Reset << std::endl;
		return 1;
	}

	std::string  message = argParser.get<std::string>("message");


	std::set<fs::path> Pathes;
	Pathes = paths(ROOT_DIR.value());

	std::string commitSha1 = readMain();
	if (commitSha1.length()==0) {
		std::vector<object> index;
		auto treeOpt = build_tree(ROOT_DIR.value());  // file tree of s-git root directory

		std::string newcommitSha1 = writeCommit(treeOpt.has_value() ? treeOpt.value().sha1 : "NULL" ,  "NULL", message);
		writeMain(newcommitSha1);

		std::cout << "first commit  \n";
		std::cout << blod_yellow <<"[main (root-commit) " << newcommitSha1.substr(0, 7) << "]" <<Reset << "\n";

		std::cout << Pathes.size() << " file inserted(+)\n";
		for (auto& path : Pathes) {
			std::cout << " " << fs::relative(path, ROOT_DIR.value()).generic_string() << '\n';
		}
	}
	else {
		std::vector<object> entries; // objects in current commit
		commit parentCommit;
		if (!readCommit(commitSha1, parentCommit)) {
			auto treeOpt = build_tree(ROOT_DIR.value()); // file tree of s-git root directory

			std::string newcommitSha1 = writeCommit(treeOpt.has_value() ? treeOpt.value().sha1 : "NULL", "NULL", message);
			writeMain(newcommitSha1);

			std::cout << "first commit  \n";
			std::cout << blod_yellow << "[main (root-commit) " << newcommitSha1.substr(0, 7) << "]" << Reset << "\n";

			std::cout << Pathes.size() << " file inserted(+)\n";
			for (auto& path : Pathes) {
				std::cout << " " << fs::relative(path, ROOT_DIR.value()).generic_string() << '\n';
			}
			return 0;
		}

		std::string branchName = readBranchName();
		if (branchName.length() != 0) {
			std::cout << "On branch " << readBranchName() << std::endl;
		}
		else {
			std::cout << "Head detached at " << commitSha1 << std::endl;
		}

		std::string sha1 = parentCommit.tree;
		readTree(ROOT_DIR.value(), sha1, entries); // current commit

		auto treeOpt = build_tree(ROOT_DIR.value());  // current work directory
		std::string treeSha = treeOpt.has_value() ? treeOpt.value().sha1 : "NULL";

		if (treeSha == sha1) {
			std::cout << "nothing added to commit \n";
		}
		else {
			int changedFiles = 0;
			int newFiles = 0;
			int deleteFiles = 0;
			std::set<fs::path> entries_path_set;
			std::unordered_map<fs::path, std::string, opt_path_hash>  entries_map;

			for (auto& e : entries) {
				entries_path_set.insert(e.path);
				entries_map[e.path] = e.sha1;
			}
			std::cout << blod_green << "changed files: \n";
			for (auto& p : Pathes) {
				if (entries_path_set.count(p) != 0) {
					std::string sha1_1 = hash_blob_path(p, false);
					std::string sha1_2 = entries_map.find(p)->second;
					if (sha1_1 != sha1_2) {
						changedFiles++;
						std::cout << '\t' << fs::relative(p).generic_string() << '\n';
					}
				}
			}
			std::cout << Reset;
			std::cout << green << "new files: \n";
			for (auto& p : Pathes) {
				if (entries_path_set.count(p) == 0) {
					++newFiles;
					std::cout << '\t' << fs::relative(p).generic_string() << '\n';
				}
			}
			std::cout << Reset;
			std::cout << cyan << "delete files: \n";
			for (auto& e : entries) {
				if (e.object_type != "blob") continue;
				auto &p = e.path;
				if (Pathes.count(p) == 0) {
					++deleteFiles;
					std::cout << '\t' << fs::relative(p).generic_string() << '\n';
				}
			}
			std::cout << Reset;

			std::string newcommitSha1 = writeCommit(treeSha, parentCommit.sha1, message);
			writeMain(newcommitSha1);

			std::cout << blod_yellow <<"[main " << newcommitSha1.substr(0, 7) << "]" << Reset << "\n";

			std::cout << " " << changedFiles << " files changed,"
				<< " " << newFiles << " files inserted(+), " << " " << deleteFiles << " files deleted(-) \n";
		}

	}

	return 0;
	// commit
}
