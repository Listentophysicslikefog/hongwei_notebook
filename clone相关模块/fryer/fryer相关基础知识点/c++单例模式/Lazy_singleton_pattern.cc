/*什么是线程安全？
在拥有共享数据的多条线程并行执行的程序中，线程安全的代码会通过同步机制保证各个线程都可以正常且正确的执行，不会出现数据污染等意外情况。

如何保证线程安全？
给共享的资源加把锁，保证每个资源变量每时每刻至多被一个线程占用。
让线程也拥有资源，不用去共享进程中的资源。如： 使用threadlocal可以为每个线程的维护一个私有的本地变量。

什么是单例模式？
单例模式指在整个系统生命周期里，保证一个类只能产生一个实例，确保该类的唯一性。
单例模式分类
单例模式可以分为懒汉式和饿汉式，两者之间的区别在于创建实例的时间不同：
懒汉式：指系统运行中，实例并不存在，只有当需要使用该实例时，才会去创建并使用实例。（这种方式要考虑线程安全）
饿汉式：指系统一运行，就初始化创建实例，当需要时，直接调用即可。（本身就线程安全，没有多线程的问题）
单例类特点
构造函数和析构函数为private类型，目的禁止外部构造和析构
拷贝构造和赋值构造函数为private类型，目的是禁止外部拷贝和赋值，确保实例的唯一性
类里有个获取实例的静态函数，可以全局访问

01 普通懒汉式单例 （ 线程不安全 ）
*/
///////////////////  普通懒汉式实现 -- 线程不安全 //////////////////





// 编译方法: g++ -o lazy_mode lazy_mode.cc -std=c++11 -lpthread
#include <iostream> // std::cout
#include <mutex>    // std::mutex
#include <pthread.h> // pthread_create

class SingleInstance
{

public:
    // 获取单例对象
    static SingleInstance *GetInstance();

    // 释放单例，进程退出时调用
    static void deleteInstance();
	
	// 打印单例地址
    void Print();

private:
	// 将其构造和析构成为私有的, 禁止外部构造和析构
    SingleInstance();
    ~SingleInstance();

    // 将其拷贝构造和赋值构造成为私有函数, 禁止外部拷贝和赋值
    SingleInstance(const SingleInstance &signal);
    const SingleInstance &operator=(const SingleInstance &signal);

private:
    // 唯一单例对象指针
    static SingleInstance *m_SingleInstance;
};

//初始化静态成员变量
SingleInstance *SingleInstance::m_SingleInstance = NULL;

SingleInstance* SingleInstance::GetInstance()
{

	if (m_SingleInstance == NULL)
	{
		m_SingleInstance = new (std::nothrow) SingleInstance;  // 没有加锁是线程不安全的，当线程并发时会创建多个实例
	}

    return m_SingleInstance;
}

void SingleInstance::deleteInstance()
{
    if (m_SingleInstance)
    {
        delete m_SingleInstance;
        m_SingleInstance = NULL;
    }
}

void SingleInstance::Print()
{
	std::cout << "我的实例内存地址是:" << this << std::endl;
}

SingleInstance::SingleInstance()
{
    std::cout << "构造函数" << std::endl;
}

SingleInstance::~SingleInstance()
{
    std::cout << "析构函数" << std::endl;
}
///////////////////  普通懒汉式实现 -- 线程不安全  //////////////////

// 线程函数
void *PrintHello(void *threadid)
{
    // 主线程与子线程分离，两者相互不干涉，子线程结束同时子线程的资源自动回收
    pthread_detach(pthread_self());

    // 对传入的参数进行强制类型转换，由无类型指针变为整形数指针，然后再读取
    int tid = *((int *)threadid);

    std::cout << "Hi, 我是线程 ID:[" << tid << "]" << std::endl;

    // 打印实例地址
    SingleInstance::GetInstance()->Print();

    pthread_exit(NULL);
}

#define NUM_THREADS 5 // 线程个数

int main(void)
{
    pthread_t threads[NUM_THREADS] = {0};
    int indexes[NUM_THREADS] = {0}; // 用数组来保存i的值

    int ret = 0;
    int i = 0;

    std::cout << "main() : 开始 ... " << std::endl;

    for (i = 0; i < NUM_THREADS; i++)
    {
        std::cout << "main() : 创建线程:[" << i << "]" << std::endl;
        
		indexes[i] = i; //先保存i的值
		
        // 传入的时候必须强制转换为void* 类型，即无类型指针
        ret = pthread_create(&threads[i], NULL, PrintHello, (void *)&(indexes[i]));
        if (ret)
        {
            std::cout << "Error:无法创建线程," << ret << std::endl;
            exit(-1);
        }
    }

    // 手动释放单实例的资源
    SingleInstance::deleteInstance();
    std::cout << "main() : 结束! " << std::endl;
	
    return 0;
}

//普通懒汉式单例运行结果：
//从运行结果可知，单例构造函数创建了两个，内存地址分别为0x7f3c980008c0和0x7f3c900008c0，所以普通懒汉式单例只适合单进程不适合多线程，因为是线程不安全的。







