#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"
#include <unordered_set>
#include <iostream>
#include <string>
#include <vector>
#include <functional>

int mergeMain(int argc, const char* argv[]);

Command MergeCommand{
	mergeMain,
	"Join two or more development histories together"
};

//数据结构定义先

int mergeMain(int argc, const char* argv[]){
    cmdline::parser argParser;
    argParser.add<std::string>("message", 'm', "merge message", true, "");
    argParser.parse_check(argc, argv);
    // 参数解析 
    
    // 判断是否需要 merge 是否可以merege先
    // 1. 判断是否有冲突
    // 2. 判断是否有未提交的修改

    std::cout << "Join two or more development histories together" << std::endl;
    return 1;
}
