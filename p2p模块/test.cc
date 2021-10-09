1    2    3     4    5    6    7

10  15   20    23    26    29   30

ip  chunk_id

map["string"]vector<pair<int,int>> slots;
class SlidingTimeWindow {
    public: 
    int limit = 100;
    int counter = 0;
    vector<pair<int,int>> slots;
    // 时间划分多少段落
    int split = 10;

    // 是否限流了,true:限流了，false：允许正常访问。
    bool isLimit = false;

}




class SlidingTimeWindow {

private:

std::unordered_map<std::string,vector<pair<unsigned long,int>> slots;

public: 

void IncHostUseCount(const std::string &host_ip, unsigned long time_now);

void IncChunkUseCount(const std::string &host_ip_chunk_id, unsigned long time_now);


void DelHostUseCountInfo(const std::string &host_ip, unsigned long time_now);

void DelChunkUseCountInfo(const std::string &host_ip_chunk_id, unsigned long time_now);


uint32_t GetChunkUseCount(const std::string &host_ip_chunk_id, unsigned long time_now);

uint32_t GetHostUseCount(const std::string &host_ip, unsigned long time_now);

}



void SlidingTimeWindow::DelChunkUseCountInfo(const std::string &host_ip_chunk_id, unsigned long time_now)
{

if (slots.find(host_ip_chunk_id) != slots.end())
{
    slots.erase(host_ip_chunk_id)
}

}


void SlidingTimeWindow::IncHostUseCount(const std::string &host_ip, unsigned long time_now)
{
    if (slots.find(host_ip) != slots.end())
       {
         slots.erase(host_ip)
      }

}




void SlidingTimeWindow::IncHostUseCount(const std::string &host_ip, unsigned long time_now)
{
   
   if (slots.find(host_ip) == slots.end())
   {

    vector<pair<unsigned long,int> host_ip_info(make_pair<0,0>);
    slots[host_ip] = host_ip_info;  // 插入一个元素

   }

   vector<pair<unsigned long,int>& host_ip_info = slots[host_ip];

    int index = 0 ;
    for(int i = 0; i < host_ip_info.size(); i++)
    {
        if(host_ip_info[i].first == 0){
            index = i;
        }

        unsigned long time_diff = time_now - host_ip_info[i].first;
        if(time_diff < 100)
        {
           host_ip_info[i].second++;
           return ;
        }

    }

    if(host_ip_info[index].first == 0)
    {

        host_ip_info[index] = make_pair<time_now,1>;

    }else{
        // 可能是时间问题
       host_ip_info.push_back(make_pair<time_now,1>);
    }

}


void SlidingTimeWindow::IncChunkUseCount(const std::string &host_ip_chunk_id, unsigned long time_now)
{
   if (slots.find(host_ip_chunk_id) == slots.end())
   {

    vector<pair<unsigned long,int> host_ip_chunk_id_info(make_pair<0,0>);
    slots[host_ip_chunk_id] = host_ip_chunk_id_info;  // 插入一个元素

   }
   
   vector<pair<unsigned long,int>& host_ip_chunk_id_info = slots[host_ip_chunk_id];

    int index = 0 ;
    for(int i = 0; i < host_ip_info.size(); i++)
    {
        if(host_ip_chunk_id_info[i].first == 0){
            index = i;
        }

        unsigned long time_diff = time_now - host_ip_chunk_id_info[i].first;
        if(time_diff < 100)
        {
           host_ip_chunk_id_info[i].second++;
           return;
        }

    }

    if(host_ip_chunk_id_info[index].first == 0)
    {

        host_ip_chunk_id_info[index] = make_pair<time_now,1>;

    }else{
        // 可能是时间问题
       host_ip_chunk_id_info.push_back(make_pair<time_now,1>);
    }

}







uint32_t SlidingTimeWindow::GetHostUseCount(const std::string &host_ip, const unsigned long time_now)
{

uint32_t totol_count = 0;

if (slots.find(host_ip) != slots.end())
{
    vector<pair<unsigned long,int>& host_ip_info = slots[host_ip];

    for(int i = 0; i < host_ip_info.size(); i++){

        unsigned long time_tick = host_ip_info[i].first;

        unsigned long time_diff = time_now - time_tick;


        if(time_diff >= 2000)
        {
          host_ip_info[i] = make_pair<0,0>;
        }

        totol_count+=host_ip_info[i].second;
    }
}else{
    vector<pair<unsigned long,int> host_ip_info(make_pair<0,0>);
    slots[host_ip] = host_ip_info;  // 插入一个元素
}
return totol_count;
}



uint32_t SlidingTimeWindow::GetChunkUseCount(const std::string &host_ip_chunk_id, const unsigned long time_now)
{

uint32_t totol_count = 0;

if (slots.find(host_ip_chunk_id) != slots.end())
{
    vector<pair<unsigned long,int>& host_ip_chunk_id_info = slots[host_ip_chunk_id];

    for(int i = 0; i < host_ip_chunk_id_info.size(); i++){

        unsigned long time_tick = host_ip_chunk_id_info[i].first;

        unsigned long time_diff = time_now - time_tick;


        if(time_diff >= 2000)
        {
          host_ip_chunk_id_info[i] = make_pair<0,0>;
        }

        totol_count+=host_ip_chunk_id_info[i].second;
    }
}else{
    vector<pair<unsigned long,int> host_ip_chunk_id_info(make_pair<0,0>);
    slots[host_ip] = host_ip_chunk_id_info;
}
return totol_count;
}
















class SlidingTimeWindow {

map["string"]vector<pair<int,int>> slots;

std::unordered_map<std::string,vector<pair<int,int>>

}




// 每个小框只允许5个

vector<pair<int,int>>& count = slots["string"];
    int time_now = time now;
    int totol=0;
    for(int i =0; i < count.size();i++){
         
        if (count[i].get(0) < time_now -2){
            //记住与当前时间差1s内的时间
            // 移除与当前时间相差较大的，或者将其置0
            // 计算总的计数
            count[i] = std::make_pair<0,0>; 

        }
        totol+=count[i];

    }

    if totol < 50{
// 计算chunk id对应的计数
// 如果全都满足就可以分配该ip chunk 然后增加对应计数
    }




if slots["string"].size()< 6{
vector<pair<int,int>>& count = slots["string"];
count.push_back(mstd::make_pair<"time",1>);
}else{
    vector<pair<int,int>>& count = slots["string"];
    int time_now = time now;
    for(int i =0; i < count.size();i++){
        if (count[i].get(1) < time_now -2){
            
        }
    }
}