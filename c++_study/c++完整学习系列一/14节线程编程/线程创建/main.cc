#include <pthread.h>
#include <iostream>
void *  PrintAs( void * unused )
{
  while( true )    std::cerr << 'a';
  return NULL;
}
void *  PrintZs( void * unused )
{
  while( true )    std::cerr << 'z';
  return NULL;
}
int  main()
{
  pthread_t  thread_id;
  pthread_create( &thread_id, NULL, &PrintAs, NULL );
  PrintZs( NULL );   //main主线程里的一个函数
  return 0;
}