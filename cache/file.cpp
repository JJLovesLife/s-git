#include "file.h"
#include "../s-git.h"

#include <fstream>

namespace fs = std::filesystem;


fs::path sha1_to_path(const std::string& sha1) {
	fs::path Dir = GIT_DIR.value();
	Dir /= "objects";
	std::string sha12 = sha1.substr(0, 2);
	std::string sha118 = sha1.substr(2, 38);
	fs::path dir1(sha12);
	fs::path dir2(sha118);
	Dir /= dir1;
	Dir /= dir2;
	return Dir;
}

void 
write_file(const fs::path &path, const char* data , size_t len) {
	// Write data bytes to file at given path.
	std::ofstream outfile;
	outfile.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
	outfile.write(data, len);
	outfile.close();
}

void
write_file(const fs::path &path, const std::vector<char> &data) {
	// Write data bytes to file at given path.
	std::ofstream outfile(path, std::ios::out | std::ios::trunc | std::ios::binary);
	outfile.write(data.data(), data.size());
	outfile.close();
}


void
write_object(const fs::path &path, const std::string &header, const std::vector<char> &data) {
	if (!fs::exists(path.parent_path())) {
		fs::create_directories(path.parent_path());
	}
	std::ofstream outfile;
	outfile.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
	outfile.write(header.data(), header.size());
	outfile.write(data.data(), data.size());
	outfile.close();
}


void
write_object(const fs::path &path, const std::string &header, const std::string &message) {
	if (!fs::exists(path.parent_path())) {
		fs::create_directories(path.parent_path());
	}
	std::ofstream outfile;
	outfile.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
	outfile.write(header.data(), header.size());
	outfile.write(message.data(), message.size());
	outfile.close();
}

std::vector<char>
readFile(const fs::path &path) {
	if (!fs::is_regular_file(path)) {
		return {};
	}

	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return {};
	}

	std::vector<char> buffer(fs::file_size(path));
	file.read(buffer.data(), buffer.size());
	file.close();
	return buffer;
}

std::string readTag(std::string tagName) {
	fs::path tagPath = GIT_DIR.value() / "refs" / "tags";
	tagPath /= tagName;

	auto sha1 = readFile(tagPath);
	return { sha1.begin(), sha1.end() };
}


std::string readBranch(std::string branchName) {
	fs::path branchPath = GIT_DIR.value() / "refs" / "heads";
	branchPath /= branchName;

	auto sha1 = readFile(branchPath);
	return { sha1.begin(), sha1.end() };
}

std::string readMain() {
	fs::path headPath = GIT_DIR.value() / "HEAD";
	auto buffer = readFile(headPath);

	constexpr const char *refPrompt = "ref: ";
	constexpr size_t refPromptSize = std::char_traits<char>::length(refPrompt);
	bool ref = buffer.size() >= refPromptSize;
	for (size_t i = 0; i < refPromptSize && ref; i++) {
		if (buffer[i] != refPrompt[i]) {
			ref = false;
		}
	}

	if (ref) {
		buffer.push_back('\0');
		fs::path refPath(&buffer[refPromptSize], fs::path::format::generic_format);
		fs::path mainDir = GIT_DIR.value() / refPath;
		buffer = readFile(mainDir);
	}
	return { buffer.begin(), buffer.end() };
}

bool writeMain(const std::string &sha1) {
	fs::path headDir = GIT_DIR.value() / "HEAD";
	auto buffer = readFile(headDir);

	constexpr const char *refPrompt = "ref: ";
	constexpr size_t refPromptSize = std::char_traits<char>::length(refPrompt);
	bool ref = buffer.size() >= refPromptSize;
	for (size_t i = 0; i < refPromptSize && ref; i++) {
		if (buffer[i] != refPrompt[i]) {
			ref = false;
		}
	}

	if (!ref) {
		write_file(headDir, sha1.data(), sha1.size());
		return true;
	}

	buffer.push_back('\0');
	fs::path refPath(&buffer[refPromptSize], fs::path::format::generic_format);
	fs::path branchPath = GIT_DIR.value() / refPath;

	write_file(branchPath, sha1.data(), sha1.size());
	return true;
}

std::string readBranchName() {
	fs::path headDir = GIT_DIR.value() / "HEAD";
	auto buffer = readFile(headDir);

	constexpr const char *refPrompt = "ref: ";
	constexpr size_t refPromptSize = std::char_traits<char>::length(refPrompt);
	bool ref = buffer.size() >= refPromptSize;
	for (size_t i = 0; i < refPromptSize && ref; i++) {
		if (buffer[i] != refPrompt[i]) {
			return {};
		}
	}

	buffer.push_back('\0');
	fs::path refPath(&buffer[refPromptSize], fs::path::format::generic_format);

	return refPath.filename().string();
}
