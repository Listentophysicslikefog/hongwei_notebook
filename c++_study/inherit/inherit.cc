#include<iostream>
using namespace std;

/*
* 继承:
*
*
*/

//多基继承的二义性问题
class A{
    public:

    void print(){
      cout<<"this is A class"<<endl;
    }

};

class B{       //B类中定义了与A类相同的函数
    public:

    void print(){
    cout<<"this is B class"<<endl;
    }
};

class C:public A,public B{    //同时继承了两个
    public:
     void disp(){       //A::print()
        //print();      //使用类名来解决二义性问题，表示使用的是哪一个基类的函数
       A::print();     //解决二义性
    }

};


//虚机类解决共同基类带来的二义性问题
//例如：class a 有方法print()   ,class b继承了class a, class c继承了class a, class d继承了class b和class c,那么当
//class d调用print()方法是不知道是从b和c那个基类调用的，因为b和c类都有该函数的副本


//虚机类解决这个问题，因为虚机类的子类没有副本，所以访问的都是同一个函数,需要在继承虚机类的子类去实现，虚类的成员和函数不可以直接访问
class VA{

public:
void print(){
    cout<<"this is VA class"<<endl;
    }
};

class VB:virtual VA{
public:
void print(){    //实现虚基类的具体方法
    cout<<"this is VB class"<<endl;
    }
};
class VC:virtual VA{
    
};

class VD:public VB,public VC{

};

int main(){
    
  C test;
  test.disp();

//虚基类解决二义性
VD test1;
test1.print();

}
