#include "object.h"
#include "../s-git.h"
#include "../cache/file.h"

#include <cctype>
#include "../3rdParty/sha1.hpp"

namespace fs = std::filesystem;

bool sha1Exist(const std::string &sha1) {
	fs::path path = sha1_to_path(sha1);
	return fs::exists(path);
}

bool checkSha1(const std::string &sha1) {
	if (sha1.length() != 40) return false;
	for (auto c : sha1) {
		if (!std::isxdigit(c) || std::isupper(c)) {
			// only lower case hex digit is acceptable
			return false;
		}
	}
	return true;
}

bool readCommit(const std::string &sha1, commit& thisCommit) {
	if (!sha1Exist(sha1)) {
		std::cerr << "Error: " << GIT_NAME << " commit file doesn't exist" << std::endl;
		return false;
	}

	fs::path path = sha1_to_path(sha1);
	std::ifstream t(path, std::ios::in | std::ios::binary);

	std::string type;
	t >> type >> thisCommit.size;
	char nullChar;
	t >> nullChar;
	if (type != "commit" || nullChar != '\0') {
		return false;
	}

	std::getline(t, thisCommit.tree, ' ');
	std::getline(t, thisCommit.parent, ' ');
	std::getline(t, thisCommit.message);

	if (!t.eof()) {
		return false;
	}
	t.close();

	thisCommit.sha1 = sha1;
	return true;
}

void readTree(const fs::path &dir, const std::string &sha1, std::vector<object>& path) {
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
					path.push_back(tmp);
				}
				else if (tmp.object_type == "tree") {
					path.push_back(tmp);
					readTree(tmp.path, tmp.sha1, path);
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

template<typename CharT, typename TraitsT = std::char_traits<CharT> >
class vectorwrapbuf : public std::basic_streambuf<CharT, TraitsT> {
public:
	vectorwrapbuf(const std::vector<CharT> &vec) {
		this->setg(const_cast<CharT *>(vec.data()),
			const_cast<CharT *>(vec.data()),
			const_cast<CharT *>(vec.data() + vec.size()));
	}
};

std::string
hash_object(const char* object_type, const std::vector<char> &data, bool write) {
	/* Generate the header */
	std::stringstream hdrStream;
	hdrStream << object_type << ' ' << data.size() << '\0';

	SHA1 checksum;
	checksum.update(hdrStream);
	vectorwrapbuf<char> buf(data);
	std::istream is(&buf);
	checksum.update(is);

	std::string sha1 = checksum.final();

	if (write) {
		fs::path objectPath = sha1_to_path(sha1);

		write_object(objectPath, hdrStream.str(), data);
	}
	return sha1;
}

std::string
hash_object(const char* object_type, const std::string &message, bool write) {
	/* Generate the header */
	std::stringstream hdrStream;
	hdrStream << object_type << ' ' << message.size() << '\0';

	SHA1 checksum;
	checksum.update(hdrStream);
	checksum.update(message);

	std::string sha1 = checksum.final();

	if (write) {
		fs::path objectPath = sha1_to_path(sha1);

		if (!fs::exists(objectPath)) {
			write_object(objectPath, hdrStream.str(), message);
		}
	}
	return sha1;
}

std::string hash_blob_path(const fs::path &path, bool write) {
	auto buffer = readFile(path);
	return hash_object("blob", buffer, write);
}



void
paths(const fs::path &path, std::set<fs::path>& Pathes) {
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
			paths(it, Pathes);
		}
		else if (fs::is_regular_file(it)) {
			Pathes.insert(it.path());
		}
	}
}
std::vector<char>
readRawFile(const std::string& sha1) {


	if (sha1 == "NULL") {
		return {};
	}
	if (sha1Exist(sha1)) {
		fs::path path = sha1_to_path(sha1);
		std::ifstream file(path, std::ios::in | std::ios::binary);

		std::string type;
		std::getline(file, type, ' ');

		if (file.peek() < '0' || file.peek() > '9') {
			std::cerr << "Error: " << GIT_NAME << sha1 << " this file is corrupted, read failed" << std::endl;
			return {};
		}
		size_t size;
		file >> size;

		char nullChar;
		file >> nullChar;
		if (nullChar != '\0') {
			std::cerr << "Error: " << GIT_NAME << sha1 << " this file is corrupted, read  failed" << std::endl;
			return{};
		}

		std::vector<char> buffer(size);
		file.read(buffer.data(), buffer.size());
		file.close();
		return buffer;

	}
	else {
		std::cerr << "Error: " << GIT_NAME << sha1 << " this file doesn't exist , read  failed" << std::endl;
		return {};
	}


}

void copy_object_tofile(const fs::path& path, const std::string& sha1) {
	
	auto buffer = readRawFile(sha1);
	if (buffer.size()) write_file(path, buffer);
}