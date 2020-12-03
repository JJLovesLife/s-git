新建空测试目录  
运行如下命令进行测试  
命令中`s-git`指定生成的可执行文件  

```shell
# input
s-git init
# output
# Initialized empty s-git repository in ".s-git"
```
check: `.s-git/HEAD`文件  

```shell
# input
s-git status
# output
# No commits yet
# Untracked files:
```
check: `No commits yet`  

```shell
# input
touch a.txt
mkdir dir
touch dir/b.txt
s-git status
# output
# No commits yet
# Untracked files:
#         a.txt
#         dir/b.txt
```
check: 两个Untracked文件，路径目录输出  

```shell
# input
s-git commit -m "first"
# output
# first commit
# [main (root-commit) 58fb12d]
# 2 file inserted(+)
#  a.txt
#  dir/b.txt
```
check: first, root, 2 inserted  

```shell
# input
s-git branch master
# output
# Added branch 'master'
```
check: Added  

```shell
# input
s-git branch -l
# output
# main
# master
```
check: main, master  

```shell
# input
s-git tag first
# output
# Added tag 'first'
```
check: Added  

```shell
# input
s-git tag -l
# output
# first
```
check: first  

```shell
# input
echo test > a.txt
rm dir/b.txt
mkdir dir2
mkdir dir2/ignore
touch dir2/ignore/.s-gitignore
touch dir2/c.txt
s-git status
# output
# On branch main
# changed files:
#         a.txt
# new files:
#         dir2/c.txt
# delete files:
#         dir/b.txt
```
check:  

1. On branch main  
2. changed: a.txt  
3. new: dir2/c.txt  
4. delete: dir/b.txt  
5. .s-gitignore已忽略  

```shell
# input
s-git commit -m "second"
# output
# On branch main
# changed files:
#         a.txt
# new files:
#         dir2/c.txt
# delete files:
#         dir/b.txt
# [main f8f861e]
#  1 files changed, 1 files inserted(+),  1 files deleted(-)
```
check: 1 changed, 1 inserted, 1 deleted  

```shell
# input
s-git log
# output
# [commit f8f861e946ea71e303e901d7df8cc7300fab55a2]
# message:
# 
# second
# 
# [commit 58fb12da1a23798537c3fcea0649bac821b2072b]
# message:
# 
# first
# 
```
check: message: second; first  

```shell
# input
s-git checkout -b master
# output
# Switched to branch 'master'
```
check:  

1. 增加dir/b.txt, a.txt  
2. 删除c.txt  
3. dir2, dir2/ignore, dir2/ignore/.s-gitignore保留  

```shell
# input
s-git branch -d master
# output
# On branch master now, unable to delete.
```
check: unable  

```shell
# input
s-git checkout -t first
# output
# HEAD detached at58fb12da1a23798537c3fcea0649bac821b2072b
```
check: 文件无更改  

```shell
# input
s-git status
# output
# Head detached at 58fb12da1a23798537c3fcea0649bac821b2072b
# changed files:
# new files:
# delete files:
# nothing to commit, working tree clean
```
check: Head detached  

```shell
# input
s-git tag -d first
# output
# Deleted tag 'first'

# input
s-git tag -l
# output
# 
```
check: deleted; 无tag  

```shell
# input
s-git branch -d master
# output
# Deleted branch 'master'

# input
s-git branch -l
# output
# main
```
check: deleted; main  

```shell
# input
s-git checkout -b main
# output
# Switched to branch 'main'
```
check: dir文件夹被删除  

```shell
# input
s-git log
# output
# [commit f8f861e946ea71e303e901d7df8cc7300fab55a2]
# message:
# 
# second
# 
# [commit 58fb12da1a23798537c3fcea0649bac821b2072b]
# message:
# 
# first
# 
```
check: message: second; first  

```shell
# input
s-git checkout -s 58fb12da1a23798537c3fcea0649bac821b2072b
# output
# HEAD detached at58fb12da1a23798537c3fcea0649bac821b2072b
```
check: 文件结构  

```shell
# input
s-git checkout -b main~1
# output
# Switched to branch 'main~1'
```
check: 文件结构无改变  
