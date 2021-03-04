#include <iostream>

using namespace std;

class Complax
{
public:
    Complax(int a = 0, int b = 0)
    {
        this->a = a;
        this->b = b;
    }
    void printC()
    {
        cout << "a = " << a << "\tb = " << b << endl;
    }
    ~Complax()
    {
    }
//private:
public:

    int a;
    int b;

};
// 1定义全局函数
Complax myAdd(Complax &c1, Complax &c2)
{
    Complax temp(c1.a + c2.a, c1.b + c2.b);
    return temp;
}
// 2函数名升级
Complax operator+(Complax &c1, Complax &c2)
{
    Complax temp(c1.a + c2.a, c1.b + c2.b);
    return temp;
}
int main()
{
    int a = 0, b = 0;
    int c; // 基础性数据类型，编译器已经知道了，如何运算
    c = a + b;

    // a + bi;  // 复数运算规则
    Complax c1(1, 2), c2(3, 4);
    Complax c3;  // 2c3是用户自定义数据类型，c++编译器不知道如何运算
    //    c3 = c1 + c2;

    // 3c++编译器应该给我们提供一种机制
    // 让自定义数据类型 有机会进行 运算符操作 ==> 运算符重载

    // 4运算符重载机制
    Complax c4 = myAdd(c1, c2);
    c4.printC();

    // 步骤二：Complax c4 = c1+c2;
    // Complax c5 = operator+(c1, c2);
    // c5.printC();

    // 步骤三：
    Complax c5 = c1 + c2;
    c5.printC();

    // 运算符重载的本质 是  函数调用
    return 0;
}


















