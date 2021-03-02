#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
using namespace std;

class Person{
public:
    string name;
    int age;

    Person(string n, int a){
        name = n;
        age = a;
    }

    bool operator==(const Person & p) const
    {
        return name == p.name && age == p.age;
    }
};

struct hash_name{
        size_t operator()(const Person & p) const{    //这里重载,参数是Person
                return hash<string>()(p.name) ^ hash<int>()(p.age); //hash<int>()(p.age) 相当于两步 hash<int>()定义一个临时变量 hash<int>()(p.age) 给这个变量传参并且执行这个函数
        }
};

int main(int argc, char* argv[]){
        unordered_map<Person, int, hash_name> ids; //不需要把哈希函数传入构造器    这里三个参数实际是为了计算 hash，我们自己定义的person类型 unorder_map好像不可以自定义的计算hash所以需要我们自己传 hash_name计算hash的函数
        ids[Person("Mark", 17)] = 40561;
    ids[Person("Andrew",16)] = 40562;
    for ( auto ii = ids.begin() ; ii != ids.end() ; ii++ )
        cout << ii->first.name
        << " " << ii->first.age
        << " : " << ii->second
        << endl;
    return 0;
}
~  