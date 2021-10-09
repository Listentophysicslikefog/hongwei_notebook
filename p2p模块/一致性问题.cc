
bsi_test41627621735082   12528  10.67.134.23  /dev/nvme1n1

[root@hn02-udisk-set3104-1 script]# /root/udisk/chunk/tools/cal_pc_md5 /dev/nvme1n1 3245726777344 4194304
45a5c2dfa82579e2093ea8d0ef5583ae

{
"is_used" : 1,
"has_detection" : 1,
"seq_no" : 1374354,
"pc_id" : 773064,
"offset" : 3245726777344,
"pg_id" : 25,
"lc_id" : 12528,
"pc_no" : 7189,
"lc_random_id" : 889783779,
"allocate_time" : 1627621750
}






1. pc计算




12531   bsi_test41627621735259   10.67.134.23   /dev/nvme1n1   7189   27

{
"is_used" : 1,
"has_detection" : 1,
"seq_no" : 1374357,
"pc_id" : 773256,
"offset" : 3246532870144,
"pg_id" : 29,
"lc_id" : 12531,
"pc_no" : 7189,
"lc_random_id" : 660947927,
"allocate_time" : 1627621750
}

[root@hn02-udisk-set3104-1 script]# /root/udisk/chunk/tools/cal_pc_md5 /dev/nvme1n1 3246532870144  4194304                                    
633c866c2f393c51980078168d3db331




12531   bsi_test41627621735259  10.67.134.25  /dev/nvme1n1   7189   31

{
"is_used" : 1,
"has_detection" : 1,
"seq_no" : 553294,
"pc_id" : 1314832,
"offset" : 5520285548544,
"pg_id" : 29,
"lc_id" : 12531,
"pc_no" : 7189,
"lc_random_id" : 660947927,
"allocate_time" : 1627621760
}

[root@hn02-udisk-set3104-3 script]# /root/udisk/chunk/tools/cal_pc_md5 /dev/nvme1n1 5520285548544 4194304
45a5c2dfa82579e2093ea8d0ef5583ae






12531   bsi_test41627621735259   10.67.134.24   /dev/nvme1n1   7189   29

{
"is_used" : 1,
"has_detection" : 1,
"seq_no" : 1012356,
"pc_id" : 1046155,
"offset" : 4392272031744,
"pg_id" : 29,
"lc_id" : 12531,
"pc_no" : 7189,
"lc_random_id" : 660947927,
"allocate_time" : 1627621984
}

[root@hn02-udisk-set3104-2 script]# /root/udisk/chunk/tools/cal_pc_md5 /dev/nvme1n1 4392272031744 4194304
45a5c2dfa82579e2093ea8d0ef5583ae





2. 日志分析




hn02_udisk_part3104:PRIMARY> db.t_pg_info.find({"id":3})
{ "_id" : ObjectId("5f61c6a233b6087ec5c762b7"), "chunk_0_id" : 36, "chunk_1_id" : 34, "primary_chunk_id" : 36, "id" : 3, "chunk_2_id" : 38 }
hn02_udisk_part3104:PRIMARY> 

3. 复现

10.67.134.22 

bsi_test11627649519550_nvme0n1

[root@hn02-udisk-set3104-0 script]# /root/udisk/chunk/tools/raw_chunk_storage_tool dumpSuperBlock /dev/nvme0n1
dev name: /dev/nvme0n1
dev size: 6401252745216
dev block size: 512


// chunk 36
super block: 
uuid: 42a36bae-62b1-44a3-9641-be9a5d51976d
version: 1
magic: 2712847316
chunk_id: 36
pc_zone_offset: 4096
pc_zone_length: 6224084660224
pc_num: 1482432
pc_meta_size: 64
pc_size: 4194304
detection_size: 4096
detection_per_pc_num: 1
journal_zone_offset: 6224084664320
journal_zone_length: 177168076800
journal_meta_size: 512
journal_num: 1320
journal_size: 134217728


{
"is_used" : 1,
"has_detection" : 1,
"seq_no" : 757827,
"pc_id" : 1063058,
"offset" : 4463237586944,
"pg_id" : 3,
"lc_id" : 12666,
"pc_no" : 2807,
"lc_random_id" : 3979842918,
"allocate_time" : 1627649664
}

[root@hn02-udisk-set3104-0 script]# /root/udisk/chunk/tools/cal_pc_md5 /dev/nvme0n1 4463237586944 4194304                                                                                                                                    
772b341ec500887bcaca742932abce01



// 10.67.134.27  chunk 34 bsi_test11627649519550

nvme0n1

dev name: /dev/nvme0n1
dev size: 6401252745216
dev block size: 512

super block: 
uuid: 4228a224-256a-4736-8d66-07e613d8cd9e
version: 1
magic: 2712847316
chunk_id: 34
pc_zone_offset: 4096
pc_zone_length: 6224084660224
pc_num: 1482432
pc_meta_size: 64
pc_size: 4194304
detection_size: 4096
detection_per_pc_num: 1
journal_zone_offset: 6224084664320
journal_zone_length: 177168076800
journal_meta_size: 512
journal_num: 1320
journal_size: 134217728
[root@hn02-udisk-set3104-5 md5_file]# 


{
"is_used" : 1,
"has_detection" : 1,
"seq_no" : 868967,
"pc_id" : 763847,
"offset" : 3207030124544,
"pg_id" : 3,
"lc_id" : 12666,
"pc_no" : 2807,
"lc_random_id" : 3979842918,
"allocate_time" : 1627649791
}


[root@hn02-udisk-set3104-5 md5_file]# /root/udisk/chunk/tools/cal_pc_md5 /dev/nvme0n1 3207030124544 4194304
772b341ec500887bcaca742932abce01




6: 叠加回收

bsi_test41627642382119 不一致   12654

10.67.134.22

dev name: /dev/nvme0n1
dev size: 6401252745216
dev block size: 512

super block: 
uuid: 42a36bae-62b1-44a3-9641-be9a5d51976d
version: 1
magic: 2712847316
chunk_id: 36
pc_zone_offset: 4096
pc_zone_length: 6224084660224
pc_num: 1482432
pc_meta_size: 64
pc_size: 4194304
detection_size: 4096
detection_per_pc_num: 1
journal_zone_offset: 6224084664320
journal_zone_length: 177168076800
journal_meta_size: 512
journal_num: 1320
journal_size: 134217728

{
"is_used" : 1,
"has_detection" : 1,
"seq_no" : 740378,
"pc_id" : 1428766,
"offset" : 5998626054144,
"pg_id" : 0,
"lc_id" : 12654,
"pc_no" : 2807,
"lc_random_id" : 1824717188,
"allocate_time" : 1627642583
}

[root@hn02-udisk-set3104-0 script]# /root/udisk/chunk/tools/cal_pc_md5 /dev/nvme0n1  5998626054144 4194304
57a10455777a41692aeec303675b6319


hn02_udisk_part3104:PRIMARY> db.t_pg_info.find({"id":0})
{ "_id" : ObjectId("5f61c6a233b6087ec5c762b4"), "chunk_0_id" : 32, "chunk_1_id" : 36, "primary_chunk_id" : 32, "id" : 0, "chunk_2_id" : 34 }








10.67.134.26  

{
"is_used" : 1,
"has_detection" : 1,
"seq_no" : 917284,
"pc_id" : 891280,
"offset" : 3742044831744,
"pg_id" : 0,
"lc_id" : 12654,
"pc_no" : 2807,
"lc_random_id" : 1824717188,
"allocate_time" : 1627642584
}





dev name: /dev/nvme0n1
dev size: 6401252745216
dev block size: 512

super block: 
uuid: 0c9fc80e-d451-4908-9adf-12b327741c54
version: 1
magic: 2712847316
chunk_id: 32
pc_zone_offset: 4096
pc_zone_length: 6224084660224
pc_num: 1482432
pc_meta_size: 64
pc_size: 4194304
detection_size: 4096
detection_per_pc_num: 1
journal_zone_offset: 6224084664320
journal_zone_length: 177168076800
journal_meta_size: 512
journal_num: 1320
journal_size: 134217728

[root@hn02-udisk-set3104-4 md5_file]# /root/udisk/chunk/tools/cal_pc_md5 /dev/nvme0n1 3742044831744 4194304
b5cfa9d6c8febd618f91ac2843d50a1c





10.67.134.27   chunk 34 


{
"is_used" : 1,
"has_detection" : 1,
"seq_no" : 841993,
"pc_id" : 519965,
"offset" : 2183115935744 ,
"pg_id" : 0,
"lc_id" : 12654,
"pc_no" : 2807,
"lc_random_id" : 1824717188,
"allocate_time" : 1627642579
}


[root@hn02-udisk-set3104-5 md5_file]# /root/udisk/chunk/tools/cal_pc_md5 /dev/nvme0n1 2183115935744 4194304
57a10455777a41692aeec303675b6319














void WriteDataToChunk(const TinyPcTaskInfoPtr& info);

void WriteDataToChunk(const TinyPcTaskInfoPtr& info, const std::string read_mode, const std::string read_md5);

void DoHelaPcTaskHandle::HandleWriteChunk(const std::string read_mode, const std::string read_md5) 

void set_retry_write(bool retry_write, const std::string read_mode, const std::string read_md5);

void DoHelaPcTaskHandle::HandleRetryCb()  


void set_pending_write(bool pending_write,const std::string read_mode, const std::string read_md5);

void DoHelaPcTaskHandle::HandlePendingCb()



