// Copyright (c) 2020 UCloud All rights reserved.
#ifndef UDISK_HELA_TASK_H_
#define UDISK_HELA_TASK_H_

#include <memory>
#include <map>
#include <list>
#include <string>
#include <tuple>
#include "umessage.h"

namespace udisk {
namespace hela {

using namespace ucloud::utimemachine;

class TinyPcTaskInfo;

// 镜像服务器的IP, PORT, 以及所在的IP在每个传输线程使用权重
// 这个数据结构主要是为了负载均衡排序使用
typedef std::tuple<std::string, uint16_t, uint64_t> ARKRouteInfo;

typedef std::list<PcRoute> PcRouteList;
typedef std::map<uint32_t, PcRouteList> PcRouteMap;      // pc_no => PcRouteList
typedef std::map<int, ShardChronoRouter> KoalaRouteMap;  // shard_idx =>
                                                         // KoalaRouter
typedef std::shared_ptr<TinyPcTaskInfo> TinyPcTaskInfoPtr;
typedef std::list<TinyPcTaskInfoPtr> TinyPcTaskInfoPtrList;
typedef std::list<TinyPcTaskInfoPtr>::iterator TinyPcTaskInfoPtrListIterator;

// ===================== HelaTask ===================== //

class HelaTask {
 public:
  HelaTask(const std::string& chrono_id);
  ~HelaTask();

  bool operator==(HelaTask& rhs) const {
    return (chrono_id_ == rhs.chrono_id_);
  }

  PbHelaTaskStatus status() { return status_; }

  void set_status(PbHelaTaskStatus helaTaskStatus) { status_ = helaTaskStatus; }

  void set_status_fail() { status_ = PB_HELA_TASK_FAIL; }

  bool status_fail() { return status_ == PB_HELA_TASK_FAIL; }

  void set_status_ready() { status_ = PB_HELA_TASK_READY; }

  bool status_ready() { return status_ == PB_HELA_TASK_READY; }

  void set_status_executing() { status_ = PB_HELA_TASK_EXECUTING; }

  bool status_executing() { return status_ == PB_HELA_TASK_EXECUTING; }

  void set_status_done() { status_ = PB_HELA_TASK_DONE; }

  bool status_done() { return status_ == PB_HELA_TASK_DONE; }

  const std::string& chrono_id() const { return chrono_id_; }

  void set_chrono_id(const std::string& chronoId) { chrono_id_ = chronoId; }

  const std::string& vdisk_id() const { return vdisk_id_; }

  void set_vdisk_id(const std::string& vdiskId) { vdisk_id_ = vdiskId; }

  const uint32_t version() const { return version_; }

  void set_version(uint32_t version) { version_ = version; }

  const VDiskType vdisk_type() const { return vdisk_type_; }

  void set_vdisk_type(VDiskType vdiskType) { vdisk_type_ = vdiskType; }

  const uint32_t lc_id() const { return lc_id_; }

  void set_lc_id(uint32_t lcId) { lc_id_ = lcId; }

  const uint32_t lc_random_id() const { return lc_random_id_; }

  void set_lc_random_id(uint32_t lcRandomId) { lc_random_id_ = lcRandomId; }

  const uint32_t lc_size() const { return lc_size_; }

  void set_lc_size(uint32_t lcSize) { lc_size_ = lcSize; }

  const uint64_t pc_size() const { return pc_size_; }

  void set_pc_size(uint64_t pcSize) { pc_size_ = pcSize; }

  const uint64_t cluster_version() const { return cluster_version_; }

  void set_cluster_version(uint64_t clusterVersion) {
    cluster_version_ = clusterVersion;
  }

  PcRouteMap& pc_routers() { return pc_routers_; }

  void set_pc_routers(PcRouteMap& pcRouters) { pc_routers_.swap(pcRouters); }

  const std::string& src_vdisk_id() const { return src_vdisk_id_; }

  void set_src_vdisk_id(const std::string& srcVdiskId) {
    src_vdisk_id_ = srcVdiskId;
  }

  const uint32_t src_version() const { return src_version_; }

  void set_src_version(uint64_t srcVersion) { src_version_ = srcVersion; }

  const VDiskType src_vdisk_type() const { return src_vdisk_type_; }

  void set_src_vdisk_type(VDiskType srcVdiskType) {
    src_vdisk_type_ = srcVdiskType;
  }

  const uint64_t shard_size() const { return shard_size_; }

  void set_shard_size(uint64_t shardSize) { shard_size_ = shardSize; }

  KoalaRouteMap& koala_routers() { return koala_routers_; }

  void set_koala_routers(KoalaRouteMap& koalaRouters) {
    koala_routers_.swap(koalaRouters);
  }

  const bool need_repair() const { return need_repair_; }

  void set_need_repair(bool needRepair) { need_repair_ = needRepair; }

  PcRouteMap& repair_pc_routers() { return repair_pc_routers_; }

  void set_repair_pc_routers(PcRouteMap& repairPcRouters) {
    repair_pc_routers_.swap(repairPcRouters);
  }

  const bool repair_job() const { return repair_job_; }

  void set_repair_job(bool isRepairJob) { repair_job_ = isRepairJob; }

 private:
  std::string chrono_id_;
  PbHelaTaskStatus status_;

  std::string vdisk_id_;
  uint32_t version_;
  VDiskType vdisk_type_;
  uint32_t lc_id_;
  uint32_t lc_random_id_;
  uint32_t lc_size_;
  uint64_t pc_size_;
  uint64_t cluster_version_;
  PcRouteMap pc_routers_;

  std::string src_vdisk_id_;  //就是udisk的id
  uint32_t src_version_;   //就是udisk的路由
  VDiskType src_vdisk_type_;   //vdisk的类型
  uint64_t shard_size_;
  KoalaRouteMap koala_routers_;  //方舟koala的路由

  bool need_repair_;   //是否需要修复
  PcRouteMap repair_pc_routers_;
  bool repair_job_;   //是否为修复的job
};

// ===================== ChunkSourceInfo ===================== //

struct ChunkSourceInfo {
  ChunkSourceInfo()
      : pc_no_(0),
        pg_id_(0),
        chunk_port_(0),
        chunk_id_(0),
        lc_id_(0),
        lc_random_id_(0),
        cluster_version_(0),
        offline_(false) {}

  uint32_t pc_no_;
  uint32_t pg_id_;

  std::string chunk_ip_;
  uint32_t chunk_port_;
  uint32_t chunk_id_;

  uint32_t lc_id_;
  uint32_t lc_random_id_;
  uint64_t cluster_version_;
  bool offline_;
};

// ===================== HelaPcTask ===================== //

class HelaPcTask {
 public:
  HelaPcTask();
  ~HelaPcTask();

  enum ResourceType {
    KOALA = 0,   //方舟读取的资源
    LOCAL_CHUNK = 1,   //本地chunk的资源
    REMOTE_CHUNK = 2, // 远端chunk的资源
  };

  enum TaskStatus {
    READY = 0,
    QUERY = 1,
    RUNNING = 2,
    DONE = 3,
    FAIL = 4,
  };

  TaskStatus status() const { return status_; }

  bool status_ready() { return status_ == READY; }

  void set_status_ready() { status_ = READY; }

  bool status_query() { return status_ == QUERY; }

  void set_status_query() { status_ = QUERY; }

  bool status_running() { return status_ == RUNNING; }

  void set_status_running() { status_ = RUNNING; }

  bool status_done() { return status_ == DONE; }

  void set_status_done() { status_ = DONE; }

  bool status_fail() { return status_ == FAIL; }

  void set_status_fail() { status_ = FAIL; }

  const uint32_t pc() const { return pc_; }

  void set_pc(uint32_t pc) { pc_ = pc; }

  const std::string& src_vdisk_id() const { return src_vdisk_id_; }

  void set_src_vdisk_id(const std::string& srcVDiskId) {
    src_vdisk_id_ = srcVDiskId;
  }

  const uint32_t lc_size() const { return lc_size_; }

  void set_lc_size(uint32_t lcSize) { lc_size_ = lcSize; }

  const uint64_t pc_size() const { return pc_size_; }

  void set_pc_size(uint64_t pcSize) { pc_size_ = pcSize; }

  const uint64_t cluster_version() const { return cluster_version_; }

  void set_cluster_version(uint64_t clusterVersion) {
    cluster_version_ = clusterVersion;
  }

  const VDiskType vdisk_type() const { return vdisk_type_; }

  void set_vdisk_type(VDiskType vdiskType) { vdisk_type_ = vdiskType; }

  TinyPcTaskInfoPtrList& info_list() { return info_list_; }

  void set_info_list(const TinyPcTaskInfoPtrList& infoList) {
    info_list_ = infoList;
  }

  ResourceType src_type() const { return src_type_; }

  void set_src_koala() { src_type_ = KOALA; }

  bool src_koala() { return src_type_ == KOALA; }

  void set_src_local_chunk() { src_type_ = LOCAL_CHUNK; }

  bool src_local_chunk() { return src_type_ == LOCAL_CHUNK; }

  void set_src_remote_chunk() { src_type_ = REMOTE_CHUNK; }

  bool src_remote_chunk() { return src_type_ == REMOTE_CHUNK; }

  const struct ChunkSourceInfo& chunk_source_info() const {
    return chunk_source_info_;
  }

  void set_chunk_source_info(const struct ChunkSourceInfo& chunkSourceInfo) {
    chunk_source_info_ = chunkSourceInfo;
  }

  const uint64_t shard_size() const { return shard_size_; }

  void set_shard_size(uint64_t shardSize) { shard_size_ = shardSize; }

  const uint32_t src_version() const { return src_version_; }

  void set_src_version(uint64_t srcVersion) { src_version_ = srcVersion; }

  const KoalaRouteMap& koala_routers() { return src_koala_router_; }

  void set_koala_routers(const KoalaRouteMap& srcKoalaRouter) {
    src_koala_router_ = srcKoalaRouter;
  }

  const bool has_data() const { return has_data_; }

  void set_has_data(bool hasData) { has_data_ = hasData; }

  const bool record_active() const { return record_active_; }

  void set_record_active(bool recordActive) { record_active_ = recordActive; }

 private:
  //任务目标pc_no, 源盘id
  uint32_t pc_;
  std::string src_vdisk_id_;

  TaskStatus status_;

  // common info
  uint32_t lc_size_;
  uint64_t pc_size_;
  uint64_t cluster_version_;
  VDiskType vdisk_type_;

  // tiny pc task info
  TinyPcTaskInfoPtrList info_list_;

  //源类型
  ResourceType src_type_;
  // Chunk源信息
  ChunkSourceInfo chunk_source_info_;
  // koala源信息
  uint64_t shard_size_;
  uint32_t src_version_;
  KoalaRouteMap src_koala_router_;
  bool has_data_;
  bool record_active_;
};

// ===================== TinyPcTaskInfo ===================== //

class TinyPcTaskInfo {
 public:
  TinyPcTaskInfo(const std::string& chrono_id);
  ~TinyPcTaskInfo();

  const std::string& chrono_id() const { return chrono_id_; }

  void set_chrono_id(const std::string& chronoId) { chrono_id_ = chronoId; }

  const bool offline() const { return offline_; }

  void set_offline(bool offLine) { offline_ = offLine; }

  const std::string& vdisk_id() const { return vdisk_id_; }

  void set_vdisk_id(const std::string& vdiskId) { vdisk_id_ = vdiskId; }

  const uint32_t version() const { return version_; }

  void set_version(uint32_t Version) { version_ = Version; }

  const VDiskType vdisk_type() const { return vdisk_type_; }

  void set_vdisk_type(VDiskType vdiskType) { vdisk_type_ = vdiskType; }

  const uint32_t lc_id() const { return lc_id_; }

  void set_lc_id(uint32_t lcId) { lc_id_ = lcId; }

  const uint32_t lc_random_id() const { return lc_random_id_; }

  void set_lc_random_id(uint32_t lcRandomId) { lc_random_id_ = lcRandomId; }

  const PcRoute& pc_router() const { return pc_router_; }

  void set_pc_router(const PcRoute& pcRouter) { pc_router_ = pcRouter; }

 private:
  std::string chrono_id_;

  bool offline_;

  std::string vdisk_id_;
  uint32_t version_;
  VDiskType vdisk_type_;

  uint32_t lc_id_;
  uint32_t lc_random_id_;
  PcRoute pc_router_;
};

}  // namespace hela
}  // namespace udisk

#endif
