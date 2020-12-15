#ifndef _CLASSBASE_H
#define _CLASSBASE_H
class Circle{
   public:
       double r;
       double s;
   public:
       double getR();
       void setr(double value);
   public:
       double gets();
       int getprotected();
   private:
           int a;
    protected:
    int h;
};
#endif




//classbase.cc:39:5: 错误：‘int Circle::getprotected()’ 重定义    ,在.h文件里声明，在.cc文件里实现的时候，声明的文件的函数不可以加{}
//只可以这样：int getprotected();,不然会报重定义的错误。
