#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
 // 编译  g++ -o test test.cc  -std=c++11 -lpthread 
struct Test
{
      Test() { std::cout << "  Test::Test()\n"; }
     ~Test() { std::cout << "  Test::~Test()\n"; }
 };
 

 void thr(std::shared_ptr<Test> p) //线程函数
 {
     
     std::this_thread::sleep_for(std::chrono::seconds(1));  //线程暂停1s
 
     
     std::shared_ptr<Test> lp = p;  //赋值操作, shared_ptr引用计数use_cont加1(c++11中是原子操作)
     {   
         
         static std::mutex io_mutex;  //static变量(单例模式),多线程同步用
 
        
         std::lock_guard<std::mutex> lk(io_mutex);  //std::lock_guard加锁
         std::cout << "local pointer in a thread:\n"
                   << "  lp.get() = " << lp.get()
                   << ", lp.use_count() = " << lp.use_count() << '\n';
     }   
 }
 
 int main()
 {
     
     std::shared_ptr<Test> p = std::make_shared<Test>();  //使用make_shared一次分配好需要内存  //std::shared_ptr<Test> p(new Test);
     
     std::cout << "Created a shared Test\n"
               << "  p.get() = " << p.get()
               << ", p.use_count() = " << p.use_count() << '\n';
 
     std::thread t1(thr, p), t2(thr, p), t3(thr, p);  //创建三个线程,t1,t2,t3  //形参作为拷贝, 引用计数也会加1 线程函数里面lp赋值操作引用计数加一 所以最多是6
     std::cout << "Shared ownership between 3 threads and released\n"
               << "ownership from main:\n"
               << "  p.get() = " << p.get()
               << ", p.use_count() = " << p.use_count() << '\n';
     
     t1.join(); t2.join(); t3.join();  //等待结束
     std::cout << "All threads completed, the last one deleted\n";
 
     return 0;
 }