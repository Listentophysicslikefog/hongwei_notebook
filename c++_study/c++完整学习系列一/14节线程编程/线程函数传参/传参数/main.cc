#include <pthread.h>
#include <iostream>

class InfoPrinted
{
public:
  InfoPrinted( char c, int n ) : _c(c), _n(n)  {  }
  void  Show() const  {  for( int i = 0; i < _n; i++ )  std::cerr << _c;  }
private:
  char _c;
  int _n;
};

void *  PrintInfo( void * info )
{
  InfoPrinted *  p = reinterpret_cast<InfoPrinted *>( info );
  if( p )    p->Show();
  return NULL;
}
//  注意：本程序大部分情况下不会输出任何结果
/*
存在的问题：一般不会产生任何输出
子线程需要使用主线程的数据，如果主线程结束，子线程如何访问这些数据？
解决方案：使用pthread_join()函数，等待子线程结束
原型：int pthread_join( pthread_t thread, void ** retval );
参数：thread为pthread_t类型的线程ID；retval接收线程返回值，不需要接收返回值时传递NULL
*/
int  main()
{
  pthread_t  tid1, tid2;
  //  构造InfoPrinted类的动态对象，作为线程函数参数传递给线程tid1
  //  输出100个‘a’
  InfoPrinted *  p = new InfoPrinted( 'a', 100 );
  pthread_create( &tid1, NULL, &PrintInfo, reinterpret_cast<void *>( p ) );
  //  构造InfoPrinted类的动态对象，作为线程函数参数传递给线程tid2
  //  输出100个‘z’
  InfoPrinted *  q = new InfoPrinted( 'z', 100 );
  pthread_create( &tid2, NULL, &PrintInfo, reinterpret_cast<void *>( q ) );
  //  使用本注释行替换上述线程，可以看到输出结果，可能仅有部分输出
  //  PrintInfo( reinterpret_cast<void *>( q ) );

   //  等待子线程结束,这样就不会有上面那个没有输出的问题了
  pthread_join( tid1, NULL );
  pthread_join( tid2, NULL );

  return 0;
}