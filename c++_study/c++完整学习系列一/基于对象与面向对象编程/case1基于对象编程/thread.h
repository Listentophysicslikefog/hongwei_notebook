#ifndef _THREAD_H_
#define _THREAD_H_

#include <pthread.h>
#include <boost/function.hpp>

class Thread
{
public:
    typedef boost::function<void ()> ThreadFunc; //⭐一种新的类型 1⭐
    explicit  (const ThreadFunc& func);  // 单参数用explicit阻止隐式的转换构造，只能显示调用   3  那多了这个函数的话就去cpp文件中实现一下
    //传进去的是一个(未定的)新类型，⭐这跟重要！！！为后面做铺垫

    void Start();//创建线程 pthread_create()出处
    void Join();

    void SetAutoDelete(bool autoDelete);

private:
    static void* ThreadRoutine(void* arg);
    void Run();//上一个是虚函数，这个就不用了
    ThreadFunc func_;//用run方法调用func_方法， 这个方法在线程的构造函数中要传递进来   2
    pthread_t threadId_;
    bool autoDelete_;
};

#endif // _THREAD_H_