#include <iostream>
#include <boost/function.hpp>// 需包含的对应头文件
#include <boost/bind.hpp>

using namespace std;

class Foo
{
public:
    void memberFunc(double d, int i, int j)
    {
        cout << d << endl; 
        cout << i << endl;    
        cout << j << endl; 
    }
};

int main()
{
    Foo foo;
    boost::function<void (int)> fp = boost::bind(&Foo::memberFunc, &foo, 0.5, _1, 10);//把一个成员函数适配成新的函数类型
    //boost::function<void(int)>：这个类型可以看成是：“返回值void， 接收一个int类型的参数” 这样的一个接口， 
    //原来的函数memberFun有三个参数，准确说是4个参数，在函数调用的时候隐含一个this指针，而我们适配成的只有1个参数，
   //调用适配函数 fp(100), 实际上调用的函数原来的memberFunc函数，但原函数有四个参数嘛
   //这个100实际放到的是谁的位置，传递的是哪一个参数呢，取决于 右边 bind()
   //看bind里参数依次，&Foo::memberFunc，&foo 指向成员函数的指针, 绑定到this的对象, 其他几个参数
   //0.5， _1, 10分别就i是那三个形参，其中第二个_1是占位符， 表示boost::function里面能接收的参数？↓
  //表示我们不打算向成员函数绑定第2个参数，这里使用 _1 来占位，同理下边还会用到 _2 _3 这样的占位符.
    fp(100);  //调用foo::memberFunc(0.5, 100, 10)，打印0.5，100，10
    
    boost::function<void (int, int)> fp2 = boost::bind(&Foo::memberFunc, boost::ref(foo), 0.5, _1, _2);  
    fp2(100, 200); //调用foo::memberFunc(0.5, 100, 200)，打印0.5，100，200

    return 0;
}