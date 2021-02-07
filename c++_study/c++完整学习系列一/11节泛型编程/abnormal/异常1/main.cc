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


class EStackEmpty  {  };  // 空的异常类：空栈异常类与满栈异常类
class EStackFull  {  };

int JuStack::Pop()
{
  if( IsEmpty() )   //  引发空栈异常，构造该异常类的一个对象并抛出
    throw EStackEmpty();
  --_top,  --_cnt;
  return _stk[_top];
}

void JuStack::Push( int value )
{
  if( IsFull() )   //  引发满栈异常，构造该异常类的一个对象并抛出
    throw EStackFull();
  _stk[_top] = value;
  _top++,  _cnt++;
}
const int err_stack_full = 1;

int main()
{
  JuStack stack(17);
  try
  {
    for( int i = 0; i < 32; i++ )
      stack.Push(i);    //  可能引发异常的函数调用
  }
  catch( const EStackFull & )    //  捕获抛出的异常，执行相应处理,只要try里面出现异常就到catch进行相应的处理
  {
    std::cerr << "Error: Stack full" << std::endl;
    return err_stack_full;
  }
  return 0;
}