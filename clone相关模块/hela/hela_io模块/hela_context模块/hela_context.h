// Copyright (c) 2020 UCloud All rights reserved.
#ifndef UDISK_HELA_CONTEXT_H_
#define UDISK_HELA_CONTEXT_H_

#include <string>
#include <cassert>
#include <tuple>
#include <map>
#include <atomic>
#include <xutil/config_parser.h>
#include <uevent/zk/name_container.h>

#include "flow_monitor.h"
#include "uevent.h"
#include "constanst.h"

namespace udisk {
namespace hela {

class ManagerHandle;
class TransmissionHandle;

// hela的配置对象，在进程启动的时候，
// 从配置文件加载到内存中。
class HelaConfig : public udisk::common::ConfigParser {
 public:
  // 加载配置文件
  std::tuple<int, std::string> init();

  // common info
  int instance() const { return instance_; }
  int thread_num() const { return thread_num_; }
  int chunk_num() const { return chunk_num_; }
  uint64_t max_chunk_write_bps() const { return max_chunk_write_bps_; }
  uint64_t max_chunk_read_bps() const { return max_chunk_read_bps_; }
  uint64_t max_read_remote_bps() const { return max_read_remote_bps_; }

  int single_pc_retry_backoff_initial_ms() const {
    return single_pc_retry_backoff_initial_ms_;
  }
  int single_pc_retry_backoff_maximum_ms() const {
    return single_pc_retry_backoff_maximum_ms_;
  }
  int single_pc_retry_backoff_step_ms() const {
    return single_pc_retry_backoff_step_ms_;
  }
  int single_pc_pending_change_koala_times() const {
    return single_pc_pending_change_koala_times_;
  }
  int single_pc_pending_wait_schedule_ms() const {
    return single_pc_pending_wait_schedule_ms_;
  }
  double single_pc_pending_wait_schedule_second() const {
    return (double)single_pc_pending_wait_schedule_ms_ / 1000.0;
  }

  int single_pc_pending_read_koala_ms() const {
    return single_pc_pending_read_koala_ms_;
  }
  double single_pc_pending_read_koala_second() const {
    return single_pc_pending_read_koala_ms_ / 1000.0;
  }

  int single_pc_pending_read_chunk_ms() const {
    return single_pc_pending_read_chunk_ms_;
  }
  double single_pc_pending_read_chunk_second() const {
    return single_pc_pending_read_chunk_ms_ / 1000.0;
  }

  int single_pc_pending_write_chunk_ms() const {
    return single_pc_pending_write_chunk_ms_;
  }
  double single_pc_pending_write_chunk_second() const {
    return single_pc_pending_write_chunk_ms_ / 1000.0;
  }

  int single_pc_read_koala_timeout_second() const {
    return single_pc_read_koala_timeout_second_;
  }
  int single_pc_read_chunk_timeout_second() const {
    return single_pc_read_chunk_timeout_second_;
  }
  int single_pc_write_chunk_timeout_second() const {
    return single_pc_write_chunk_timeout_second_;
  }
  int single_pc_huge_image_pending_times() const {
    return single_pc_huge_image_pending_times_;
  }

  int batch_hela_task_timeout_second() const {
    return batch_hela_task_timeout_second_;
  }
  int batch_hela_task_schedule_second() const {
    return batch_hela_task_schedule_second_;
  }
  bool batch_hela_task_statistics_on() const {
    return batch_hela_task_statistics_on_;
  }
  bool batch_hela_taks_schedule_shuffle_on() const {
    return batch_hela_taks_schedule_shuffle_on_;
  }
  int batch_task_dispatch_interval() const {
    return batch_task_dispatch_interval_;
  }
  int batch_task_dispatch_limit_num() const {
    return batch_task_dispatch_limit_num_;
  }
  int total_task_dispatch_limit_num() const {
    return total_task_dispatch_limit_num_;
  }
  int task_merge_threshold() const { return task_merge_threshold_; }
  int flow_report_interval() const { return flow_report_interval_; }
  int mem_check_interval() const { return mem_check_interval_; }
  bool enable_dynamic_concurence_pc_num() const {
    return enable_dynamic_concurence_pc_num_;
  }
  int max_concurence_pc_num() const { return max_concurence_pc_num_; }
  int concurence_sectors() const { return concurence_sectors_; }
  uint32_t operate_size() const { return operate_size_; }
  int resource_expiration_second() const { return resource_expiration_second_; }

  // network
  int listen_port() const { return listen_port_; }
  int transmission_listen_port() const { return transmission_listen_port_; }

  // zookeeper
  inline std::string idun_zk_path() const { return idun_zk_path_; }
  int zk_get_route_interval() const { return zk_get_route_interval_; }

  // 从文件加载配置项,覆盖当前内容
  // 失败的话返回(非0,错误消息),
  // 否则返回(0,"")
  std::tuple<int, std::string> load();

  static const std::string kIdunName;
  static const std::string kInstance;
  static const std::string kThreadNum;
  static const std::string kChunkNum;
  static const std::string kMaxChunkWriteMbps;
  static const std::string kMaxChunkReadMbps;
  static const std::string kMaxReadRemoteMbps;

  // sing pc param
  static const std::string kSinglePcRetryBackoffInitialMS;
  static const std::string kSinglePcRetryBackoffMaximumMS;
  static const std::string kSinglePcRetryBackoffStepMS;
  static const std::string kSinglePcPendingChangeKoalaTimes;
  static const std::string kSinglePcPendingWaitScheduleMS;
  static const std::string kSinglePcPendingReadKoalaMS;
  static const std::string kSinglePcPendingReadChunkMS;
  static const std::string kSinglePcPendingWriteChunkMS;
  static const std::string kSinglePcReadKoalaTimeoutSecond;
  static const std::string kSinglePcReadChunkTimeoutSecond;
  static const std::string kSinglePcWriteChunkTimeoutSecond;
  static const std::string kSinglePcHugImagePendingTimes;

  static const std::string kBatchHelaTaskTimeoutSecond;
  static const std::string kBatchHelaTaskScheduleSecond;
  static const std::string kBatchHelaTaskStatisticsOn;
  static const std::string kBatchHelaTaskScheduleShufferOn;
  static const std::string kBatchTaskDispatchInterval;
  static const std::string kBatchTaskDispatchLimit;

  static const std::string kTotalTaskDispatchLimit;
  static const std::string kTaskMergeThreshold;
  static const std::string kFlowReportInterval;
  static const std::string kMemCheckInterval;
  static const std::string kEnableDynamicConcurencePcNum;
  static const std::string kMaxConcurencePcNum;
  static const std::string kConcurenceSectors;
  static const std::string kResourceExpiration;

  static const std::string kTransmissionListenPort;
  static const std::string kZkGetRouteInterval;

 private:
  explicit HelaConfig(const std::string &file)
      : ConfigParser(file),
        instance_(0),
        thread_num_(10),
        chunk_num_(6),
        max_chunk_write_bps_(800 << 20),
        max_chunk_read_bps_(800 << 20),
        max_read_remote_bps_(800 << 20),
        single_pc_retry_backoff_initial_ms_(2000),
        single_pc_retry_backoff_maximum_ms_(20000),
        single_pc_retry_backoff_step_ms_(2000),
        single_pc_pending_change_koala_times_(100),
        single_pc_pending_wait_schedule_ms_(100),
        single_pc_pending_read_koala_ms_(100),
        single_pc_pending_read_chunk_ms_(300),
        single_pc_pending_write_chunk_ms_(500),
        single_pc_read_koala_timeout_second_(15),
        single_pc_read_chunk_timeout_second_(15),
        single_pc_write_chunk_timeout_second_(10),
        single_pc_huge_image_pending_times_(10),
        batch_hela_task_timeout_second_(1800),
        batch_hela_task_statistics_on_(false),
        batch_hela_taks_schedule_shuffle_on_(true),
        batch_task_dispatch_interval_(1),
        batch_task_dispatch_limit_num_(5),
        total_task_dispatch_limit_num_(60),
        task_merge_threshold_(5),
        flow_report_interval_(1),
        mem_check_interval_(300),
        enable_dynamic_concurence_pc_num_(false),
        max_concurence_pc_num_(10),
        concurence_sectors_(2048),
        resource_expiration_second_(3600),
        operate_size_(0),
        transmission_listen_port_(0) {}

  HelaConfig(const HelaConfig &) = delete;
  HelaConfig &operator=(const HelaConfig &) = delete;

  // 只允许HelaContext构造对象
  friend class HelaContext;

  std::string config_file_;

  // common info
  int instance_;
  int thread_num_;
  int chunk_num_;
  uint64_t max_chunk_write_bps_;
  uint64_t max_chunk_read_bps_;
  uint64_t max_read_remote_bps_;
  int single_pc_retry_backoff_initial_ms_;
  int single_pc_retry_backoff_maximum_ms_;
  int single_pc_retry_backoff_step_ms_;
  int single_pc_pending_change_koala_times_;
  int single_pc_pending_wait_schedule_ms_;
  int single_pc_pending_read_koala_ms_;
  int single_pc_pending_read_chunk_ms_;
  int single_pc_pending_write_chunk_ms_;
  int single_pc_read_koala_timeout_second_;
  int single_pc_read_chunk_timeout_second_;
  int single_pc_write_chunk_timeout_second_;
  int single_pc_huge_image_pending_times_;
  int batch_hela_task_timeout_second_;
  int batch_hela_task_schedule_second_;
  bool batch_hela_task_statistics_on_;
  bool batch_hela_taks_schedule_shuffle_on_;
  int batch_task_dispatch_interval_;
  int batch_task_dispatch_limit_num_;

  int total_task_dispatch_limit_num_;
  int task_merge_threshold_;
  int flow_report_interval_;
  int mem_check_interval_;
  bool enable_dynamic_concurence_pc_num_;
  int max_concurence_pc_num_;
  int concurence_sectors_;
  int resource_expiration_second_;
  uint32_t operate_size_;

  // network
  int transmission_listen_port_;

  // zookeeper
  std::string idun_zk_path_;
  int zk_get_route_interval_;
};

// 这个类管理所有的全局
// 变量，包括日志对象，配置
// 文件对象等等
class HelaContext {
 public:
  explicit HelaContext(const std::string &conf_file);

  HelaContext(const HelaContext &) = delete;
  HelaContext &operator=(const HelaContext &) = delete;

  // 初始化开始
  std::tuple<int, std::string> init();

  const HelaConfig &config() const {
    return config_;
  };

  HelaConfig *mutable_config() { return &config_; }

  void set_flow_monitor(FlowMonitor *flow_monitor) {
    flow_monitor_ = flow_monitor;
  }
  FlowMonitor *flow_monitor() { return flow_monitor_; }

  void set_manager_handle(ManagerHandle *manager_handle) {
    manager_handle_ = manager_handle;
  }
  ManagerHandle *manager_handle() { return manager_handle_; }
  void set_manager_listener(uevent::ListenerUevent *listener) {
    manager_listener_ = listener;
  }
  uevent::ListenerUevent *manager_listener() { return manager_listener_; }
  void set_manager_loop(uevent::UeventLoop *loop) { manager_loop_ = loop; }
  uevent::UeventLoop *manager_loop() { return manager_loop_; }

  void set_transmission_listener(uevent::ListenerUevent *listener) {
    transmission_listener_ = listener;
  }
  uevent::ListenerUevent *transmission_listener() {
    return transmission_listener_;
  }

  void set_transmission_listen_loop(uevent::UeventLoop *loop) {
    transmission_listen_loop_ = loop;
  }
  uevent::UeventLoop *transmission_listen_loop() {
    return transmission_listen_loop_;
  }

  void set_nc(uevent::NameContainer *nc) { nc_ = nc; }
  uevent::NameContainer *nc() { return nc_; }

  uevent::UeventLoop *zk_loop() { return zk_loop_; }
  void set_zk_loop(uevent::UeventLoop *zk_loop) { zk_loop_ = zk_loop; }

  void set_zk_reg_success(bool success) { zk_reg_success_.store(success); }
  bool zk_reg_success() const { return zk_reg_success_.load(); }

  // 当路径对应的服务不存在时，返回("",0)
  std::pair<std::string, int> GetZkRouteCache(const std::string &name);
  std::pair<std::string, int> GetZkRoute(const std::string &name);
  void UpdateZkRoute(const std::string &name, const std::string &ip, int port);
  void ZkRouteTimerTask();
  void AddZkRouteTimer();
  void AddZkRouteTask();

 private:
  HelaConfig config_;

  FlowMonitor *flow_monitor_;

  ManagerHandle *manager_handle_;
  uevent::UeventLoop *manager_loop_;
  uevent::ListenerUevent *manager_listener_;

  uevent::UeventLoop *transmission_listen_loop_;
  uevent::ListenerUevent *transmission_listener_;

  uevent::NameContainer *nc_;
  uevent::UeventLoop *zk_loop_ = nullptr;
  std::atomic_bool zk_reg_success_;

  // name -- <ip, port>
  std::map<std::string, std::pair<std::string, int>> cache_zk_route_;
};

};  // end of ns hela
};  // end of ns udisk

extern udisk::hela::HelaContext *g_context;

#endif
