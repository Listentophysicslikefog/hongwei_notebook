#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
std::mutex  x;
std::condition_variable  cond;
bool  ready = false;
bool  IsReady()  {  return ready;  }
void  Run( int no )
{
  std::unique_lock<std::mutex>  locker( x );
  while( !ready )		//  若标志位非true，阻塞当前线程
    cond.wait( locker );	//  解锁并睡眠，被唤醒后重新加锁
  //  以上两行代码等价于cond.wait( locker, &IsReady );
  //  第二个参数为谓词，亦可使用函子实现
  std::cout << "thread " << no << '\n';
}
int  main()
{
  std::thread  threads[8];
  for( int i = 0; i < 8; ++i )
    threads[i] = std::thread( Run, i );
  std::cout << "8 threads ready...\n";
  {
    std::unique_lock<std::mutex>  locker( x );    //  互斥加锁
    ready = true;		//  设置全局标志位为true
    cond.notify_all();	//  唤醒所有线程
  }    //  离开作用域，自动解锁；可将此复合语句块实现为函数
  //  基于区间的循环结构，对属于threads数组的所有元素t，执行循环体
  for( auto & t: threads )
    t.join();
  return 0;
}