#include<iostream>

using namespace std;

int main(){

char next;
char input[10];
cin.get(input,10);
cout<<input<<endl;
while(cin){   //输入的不为null就会一直接收输入
cin.get(next);
while(next != '\n')
cin.get(next);
cin.get(input,10);

cout<<input<<endl;

}

return 0;
}
