// Copyright (c) 2020 UCloud All rights reserved.
#ifndef UDISK_HELA_DO_HELA_PC_TASK_H_
#define UDISK_HELA_DO_HELA_PC_TASK_H_

#include "hela_dec.h"
#include "hela_task.h"
#include <xutil/hela_io_proto.h>
#include "resource_manager.h"
#include "transmission_handle.h"
#include "exponential_backoff.h"
#include <xutil/msgr.h>
#include "vdisk_write.h"
#include "uevent.h"
#include <string>
#include <memory>

namespace udisk {
namespace hela {

class DoHelaPcTaskHandle
    : public std::enable_shared_from_this<DoHelaPcTaskHandle> {
 public:

 //typedef std::function<void(int ret_code, uint32_t pc_no)> ResponseHook;

  DoHelaPcTaskHandle(ResponseHook hook, HelaPcTaskPtr pc_task,
                     TransmissionHandle* loop_handle);
  ~DoHelaPcTaskHandle();

  bool retry_read() const { return retry_read_; }   //bool retry_read_; bool值
  bool retry_write() const { return retry_write_; }  //bool retry_write_; bool值
  bool pending_write() const { return pending_write_; }  //bool pending_write_; bool值
  bool pending_read() const { return pending_read_; }  //bool pending_read_; bool值

  void set_retry_read(bool retry_read);
  void set_retry_write(bool retry_write);
  void set_pending_write(bool pending_write);
  void set_pending_read(bool pending_read);

  void HandleRetryCb();
  void HandlePendingCb();

  void ReadKoalaTimeOut();
  void ReadChunkTimeOut();
  void WriteChunkTimeOut();

  void Start();

  void DecideKoalaSource(bool new_sector, bool change_koala_source);
  void ReadDataFromKoala();
  void BuildKoalaReadRequest(udisk::common::MessageHeaderV1* common_hdr,
                             udisk::common::HelaKoalaRequestHead* koala_req);
  void HandleReadKoalaResponse(void* msg_hdr, void* msg_data,
                               const ConnectionUeventPtr& conn);

  void ReadDataFromChunk();
  void BuildChunkReadRequest(udisk::common::MessageHeaderV2* common_hdr,
                             udisk::common::HelaChunkRequestHeadV2* chunk_hdr);
  void HandleReadChunkResponse(void* msg_hdr, void* msg_data,
                               const ConnectionUeventPtr& conn);

  void HandleWriteChunk();
  void WriteDataToChunk(const TinyPcTaskInfoPtr& info);
  void BuildChunkWriteRequest(udisk::common::MessageHeaderV2* common_hdr,
                              udisk::common::HelaChunkRequestHeadV2* chunk_hdr,
                              const TinyPcTaskInfoPtr& info);
  void HandleWriteChunkResponse(void* msg_hdr, void* msg_data,
                                const ConnectionUeventPtr& conn);

  void ReadNextPart();

  bool CheckCluster();

  void FinishHelaPcTask(int retcode);

 private:
  void FillKoalaSourceRouter();

 private:
  ResponseHook rsp_hook_;
  HelaPcTaskPtr pc_task_;    //typedef std::shared_ptr<HelaPcTask> HelaPcTaskPtr;
  TransmissionHandle* loop_handle_;

  uevent::TimerId timer_id_;
  bool retry_read_;  
  bool retry_write_;
  bool pending_read_;
  bool pending_write_;
  bool change_ark_immediately_ = false;

  uint32_t pc_;
  std::string src_vdisk_id_;  // 镜像源 disk id

  // src源路由
  std::string src_ip_;   // 镜像源  ip
  uint32_t src_port_;   // 镜像源  port

  // koala源info
  uint32_t src_version_;   //koala源  version
  uint64_t shard_size_;  
  KoalaRouteMap src_koala_router_;   //typedef std::map<int, ShardChronoRouter> KoalaRouteMap;

  // chunk源info
  uint32_t cluster_version_;  //chunk version
  uint32_t lc_size_;         // lc盘的大小
  uint64_t pc_size_;         //pc分片大小
  uint32_t src_lc_id_;         //镜像源的id
  uint32_t src_lc_random_id_;  //镜像源的 random_id
  uint32_t src_pg_id_;          // 镜像源的 pg id
  struct ChunkSourceInfo chunk_source_info_;

  // tiny pc task
  TinyPcTaskInfoPtrList info_list_;
  TinyPcTaskInfoPtrListIterator iter_;

  uint64_t start_sector_;   //扇面 开始
  uint32_t sector_num_;   //扇面个数
  uint64_t next_read_sector_;  // 下一个 读的扇面
  uint64_t end_sector_;   //结束的扇面

  common::StrBuffer* buffer_;

  // 当前分片的当前sector的路由信息
  std::unordered_map<std::string, ARKRouteInfo> current_ark_router_;
/*

// 镜像服务器的IP, PORT, 以及所在的IP在每个传输线程使用权重
// 这个数据结构主要是为了负载均衡排序使用
typedef std::tuple<std::string, uint16_t, uint64_t> ARKRouteInfo;

*/

  int huge_image_pending_times_ = 0;
  ExponentialBackoff<std::chrono::milliseconds> back_off_;  //退避算法
};

}  // namespace hela
}  // namespace udisk

#endif
