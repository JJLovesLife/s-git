#pragma once
#include <filesystem>
#include <vector>
#include <string>

void
write_file(const std::filesystem::path &path, const char* data, size_t len);

void
write_file(const std::filesystem::path &path, const std::vector<char> &data);

void
write_object(const std::filesystem::path &path, const std::string &header, const std::vector<char> &data);

void
write_object(const std::filesystem::path &path, const std::string &header, const std::string &message);

std::vector<char>
readFile(const std::filesystem::path &path);

std::string readMain();

bool writeMain(const std::string &sha1);
