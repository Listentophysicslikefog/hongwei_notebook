#include "Thread.h"
#include<iostream>
#include<unistd.h>
#include<unistd.h>
using namespace std;

class TestThread : public Thread//继承
{
public:
	TestThread(int count) : count_(count)
	{
		cout<<"Test..."<<endl;
	}	
	~TestThread()
	{
		cout<<"~TestThread..."<<endl;
	}
	
	viod Run()//实现run方法，这个抽象的方法
	{
		//假设在函数中打印输出，那么会希望它传递一个count参数
		//虽然方法没有参数，但是这个类可以有一些状态， 相当于成员变量啦
		while(count_--)
		{
			cout<<"this is a test..."<<endl;
			sleep(1);//睡眠函数<unistd.h>
		}
	}//这就是实际线程的执行体函数
	
	int count_;//成员变量还可以有多个，就相当于成员变量的参数了，反正可以访问,在构造函数中传进去并初始化就ok
};

int main(void)
{
	//不能直接使用基类，要使用派生类实例化一个对象
	TestThread t(5);
	t.Start();
	t.Join();//避免主线程结束了子线程还没有运行的情况，主线程进入等待池
	
	return 0;
}



/*


另外需要注意pthread的运行需要去链接线程库，在cmake中，要target_link_libraries(Thread_test_pthread)，链接这个库
为什么不能直接把Run函数作为线程的入口函数
因为Run是普通的成员函数，隐含的第一个参数是Thread* (this)，即会把this指针传递进来，调用的时候是thiscall约定，而thiscall约定呢，某一个寄存器会保存this指针，而int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void*), void *restrict arg);----> (传出一个线程的id，线程的属性默认属性就用空指针，线程的入口函数，参数)
*start_routine是一个普通的函数调用约定， 所以run函数不能作为它的入口函数(?)，
解决方法：编写一个这种接口的void *(*start_routine)(void*)全局的函数肯定可以解决，但是全局的话很多人都看的到，所以还是作为一个成员函数ThreadRoutine， 加一个静态，这样函数就没有一个隐含的this指针了，在调用的时候就不会传递一个隐含的this指针，
那另一个问题就是线程函数的参数要怎么传递过来：就用this指针，指向当前第项自身的指针
关于thiscall：thiscall不是关键字。它是C++类成员函数缺省的调用约定，即如果参数个数确定，this指针通过ecx传递给被调用者；如果参数个数不确定，this指针在所有参数压栈后被压入堆栈；

总结这部分代码：（虚函数多态的体现
⭐一个派生类对象t，调用t.Start()，t的第一个参数隐含的是一个this指针，相当于就是取地址t，&t，
Start函数中又调用ThreadRoutine，参数是this指针指向对象自身，也就是指向了取地址t，即ThreadRoutine函数中的参数arg就是&t， 第一行代码Thread* thread = static_cast<Thread*>(arg)就成功完成了“基类指针指向派生类对象”,然后再通过基类指针调用的应该是派生类所实现的虚函数；这使用到了虚函数的多态
那么如果线程类是库实现的， 现在创建了一个线程就相当于是线程库里面回调了Run方法，基类指针指向了派生类对象，调用的是派生类的Run接口
也就是说虚函数具有回调的功能
为什么要把Run()放在private：
❗❗❗❗❗
如果run没有做成private， 然后直接创建实例对象t，然后跑t.Run()，可以跑起来，但是t.Run() 本质上还是在主线程中执行，并不能够满足“是创建了一个线程，成为一个线程的执行体函数”，这不算一个线程， 所以一定要调用Start，内部才会创建一个线程，让线程回调这个执行体函数，
所以，不应该直接调用它，所以就不可以把它放到public中
线程对象的生命周期与线程的生命周期
这两者是不一样的，线程执行完毕线程对象不一定被销毁， 比如：在线程执行完之后，如果再sleep10秒，那么线程对象其实在这个时间里其实还是存在的，线程对象要等程序运行完才会销毁
但还是可以实现线程执行完毕，线程对象自动销毁：
：那就要delete去销毁, 那么必须要动态创建对象才可以
可以在.h中增加一个属性：bool autoDelete_;是否自动销毁，构造函数的时候也相应的要去初始化这个属性， 对应增加一个函数void SetAutoDelete(bool autoDelete);
int main(void)
{
	TestThread* t2 = new TestThread(5);
	t2->SetAutoDelete(true);//在start之前定义一下
	t2->Start();
	t2->Join();	
	for(; ;)
		pause();//线程结束，但线程对象还没有结束
	return 0;
}


void Thread::SetAutoDelete(bool autoDelete)
{
	autoDelete_ = autoDelete;
}
void* Thread::ThreadRoutine(void* arg)
{
	Thread* thread = statiic_cast<Thread*>(arg);
	thread->Run();
	if(thread->autoDelete_)
	{
		delete thread;//这下，当线程的执行体Run执行完毕，就可以把线程对象delete销毁掉了
	}
	return NULL;
}

*/