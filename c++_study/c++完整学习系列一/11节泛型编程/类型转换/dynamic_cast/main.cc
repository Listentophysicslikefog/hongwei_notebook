#include<iostream>
using namespace std;

class Employee  //基类
{
public:
  virtual void PaySalary();
  virtual void PayBonus();
};
class Manager: public Employee
{
public:
  virtual void PaySalary();
  virtual void PayBonus();
};

class Programmer: public Employee
{
public:
  virtual void PaySalary();
  virtual void PayBonus();
};
class Company
{
public:
  virtual void PayRoll( Employee * e );
  virtual void PayRoll( Employee & e );
private:
  vector<Employee*> _employees;
};

void Company::PayRoll( Employee * e )		//  版本一
{    //  调用哪个成员函数？如何区分程序员和经理？
  e->PaySalary();
  e->PayBonus();
};

void Company::PayRoll( Employee * e )		//  版本二    Employee的指针
{
  Programmer * p = dynamic_cast<Programmer*>( e );  //向下转型
  if( p )    //  p确实指向程序员对象   转型失败p就为nul  如果p不为nul就表示转型成功
  {
    p->PaySalary();
    p->PayBonus();
  }
  else     //  p不指向程序员，不发奖金
    e->PaySalary();
};

void Company::PayRoll( Employee & e )	//  版本三  Employee的引用
{
  try
  {
    Programmer & p = dynamic_cast<Programmer&>( e );
    p.PaySalary();
    p.PayBonus();
  }
  catch( std::bad_cast )   //  引用转型失败只会返回bad_cast  catch捕获bad_cast异常   这里处理的是另外的一种情况，是为了特定的功能所写的catch
  {
    e.PaySalary();
  }
};

