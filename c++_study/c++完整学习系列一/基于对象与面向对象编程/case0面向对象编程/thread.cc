#include "Thread.h"
#include<iostream>
using namespace std;

Thread::Thread()
{
//...
}
Thread::~Thread()
{
//...
}
void Thread::Start()
{
	
	//第三个参数就是线程的入口函数，但是注意不能直接把Run()放进来
	pthread_create(&threadId_, NULL, ThreadRoutine, this);
	//那这边我们所创建的线程的线程入口函数是ThreadRoutine，并不是我们真正要执行的run， 因此我们要在ThreadRoutine里调用run方法
}
void Thread::Join()
{
	pthread_join(threadId_, NULL);
}
//
void* Thread::ThreadRoutine(void* arg)//静态成员函数
{
	//Run();//但也不能直接调用啦， 因为ThreadRoutine是静态成员函数，不能调用非静态的
	//可以靠传递过来的this⭐妙
	Thread* thread = static_cats<Thread*>(arg);//this指针转换成基类指针，
	//到时候this指针一定是一个派生类对象的指针，现在将基类指针指向派生类对象
	//并且调用Run函数
	thread->Run();
	return NULL;
}