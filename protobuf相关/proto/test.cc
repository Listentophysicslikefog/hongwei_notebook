#include <iostream>
#include <fstream>
#include "person.pb.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
 
using namespace test;
int main(){
  Person p;
  p.set_name("test");
  p.set_id(1);
  p.set_email("a.iabc.com");

 // std::string str; //将pb二进制信息保存到字符串
 // p.SerializeToString(&str);
 // std::cout<<str<<std::endl;  
 
  
 // std::ofstream fw; //将pb文本信息写入文件
 // fw.open("./Person.txt", std::ios::out | std::ios::binary);
 // google::protobuf::io::OstreamOutputStream *output = new google::protobuf::io::OstreamOutputStream(&fw);
 // google::protobuf::TextFormat::Print(p, output);
 
  //delete output;
  //fw.close();
 

  std::string str1; //将pb文本信息保存到字符串
  google::protobuf::TextFormat::PrintToString(p, &str1);
  std::cout<<str1<<std::endl;
 
  
  Person p1; //反序列化
  p1.ParseFromString(str1);
  std::cout<<"name:"<<p1.name()<<",email:"<<p1.email()<<std::endl;
 
  return 0;
}


