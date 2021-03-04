#include <iostream>
using namespace std;

int f( void* p )
{
  unsigned int n = reinterpret_cast<unsigned int>( p );
  return n;
}
// 将任意型式的数据对象转型为目标型式，即重新解释其位序列的意义
int main()
{
  int a[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  int n = f( a );
  cout << n << endl;
}