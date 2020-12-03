# build
本程序基于CMake进行构建  

##参考build指令  
### VS
使用PowerShell运行下面命令（或者使用GUI完成相同命令）  
```powershell
mkdir vs_build
cd vs_build
cmake -G "Visual Studio 15 2017 Win64" ..
./S-GIT.sln
```

### GNU(g++)
注： g++版本需高于8  
运行下面命令  
```shell
mkdir gnu_build
cd gnu_build
cmake ..
make
```
