//不要一棍子打死，还没有做就说不行，如果一个医生还没有开始给病人治病就说没救了，直接判死刑

/*
原文链接：https://blog.csdn.net/qq_44783220/article/details/102910928
智能指针 
1.智能指针的存在就是为了解决有时候我们申请的空间来不及释放的问题，智能指针会自己去判断是否应该释放，在这一点上比较智能，由此得名。 
早期的智能指针：C++98 中的auto_ptr （不使用） 不使用auto_ptr的原因就是因为在使用中如果进行拷贝构造的时候，前一份的值会被悬空，这个问题就导致我们之后不会使用它 C++11 unique_ptr：unique_ptr将auto_ptr存在的问题直接避免，就是方法有些不太好，直接就不能拷贝了.
 shared_ptr：这个问题的解决方法是，找一个计数器count，每次构造和拷贝的时候计数器+1，析构的时候减1，如果析构减完之后是零的话才会真正析构，别的情况就不会析构，但是shared_ptr中也存在一些问题.
 比如下面的循环引用问题，这个问题主要是因为我们创建两个对象的时候count变成1，然后用链表的方式连起来的时候count变成了2，这个时候析构之后变成1，没有析构这两个结点这个时候发现如果要析构这两个解决的话，就必须先析构next与pre，但是next与pre又是两个对象里面的成员，成员要靠对象析构，但是对象还在等着成员析构，这样一个等着一个的情况就造成了循环引用的问题。
 下面是循环引用的代码示例：
*/
 // 编译： g++ -o share_ptr share_ptr.cc -std=c++11
#include <iostream>
#include <memory>
using namespace std;

template <class T>
class Node
{
public:
	shared_ptr<Node<T>> _pPre;
	shared_ptr<Node<T>> _pNext;
	T _value;
	Node(const T& value)
		:_pPre(nullptr),
		_pNext(nullptr),
		_value(value)
	{
		cout << "Node()" << endl;
	}
	~Node()
	{
		cout << "~Node()" << endl;
		cout << "this:" << this << endl;
	}
};

void Funtest()
{
	shared_ptr<Node<int>> sp1(new Node<int>(1));
	shared_ptr<Node<int>> sp2(new Node<int>(2));

	cout << "sp1.use_count:" << sp1.use_count() << endl;
	cout << "sp2.use_count:" << sp2.use_count() << endl;

	sp1->_pNext = sp2;
	sp2->_pPre = sp1;

	cout << "sp1.use_count:" << sp1.use_count() << endl;
	cout << "sp2.use_count:" << sp2.use_count() << endl;
}



int main()
{
	Funtest();
	return 0;
}

//上面循环引用的解决方法就是：weak_ptr （它是一个只保存指针值的指针）

/*
//编译 g++ -o share_ptr share_ptr.cc -std=c++11
#include <iostream>
#include <memory>
using namespace std;

template <class T>
class Node
{
public:
	weak_ptr<Node<T>> _pPre;
	weak_ptr<Node<T>> _pNext;
	T _value;
	Node(const T& value)
		:
		_value(value)
	{
		cout << "Node()" << endl;
	}
	~Node()
	{
		cout << "~Node()" << endl;
		cout << "this:" << this << endl;
	}
};

void Funtest()
{
	shared_ptr<Node<int>> sp1(new Node<int>(1));
	shared_ptr<Node<int>> sp2(new Node<int>(2));

	cout << "sp1.use_count:" << sp1.use_count() << endl;
	cout << "sp2.use_count:" << sp2.use_count() << endl;

	sp1->_pNext = sp2;
	sp2->_pPre = sp1;

	cout << "sp1.use_count:" << sp1.use_count() << endl;
	cout << "sp2.use_count:" << sp2.use_count() << endl;
}



int main()
{
	Funtest();
	return 0;
}

结果：

Node()
Node()
sp1.use_count:1
sp2.use_count:1
sp1.use_count:1
sp2.use_count:1
~Node()
this:0x1844060
~Node()
this:0x1844010


*/