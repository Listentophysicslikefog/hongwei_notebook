1. log模块编译完成可以正常运行，需要安装protobufer

启动需要配置日志的路径环境变量

代码重点流程：read.md

更改点：glog的write函数是新版本的，当前cyber的重写函数参数不一样，这里更改了，编译方法更改为使用cmake

proto的他文件是自己根据protobufer编译的，到proto的msg未文件夹里面： protoc -I=./ --proto_path=. --cpp_out=./ *.proto



2. main_board编译运行完成

运行的时候报错./main_board: symbol lookup error: ./main_board: undefined symbol: _ZN8eprosima7fastcdr3Cdr9serializeEPKcm
需要链接so，这里的是cyber依赖的so ：export LD_LIBRARY_PATH=/home/midware/hongwei.liu/my_cyber/CyberRT/install/lib
然后就可以了

编译链接汇编的时候报错: permision denied是因为指定了编译汇编的执行文件错误了，去掉就可以了
去掉在个：set(CMAKE_ASM_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/../croutine/detail/")
解决这种编译问题看报错的具体cmake文件就可以了

原子变量需要链接atomic的so

fastdds编译相关报错解决需要使用cyber上传对应的版本头文件和so不能使用自己源码编译的不匹配的版本

编译时需要解压 nlohmann_json.zip
