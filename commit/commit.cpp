#define  _CRT_SECURE_NO_DEPRECATE

#include "../s-git.h"
#include "../cache/file.h"
#include "..//object/object.h"
int commitMain(int argc, const char* argv[]);
extern Command CommitCommand{
	commitMain,
	"Record changes to the repository"
};

#include <filesystem>
#include <iostream>
#include <optional>



namespace fs = std::filesystem;


std::string writeCommit(std::string treeSha1,std::string parent,std::string message) {
	std::stringstream buffer;
	buffer << treeSha1 << ' ' << parent << ' ' << message;

	auto str = buffer.str();
	std::string sha1 = hash_object("commit", str, false);

	if (!sha1Exist(sha1)) {
		hash_object("commit", str, true);
	}

	return sha1;
}


object
fileIndex(fs::path path) {
	//std::cout << path << "\n";

	object thisBlob;
	thisBlob.object_type = "blob";
	thisBlob.path = path;
	thisBlob.sha1 = hash_blob_path(path, false);
	

	if (!sha1Exist(thisBlob.sha1)) {
		hash_blob_path(path, true);
	}
	return thisBlob;
}
bool cmp(const object& a, const object& b) {
	return a.sha1 < b.sha1;
}

std::string write_tree(std::vector<object> index) {
	std::stringstream buffer;
	for (auto &v : index) {
		buffer << v.object_type << ' ' << v.sha1 << ' ' << v.path.filename().u8string() << '\n';
	}

	auto str = buffer.str();

	std::string sha1 = hash_object("tree", str, false);

	if (!sha1Exist(sha1)){
		hash_object("tree", str, true);
	}

	return sha1;
}


std::optional<object>
build_tree(fs::path path) {
	fs::directory_entry entry(path);		//文件入口
	if (entry.status().type() != fs::file_type::directory) {
		std::cerr << "Error: " << GIT_NAME << " is not a directory" << std::endl;
		return object{};
	}
	fs::directory_iterator list(path);
	std::vector<object> v;
	//int level = 1;
	//std::cout << "start\n";
	for (auto& it : list) {
		auto filename = it.path().filename();
		// if (it.path() == (fs::path(std::string{ ".\\." } +GIT_NAME)) || it.path() == fs::path{ ".\\.git" }
		//	|| it.path() == fs::path{ ".\\s-git.tlog" } || it.path() == fs::path{".\\ss"}
		//	|| it.path() == fs::path{ ".\\test" }		) {
		//	//std::cout << it.path() << "\n";
		//	continue;
		//}
		if (it.path() == GIT_DIR.value()) {
			continue;
		}
		//std::cout << it.path() << "\n";
		if (fs::is_directory(it.status()))
		{
			auto obj = build_tree(it);
			if (obj.has_value()) {
				v.push_back(obj.value());
			}
			//std::cout << filename << "\n";
			//std::cout << "\n";
		}
		else if (fs::is_regular_file(it)) {
			v.push_back(fileIndex(it.path()));
				/* if (it.path().has_extension() && it.path().extension() == ".txt") { //fs::is_regular_file(entry.status())
				//std::cout << filename << "\n";
				v.push_back(fileIndex(it.path(), index));
				//DisplayFileInfo(entry, lead, filename);
				//v.push_back(fileIndex(it));
			}
			else {
				//std::cout << filename << "\n";
				//std::cerr << "Error: " << " is not a directory and is not a file" << std::endl;
			}*/
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
	//thisTree.path = sha1_to_path(thisTree.sha1);
	return thisTree;
}


int commitMain(int argc, const char* argv[]) {
	cmdline::parser argParser;

	argParser.add<std::string>("message", 'm', "commit message", true, "");
	argParser.parse_check(argc, argv);

	std::string  message = argParser.get<std::string>("message");

	std::string commitSha1 = readMain();


	std::unordered_set<fs::path, opt_path_hash> Pathes;
	paths(ROOT_DIR.value(), Pathes);
	if (commitSha1.length()==0) {
		std::vector<object> index;
		auto treeOpt = build_tree(ROOT_DIR.value());  //修改后的

		std::string newcommitSha1 = writeCommit(treeOpt.has_value() ? treeOpt.value().sha1 : "NULL" ,  "NULL", message);
		writeMain(newcommitSha1);

		std::cout << "first commit  \n";
		std::cout << "[main (root-commit) " << newcommitSha1.substr(0, 7) << "]\n";
		std::cout << Pathes.size() << " file inserted(+)\n";
		for (auto& path : Pathes) {
			std::cout << " " << fs::relative(path, ROOT_DIR.value()).generic_string() << '\n';
		}
	}
	else {
			std::vector<object> entries; // 原有的
			commit parentCommit;
			if (!readCommit(commitSha1, parentCommit)) {
				auto treeOpt = build_tree(ROOT_DIR.value());  //修改后的

				std::string newcommitSha1 = writeCommit(treeOpt.has_value() ? treeOpt.value().sha1 : "NULL", "NULL", message);
				writeMain(newcommitSha1);

				std::cout << "first commit  \n";
				std::cout << "[main (root-commit) " << newcommitSha1.substr(0, 7) << "]\n";
				std::cout << Pathes.size() << " file inserted(+)\n";
				for (auto& path : Pathes) {
					std::cout << " " << fs::relative(path, ROOT_DIR.value()).generic_string() << '\n';
				}
				return 0;
			}

			std::cout << "On branch main\n";
			std::string sha1 = parentCommit.tree;
			readTree(ROOT_DIR.value(), sha1, entries); //读取

			auto treeOpt = build_tree(ROOT_DIR.value());  //修改后的
			std::string treeSha = treeOpt.has_value() ? treeOpt.value().sha1 : "NULL";

			if (treeSha == sha1) {
				std::cout << "nothing added to commit \n";
			}
			else {
				//std::vector<fs::path> changedFiles;
				//std::vector<fs::path> newFiles;
				//std::vector<fs::path> deleteFiles;
				int changedFiles = 0;
				int newFiles = 0;
				int deleteFiles = 0;
				std::unordered_set<fs::path, opt_path_hash> entries_path_set;
				std::unordered_map<fs::path, std::string, opt_path_hash>  entries_map;

				for (auto& e : entries) {
					entries_path_set.insert(e.path);
					entries_map[e.path] = e.sha1;
				}

				std::cout << "changed files: \n";
				for (auto& p : Pathes) {
					if (entries_path_set.count(p) != 0) {
						std::string sha1_1 = hash_blob_path(p, false);
						//std::cout << "\t"<< sha1_1 << "\n";
						//std::cout<<
						std::string sha1_2 = entries_map.find(p)->second;
						//std::cout << "\t"<< sha1_2 << "\n";
						if (sha1_1 != sha1_2) {
							//changedFiles.push_back(p.value());
							changedFiles++;
							std::cout << '\t' << fs::relative(p).generic_string() << '\n';
						}
					}
				}
				std::cout << "new files: \n";
				for (auto& p : Pathes) {
					if (entries_path_set.count(p) == 0) {
						//newFiles.push_back(p.value());
						++newFiles;
						std::cout << '\t' << fs::relative(p).generic_string() << '\n';
					}
				}
				std::cout << "delete files: \n";
				for (auto& e : entries) {
					if (e.object_type != "blob") continue;
					auto &p = e.path;
					if (Pathes.count(p) == 0) {
						//deleteFiles.push_back(p.value());
						++deleteFiles;
						std::cout << '\t' << fs::relative(p).generic_string() << '\n';
					}
				}

				std::string newcommitSha1 = writeCommit(treeSha, parentCommit.sha1, message);
				writeMain(newcommitSha1);

				std::cout << "[main " << newcommitSha1.substr(0, 7) << "]\n";

				std::cout << " " << changedFiles << " files changed,"
					<< " " << newFiles << " insertions(+), " << " " << deleteFiles << " deletions(-) \n";
			}

		}
		
	return 0;
	// commit
}
