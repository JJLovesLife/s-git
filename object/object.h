#define  _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <string>
#include <iostream>

#include "../s-git.h"
#include "../cache/file.h"

#include <filesystem>
#include <optional>
#include <unordered_set>
#include <functional >
#include <unordered_map>

namespace fs = std::filesystem;
struct object {
	std::string object_type;
	std::string sha1;
	fs::path path;
	bool operator==(const object& obj)const {
		return this->sha1 == obj.sha1;
	}
};


struct opt_path_hash {
	std::size_t operator()(const std::optional<fs::path>& path) const {
		return path ? hash_value(path.value()) : 0;
	}
};
struct object_hash {
	std::size_t operator()(const object& obj) const {
		return std::hash<std::string>()(obj.sha1);
	}
};

struct commit {
	std::string sha1;
	int size;
	std::string tree;//sha1
	std::string author;
	std::string parent; //sha1
	std::string message;
};
int readCommit(std::string sha1, commit& thisCommit);
bool sha1Exist(std::string sha1);

int readSha1(std::string sha1, char* &buf); // 返回长度 无则-1

void readTree(std::string sha1, std::vector<object>& path);

std::string
hash_object(const char* object_type, char* buf, long unsigned len, bool write=true);


std::string hash_blob_path(fs::path path, bool write);

fs::path sha1_to_path(std::string sha1);

void
paths(fs::path path, std::unordered_set<std::optional<fs::path>, opt_path_hash>& Pathes);