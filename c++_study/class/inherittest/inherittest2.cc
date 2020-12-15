//private继承

//仔细看代码中派生类B中定义了和基类同名的成员a，此时基类的a仍然存在，可以验证。

/*
所以派生类包含了基类所有成员以及新增的成员，同名的成员被隐藏起来，调用的时候只会调用派生类中的成员。
如果要调用基类的同名成员，可以用以下方法：
int main(){
 
  B b(10);
  cout << b.a << endl;
  cout << b.A::a << endl;
 
  system("pause");
  return 0;
}
*/
#include<iostream>
#include<assert.h>
using namespace std;
class A{
public:
  int a;
  A(){
    a1 = 1;
    a2 = 2;
    a3 = 3;
    a = 4;
  }
  void fun(){
    cout << a << endl;    //正确
    cout << a1 << endl;   //正确
    cout << a2 << endl;   //正确
    cout << a3 << endl;   //正确
  }
public:
  int a1;
protected:
  int a2;
private:
  int a3;
};

class B : private A{
public:
  int a;
  B(int i){
    A();
    a = i;
  }
  void fun(){
    cout << a << endl;       //正确，public成员。
    cout << a1 << endl;       //正确，基类public成员,在派生类中变成了private,可以被派生类访问。
    cout << a2 << endl;       //正确，基类的protected成员，在派生类中变成了private,可以被派生类访问。
    cout << a3 << endl;       //错误，基类的private成员不能被派生类访问。
  }
};
int main(){
  B b(10);
  cout << b.a << endl;       //正确。public成员，这个是子类自己定义的
  cout << b.a1 << endl;      //错误，private成员不能在类外访问。
  cout << b.a2 << endl;      //错误, private成员不能在类外访问。
  cout << b.a3 << endl;      //错误，private成员不能在类外访问。

cout << sizeof(A) << endl;    //16
cout << sizeof(B) << endl;    //20   子类也定义了一个和基类同名的变量，基类的也还在
 

  system("pause");
  return 0;
}