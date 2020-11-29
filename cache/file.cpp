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
	fs::path gitDir(std::string{ "." }+GIT_NAME);
	fs::path headDir = gitDir / "HEAD";
	//std::cout << headDir << "\n";
	char* buffer;
	int size = readFile(headDir, buffer);
	std::string mainpath(buffer, size);
	if (buffer)delete buffer;
	fs::path mainDir = gitDir / fs::path(mainpath);
	//std::cout << mainDir << "\n";
	char* buffer1;
	size = readFile(mainDir, buffer1);
	std::string sha1(buffer1, size);
	if (size) delete buffer1;
	//std::cout << sha1 << "";
	return sha1;
}
int writeMain(std::string sha1) {
	fs::path gitDir(std::string{ "." }+GIT_NAME);
	fs::path headDir = gitDir / "HEAD";
	char* buffer;
	int size = readFile(headDir, buffer);
	std::string mainpath(buffer, size);
	if (buffer)delete buffer;
	fs::path mainDir = gitDir / fs::path(mainpath);
	std::cout << mainDir << "\n";
	write_file(mainDir, (char*)sha1.c_str(), sha1.length());
	return 1;
}

