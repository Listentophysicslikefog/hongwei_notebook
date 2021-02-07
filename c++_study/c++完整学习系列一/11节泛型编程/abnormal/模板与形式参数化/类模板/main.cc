#include <iostream>
#include <cstdlib>

class EQueueEmpty  {  };  //  空队列异常类

template< typename T >  class JuQueueItem;  //  队列项类前置声明

template< typename T >  class JuQueue  //  队列类
{
public:
  JuQueue(): _head(NULLL), _tail(NULL)  {  }
  virtual ~JuQueue();
  virtual void Enter( const T & item );
  virtual T Leave();
  bool IsEmpty() const  { return _head == 0;  }
private:
  JuQueueItem<T> *_head, *_tail;
};

template< typename T >  class JuQueueItem  //  队列项类，单向链表结构
{
  friend class JuQueue<T>;
public:
  JuQueueItem( const T & item ) : _item(item), _next(0)  {  }
private:
  T _item;
  JuQueueItem<T> * _next;
};


template< typename T >  JuQueue<T>::~JuQueue()  //  队列类析构函数
{
  while( !IsEmpty() )
    Leave();
}

template< typename T >  void JuQueue<T>::Enter( const T & item )  //  入队
{
  JuQueueItem<T> * p = new JuQueueItem<T>( item );
  if( IsEmpty() )    _head = _tail = p;
  else    _tail->_next = p,  _tail = p;
}


template< typename T >  T JuQueue<T>::Leave()  //  出列
{
  if( IsEmpty() )    throw EQueueEmpty();
  JuQueueItem<T> * p = _head;
  T _retval = p->_item;
  _head = _head->_next;
  delete p;
  return _retval;
}

int main()
{
  JuQueue<int> * p = new JuQueue<int>;
  for( int i =0; i < 10; i++ )
    p->Enter( i );
  std::cout << p->Leave() << std::endl;

  int * r = new int(10), * q = new int(20);
  JuQueue<int*> * t = new JuQueue<int*>;
  t->Enter( r );
  t->Enter( q );
  int * s = t->Leave();
  std::cout << *s << std::endl;

  return 0;
}