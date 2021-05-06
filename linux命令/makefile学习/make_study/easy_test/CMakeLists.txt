# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8) # 指定运行此配置文件所需的 CMake 的最低版本；

#项目名称
PROJECT(HELLO)

#定义变量
SET(SRC_LIST hello.cpp)

#打印用户的一些信息
MESSAGE(STATUS "This is BINARY dir " ${HELLO_BINARY_DIR})
MESSAGE(STATUS "This is SOURCE dir "${HELLO_SOURCE_DIR})

#生成可执行文件
ADD_EXECUTABLE(hello ${SRC_LIST})


# 在哪里执行cmake 对应的文件就会生成在哪里，只不过后面要指定CMakeLists.txt的路径即可
# CMakeLists.txt 的语法比较简单，由命令、注释和空格组成，其中命令是不区分大小写的。符号 # 后面的内容被认为是注释。命令由命令名称、小括号和参数组成，参数之间使用空格进行间隔。
# https://blog.csdn.net/qq_35703954/article/details/81771858
# https://blog.csdn.net/zhuiyunzhugang/article/details/88142908
# https://github.com/wzpan/cmake-demo
# https://www.hahack.com/codes/cmake/

#https://blog.csdn.net/qq_38410730/article/details/102502023

