#include "Thread.h"
#include <iostream>
using namespace std;

//变化的就是构造函数的参数和run函数
// 构造函数
Thread::Thread(const ThreadFunc& func) : func_(func), autoDelete_(false)
{
	cout<<"Thread..."<<endl;
}
Thread::~Thread()
{
	cout<<"~Thread..."<<endl;
}

// 创建线程
void Thread::Start()
{
    pthread_create(&threadId_, NULL, ThreadRoutine, this);
}

// 等待线程结束
void Thread::Join()
{
    pthread_join(threadId_, NULL);
}

// 线程入口函数，这些都不用变
void* Thread::ThreadRoutine(void* arg)
{//入口函数的参数是是第一个（隐式）参数this，就是对象本身的地址， 
    Thread* thread = static_cast<Thread*>(arg);//因此，基类指针指向了派生类对象
    thread->Run();//调用派生类实现的Run
    if (thread->autoDelete_)//自动销毁
        delete thread;
    return NULL;
}

// 设置自动销毁标识
void Thread::SetAutoDelete(bool autoDelete)
{
    autoDelete_ = autoDelete;
}

//不是纯虚函数要自己实现， Run方法就是直接调用func_()， 
void Thread::Run()
{
    func_();
}