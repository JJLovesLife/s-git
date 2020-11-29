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



namespace fs = std::filesystem;


std::string writeCommit(std::string treeSha1,std::string parent,std::string message,std::string author) {
	int size = 0;
	size = treeSha1.length() + author.length() + parent.length() + message.length() + 4;

	char* buffer = new char[size];
	sprintf((char*)buffer, "%s %s %s %s ", treeSha1.c_str(), author.c_str(), parent.c_str() , message.c_str());

	std::string sha1 = hash_object("commit", buffer, size, false);

	if (!sha1Exist(sha1)) {
		hash_object("commit", buffer, size, true);
	}


	return sha1;
}


object
fileIndex(fs::path path, std::vector<object>& index) {
	//std::cout << path << "\n";

	object thisBlob;
	thisBlob.object_type = "blob";
	thisBlob.path = path;
	thisBlob.sha1 = hash_blob_path(path, false);
	

	if (!sha1Exist(thisBlob.sha1)) {
		hash_blob_path(path, true);
	}
	index.push_back(thisBlob);
	return thisBlob;
}
bool cmp(const object& a, const object& b) {
	return a.sha1 < b.sha1;
}

std::string write_tree(std::vector<object> index) {
	int size=0;
	for (auto& v : index) {
		//std::cout << v.object_type << " " << v.sha1 << " " << v.path.u8string() << " \n";
		//std::cout << v.object_type.length() + v.sha1.length() + v.path.u8string().length() + 3<<"\n";
		size += (v.object_type.length() + v.sha1.length() + v.path.u8string().length() + 3);
	}

	char* buffer = new char[size];
	int offset = 0;
	for (auto& v : index) {
		sprintf((char *)buffer + offset, "%s %s %s ", v.object_type.c_str(), v.sha1.c_str(),v.path.u8string().c_str());
		offset += (v.object_type.length() + v.sha1.length() + v.path.string().length() + 3);
		//std::cout << offset << "\n";
	}
	//std::cout << size << "\n";

	std::string sha1 =hash_object("tree", buffer, size, false);

	if (!sha1Exist(sha1)){
		hash_object("tree", buffer, size,true);
	}

	return sha1;
}


object
build_tree(fs::path path, std::vector<object>& index) {
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
		if (it.path() == (fs::path(std::string{ ".\\." } +GIT_NAME)) || it.path() == fs::path{ ".\\.git" }
			|| it.path() == fs::path{ ".\\s-git.tlog" } || it.path() == fs::path{".\\ss"}
			|| it.path() == fs::path{ ".\\test" }		) {
			//std::cout << it.path() << "\n";
			continue;
		}
		//std::cout << it.path() << "\n";
		if (fs::is_directory(it.status()))
		{
			v.push_back(build_tree(it, index));
			//std::cout << filename << "\n";
			//std::cout << "\n";
		}
		else if (fs::is_regular_file(entry)) {
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
	
	object thisTree;
	sort(v.begin(), v.end(), cmp);
	thisTree.object_type = "tree";
	thisTree.sha1 = write_tree(v);
	thisTree.path = sha1_to_path(thisTree.sha1);
	index.push_back(thisTree);
	return thisTree;
}


int commitMain(int argc, const char* argv[]) {

	cmdline::parser argParser;

	argParser.add<std::string>("message", 'm', "commit message", true, "");
	// argParser.add<std::string>("author", 'a', "commit author", true, "");
	argParser.parse_check(argc, argv);

	std::string  message = argParser.get<std::string>("message");
	std::string  author = argParser.get<std::string>("author");

	fs::path gitDir(std::string{ '.' } +GIT_NAME);
	fs::path workDir(".");


	std::string commitSha1 = readMain();


	std::unordered_set<fs::path, opt_path_hash> Pathes;
	paths(workDir, Pathes);
	if (commitSha1.length()==0) {
		std::vector<object> index;
		object tree = build_tree(workDir, index);  //修改后的
		std::cout << "first commit  \n";
		std::cout << "[main (root-commit) " << tree.sha1.substr(0, 7) << "]\n";
		std::cout << Pathes.size() << " file changed\n";
		for (auto& path : Pathes) {
			std::cout << " " << fs::relative(path).generic_string() << '\n';
		}
		//std::cout << tree.sha1 << "\n";
		std::string newcommitSha1 = writeCommit(tree.sha1, "NULL", message, author);
		std::cout << newcommitSha1 << '\n';
		writeMain(newcommitSha1);
		//for ()
	}
	else {

			std::vector<object> entries; // 原有的
			commit parentCommit;
			if (!readCommit(commitSha1, parentCommit)) {
				std::vector<object> index;
				object tree = build_tree(workDir, index);  //修改后的
				std::cout << "first commit  \n";
				std::cout << "[main (root-commit) " << tree.sha1.substr(0, 7) << "]\n";
				std::cout << Pathes.size() << " file changed\n";
				for (auto& path : Pathes) {
					std::cout << " " << fs::relative(path).generic_string() << '\n';
				}
				//std::cout << tree.sha1 << "\n";
				std::string newcommitSha1 = writeCommit(tree.sha1, "NULL", message, author);
				std::cout << newcommitSha1 << '\n';
				writeMain(newcommitSha1);
				return 0;
			}
			std::string sha1 = parentCommit.tree;
			readTree(sha1, entries); //读取

			std::vector<object> index; 
			object tree = build_tree(workDir, index);  //修改后的

			if (tree.sha1 == sha1) {
				std::cout << "On branch main\n";
				std::cout << "nothing added to commit \n";
			}
			else {
				std::cout << "「main " << tree.sha1.substr(0, 7) << "」\n";
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
							std::cout << '\t' << p << '\n';
						}
					}
				}
				std::cout << "new files: \n";
				for (auto& p : Pathes) {
					if (entries_path_set.count(p) == 0) {
						//newFiles.push_back(p.value());
						++newFiles;
						std::cout << '\t' << p << '\n';
					}
				}
				std::cout << "delete files: \n";
				for (auto& p : entries_path_set) {
					if (Pathes.count(p) == 0) {
						//deleteFiles.push_back(p.value());
						++deleteFiles;
						std::cout << '\t' << p << '\n';
					}
				}

				std::cout << " " << changedFiles << " files changed," 
					<< " " << newFiles << " insertions(+), " << " " << deleteFiles << " deletions(-) \n";
				
				std::string newcommitSha1= writeCommit(tree.sha1, parentCommit.sha1, message, author);
				writeMain(newcommitSha1);
				//writeCommit(tree.sha1);
			}

		}
		
	return 0;
	// commit
}
