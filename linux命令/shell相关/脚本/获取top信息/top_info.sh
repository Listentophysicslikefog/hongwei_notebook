#!/bin/bash
while : 
do
timecpu=$(date "+%Y-%m-%d %H:%M:%S")
cpu_sort=$(top -b -o +%CPU -n 1 | head -n 59)
top_cpu_sort=`echo "${cpu_sort}"`

timemem=$(date "+%Y-%m-%d %H:%M:%S")
memory_sort=$(top -b -o +%MEM -n 1 | head -n 59)
top_memory_sort=`echo "${memory_sort}"`
tag=$(date "+%Y%m%d")
echo "*********************************** Top Sort By CPU  Time: ${timecpu}***********************************" >> /var/log/top_monitor/top_info_${tag}.log
echo -e  "\n\n" >> /var/log/top_monitor/top_info_${tag}.log
echo -e  "${top_cpu_sort}" >> /var/log/top_monitor/top_info_${tag}.log
echo -e  "\n\n\n" >> /var/log/top_monitor/top_info_${tag}.log

echo "*********************************** Top Sort By MEM  Time: ${timemem}***********************************" >> /var/log/top_monitor/top_info_${tag}.log
echo -e  "\n\n" >> /var/log/top_monitor/top_info_${tag}.log
echo -e  "${top_memory_sort}" >> /var/log/top_monitor/top_info_${tag}.log
echo -e  "\n\n\n" >> /var/log/top_monitor/top_info_${tag}.log

sleep 10
done
# 输出的按照每天记录一个文件夹