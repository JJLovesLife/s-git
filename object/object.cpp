#include "object.h"
#include "..\3rdParty\sha1.hpp"
#include <string>


bool sha1Exist(std::string sha1) {

	fs::path Dir = sha1_to_path(sha1);
	if (!fs::exists(Dir.parent_path())) {
		return 0;
	}
	else {
		if (!fs::exists(Dir)) {
			return 0;
		}else{
			return 1;
		}
	}
}

int 
readSha1(std::string sha1,char *  & buf) {
	
	fs::path Dir(std::string{ '.' } +GIT_NAME);
	Dir /= "objects";
	std::string sha12 = sha1.substr(0, 2);
	fs::path dir1(sha12);
	std::string sha118 = sha1.substr(2, 38);
	fs::path dir2(sha118);
	Dir /= dir1;

	if (!fs::exists(Dir)) {
		std::cerr << "Error: " << GIT_NAME << " repository don't exists" << std::endl;
		return -1;
	}
	else {
		Dir /= dir2;
		if (!fs::exists(Dir)) {
			std::cerr << "Error: " << GIT_NAME << " repository don't exists" << std::endl;
			return -1;
		}
		else {
			int size = readFile(Dir,buf);
			return size;
		}
	}
}

// sha1 文件名
//type size 
//buf ::
//type sha1 path
//0123456
//11 01 1
//size = 7
//spilt = 2;
//spilt1=	5

int readCommit(std::string sha1, commit& thisCommit) {
	if (!sha1Exist(sha1)) {
		//std::cerr << "Error: " << GIT_NAME << " commit don't exists" << std::endl;
		return 0;
	}
	fs::path path = sha1_to_path(sha1);
	std::ifstream t(path, std::ios::in | std::ios::binary);
	//commit thisCommit;
	t >> thisCommit.sha1 >> thisCommit.size >>
		thisCommit.tree >> thisCommit.author >> thisCommit.parent >> thisCommit.message;
	t.close();
	return 1;
	//return thisCommit;
}
void readTree(std::string sha1, std::vector<object>& path) {
	//std::cout << sha1 << "\n";
	if (sha1Exist(sha1)) {
		//char* data;
		//int size = readSha1(sha1, data);
		fs::path treePath = sha1_to_path(sha1);
		std::ifstream t(treePath, std::ios::in | std::ios::binary);
		std::string type;
		int size;
		t >> type >> size;
		//std::cout << type << " " << size;
		if (type != "tree") {
			std::cerr << "Error: " << GIT_NAME << sha1 << " this is not a tree, read tree failed" << std::endl;
			return;
		}
		else {
			while (!t.eof() && t.peek() != EOF) {
				object tmp;
				t >> tmp.object_type >> tmp.sha1 >> tmp.path;
				//std::cout << tmp.object_type << " " << tmp.sha1 << " " << tmp.path << "\n";
				if (tmp.object_type == "blob") {
					path.push_back(tmp);
				}
				else {
					if (tmp.object_type == "tree") {
						path.push_back(tmp);
						readTree(tmp.sha1, path);
					}
					else {
						if (tmp.object_type == "") {
							break;
						}
						else {
							std::cerr << "Error:" << GIT_NAME << " " << sha1 << " Tree object  raw data damaged" << std::endl;
						}
						

					}
				}
			}
		}
	}
	else {
		std::cerr << "Error: " << GIT_NAME << " Tree don't exists" << std::endl;
		return;
	}
	
		/*//std::cout << sha1 << " \n";
		int spilt= find(data , ' ', size);
		std::string type(data, spilt);
		std::cout << spilt << " " << type;
		//= data.substr(0, spilt);
		int spilt1 = find(data + spilt + 1, ' ', size - spilt - 1);// data.find(" ", spilt);
		//int size = std::stoi(data.substr(spilt, spilt1));
		std::string temp(data + spilt + 1, spilt1 - spilt - 1);
		int raw_size = std::stoi(temp);

		std::cout << "size =" << size << "\n";
		std::cout << "type =" << type << "\n";
		std::cout << "raw_size =" << raw_size << "\n";

		if (type != "tree") {
			std::cerr << "Error: " << GIT_NAME << sha1 << " this is not a tree, read tree failed" << std::endl;
			return;
		}

		if (raw_size != size - spilt1-1) {
			std::cerr << "Error: " << GIT_NAME << sha1 << " Tree object damaged" << std::endl;
			return;
		}


		char* raw_data = data + spilt1 + 1;		// = data.substr(spilt1);
		int offset = 0;


		while (offset < raw_size-1) {
			object tmp;
			tmp.object_type = std::string
			(raw_data + offset, find(raw_data + offset , ' ', raw_size - offset));
			//raw_data.substr(offset, raw_data.find(" ", offset));
			offset += (find(raw_data + offset, ' ', raw_size - offset) + 1);
			tmp.sha1 = std::string
			(raw_data + offset, find(raw_data + offset, ' ', raw_size - offset ));
			//tmp.sha1 = raw_data.substr(offset, raw_data.find(" ", offset));
			offset += (find(raw_data + offset, ' ', raw_size - offset) + 1);
			//std::cout << offset << " " << find(raw_data + offset, ' ', raw_size - offset) + 1;
			tmp.path = std::string
			(raw_data + offset, find(raw_data + offset, ' ', raw_size - offset));
			//tmp.path = raw_data.substr(offset, raw_data.find(" ", offset));
			offset += (find(raw_data + offset, ' ', raw_size - offset) + 1);
			//std::cout << offset << " " << raw_size << "\n";
			//std::cout << tmp.object_type << "\n" <<(tmp.object_type == "blob\0" ) <<"\n";
			if (tmp.object_type == "blob") {
				//std::cout << "!\n";
				path.push_back(tmp);
			}
			else {
				if (tmp.object_type == "tree") {
					path.push_back(tmp);
					readTree(tmp.sha1, path);
				}
				else {
					std::cerr << "Error:" << GIT_NAME <<" "<< sha1 << " Tree object  raw data damaged" << std::endl;

				}
			}
		}
		//string type = data.substr(spilt);
		//string data
	}*/


}

std::string
hash_object(const char* object_type, char * buf , long unsigned len ,bool write){
	/* Generate the header */
	std::stringstream hdrStream;
	hdrStream << object_type << ' ' << len << '\0';
	std::string hdr = hdrStream.str();

	SHA1 checksum;
	checksum.update(hdr);
	std::stringstream bufStream;
	bufStream.write(buf, len);
	checksum.update(bufStream);

	auto resultStr = checksum.final();
	unsigned char result[20];
	for (size_t i = 0; i < 20; i++) {
		result[i] = resultStr[i];
	}

	std::stringstream shastr;
	shastr << std::hex << std::setfill('0');
	for (const auto& byte:result)
	{
		shastr << std::setw(2) << (int)byte;
	}
	std::string sha1 = shastr.str();
	if (write){
		//std::string SHA1 =
		
		std::string sha12 = sha1.substr(0,2);
		std::string sha128 = sha1.substr(2,38);
		//memcpy(sha12,sha1,2);
		//memcpy(sha128,sha1+2,18);
		
		fs::path objectDir(std::string{ '.' } + GIT_NAME);
		objectDir/="objects";
		objectDir/=sha12;
		objectDir/=sha128;
		
		char *data = new char[(long unsigned)hdr.size() + len];

		memcpy(data , hdr.data(), hdr.size());
		memcpy(data + hdr.size(), buf, len);

		//std::cout << objectDir << "\n";

		//write_file(objectDir, (char *)data ,);

		write_file(objectDir, data, hdr.size() + len );
		//delete data;
		//delete[] data;
	}
	return sha1;
}

std::string hash_blob_path(fs::path path, bool write) {
	char* buffer;
	long unsigned size = readFile(path, buffer);
	//std::cout << size << "\n";
	return hash_object("blob", buffer, size, write);
}

fs::path sha1_to_path(std::string sha1) {
	fs::path Dir(std::string{ '.' } +GIT_NAME);
	Dir /= "objects";
	std::string sha12 = sha1.substr(0, 2);
	std::string sha118 = sha1.substr(2, 38);
	fs::path dir1(sha12);
	fs::path dir2(sha118);
	Dir /= dir1;
	Dir /= dir2;
	return Dir;
}

/*
index
fileIndex(fs::path path, std::set<index>& indexSet) {
	index thisBlob;
	thisBlob.object_type = "blob";
	thisBlob.path = path;
	memcpy(thisBlob.sha1, hash_object("blob", readFile(path).c_str(), false), 20);

	//thisBlob.flag = ;
	//thisBlob.changetime = to_time_t(fs::last_write_time(path));
	thisBlob.size = fs::file_size(path);
	indexSet.emplace(thisBlob);
	return thisBlob;
}

index
fileTree(fs::path path ,std::set<index> &indexSet) {
	fs::directory_entry entry(path);		//文件入口
	if (entry.status().type() != fs::file_type::directory) {
		std::cerr << "Error: " << GIT_NAME << " is not a directory" << std::endl;
		return index{};
	}
	fs::directory_iterator list(path);
	std::vector<index> v;
	//int level = 1;
	std::cout << "start\n";
	for (auto& it : list) {
		auto filename = it.path().filename();
		if (fs::is_directory(it.status()))
		{
			//std::cout << std::lead << "[+] " << filename << "\n";
			//DisplayDirTree(entry, level + 1);
			v.push_back(fileTree(it,indexSet));
			std::cout << filename << "\n";
			//std::cout << "\n";
		}
		else {
			if (fs::is_regular_file(entry.status())) {
				std::cout << filename << "\n";
				v.push_back(fileIndex(it, indexSet));
				//DisplayFileInfo(entry, lead, filename);
				//v.push_back(fileIndex(it));
			}
			else {
				//std::cout << filename << "\n";
				//std::cerr << "Error: " << " is not a directory and is not a file" << std::endl;
			}
		}
	}
	return index();
	index thisTree;
	for (auto& vt : v) {

	}
	indexSet.emplace(thisTree);
	return thisTree;
}
*/
void
paths(fs::path path, std::unordered_set<std::optional<fs::path>, opt_path_hash>& Pathes) {
	//std::cout << "start\n";
	fs::directory_entry entry(path);		//文件入口
	if (entry.status().type() != fs::file_type::directory) {
		//std::cerr << "Error: " << GIT_NAME << " is not a directory" << std::endl;
		return;
	}
	fs::directory_iterator list(path);
	//int level = 1;
	//std::cout << "start\n";
	for (auto& it : list) {
		auto filename = it.path().filename();
		if (fs::is_directory(it.status())) {
			//std::cout << std::lead << "[+] " << filename << "\n";
			//DisplayDirTree(entry, level + 1);
			paths(it, Pathes);
			//std::cout << filename << "\n";
			//std::cout << "\n";
		}
		else {
			if (it.path().has_extension() && it.path().extension() == ".txt") { //修改 todo   fs::is_regular_file(it.status())
				Pathes.insert(it.path());
				//std::cout << filename << "\n";
				//v.push_back(fileIndex(it, indexSet));
				//DisplayFileInfo(entry, lead, filename);
				//v.push_back(fileIndex(it));
			}
			else {
				//std::cout << filename << "\n";
				//std::cerr << "Error: " << " is not a directory and is not a file" << std::endl;
			}
		}
	}
}
/*
int main(){
	//char* buffer;
	auto buffer =readFile("file.txt");
	std::cout << buffer <<"\n";
	unsigned char* sha1 = hash_object("blob", (const char*)buffer.c_str(), 0);
	for (int i = 0; i < 20; ++i) {
		printf("%02x", sha1[i]);
	}
	system("pause");
	return 0;
}
*/
