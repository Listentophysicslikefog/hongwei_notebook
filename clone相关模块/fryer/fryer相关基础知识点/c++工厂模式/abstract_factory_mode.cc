#include<iostream>
using namespace std;


class Clothe  // 基类 衣服
{
public:
    virtual void Show() = 0;
    virtual ~Clothe() {}
};


class NiKeClothe : public Clothe  // 耐克衣服
{
public:
    void Show()
    {
        std::cout << "我是耐克衣服，时尚我最在行！" << std::endl;
    }
};


class Shoes  // 基类 鞋子
{
public:
    virtual void Show() = 0;
    virtual ~Shoes() {}
};


class NiKeShoes : public Shoes  // 耐克鞋子
{
public:
    void Show()
    {
        std::cout << "我是耐克球鞋，让你酷起来！" << std::endl;
    }
};




class Factory  // 总厂
{ 
public:
    virtual Shoes *CreateShoes() = 0;
	virtual Clothe *CreateClothe() = 0;
    virtual ~Factory() {}
};


class NiKeProducer : public Factory  // 耐克生产者/生产链
{
public:
    Shoes *CreateShoes()
    {
        return new NiKeShoes();
    }
	
	Clothe *CreateClothe()
    {
        return new NiKeClothe();
    }
};


int main()
{
   
    
    Factory *niKeProducer = new NiKeProducer();   // ================ 生产耐克流程 ==================== //    // 鞋厂开设耐克生产线
    
	
    Shoes *nikeShoes = niKeProducer->CreateShoes();  // 耐克生产线产出球鞋

    Clothe *nikeClothe = niKeProducer->CreateClothe();  	// 耐克生产线产出衣服
     
    nikeShoes->Show();  	// 耐克球鞋广告喊起
	
    nikeClothe->Show();  // 耐克衣服广告喊起 
    
    delete nikeShoes;  // 释放资源
	delete nikeClothe;
    delete niKeProducer;
    return 0;
}