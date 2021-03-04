/*
unique_ptr的使用 
unique_ptr "唯一"拥有其所指对象，同一时刻只能有一个unique_ptr指向给定对象（通过禁止拷贝语义，只有移动语义来实现），并在unique_ptr离开作用域时自动释放该对象。
unique_ptr指针与其所指向的对象关系：在智能指针生命周期内，可以改变智能指针所指对象。如：创建智能指针时通过构造函数指定，通过reset方法重新指定、通过release方法释放所有权、通过移动语义转移所有权。
下面通过代码介绍unique_ptr的常规使用

*/
// 例子1： 
#include <iostream>
#include <memory>

int main() {
    {
        std::unique_ptr<int> uptr(new int(10));  //绑定动态对象
        //std::unique_ptr<int> uptr2 = uptr;  //不能赋值
        //std::unique_ptr<int> uptr2(uptr);  //不能拷贝
        std::unique_ptr<int> uptr2 = std::move(uptr); //转换所有权
        uptr2.release(); //释放所有权
    }
    //超過uptr的作用域，內存釋放
}


// 例子2：

#include <iostream>
#include <memory>
using namespace std;

int main()
{
    std::unique_ptr<int> p5(new int);
    *p5 = 10;
    // p 接收 p5 释放的堆内存
    int * p = p5.release();
    cout << *p << endl;
    //判断 p5 是否为空指针
    if (p5) {
        cout << "p5 is not nullptr" << endl;
    }
    else {
        cout << "p5 is nullptr" << endl;
    }

    std::unique_ptr<int> p6;
    //p6 获取 p 的所有权
    p6.reset(p);
    cout << *p6 << endl;;
    return 0;
}

/*
程序执行结果为： 
10
p5 is nullptr
10
*/