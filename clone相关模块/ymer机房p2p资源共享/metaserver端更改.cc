#include<iostream>
#include <unordered_map>
#include <utility>
#include<string>
#include<vector>
#include <sys/time.h>
using namespace std;
typedef pair<string, long> clone_disk;
int main()
{
unordered_map <string,vector<pair<string,long>>> test;
vector<pair<string,long>> temp;

clone_disk pa;
struct timeval tv;
gettimeofday(&tv, NULL);
long time_now = tv.tv_sec;
pa = make_pair("udisk_id",time_now);
temp.push_back(pa);
unordered_map<string, vector<pair<string,long>>>::iterator iter;
if(test.find("10.189.151.131")==test.end())
{
test["10.189.151.131"]=temp;
}

if(test.find("10.189.151.131")!=test.end())
{
auto vec = test["10.189.151.131"];
auto vec_iter = vec.begin();
cout<<"ip : "<<(*test.begin()).first<<"count: "<<vec.size()<<endl;
for(vec_iter;vec_iter!=vec.end();vec_iter++)
{
cout<<"id: "<<(*vec_iter).first<<"time: "<<(*vec_iter).second<<endl;

}
}

return 0;
}
~               