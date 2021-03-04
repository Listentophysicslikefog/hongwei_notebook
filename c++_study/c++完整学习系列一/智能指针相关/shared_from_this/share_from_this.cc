#include<iostream>
#include<memory>
#include<stdlib.h>
using namespace std;


/*
参考博客：https://www.jianshu.com/p/4444923d79bd
如果一个T类型的对象t，是被std::shared_ptr管理的，且类型T继承自std::enable_shared_from_this，那么T就有个shared_from_this的成员函数，这个函数返回一个新的std::shared_ptr的对象，也指向对象t。
那么这个特性的应用场景是什么呢？一个主要的场景是保证异步回调函数中操作的对象仍然有效。
*/

class Foo : public std::enable_shared_from_this<Foo>
{
   public:
     void Bar(std::function<void(Foo*)> p_fnCallback) //这个函数Bar需要的参数是一个函数，并且需要的函数的参数是Foo*类型的。其实要的就是Foo::Bar的this指针作为Bar的参数
           {


           }
     std::shared_ptr<Foo> getptr()
      {
        return shared_from_this();
      }

    ~Foo()
     {
      cout<<"~Foo()"<<endl;

     }
};

int main()
{

  // 大括号用于限制作用域，这样智能指针就能在system("pause")之前析构
  {
   shared_ptr<Foo> gp1(new Foo());
   shared_ptr<Foo> gp2=gp1->getptr();
  //打印引用计数
  cout<<"gp1.use_count()="<<gp1.use_count()<<endl;
  cout<<"gp2.use_count()="<<gp2.use_count()<<endl;

   }
getchar();
return 0;
}
