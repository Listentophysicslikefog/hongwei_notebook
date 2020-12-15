#include<iostream>
using namespace std;

//派生类的构造函数和析构函数
//构造函数和析构函数是不可以被继承的
//派生类中包含了基类的数据成员，所以创建派生类对象的时后首先派生类的构造函数调用基类的构造函数，
//完成基类成员的初始化，然后再对基类新增加的成员进行初始化
//先调用基类的构造函数再执行派生类的构造函数，先调用派生类的析构函数再执行基类的析构函数

class A{
    public:
    A(){
        cout<<"A的构造函数"<<endl;
    }
    ~A(){
        cout<<"A的析构函数"<<endl;
    }
};
class B : public A{
    public:
    B(){
        cout<<"B的构造函数"<<endl;
    }
    ~B(){
        cout<<"B的析构函数"<<endl;
    }
};


//基类与派生类对象的相互转换   赋值转换，指针转换
//派生类

class a{
    private:
      int x;
    public:
    a(int xp=0){
        x=xp;
    }
    void dispa(){
        cout<<"this is a and x"<<endl;
    }
    int getx(){    //写一个方法，让子类可以访问private成员x
        return x;
    }
};

class b:public a{
    private:
    int y;

    public:

    b(int xp=1,int gv=0):a(xp){
        y=gv;
    }
    void dispay8(){
        cout<<"this is b and y,y:"<<y<<endl;
    }
};
int main(){
B test;



//
a testa;    //声明一个a类对象，就是基类对象
testa.dispa();
b test2(1,2);    //派生类对象
test2.dispay8();
testa=test2;    //通过test2对象为基类对象test赋值，子类为基类赋值
testa.dispa();  
cout<<testa.getx()<<endl;  
return 0;
}