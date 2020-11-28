#pragma once
#include <filesystem>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;
void
write_file(fs::path dir, char* data, int len);

long unsigned
readFile(fs:: path dir,char *  &buffer);

int find(char* data, char t, int size);

std::string readMain();

int writeMain(std::string sha1);
