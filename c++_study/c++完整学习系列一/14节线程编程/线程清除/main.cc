#include <malloc.h>
#include <pthread.h>
void *  AllocateBuffer( size_t size )
{
  return malloc( size );
}
void  DeallocateBuffer( void * buffer )
{
  free( buffer );
}
void  DoSomeWork()
{
  void *  temp_buffer = AllocateBuffer( 1024 );
  //  注册清除处理函数
  pthread_cleanup_push( DeallocateBuffer, temp_buffer );
  //  此处可以调用pthread_exit()退出线程或者撤销线程
  //  取消注册，传递非0值，实施清除任务
  pthread_cleanup_pop( 1 );
}