/*
产品注册模板类+单例工厂模板类
前面的模板工厂虽然在新增产品的时候，不需要新增具体的工厂类，但是缺少一个可以统一随时随地获取指定的产品对象的类。
还有改进的空间，我们可以把产品注册的对象用std::map的方式保存，通过key-valve的方式可以轻松简单的获取对应的产品对象实例。
实现大致思路：
把产品注册的功能封装成产品注册模板类。注册的产品对象保存在工厂模板类的std::map，便于产品对象的获取。
把获取产品对象的功能封装成工厂模板类。为了能随时随地获取指定产品对象，则把工厂设计成单例模式。
*/


#include<iostream>
#include<map>
using namespace std;

// 基类 鞋子
class Shoes
{
public:
    virtual void Show() = 0;
    virtual ~Shoes() {}
};

// 耐克鞋子
class NiKeShoes : public Shoes
{
public:
    void Show()
    {
        std::cout << "我是耐克球鞋，我的广告语：Just do it" << std::endl;
    }
};

// 基类 衣服
class Clothe
{
public:
    virtual void Show() = 0;
    virtual ~Clothe() {}
};

// 优衣库衣服
class UniqloClothe : public Clothe
{
public:
    void Show()
    {
        std::cout << "我是优衣库衣服，我的广告语：I am Uniqlo" << std::endl;
    }
};



// 基类，产品注册模板接口类
// 模板参数 ProductType_t 表示的类是产品抽象类
template <class ProductType_t>
class IProductRegistrar
{
public:
   // 获取产品对象抽象接口
   virtual ProductType_t *CreateProduct() = 0;

protected:
   // 禁止外部构造和虚构, 子类的"内部"的其他函数可以调用
   IProductRegistrar() {}
   virtual ~IProductRegistrar() {}

private:
   // 禁止外部拷贝和赋值操作
   IProductRegistrar(const IProductRegistrar &);
   const IProductRegistrar &operator=(const IProductRegistrar &);
};

// 工厂模板类，用于获取和注册产品对象
// 模板参数 ProductType_t 表示的类是产品抽象类
template <class ProductType_t>
class ProductFactory
{
public:
   // 获取工厂单例，工厂的实例是唯一的
   static ProductFactory<ProductType_t> &Instance()
   {
      static ProductFactory<ProductType_t> instance;
      return instance;
   }

   // 产品注册
   void RegisterProduct(IProductRegistrar<ProductType_t> *registrar, std::string name)
   {
      m_ProductRegistry[name] = registrar;
   }

   // 根据名字name，获取对应具体的产品对象
   ProductType_t *GetProduct(std::string name)
   {
      // 从map找到已经注册过的产品，并返回产品对象
      if (m_ProductRegistry.find(name) != m_ProductRegistry.end())
      {
         return m_ProductRegistry[name]->CreateProduct();
      }

      // 未注册的产品，则报错未找到
      std::cout << "No product found for " << name << std::endl;

      return NULL;
   }

private:
   // 禁止外部构造和虚构
   ProductFactory() {}
   ~ProductFactory() {}

   // 禁止外部拷贝和赋值操作
   ProductFactory(const ProductFactory &);
   const ProductFactory &operator=(const ProductFactory &);

   // 保存注册过的产品，key:产品名字 , value:产品类型
   std::map<std::string, IProductRegistrar<ProductType_t> *> m_ProductRegistry;
};

// 产品注册模板类，用于创建具体产品和从工厂里注册产品
// 模板参数 ProductType_t 表示的类是产品抽象类（基类），ProductImpl_t 表示的类是具体产品（产品种类的子类）
template <class ProductType_t, class ProductImpl_t>
class ProductRegistrar : public IProductRegistrar<ProductType_t>
{
public:
   // 构造函数，用于注册产品到工厂，只能显示调用
   explicit ProductRegistrar(std::string name)
   {
      // 通过工厂单例把产品注册到工厂
      ProductFactory<ProductType_t>::Instance().RegisterProduct(this, name);
   }

   // 创建具体产品对象指针
   ProductType_t *CreateProduct()
   {
      return new ProductImpl_t();
   }
};

int main()
{
   // ========================== 生产耐克球鞋过程 ===========================//
   // 注册产品种类为Shoes（基类），产品为NiKe（子类）到工厂，产品名为nike
   ProductRegistrar<Shoes, NiKeShoes> nikeShoes("nike");
   // 从工厂获取产品种类为Shoes，名称为nike的产品对象
   Shoes *pNiKeShoes = ProductFactory<Shoes>::Instance().GetProduct("nike");
   // 显示产品的广告语
   pNiKeShoes->Show();
   // 释放资源
   if (pNiKeShoes)
   {
      delete pNiKeShoes;
   }

   // ========================== 生产优衣库衣服过程 ===========================//
   // 注册产品种类为Clothe（基类），产品为UniqloClothe（子类）到工厂，产品名为uniqlo
   ProductRegistrar<Clothe, UniqloClothe> adidasShoes("uniqlo");
   // 从工厂获取产品种类为Shoes，名称为adidas的产品对象
   Clothe *pUniqloClothe = ProductFactory<Clothe>::Instance().GetProduct("uniqlo");
   // 显示产品的广告语
   pUniqloClothe->Show();
   // 释放资源
   if (pUniqloClothe)
   {
      delete pUniqloClothe;
   }

   return 0;
}








