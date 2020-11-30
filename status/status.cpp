#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"
#include <unordered_set>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <functional >



void paths(fs::path path, std::unordered_set<std::optional<fs::path>, opt_path_hash>& Pathes);

int status(int argc, const char* argv[]);

extern Command StatusCommand{
	status,
	"Show the working tree status"
};



namespace fs = std::filesystem;




/*template <typename TP>
std::time_t to_time_t(TP tp)
{
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
		+ system_clock::now());
	return system_clock::to_time_t(sctp);
}*/

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
		std::cout << "fatal: not a " << GIT_NAME << " repository(or any of the parent directories)";
		return 1;
	}

	//fs::path workDir = fs::path(".");

	//std::set<index> indexWork;
	std::unordered_set<fs::path, opt_path_hash> Pathes;
	//std::vector<fs::path> Pathes;

	//paths(workDir, Pathes);
	paths(ROOT_DIR.value(), Pathes);
	std::string commitSha1 = readMain();
	
	//std::cout << sha1 << "\n";
	if (commitSha1.length()==0) {
		std::cout << "No commits yet  \n";
		std::cout << "Untracked files:\n";
		for (auto& path : Pathes) {

			std::cout << '\t' << fs::relative(path).generic_string() << '\n';
		}
	}
	else {
		std::vector<object> entries; // 原有的
		//std::vector<fs::path> changedFiles;
		//std::vector<fs::path> newFiles;
		//std::vector<fs::path> deleteFiles;
		std::unordered_set<fs::path, opt_path_hash> entries_path_set;
		std::unordered_map<fs::path, std::string, opt_path_hash>  entries_map;
		commit parentCommit;
		if (!readCommit(commitSha1, parentCommit)) {
			std::cout << "No commits yet  \n";
			std::cout << "Untracked files:\n";
			for (auto& path : Pathes) {

				std::cout << '\t' << fs::relative(path).generic_string() << '\n';
			}
			return 0;
		}

		std::cout << "On branch main\n";
		bool clean = true;

		std::string sha1 = parentCommit.tree;
		readTree(ROOT_DIR.value(), sha1, entries);
		for (auto& e : entries) {
			entries_path_set.insert(e.path);
			entries_map[e.path] = e.sha1;
		}
		/*for (auto& e : entries_map) {
			std::cout << e.first.value() << " " << e.second << std::endl;
		}*/
		std::cout << "changed files: \n";
		for (auto &p:Pathes) {
			if (entries_path_set.count(p) != 0) {
				std::string sha1_1 = hash_blob_path(p, false);
				//std::cout << "\t"<< sha1_1 << "\n";
				//std::cout<<
				std::string sha1_2= entries_map.find(p)->second;
				//std::cout << "\t"<< sha1_2 << "\n";
				if (sha1_1 != sha1_2) {
					//changedFiles.push_back(p.value());
					std::cout << '\t' << fs::relative(p).generic_string() << '\n';
					clean = false;
				}
			}
		}
		std::cout << "new files: \n";
		for (auto& p : Pathes){
			if (entries_path_set.count(p)==0) {
				//newFiles.push_back(p.value());
				std::cout << '\t' << fs::relative(p).generic_string() << '\n';
				clean = false;
			}
		}
		std::cout << "delete files: \n";
		for (auto& e : entries) {
			if (e.object_type != "blob") continue;
			auto &p = e.path;
			if (Pathes.count(p) == 0) {
				std::cout << '\t' << fs::relative(p).generic_string() << '\n';
				clean = false;
			}
		}

		if (clean) {
			std::cout << "nothing to commit, working tree clean\n";
		}
	}

	//if (fs::exists())
	/*
	
	*/
	/*std::cout << Pathes.size() << std::endl;
	for (auto& it : Pathes) {
		//std::cout << it.filename();
		std::cout << it.value() << std::endl;
	}*/
	//
	
	//std::vector<index> entries;
	//readIndex()


	//std::unordered_set<std::optional<fs::path>, opt_path_hash> headPathes;
	/*//fs::path gitDir(std::string{ '.' });
	//TODO 从最近的上一次commit读入 状态 （函数）
	//TODO 从现在的工作目录计算树 (函数)  只要文件名即可  ✔
	//TODO 算出差值 文件 同名文件改变量 增加量 删除量 此函数计算
	/*std::cout << "Initialized empty " << GIT_NAME << " repository in "
		<< gitDir << std::endl;*/
	return 0;
}
