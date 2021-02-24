#include "Thread.h"
#include <boost/bind.hpp>
#include <unistd.h>
#include <iostream>
using namespace std;

class Foo
{//不用继承，也不用覆盖run方法
public:
    Foo(int count) : count_(count)
    {
    }

    void MemberFun()
    {
        while (count_--)
        {
            cout<<"this is a test ..."<<endl;
            sleep(1);
        }
    }

    void MemberFun2(int x)
    {
        while (count_--)
        {
            cout<<"x="<<x<<" this is a test2 ..."<<endl;
            sleep(1);
        }
    }

    int count_;
}; //why this class

void ThreadFunc()//提供一个线程函数， 开始定义的typedef boost::function<void ()> ThreadFunc
{
    cout<<"ThreadFunc ..."<<endl;
}

void ThreadFunc2(int count)
{
    while (count--)
    {
        cout<<"ThreadFunc2 ..."<<endl;
        sleep(1);
    }
}

int main()
{
    // 调用普通函数
    Thread t1(ThreadFunc);//创建 一个线程对象，把ThreadFunc传递进来
    /*
    t1.start();
    t1.join();
    */
    
    //typedef boost::function<void ()> ThreadFunc
    Thread t2(boost::bind(ThreadFunc2, 3));//实际执行ThreadFunc2这个函数
    
    // 调用成员函数
    Foo foo(3);//count_ = 3
    Thread t3(boost::bind(&Foo::MemberFun, &foo));//分别是指向成员函数的指针, 绑定到this的对象，无形参
    Foo foo2(3);
    Thread t4(boost::bind(&Foo::MemberFun2, &foo2, 1000));//一个形参

    t1.Start();
    t2.Start();
    t3.Start();
    t4.Start();

    t1.Join();
    t2.Join();
    t3.Join();
    t4.Join();
//这是三个线程交替执行的
    return 0;
}

/*
使用基于对象的编程风格来封装一个线程类
面向对象中封装了一个Thread的抽象类， 基于对象使用的是具体类：
接口大致上还是一样的：启动线程Start(), 等待Join()，线程的执行体Run()， 线程的入口函数ThreadRoutine()去调用线程的执行体
🌂多一个ThreadFunc函数， 定义了一个boost::function ThreadFunc（所以是适配函数咯），这个适配函数正是类图中线程构造函数中传递的线程函数， 也就是说线程实际执行的函数是这个ThreadFun函数，可想而知这个函数要在Run方法中调用
面向对象中，实现一个具体的线程类是覆盖Run方法， 在Run方法中提供这个线程类的具体实现，但这边基于对象的具体实现是通过TreadFunx传递进来的，只需要通过boost::bind转换成boost::function

用例子总结：
实现C编程风格下，注册三个全局函数到网络库，网络库通过函数指针来回调：
面向对象风格： 用一个EchoServer 继承 TcpServer这个抽象类，再去实现各自的三个接口函数
基于对象风格： 用一个EchoServer 包含 一个TcpServer这个具体类对象， 在构造函数中用boost::bind来注册三个成员函数

*/