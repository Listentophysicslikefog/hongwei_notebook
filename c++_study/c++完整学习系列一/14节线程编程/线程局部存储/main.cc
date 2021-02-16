#include <pthread.h>
#include <stdio.h>
static pthread_key_t  tlk;    //  关联线程日志文件指针的键
void  WriteToThreadLog( const char * msg )
{
  FILE *  fp = ( FILE * )pthread_getspecific( tlk );  //通过tlk键值获取文件指针
  fprintf( fp, "%d: %s\n", (int)pthread_self(), msg );  //将信息存放到文件里面
}
void  CloseThreadLog( void * fp )
{
  fclose( ( FILE * )fp );
}
void *  ThreadFunc( void * args )
{
  char  filename[255];
  FILE *  fp;
  //  生成与线程ID配套的日志文件名，不同的线程的日志文件会存储到不同的文件里面
  sprintf( filename, "thread%d.log", (int)pthread_self() );
  fp = fopen( filename, "w" );
  //  设置线程日志文件指针与键的局部存储关联
  pthread_setspecific( tlk, fp );  //将fp(创建的文件的指针)和tlk就是对应的键关联起来
  //  向日志中写入数据，不同的线程会写入不同的文件
  WriteToThreadLog( "Thread starting..." );
  return NULL;
}
int  main()
{
  int  i;
  pthread_t  threads[8];
  //  创建键，使用CloseThreadLog()函数作为其清除程序  后面会调用这个函数
  pthread_key_create( &tlk, CloseThreadLog );  //如果想让pthread_getspecific、pthread_setspecific工作那么就需要在线程创建之前完成线程局部存储键值的创建
  for( i = 0; i < 8; ++i )
    pthread_create( &threads[i], NULL, ThreadFunc, NULL );
  for( i = 0; i < 8; ++i )
    pthread_join( threads[i], NULL );
  pthread_key_delete( tlk );   //删除键
  return 0;
}

//启动一系列线程，每个线程都有自己独立的日志文件