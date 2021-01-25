#include<iostream>
using namespace std;

class Shoes  // 鞋子抽象类
{
public:
    virtual ~Shoes() {}
    virtual void Show() = 0;
};

class NiKeShoes : public Shoes  // 耐克鞋子
{
public:
    void Show()
    {
        std::cout << "我是耐克球鞋，我的广告语：Just do it" << std::endl;
    }
};

class AdidasShoes : public Shoes  // 阿迪达斯鞋子
{
public:
    void Show()
    {
        std::cout << "我是阿迪达斯球鞋，我的广告语:Impossible is nothing" << std::endl;
    }
};

class LiNingShoes : public Shoes  // 李宁鞋子
{
public:
    void Show()
    {
        std::cout << "我是李宁球鞋，我的广告语：Everything is possible" << std::endl;
    }
};

enum SHOES_TYPE
{
    NIKE,
    LINING,
    ADIDAS
};


class ShoesFactory  // 总鞋厂
{
public:
    
    Shoes *CreateShoes(SHOES_TYPE type)  // 根据鞋子类型创建对应的鞋子对象
    {
        switch (type)
        {
        case NIKE:
            return new NiKeShoes();
            break;
        case LINING:
            return new LiNingShoes();
            break;
        case ADIDAS:
            return new AdidasShoes();
            break;
        default:
            return NULL;
            break;
        }
    }
};

int main()
{
    
    ShoesFactory shoesFactory;  // 构造工厂对象

    
    Shoes *pNikeShoes = shoesFactory.CreateShoes(NIKE);  // 从鞋工厂对象创建阿迪达斯鞋对象
    if (pNikeShoes != NULL)
    {
        
        pNikeShoes->Show();  // 耐克球鞋广告喊起

        
        delete pNikeShoes;  // 释放资源
        pNikeShoes = NULL;
    }

    
    Shoes *pLiNingShoes = shoesFactory.CreateShoes(LINING);  // 从鞋工厂对象创建阿迪达斯鞋对象
    if (pLiNingShoes != NULL)
    {
        
        pLiNingShoes->Show();  // 李宁球鞋广告喊起

        
        delete pLiNingShoes;   // 释放资源
        pLiNingShoes = NULL;
    }

    
    Shoes *pAdidasShoes = shoesFactory.CreateShoes(ADIDAS);  // 从鞋工厂对象创建阿迪达斯鞋对象
    if (pAdidasShoes != NULL)
    {
        
        pAdidasShoes->Show(); // 阿迪达斯球鞋广告喊起

        
        delete pAdidasShoes;  // 释放资源
        pAdidasShoes = NULL;
    }

    return 0;
}