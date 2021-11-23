1. pb connection 连接失败，可能是pb字段没有填充完整导致服务端或者客户端主动关闭连接, pb的必填字段必须填充

2. 日志丢失，可能是定时脚本的问题导致删除了日志，删除的日志只要一直有持续的输入就可以找回

3. 更改pb里面的const结构，有一个接口mutble函数

4. 程序core可能有很多情况，编译时问题编译的时候可以发现，但是有些是运行时出现问题，如报错：
basic_string::_S_construct NULL not valid
就是使用 nullptr去初始化 std::string 导致的问题

5. runninloop问题

6. 注意[&](){}、[=](){},如果使用引用捕获，在表达式里面使用了临时变量，并且表达式不阻塞当前线程，那么捕获的临时变量销毁后，表达式里面就会非法访问

7. https://www.cnblogs.com/kerrycode/p/5077687.html

hexdump 一致性比对

8. 管理线程正常下发任务，但是io线程没有执行卡住了

首先看对应的日志看在那里卡住了，是在哪一个流程那个地方卡住的，看线程的pid是不是只有一个在执行是不是其他的线程卡住了，是不是io线程卡住了，然后看一下top信息，看io线程执行情况，如果io线程的cpu都跑满了，那么很可能是io线程的任务死循环或者死锁了。
然后就看对应的堆栈信息，看是卡在那里。


 pstack 5432(进程id)
 top -Hp 5432 (看进程的所有线程)
 cat /proc/5432(线程id)/stack   栈信息
 /tools/bin/gdb -p 5432  调试进程
 thread apply all bt 查看进程里面所有线程的堆栈信息





 [root@hn02-udisk-set1105-meta1 ~]# cat metaserver.conf 
[common]
db_name                       = udisk
db_timeout                    = 10
mark_offline                  = 0
gate_io_timeout               = 9
chunk_error_limit             = 35
chunk_thread_hang_limit       = 20

[names]
my_instance                   = 1
my_set                        = 1102
my_name                       = /NS/udisk/set1102/metaserver
umongo                        = /NS/udisk/set1102/umongo_gateway
odin                          = /NS/udisk/set1102/odin
freyr                         = /NS/udisk/set1102/freyr
lease_freyr                   = /NS/udisk/global/freyr
lease_idun                    = /NS/udisk/global/idun

#ark_access                    = /NS/utimemachine/access
ark_access                    = /NS/utimemachine/grey/access
peer_metaserver_ip            = 172.27.26.72
peer_metaserver_port          = 12500

[network]
listen_ip                     = 172.27.26.73
listen_port                   = 12500
chunk_listen_port             = 12501
gate_listen_port              = 12502
cluster_frontend_net          = tcp
chunk_machine_netcard_bw      = 10000
netcard_bw_high_level_percent = 70
turn_on_chunk_flow_ctrl       = 1

[log]
path                          = /data/log/udisk/metaserver
level					= info

[qos]
iops_min                      = 1200
iops_max                      = 24000
bw_min                        = 80
bw_max                        = 260
iops_per_gb                   = 30
bw_per_gb                     = 50

[zookeeper]
server                        = 172.27.5.43:2181,172.27.5.44:2181,172.27.5.45:2181
global_server                 = 172.27.117.185:2181,172.27.117.186:2181,172.27.117.187:2181,172.27.117.188:2181,172.27.117.189:2181
timeout                       = 10












[common]
metaserver_heartbeat_interval = 2
metaserver_timeout            = 5
db_timeout                    = 5
db_name                       = udisk
ping_ark_interval             = 10
concurrent_jobs               = 32
concurrent_sectors            = 512
enable_hela                   = 1
schedule_hela_job_interval    = 1
hela_job_retry_interval       = 5
hela_job_retry_max_num        = 10
hela_clone_timeout            = 86400
hela_clone_expired_timeout    = 30

[names]
my_instance                   = 0
# my_name                       = /NS/udisk/set1102/freyr
# my_name                       = /NS/udisk/global/freyr
my_name                       = /NS/udisk/set1102/freyr
my_set                        = 1102
umongo                        = /NS/udisk/set1102/umongo_gateway
metaserver                    = /NS/udisk/set1102/metaserver
access                        = /NS/udisk/set1102/fake_access
ark_access                    = /NS/utimemachine/access
hela                          = /NS/udisk/set1102/hela
# idun                          = /NS/udisk/set1102/idun
idun                          = /NS/udisk/global/idun

[network]
listen_ip                     = 172.27.40.92
listen_port                   = 13900

[log]
path                          = /var/log/udisk/freyr
level                         = trace

[zookeeper]
server                        = 172.27.119.238:2181,172.27.26.76:2181,172.27.26.77:2181,172.27.4.2:2181
global_server                 = 172.27.117.185:2181,172.27.117.186:2181,172.27.117.187:2181,172.27.117.188:2181,172.27.117.189:2181
timeout                       = 10
zk_force_reconnect_timeout    = 180
zk_get_route_interval         = 10







[common]
subsys                             = ymer
module                             = idun
set                                = set1102
instance                           = 0

[idun]
db_name                            = ymer
metaserver_heartbeat_interval      = 3
metaserver_timeout                 = 5
max_reserve_resource_num           = 10
resource_timeout                   = 86400
clear_resource_interval            = 3600
slice_merge_insert_num             = 5
max_return_host_pc_per_second      = 20
max_return_chunk_pc_per_second     = 15
idun_lease_time                    = 22
idun_single_mode                   = 1
peer_idun_ip                       = 127.0.0.1
peer_idun_port                     = 60000

[names]
my_name                            = /NS/udisk/global/idun
umongo                             = /NS/udisk/@{common.set}/umongo_gateway
metaserver                         = /NS/udisk/@{common.set}/metaserver

[network]
listen_ip                          = 172.27.40.92
listen_port                        = 60000

[log]
path                               = /var/log/udisk/idun
roll_size                          = 50M
level                              = trace

[zookeeper]
server                             = 172.27.119.238:2181,172.27.26.76:2181,172.27.26.77:2181,172.27.4.2:2181
global_server                      = 172.27.117.185:2181,172.27.117.186:2181,172.27.117.187:2181,172.27.117.188:2181,172.27.117.189:2181
timeout                            = 10
zk_force_reconnect_timeout         = 180
zk_get_route_interval              = 10



[common]
subsys                                = ymer
module                                = hela
set                                   = 1102
instance                              = 0
thread_num                            = 6
chunk_num                             = 6

# 写本地chunk,  读本地chunk, 读远程(chunk,ark)
max_chunk_write_mbps                  = 300
max_chunk_read_mbps                   = 300
max_read_remote_mbps                  = 300

# 单个PC任务的重试回退参数, 总的错误次数, 读失败次数，等待调度重试时间
# 读取koala的超时时间必须要比镜像读取磁盘的超时时间长(10s)
# 超大镜像允许一段次数进入pending而不是错误计数而导致任务失败
single_pc_retry_backoff_initial_ms    = 1000
single_pc_retry_backoff_maximum_ms    = 20000
single_pc_retry_backoff_step_ms       = 100
single_pc_pending_change_koala_times  = 100
single_pc_pending_wait_schedule_ms    = 200
single_pc_pending_read_koala_ms       = 100
single_pc_pending_read_chunk_ms       = 300
single_pc_pending_write_chunk_ms      = 500
single_pc_read_koala_timeout_second   = 15
single_pc_read_chunk_timeout_second   = 10
single_pc_write_chunk_timeout_second  = 10
single_pc_huge_image_pending_times    = 5

# 单个批量PC任务的超时时间, 理论上需要根据公式计算
batch_hela_task_timeout_second        = 7200
batch_hela_task_schedule_second       = 1
batch_hela_task_statistics_on         = 0
batch_hela_taks_schedule_shuffle_on   = 0

# 管理线程生成批量任务的调度参数
batch_task_dispatch_interval          = 5
batch_task_dispatch_limit_num         = 40
total_task_dispatch_limit_num         = 60
task_merge_threshold                  = 20
flow_report_interval                  = 1
mem_check_interval                    = 300
enable_dynamic_concurence_pc_num      = 0
max_concurence_pc_num                 = 1024
# 流控令牌桶相关参数
concurence_sectors                    = 512
# 本地源失效时间,防止Freyr一直不logout
# 和Idun的盘失效时间策略保持一致
resource_expiration_second            = 3600

[names]
# my_name                               = /NS/udisk/@{common.set}/hela
# idun                                  = /NS/udisk/@{common.set}/idun
# metaserver                            = /NS/udisk/@{common.set}/metaserver
my_name                               = /NS/udisk/set@{common.set}/hela
idun                                  = /NS/udisk/global/idun
metaserver                            = /NS/udisk/set@{common.set}/metaserver

[network]
listen_ip                             = 172.27.40.92
listen_port                           = 22000
transmission_listen_port              = 22001

[log]
path                                  = /var/log/udisk/hela
roll_size                             = 50M
level                                 = trace

[zookeeper]
server                                = 172.27.119.238:2181,172.27.26.76:2181,172.27.26.77:2181,172.27.4.2:2181
global_server                         = 172.27.117.185:2181,172.27.117.186:2181,172.27.117.187:2181,172.27.117.188:2181,172.27.117.189:2181
timeout                               = 10
zk_force_reconnect_timeout            = 180
zk_get_route_interval                 = 10










20211122 19:48:10.627157Z ][ 5887 ][ERROR ]conn<15965484624675> unknow message type: 0 - transmission_handle.cc:163