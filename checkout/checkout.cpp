#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"

#include "../cache/file.h"

#include "../3rdParty/sha1.hpp"
#include <iostream>

namespace fs = std::filesystem;

int checkoutMain(int argc, const char* argv[]);
extern Command CheckoutCommand{
	checkoutMain,
	"Switch branches or restore working tree files"
};

void deleteDir(fs::path path) {

	fs::directory_entry entry(path);
	if (entry.status().type() != fs::file_type::directory) {
		std::cerr << "Error: " << fs::relative(path) << " is supposed to be a directory" << std::endl;
		return;
	}

	fs::path ignore = path / ".s-gitignore";
	if (fs::exists(ignore) && fs::file_size(ignore) == 0) {
		// ignore this directory if there exists file ".s-gitignore" with empty content
		return;
	}

	fs::directory_iterator list(path);

	for (auto& it : list) {
		if (it.path() == GIT_DIR) continue; // .s-git will not be archived

		if (fs::is_directory(it.status())) {
			deleteDir(it.path());
		}
		else if (fs::is_regular_file(it)) {
			fs::remove(it.path());
		}
	}
}

void copyBranches(const fs::path& dir, const std::string& sha1) {
	if (sha1 == "NULL") {

		return;
	}
	if (sha1Exist(sha1)) {
		fs::path treePath = sha1_to_path(sha1);
		std::ifstream file(treePath, std::ios::in | std::ios::binary);

		std::string type;
		std::getline(file, type, ' ');

		if (file.peek() < '0' || file.peek() > '9') {
			std::cerr << "Error: " << GIT_NAME << sha1 << " this file is corrupted, read tree failed" << std::endl;
			return;
		}
		size_t size;
		file >> size;

		char nullChar;
		file >> nullChar;
		if (nullChar != '\0') {
			std::cerr << "Error: " << GIT_NAME << sha1 << " this file is corrupted, read tree failed" << std::endl;
			return;
		}

		if (type != "tree") {
			std::cerr << "Error: " << GIT_NAME << sha1 << " this is not a tree, read tree failed" << std::endl;
			return;
		}
		else {
			while (file.peek() != decltype(file)::traits_type::eof()) {
				object tmp;
				std::getline(file, tmp.object_type, ' ');
				std::getline(file, tmp.sha1, ' ');

				std::string tmpPath;
				std::getline(file, tmpPath);
				tmp.path = tmpPath;
				tmp.path = dir / tmp.path;
				if (tmp.object_type == "blob") {
					copy_object_tofile(tmp.path, tmp.sha1);
					// 复制文件过来
					//path.push_back(tmp);
				}
				else if (tmp.object_type == "tree") {
					//path.push_back(tmp);
					// 新建文件夹
					copyBranches(tmp.path, tmp.sha1);
					// 回去
				}
				else {
					std::cerr << "Error:" << GIT_NAME << " " << sha1 << " Tree object  raw data damaged" << std::endl;
				}
			}
		}
	}
	else {
		std::cerr << "Error: " << GIT_NAME << " Tree don'file exists" << std::endl;
		return;
	}
}


int checkoutMain(int argc, const char* argv[]) {
	cmdline::parser argParser;
	
	argParser.add<std::string>("tag", 't', "Switch tags",false);
	argParser.add<std::string>("branches", 'b', "Switch branches",false);
	argParser.add<std::string>("sha1", 's', "Switch commit sha1", false);

	argParser.footer("<which to switch  > ...");
	argParser.parse_check(argc, argv);
	//argParser.add<std::string>("sha1", 's', "Switch sha1", true, "");

	argParser.parse_check(argc, argv);
	//std::string ar;
	if (argParser.get<std::string>("sha1").length()) {
		std::string sha1 = argParser.get<std::string>("sha1");
		deleteDir(ROOT_DIR.value());
		commit  Commit;
		readCommit(sha1, Commit);
		copyBranches(ROOT_DIR.value(), Commit.tree);
		//TODO 写回 head
		fs::path headDir = GIT_DIR.value() / "HEAD";
		write_file(headDir, Commit.sha1.data(), Commit.sha1.size());

		return 0;
	}
	if (argParser.get<std::string>("branches").length()) {
		std::string branch = argParser.get<std::string>("branches");
		std::string sha1 = readBranch(branch);
		if (sha1.length() == 0) {
			std::cerr << "Error:" << " doesn't have this  branch :" << branch << "\n";
			return 1;
		}
		deleteDir(ROOT_DIR.value());
		commit  branchCommit;
		readCommit(sha1, branchCommit);
		copyBranches(ROOT_DIR.value(), branchCommit.tree);
		//TODO 写回 head branchCommit.sha1
		fs::path branchPath = GIT_DIR.value() / "refs" / "branch";
		fs::path headDir = GIT_DIR.value() / "HEAD";
		std::string branchWrite = std::string{ "refs: " } +branchPath.u8string();
		write_file(headDir, branchWrite.data(),branchWrite.size());
		//writeMain(branchCommit.sha1);
		return 0;
	}
	if (argParser.get<std::string>("tag").length()) {
		std::string tag = argParser.get<std::string>("sha1");
		std::string sha1 = readTag(tag);
		if (sha1.length() == 0) {
			std::cerr << "Error:" << " doesn't have this tag :" << tag << "\n";
			return 1;
		}
		deleteDir(ROOT_DIR.value());
		commit  tagCommit;
		readCommit(sha1, tagCommit);
		copyBranches(ROOT_DIR.value(), tagCommit.tree);
		//TODO 写回 head tagCommit.sha1;
		fs::path headDir = GIT_DIR.value() / "HEAD";
		write_file(headDir, tagCommit.sha1.data(), tagCommit.sha1.size());
		return 0;
	}
	std::cout << argParser.usage();

	return 0;
}
