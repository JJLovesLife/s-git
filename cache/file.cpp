#include "file.h"
#include "../s-git.h"

void 
write_file(fs::path dir, const char* data ,int len) {
	//"""Write data bytes to file at given path."""
	//std::cout << dir.parent_path() << "\n";
	if (!fs::exists(dir.parent_path())) {
		fs::create_directories(dir.parent_path());
	}
	std::ofstream outfile;
	outfile.open(dir, std::ios::out | std::ios::trunc | std::ios::binary);
	outfile.write(data, len);
	outfile.close();
}

void
write_file(const fs::path &path, const std::vector<char> &data) {
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

long unsigned
readFile(fs::path dir,char * &buffer ) {

	std::ifstream t(dir, std::ios::in | std::ios::binary);
	//std::string str;

	t.seekg(0, t.end);
	int length = t.tellg();
	if (length <= 0) {
		t.close();
		return 0;
	}
	//str.reserve(t.tellg());
	t.seekg(0, t.beg);
	buffer = new char[length];
	t.read(buffer, length);
	/*str.assign((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());*/
	t.close();
	//write_file(fs::path{ ".//todo1.txt" }, buffer, length);
	return length;
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

int find(char* data, char t, int size) {  
	for (int i = 0; i < size; ++i) {
		if (data[i] == t  ) {
			return i;
		}
	}
	return size-1;
}
/*
std::string substr(char* data, int begin, int size) {
	std::string temp(data + begin, size);

}*/

std::string readMain() {
	fs::path headDir = GIT_DIR.value() / "HEAD";
	//std::cout << headDir << "\n";
	auto buffer = readFile(headDir);

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

bool writeMain(std::string sha1) {
	fs::path headDir = GIT_DIR.value() / "HEAD";
	//std::cout << headDir << "\n";
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

