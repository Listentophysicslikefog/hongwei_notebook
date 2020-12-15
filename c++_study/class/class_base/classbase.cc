#include<iostream>
#include"classbase.h"
using namespace std;


/*
*封装:  把数据（属性）和函数（操作）合成一个整体，这在计算机世界中是用类与对象实现的,把客观事物封装成抽象的类，并且类可以把自己的数据和方法只让可信的类或者对象操作，对不可信的进行信息隐藏。
备注：有2层含义（把属性和方法进行封装 对属性和方法进行访问控制）
C++中类的封装
成员变量，C++中用于表示类属性的变量
成员函数，C++中用于表示类行为的函数
2）类成员的访问控制
在C++中可以给成员变量和成员函数定义访问级别
Public修饰成员变量和成员函数可以在类的内部和类的外部被访问,就是在类的外部可以通过对象访问
Private修饰成员变量和成员函数只能在类的内部被访问

*/


//类是把属性和方法封装 同时对信息进行访问控制
//类的内部，类的外部
//我们抽象了一个类，用类去定义对象
//类是一个数据类型，类是抽象的
//对象是一个具体的变量。占用内存空间。

double Circle::getR(){
    return r;
}

void Circle::setr(double value){
       this->r=value;
       }
double Circle::gets() //增加功能时，是在修改类, 修改类中的属性或者是方法
	{
		 s = 3.14f*r*r;
		 return s;
	}

int Circle::getprotected(){
	return h;
}
int main(){

Circle test;

test.setr(9);   

//  test.a 对象不可以访问私有成员,所以不可以这么写,只可以在类的里面访问，一般类的外部只可以访问public修饰的变量或者函数

cout<<test.getR()<<"   "<<test.r<<"   "<<test.getprotected()<<endl;

return 0;

}