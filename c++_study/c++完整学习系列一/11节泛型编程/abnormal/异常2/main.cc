#include<iostream>
using namespace std;

class JuStack
{
public:
  JuStack( int cap ) : _stk(new int[cap+1]), _cap(cap), _cnt(0), _top(0)  {  }
  virtual ~JuStack()  {  if( _stk )  delete _stk, _stk = NULL;   }
public:
  int Pop();
  void Push( int value );
  bool IsFull() const  {  return _cap == _cnt;  }
  bool IsEmpty() const  {  return _cnt == 0;  }
  int GetCapacity() const  {  return _cap;  }
  int GetCount() const  {  return _cnt;  }
private:
  int * _stk;
  int   _cap,  _cnt,  _top;
}; 


class EStackFull // 精心设计异常类，提供必要的异常信息
{
public:
  EStackFull( int i ) : _value( i )  {  }
  int GetValue()  {  return _value;  }    //int GetValue()const  {  return _value;  }   这样才不会报错
private:                                  //报错原因  参考博客：https://blog.csdn.net/snail_running/article/details/50705437
  int _value;
};

void JuStack::Push( int value )
{
  if( IsFull() )
    throw EStackFull( value );    //  使用value构造异常类对象并抛出    if条件包含下面一句  if满足才走throw异常  走到异常的流程
  _stk[_top] = value;
  _top++,  _cnt++;
}
const int err_stack_full = 1;

int main()
{
  JuStack stack( 17 );
  try
  {
    for( int i = 0; i < 32; i++ )
      stack.Push( i );
  }
  catch( const EStackFull & e ) //处理异常的类型   这里表示只处理 EStackFull类(就是throw扔出的 类型的异常 ) //  使用异常对象获取详细信息  异常的对象会记录发生异常时的详细信息
  {
    std::cerr << "Stack full when trying to push " << e.GetValue() << std::endl;
    return err_stack_full;
  }
  return 0;
}