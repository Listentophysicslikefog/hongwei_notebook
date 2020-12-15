/*
继承中的特点：
先记住：不管是否继承，1.protected成员可以被派生类对象访问，还是只能在类的内部访问，不能被用户代码（类外，就是使用对象访问）访问。
                    2.public成员可以被用户代码（类外）访问    3.private成员不能被用户代码（类外）访问    。规则永远适用！
有public, protected, private三种继承方式，它们相应地改变了基类成员的访问属性。
1.public继承：基类public成员，protected成员，private成员的访问属性在派生类中分别变成：public, protected, private，并且基类的private成员派生类无法访问
2.protected继承：基类public成员，protected成员，private成员的访问属性在派生类中分别变成：protected, protected, private，并且基类的private成员派生类无法访问
3.private继承：基类public成员，protected成员，private成员的访问属性在派生类中分别变成：private, private, private，并且基类的private成员派生类无法访问
  

继承相当于：继承了父类的成员，但是权限由继承改变，只要父类没有private成员，那么无论子类这么继承，父类的所有成员子类都可以在自己的类内部访问

   但无论哪种继承方式，上面两点都没有改变：

1.private成员只能被本类成员（类内部）和友元访问，不能被派生类访问；
2.protected成员可以被派生类访问。
*/


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
class B : public A{
public:
  int a;
  B(int i){
    A();
    a = i;
  }
  void fun(){
    cout << a << endl;       //正确，public成员
    cout << a1 << endl;       //正确，基类的public成员，在派生类中仍是public成员。
    cout << a2 << endl;       //正确，基类的protected成员，在派生类中仍是protected可以被派生类访问。
    cout << a3 << endl;       //错误，基类的private成员不能被派生类访问。
  }
};
int main(){
  B b(10);
  cout << b.a << endl;
  cout << b.a1 << endl;   //正确
  cout << b.a2 << endl;   //错误，类外不能访问protected成员
  cout << b.a3 << endl;   //错误，类外不能访问private成员

cout << sizeof(A) << endl;    //16
cout << sizeof(B) << endl;    //20   子类也定义了一个和基类同名的变量，基类的也还在
  system("pause");
  return 0;
}