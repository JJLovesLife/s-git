#pragma once
#include <filesystem>
#include <string>
#include <set>
#include <future>
#include <vector>
#include <optional>

namespace fs = std::filesystem;

struct object {
	std::string object_type;
	std::string sha1;
	std::filesystem::path path;
	bool operator==(const object& obj)const {
		return this->sha1 == obj.sha1;
	}
};

struct commit {
	std::string sha1;
	size_t size;
	std::string tree; //sha1
	std::string parent; //sha1
	std::string message;
};

bool readCommit(const std::string &sha1, commit& thisCommit);

bool sha1Exist(const std::string &sha1);

bool checkSha1(const std::string &sha1);



std::string
hash_object(const char* object_type, const std::vector<char> &data, bool write = true);

std::string
hash_object(const char* object_type, const std::string &message, bool write = true);

std::string hash_blob_path(const std::filesystem::path &path, bool write);


void readTree(const std::filesystem::path &dir, const std::string &sha1, std::vector<object>& path);

struct opt_path_hash {
	std::size_t operator()(const std::filesystem::path& path) const {
		return std::filesystem::hash_value(path);
	}
};

std::set<fs::path>
paths(const std::filesystem::path &path);

void copy_object_tofile(const std::filesystem::path& path, const std::string& sha1);

std::optional<std::string> resolveBranch(std::string &branch);

struct commit Parent(struct commit child);

int commitLength(const struct commit c);

struct object_equal {
    bool operator()(const object& obj1, const object& obj2) const {
        return obj1.path.string() == obj2.path.string();
    }
};

struct object_hash {
    std::size_t operator()(const object& obj) const {
        return std::hash<std::string>()(obj.path.string());
    }
};