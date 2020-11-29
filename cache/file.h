#pragma once
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

void
write_file(fs::path dir, const char* data, int len);

long unsigned
readFile(fs:: path dir,char *  &buffer);

void
write_file(const fs::path &path, const std::vector<char> &data);

std::vector<char>
readFile(const fs::path &path);

int find(char* data, char t, int size);

std::string readMain();

int writeMain(std::string sha1);
