#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"
#include <unordered_set>
#include <iostream>
#include <string>
#include <vector>
#include <functional>

int merge(int argc, const char* argv[]);

Command MergeCommand{
	merge,
	"Join two or more development histories together"
};

int merge(int argc, const char* argv[]){
    std::cout << "Join two or more development histories together" << std::endl;
    return 1;
}
