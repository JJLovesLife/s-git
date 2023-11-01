#include "../s-git.h"
#include "../cache/file.h"
#include "../object/object.h"
#include <unordered_set>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <fstream>
namespace fs = std::filesystem;
int mergeMain(int argc, const char* argv[]);

struct MergeResult{
    std::vector<commit> parents;
    std::string sha1;
};
Command MergeCommand{
	mergeMain,
	"Join two or more development histories together"
};
// 定义数据结构

bool isAncestor(const commit& ancestor, const commit& child){
    //判断ancestor是否是child的祖先
    // 通过child的parent来判断
    if (child.sha1 == "")return false;
    if (ancestor.sha1 == child.sha1) {
        return true;
    }
    if (child.parent == "") {
        return false;
    }
    commit parentCommit;
    if (!readCommit(child.parent, parentCommit)) {
        return false;
    }
    return (isAncestor(ancestor,parentCommit));
}

//直接将当前分支指向mergeFromCommit
extern 
int checkout(std::string branch);

// 寻找最近公共祖先,简易版本
commit LCA(const commit& A, const commit& B){
    // 先计算深度
    int alength = commitLength(A);
    int blength = commitLength(B);
    commit currA,currB;
    if (alength > blength) {
        // 交换A,B
        currA = B;
        currB = A;
   
    }else{    
        currA = A;
        currB = B;
    }
    // 从B开始向上找
    for  (int i = 0;i < std::abs(alength - blength);i++) {
        currB = Parent(currB);
    }
    while (currA.sha1 != currB.sha1 && currA.sha1 != "" && currB.sha1 != "") {
        currA = Parent(currA);
        currB = Parent(currB);
    }
    if (currA.sha1 == currB.sha1) {
        return currA;
    }
}
std::string mergeContent(std::string based,std::string source,std::string curr){
    //逐行对比 通过换行符来分割

    std::vector<std::string> basedLines;
    std::vector<std::string> sourceLines;
    std::vector<std::string> currLines;
    std::string line;
    std::stringstream basedStream(based);
    std::stringstream sourceStream(source);
    std::stringstream currStream(curr);
    while (std::getline(basedStream,line)) {
        basedLines.push_back(line);
    }
    while (std::getline(sourceStream,line)) {
        sourceLines.push_back(line);
    }
    while (std::getline(currStream,line)) {
        currLines.push_back(line);
    }
    // 三路归并
    std::vector<std::string> mergedLines;
    //通过填补空行来实现三者行数对齐
    int basedLinesSize = basedLines.size();
    int sourceLinesSize = sourceLines.size();
    int currLinesSize = currLines.size();
    int maxLinesSize = std::max(basedLinesSize,std::max(sourceLinesSize,currLinesSize));
    for (int i = 0;i<maxLinesSize-basedLinesSize;i++) {
        basedLines.push_back("");
    }
    for (int i = 0;i<maxLinesSize-sourceLinesSize;i++) {
        sourceLines.push_back("");
    }
    for (int i = 0;i<maxLinesSize-currLinesSize;i++) {
        currLines.push_back("");
    }
    for (int i = 0;i < std::min(basedLines.size(),std::min(sourceLines.size(),currLines.size()));i++) {
        if (basedLines[i] == sourceLines[i] && sourceLines[i] == currLines[i]) {
            mergedLines.push_back(basedLines[i]);
        }else{
            // 三者不同
            // 1. 如果based和source相同，返回curr
            if (basedLines[i] == sourceLines[i]) {
                mergedLines.push_back(currLines[i]);
            }
            // 2. 如果based和curr相同，返回source
            if (basedLines[i] == currLines[i]) {
                mergedLines.push_back(sourceLines[i]);
            }
            // 3. 如果source和curr相同，返回source
            if (sourceLines[i] == currLines[i]) {
                mergedLines.push_back(sourceLines[i]);
            }
            // 4. 如果三者都不同，返回冲突
            mergedLines.push_back("<<<<<<<");
            mergedLines.push_back(basedLines[i]);
            mergedLines.push_back("=======");
            mergedLines.push_back(sourceLines[i]);
            mergedLines.push_back(">>>>>>>");
            mergedLines.push_back(currLines[i]);
        }
    }
    std::string mergedContent;
    for (auto &line : mergedLines) {
        mergedContent += line + "\n";
    }
    return mergedContent;
}

object mergeFile(object based,object source,object curr){
    if (based.object_type == "blob") {
        // 三者都是blob
        // 1. 如果三者都相同，直接返回
        if (based.sha1 == source.sha1 && source.sha1 == curr.sha1) {
            return based;
        }
        // 2. 如果based和source相同，返回curr
        if (based.sha1 == source.sha1) {
            return curr;
        }
        // 3. 如果based和curr相同，返回source
        if (based.sha1 == curr.sha1) {
            return source;
        }
        // 4. 如果source和curr相同，返回source
        if (source.sha1 == curr.sha1) {
            return source;
        }
        // 5. 如果三者都不同，返回冲突

        fs::path basedPath = sha1_to_path(based.sha1);
        auto basedbuffer = readFile(basedPath);
        fs::path sourcePath = sha1_to_path(source.sha1);
        auto sourcebuffer = readFile(sourcePath);
        fs::path currPath = sha1_to_path(curr.sha1);
        auto currbuffer = readFile(currPath);
        std::string mergedContent = mergeContent({basedbuffer.begin(),basedbuffer.end()},
                {sourcebuffer.begin(),sourcebuffer.end()},
                {currbuffer.begin(),currbuffer.end()});
        // 应该直接删除对应的blob文件，写下新的blob文件，然后直接复用build_tree这个提交commit的方法。
        write_file(curr.path,mergedContent.data(),mergedContent.size());

        std::string sha1 = hash_object("blob",mergedContent);
        object mergedObject;
        mergedObject.object_type = "blob";
        mergedObject.sha1 = sha1;
        mergedObject.path = based.path;
        return mergedObject;;
    }
}

int mergeMain(int argc, const char* argv[]){
    cmdline::parser argParser;
    argParser.add<std::string>("message", 'm', "merge message", true, "");
    argParser.footer("<merge from> ...");

    argParser.parse_check(argc, argv);
    // 首先获得当前分支的,
    if (argParser.rest().size() == 0) {
        std::cout << argParser.usage();
        return 0;
    }
    //从rest中获得需要merge的分支
    if (argParser.rest().size() > 1) {
        std::cout << "merge only support one branch" << std::endl;
        return 0;
    }
    std::string mergeFrom = argParser.rest()[0];
    //判断mergeFrom是否存在
    auto resolvebranch = resolveBranch(mergeFrom);
    std::string sha1 = resolvebranch.has_value() ? resolvebranch.value() : readBranch(mergeFrom);
    if (sha1.length() == 0) {
        std::cerr << "Error:" << " doesn't have this branch :" << mergeFrom << "\n";
        return 1;
    }
    commit mergeFromCommit ;
    readCommit(sha1,mergeFromCommit);
    // 这个部分可以重用吧，从branch名字读取到这个commit的sha1
	std::string currBranch = readBranchName();
    auto resolvecurrBranch = resolveBranch(currBranch);
    sha1 = resolvecurrBranch.has_value() ? resolvecurrBranch.value() : readBranch(currBranch);
    if (sha1.length() == 0) {
        std::cerr << "Error:" << " doesn't have this branch :" << currBranch << "\n";
        return 1;
    }
    commit currBranchCommit ;
    readCommit(sha1,currBranchCommit);
    
    //首先判断是否是同一个分支的路，通过commit的parent来判断
    std::cout << "Merge from " << mergeFrom << std::endl;
    std::cout << "On branch " << currBranch << std::endl;

    //判断两者是否互为祖先
    if (isAncestor(currBranchCommit,mergeFromCommit)) {
        std::cout << "Already up to date." << std::endl;
        return 0;
    }
    if (isAncestor(mergeFromCommit,currBranchCommit)) {
        std::cout << "Fast-forward" << std::endl;
        checkout(mergeFrom);
        return 0;
    }
    // 寻找最近公共祖先 //先假设是有祖先的情况
    commit ancestor = LCA(currBranchCommit,mergeFromCommit);
    std::cout << ancestor.message << std::endl;
    // 三路归并文件树
    // 1. 读取ancestor的文件树
    std::vector<object> ancestorEntries; 
    readTree(ROOT_DIR.value(), ancestor.tree, ancestorEntries); 
    // 将 ancestorEntries 转换为set
    std::unordered_set<object,object_hash, object_equal> ancestorSet;
    for (auto &entry : ancestorEntries) {
        ancestorSet.insert(entry);
    }
    for (auto &entry : ancestorSet) {
        std::cout << entry.path << std::endl;
    }
    std::cout << std::endl;
    std::vector<object> mergeFromEntries; 
    readTree(ROOT_DIR.value(), mergeFromCommit.tree, mergeFromEntries); 
     // 将 mergeFromEntries 转换为set
    std::unordered_set<object,object_hash, object_equal> mergeFromSet;
    for (auto &entry : mergeFromEntries) {
        mergeFromSet.insert(entry);
    }
    for (auto &entry : mergeFromEntries) {
        std::cout << entry.path << std::endl;
    }
    std::cout << std::endl;
    std::vector<object> currBranchEntries;
    readTree(ROOT_DIR.value(), currBranchCommit.tree, currBranchEntries);
    // 将 currBranchEntries 转换为set
    std::unordered_set<object,object_hash, object_equal> currBranchSet;
    for (auto &entry : currBranchEntries) {
        currBranchSet.insert(entry);
    }
    for (auto &entry : currBranchSet) {
        std::cout << entry.path << std::endl;
    }
    std::cout << std::endl;
    std::vector<object> mergedEntries;
    std::unordered_set<object,object_hash, object_equal> allSet;
    allSet.insert(ancestorSet.begin(),ancestorSet.end());
    allSet.insert(mergeFromSet.begin(),mergeFromSet.end());
    allSet.insert(currBranchSet.begin(),currBranchSet.end());
    for (auto &entry :allSet){
        auto ancestorenobject = ancestorSet.find(entry);
        auto mergeFromobject = mergeFromSet.find(entry);
        auto currBranchobject = currBranchSet.find(entry);
        if (mergeFromobject != mergeFromSet.end() && currBranchobject != currBranchSet.end()) {
            //三个地方都有
            // 其实要处理三者是否相同
            // 三路归并文件
            
           auto  mergedEntry = mergeFile(*ancestorenobject,*mergeFromobject,*currBranchobject);
            mergedEntries.push_back(entry);
         }
         if (mergeFromSet.find(entry) != mergeFromSet.end() && currBranchSet.find(entry) == currBranchSet.end()) {
             // mergeFrom有，currBranch没有

             mergedEntries.push_back(entry);
         }
         if (mergeFromSet.find(entry) == mergeFromSet.end() && currBranchSet.find(entry) != currBranchSet.end()) {
             // mergeFrom没有，currBranch有

             mergedEntries.push_back(entry);
         }
    }


    std::cout << "mergedEntries" << std::endl;
    for (auto &entry : mergedEntries) {
        std::cout << entry.path << std::endl;
    }
    // 将mergedEntries 建树 并提交一份新的commit
    commit newCommit;
    
    std::cout << "Join two or more development histories together" << std::endl;
    return 1;
}
