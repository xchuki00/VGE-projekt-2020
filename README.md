###Brno University of Technology 
####Faculty of Information Technology

FYO - school project 
2019/2020
 
Author:
 
    Patrik Chukir 
    chuki00@stud.fit.vutbr.cz || p.chukir@gmail.com
   
This projekt is based on 
   * NanoGui - [https://github.com/wjakob/nanogui](https://github.com/wjakob/nanogui)
   * NanoGui test - [https://github.com/darrenmothersele/nanogui-test](https://github.com/darrenmothersele/nanogui-test)
   
Build on Linux:
```
git clone --recursive https://github.com/xchuki00/FyoProject.git
cd FyoProject
mkdir build
cd build
cmake ../
make
```

if cmake return error about missing resource/library/.... try
```
git submodule update --init --recursive
```

For windows I add already compilede binaries, so you can simple download as zip and in directory win you finde exe file and nanogui library. Downloading take some time, library isnt small.

