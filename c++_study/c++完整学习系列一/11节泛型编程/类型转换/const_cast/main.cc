#include <iostream>

class ConstCastTest
{
public:
  void SetNum( int num ){ _num = num; }
  void PrintNum() const;
private:
  int _num;
};
//用于取消或设置量的const状态，如果原来的值是const类型，那么 转换后就不是const类型，如果原来的值不为const类型转换后就是const类型
void ConstCastTest::PrintNum() const
{
  //  临时取消常量约束，修改目标对象的内容
  const_cast<ConstCastTest*>( this )->num--;    //原来为const 现再设置后 不为const类型 就可以改值
  std::cout << _num;
}