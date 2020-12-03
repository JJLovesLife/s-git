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

## 版本历史
- v0.1.0  
	init, status, commit commands  
- v0.2.0  
	allow working in subdirectory of repository  
- v0.3.0  
	improve status, commit output  
	add ".s-gitignore" mechanism  
- v0.4.0  
	add tag supports with tag command  
- v0.5.0  
	add branch supportes with branch command  
	add checkout command, now we can go back to the past  
	add log command to check history  
- v0.6.0  
	add revision support in checkout command  
