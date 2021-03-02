#include<iostream>
using namespace std;

template<class T>
class NamedObject{

public:
   NamedObject(std::string& name,const T& value);

private:
string& nameValue;
const T objectValue;

};

template< typename T > NamedObject<T>::NamedObject(std::string& name,const T& value):nameValue(name),objectValue(value)
{
cout<< name<<value<<endl;

}

int main()
{

string newdog("dog1");
NamedObject<int> p(newdog,666);
return 0;
}