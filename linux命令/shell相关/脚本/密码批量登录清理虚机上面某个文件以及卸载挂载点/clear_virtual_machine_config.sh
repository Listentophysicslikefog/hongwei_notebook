#!/bin/bash
db_table_name=$1
/usr/bin/mongoexport -h 127.0.0.1 --port=1970 -d $db_table_name -c t_resource_common -f internal_ip --type=csv   -o ${db_table_name}.drity
#/usr/bin/mongoexport -h 127.0.0.1 --port=1970 -d chaoseris_hn02_1100 -c t_resource_common -f internal_ip --type=csv   -o ip.drity
#line=`sshpass -p \'ChaosPassword\' ssh root@10.13.0.79 -p 22 -q \"cat /etc/fstab | grep -n \"UCLOUD_DISK_VDB\"\" | awk -F \":\" \'\{print \$1\}\'`
readonly basedir=$(pwd)
i=0
sed -i '1d' $basedir/${db_table_name}.drity
for ip in $(cat ip.drity) #$(cat $basedir/{$db_table_name}.drity)
do
i=$(($i+1))
sshpass -p 'ChaosPassword' ssh root@$ip -p 22 -q "cat /etc/fstab"
fstab_line=`sshpass -p 'ChaosPassword' ssh root@$ip -p 22 -q "cat /etc/fstab | grep -n \"UCLOUD_DISK_VDB\"|wc -l"`
if [ $fstab_line != 0 ]
then
echo $fstab_line
line=`sshpass -p 'ChaosPassword' ssh root@$ip -p 22 -q "cat /etc/fstab | grep -n \"UCLOUD_DISK_VDB\"" | awk -F ':' '{print \$1}'`
echo "fstab configuration line number: $line"
sshpass -p 'ChaosPassword' ssh root@$ip -p 22 " sed -i '${line}d' /etc/fstab"
fi
fstab_exit=`sshpass -p 'ChaosPassword' ssh root@$ip -p 22 -q "cat /etc/fstab | grep -n \"UCLOUD_DISK_VDB\"|wc -l"`
if [ $fstab_exit != 0 ]
then
echo "$ip fstab  does not have vdb configuration clear"
exit -1
fi
data_number=`sshpass -p 'ChaosPassword' ssh root@$ip -p 22 -q "lsblk | grep data | wc -l"`
if [ $data_number != 0 ]
then
sshpass -p 'ChaosPassword' ssh root@$ip -p 22 -q "cd /root;umount /data"
fi
data_exit=`sshpass -p 'ChaosPassword' ssh root@$ip -p 22 -q "lsblk | grep data | wc -l"`
if [ $data_exit != 0 ]
then
echo "$ip vdb data mount point is not unmounted"
exit -1
fi

done
echo "Number of virtual machines: $i"