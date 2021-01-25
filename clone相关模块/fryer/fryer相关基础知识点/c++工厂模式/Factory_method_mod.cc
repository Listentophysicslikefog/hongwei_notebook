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


class ShoesFactory // 总鞋厂
{
public:
    virtual Shoes *CreateShoes() = 0;
    virtual ~ShoesFactory() {}
};


class NiKeProducer : public ShoesFactory  // 耐克生产者/生产链
{
public:
    Shoes *CreateShoes()
    {
        return new NiKeShoes();
    }
};


class AdidasProducer : public ShoesFactory  // 阿迪达斯生产者/生产链
{
public:
    Shoes *CreateShoes()
    {
        return new AdidasShoes();
    }
};


class LiNingProducer : public ShoesFactory  // 李宁生产者/生产链
{
public:
    Shoes *CreateShoes()
    {
        return new LiNingShoes();
    }
};

int main()
{
    // ================ 生产耐克流程 ==================== //
    // 鞋厂开设耐克生产线
    ShoesFactory *niKeProducer = new NiKeProducer();
    // 耐克生产线产出球鞋
    Shoes *nikeShoes = niKeProducer->CreateShoes();
    // 耐克球鞋广告喊起
    nikeShoes->Show();
    // 释放资源
    delete nikeShoes;
    delete niKeProducer;

    // ================ 生产阿迪达斯流程 ==================== //
    // 鞋厂开设阿迪达斯生产者
    ShoesFactory *adidasProducer = new AdidasProducer();
    // 阿迪达斯生产线产出球鞋
    Shoes *adidasShoes = adidasProducer->CreateShoes();
    // 阿迪达斯球鞋广喊起
    adidasShoes->Show();
    // 释放资源
    delete adidasShoes;
    delete adidasProducer;

    return 0;
}