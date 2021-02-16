#include <pthread.h>
//  线程函数
void *  ThreadFunc( void * arg )  {  ...  }
int  main()
{
  pthread_attr_t  attr;
  pthread_t  thread;
  //  初始化线程属性
  pthread_attr_init( &attr );
  //  设置线程属性的分离状态
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
  //  创建线程
  pthread_create( &thread, &attr, &ThreadFunc, NULL );
  //  清除线程属性对象
  pthread_attr_destroy( &attr );
  //  无需联结该线程
  return 0;
}