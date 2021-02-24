#ifndef _THREAD_H_
#define _THREAD_H_

#include<pthread.h>

class Thread
{
public:
	Thread();
	virtual ~Thread();//要纯虚哦
	
	void Start();
	void Join();
private:
	static void* ThreadRoutine(void* arg);//不能直接在创建线程时把run作为函数入口， 用这个静态函数开个路
	virtual void Run() = 0;	//纯虚函数
	pthread_t threadId_;
};
#endif //_THREAD_H_