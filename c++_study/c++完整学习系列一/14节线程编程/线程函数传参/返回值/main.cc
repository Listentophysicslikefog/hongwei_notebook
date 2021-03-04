#include <pthread.h>
#include <cmath>
#include <iostream>

void *  IsPrime( void * n )  //判断一个整数是否为素数
{
  unsigned int  p = reinterpret_cast<unsigned int>( n );
  unsigned int  i = 3u, t = (unsigned int)sqrt( p ) + 1u;
  if( p == 2u )
    return reinterpret_cast<void *>( true );
  if( p % 2u == 0u )
    return reinterpret_cast<void *>( false );
  while( i <= t )
  {
    if( p % i == 0u )
      return reinterpret_cast<void *>( false );
    i += 2u;
  }
  return reinterpret_cast<void *>( true );
}

//  使用g++ main.cpp –pthread –lm –fpermissive编译
//  以防止编译器将void*到int的转型当作错误
int  main()
{
  pthread_t  tids[8];
  bool  primalities[8];
  int i;
  for( i = 0; i < 8; i++ )  //创建8个线程
    pthread_create( &tids[i], NULL, &IsPrime, reinterpret_cast<void *>( i+2 ) );
  for( i = 0; i < 8; i++ )
    pthread_join( tids[i], reinterpret_cast<void **>( &primalities[i] ) );// pthread_join函数第一个参数： 线程id  第二个参数：线程返回值   这里是亚型指针的指针：表示 把第i号元就是存放线程数组的里第i的一个的地址取一个&然后转型为void**作为参数传进去
  for( i = 0; i < 8; i++ )
    std::cout << primalities[i] << " ";
  std::cout << std::endl;
  return 0;
}