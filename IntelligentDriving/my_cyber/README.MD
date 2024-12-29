log模块编译完成可以正常运行，需要安装protobufer

启动需要配置日志的路径环境变量

代码重点流程：read.md

更改点：glog的write函数是新版本的，当前cyber的重写函数参数不一样，这里更改了，编译方法更改为使用cmake

proto的他文件是自己根据protobufer编译的，到proto的msg未文件夹里面： protoc -I=./ --proto_path=. --cpp_out=./ *.proto
