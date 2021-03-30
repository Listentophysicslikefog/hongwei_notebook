// Copyright (c) 2020 UCloud All rights reserved.
#include "hela_context.h"
#include "ini_config.h"  // for config parser
#include <cerrno>
#include <cstring>
#include <cstdlib>

#include "manager_handle.h"

namespace udisk {
namespace hela {

const std::string HelaConfig::kIdunName = std::string("idun");
const std::string HelaConfig::kInstance = std::string("instance");
const std::string HelaConfig::kThreadNum = std::string("thread_num");
const std::string HelaConfig::kChunkNum = std::string("chunk_num");
const std::string HelaConfig::kMaxChunkWriteMbps =
    std::string("max_chunk_write_mbps");
const std::string HelaConfig::kMaxChunkReadMbps =
    std::string("max_chunk_read_mbps");
const std::string HelaConfig::kMaxReadRemoteMbps =
    std::string("max_read_remote_mbps");

const std::string HelaConfig::kSinglePcRetryBackoffInitialMS =
    std::string("single_pc_retry_backoff_initial_ms");
const std::string HelaConfig::kSinglePcRetryBackoffMaximumMS =
    std::string("single_pc_retry_backoff_maximum_ms");
const std::string HelaConfig::kSinglePcRetryBackoffStepMS =
    std::string("single_pc_retry_backoff_step_ms");
const std::string HelaConfig::kSinglePcPendingChangeKoalaTimes =
    std::string("single_pc_pending_change_koala_times");
const std::string HelaConfig::kSinglePcPendingWaitScheduleMS =
    std::string("single_pc_pending_wait_schedule_ms");
const std::string HelaConfig::kSinglePcPendingReadKoalaMS =
    std::string("single_pc_pending_read_koala_ms");
const std::string HelaConfig::kSinglePcPendingReadChunkMS =
    std::string("single_pc_pending_read_chunk_ms");
const std::string HelaConfig::kSinglePcPendingWriteChunkMS =
    std::string("single_pc_pending_write_chunk_ms");
const std::string HelaConfig::kSinglePcReadKoalaTimeoutSecond =
    std::string("single_pc_read_koala_timeout_second");
const std::string HelaConfig::kSinglePcReadChunkTimeoutSecond =
    std::string("single_pc_read_chunk_timeout_second");
const std::string HelaConfig::kSinglePcWriteChunkTimeoutSecond =
    std::string("single_pc_write_chunk_timeout_second");
const std::string HelaConfig::kSinglePcHugImagePendingTimes =
    std::string("single_pc_huge_image_pending_times");

const std::string HelaConfig::kBatchHelaTaskTimeoutSecond =
    std::string("batch_hela_task_timeout_second");
const std::string HelaConfig::kBatchHelaTaskScheduleSecond =
    std::string("batch_hela_task_schedule_second");
const std::string HelaConfig::kBatchHelaTaskStatisticsOn =
    std::string("batch_hela_task_statistics_on");
const std::string HelaConfig::kBatchHelaTaskScheduleShufferOn =
    std::string("batch_hela_taks_schedule_shuffle_on");
const std::string HelaConfig::kBatchTaskDispatchInterval =
    std::string("batch_task_dispatch_interval");
const std::string HelaConfig::kBatchTaskDispatchLimit =
    std::string("batch_task_dispatch_limit_num");
const std::string HelaConfig::kTotalTaskDispatchLimit =
    std::string("total_task_dispatch_limit_num");
const std::string HelaConfig::kTaskMergeThreshold =
    std::string("task_merge_threshold");
const std::string HelaConfig::kFlowReportInterval =
    std::string("flow_report_interval");
const std::string HelaConfig::kMemCheckInterval =
    std::string("mem_check_interval");
const std::string HelaConfig::kEnableDynamicConcurencePcNum =
    std::string("enable_dynamic_concurence_pc_num");
const std::string HelaConfig::kMaxConcurencePcNum =
    std::string("max_concurence_pc_num");
const std::string HelaConfig::kConcurenceSectors =
    std::string("concurence_sectors");
const std::string HelaConfig::kResourceExpiration =
    std::string("resource_expiration_second");

const std::string HelaConfig::kTransmissionListenPort =
    std::string("transmission_listen_port");
const std::string HelaConfig::kZkGetRouteInterval =
    std::string("zk_get_route_interval");

std::tuple<int, std::string> HelaConfig::init() {
  ConfigParser::Init();

  if (my_name_zk_path().empty()) {
    LOG_ERROR << "my name is empty";
    std::exit(1);
  }
  if (listen_ip().empty()) {
    LOG_ERROR << "listen_ip is empty";
    std::exit(1);
  }
  if (listen_port() == 0) {
    LOG_ERROR << "listen_port is empty";
    std::exit(1);
  }
  if (zk_server().empty()) {
    LOG_ERROR << "zk server is empty";
    std::exit(1);
  }
  if (metaserver_zk_path().empty()) {
    LOG_ERROR << "metaserver is empty";
    std::exit(1);
  }
  std::tuple<int, std::string> ret = load();
  if (std::get<0>(ret)) {
    std::string msg =
        "load conf " + config_file_ + " failed for " + std::get<1>(ret);
    return std::make_tuple(std::get<0>(ret), msg);
  }
  if (idun_zk_path().empty()) {
    LOG_ERROR << "idun is empty";
    std::exit(1);
  }
  return std::make_tuple(0, "");
}

std::tuple<int, std::string> HelaConfig::load() {
  int ret = 0;
  std::string msg("");

  idun_zk_path_ = parser_.GetValue(kSectionName, kIdunName);
  if (idun_zk_path_.empty()) {
    ret = -1;
    msg = "idun_zk_path_ is empty";
    return std::make_tuple(ret, msg);
  }

  std::string instance = parser_.GetValue(kSectionCommon, kInstance);
  if (instance.empty()) {
    ret = -1;
    msg = "instance is empty";
    return std::make_tuple(ret, msg);
  } else {
    instance_ = std::atoi(instance.c_str());
    LOG_INFO << "my hela instance: " << instance_;
  }

  int thread_num = parser_.IntValue(kSectionCommon, kThreadNum);
  if (thread_num != 0) {
    thread_num_ = thread_num;
  }
  LOG_INFO << "thread num: " << thread_num_;

  if (listen_ip().empty()) {
    ret = -1;
    msg = "listen_ip is empty";
    return std::make_tuple(ret, msg);
  }

  if (listen_port_ == 0) {
    ret = -1;
    msg = "listen_port is empty or 0";
    return std::make_tuple(ret, msg);
  }

  int chunk_num = parser_.IntValue(kSectionCommon, kChunkNum);
  if (chunk_num != 0) {
    chunk_num_ = chunk_num;
  }
  LOG_INFO << "chunk num: " << chunk_num_;

  int max_chunk_write_mbps =
      parser_.IntValue(kSectionCommon, kMaxChunkWriteMbps);
  if (max_chunk_write_mbps != 0) {
    max_chunk_write_bps_ = max_chunk_write_mbps << 20;
  }
  max_chunk_write_mbps = max_chunk_write_bps_ >> 20;
  LOG_INFO << "max chunk write mbps: " << max_chunk_write_mbps;

  int max_chunk_read_mbps = parser_.IntValue(kSectionCommon, kMaxChunkReadMbps);
  if (max_chunk_read_mbps != 0) {
    max_chunk_read_bps_ = max_chunk_read_mbps << 20;
  }
  max_chunk_read_mbps = max_chunk_read_bps_ >> 20;
  LOG_INFO << "max chunk read mbps: " << max_chunk_read_mbps;

  int max_read_remote_mbps =
      parser_.IntValue(kSectionCommon, kMaxReadRemoteMbps);
  if (max_read_remote_mbps != 0) {
    max_read_remote_bps_ = max_read_remote_mbps << 20;
  }
  max_read_remote_mbps = max_read_remote_bps_ >> 20;
  LOG_INFO << "max read remote mbps: " << max_read_remote_mbps;

  int single_pc_retry_backoff_initial_ms =
      parser_.IntValue(kSectionCommon, kSinglePcRetryBackoffInitialMS);
  if (single_pc_retry_backoff_initial_ms != 0) {
    single_pc_retry_backoff_initial_ms_ = single_pc_retry_backoff_initial_ms;
  }
  LOG_INFO << "single_pc_retry_backoff_initial_ms: "
           << single_pc_retry_backoff_initial_ms_;

  int single_pc_retry_backoff_maximum_ms =
      parser_.IntValue(kSectionCommon, kSinglePcRetryBackoffMaximumMS);
  if (single_pc_retry_backoff_maximum_ms != 0) {
    single_pc_retry_backoff_maximum_ms_ = single_pc_retry_backoff_maximum_ms;
  }
  LOG_INFO << "single_pc_retry_backoff_maximum_ms: "
           << single_pc_retry_backoff_maximum_ms_;

  int single_pc_retry_backoff_step_ms =
      parser_.IntValue(kSectionCommon, kSinglePcRetryBackoffInitialMS);
  if (single_pc_retry_backoff_step_ms != 0) {
    single_pc_retry_backoff_step_ms_ = single_pc_retry_backoff_step_ms;
  }
  LOG_INFO << "single_pc_retry_backoff_step_ms: "
           << single_pc_retry_backoff_step_ms_;

  int single_pc_pending_change_koala_times =
      parser_.IntValue(kSectionCommon, kSinglePcPendingChangeKoalaTimes);
  if (single_pc_pending_change_koala_times != 0) {
    single_pc_pending_change_koala_times_ =
        single_pc_pending_change_koala_times;
  }
  LOG_INFO << "single_pc_pending_change_koala_times: "
           << single_pc_pending_change_koala_times_;

  int single_pc_pending_wait_schedule_ms =
      parser_.IntValue(kSectionCommon, kSinglePcPendingWaitScheduleMS);
  if (single_pc_pending_wait_schedule_ms != 0) {
    single_pc_pending_wait_schedule_ms_ = single_pc_pending_wait_schedule_ms;
  }
  LOG_INFO << "single_pc_pending_wait_schedule_ms: "
           << single_pc_pending_wait_schedule_ms_
           << " single_pc_pending_wait_schedule_second: "
           << single_pc_pending_wait_schedule_second();

  int single_pc_pending_read_koala_ms =
      parser_.IntValue(kSectionCommon, kSinglePcPendingReadKoalaMS);
  if (single_pc_pending_read_koala_ms != 0) {
    single_pc_pending_read_koala_ms_ = single_pc_pending_read_koala_ms;
  }
  LOG_INFO << "single_pc_pending_read_koala_ms: "
           << single_pc_pending_read_koala_ms
           << " single_pc_pending_read_koala_second: "
           << single_pc_pending_read_koala_second();

  int single_pc_pending_read_chunk_ms =
      parser_.IntValue(kSectionCommon, kSinglePcPendingReadChunkMS);
  if (single_pc_pending_read_chunk_ms != 0) {
    single_pc_pending_read_chunk_ms_ = single_pc_pending_read_chunk_ms;
  }
  LOG_INFO << "single_pc_pending_read_chunk_ms: "
           << single_pc_pending_read_chunk_ms_
           << " single_pc_pending_read_chunk_second: "
           << single_pc_pending_read_chunk_second();

  int single_pc_pending_write_chunk_ms =
      parser_.IntValue(kSectionCommon, kSinglePcPendingWriteChunkMS);
  if (single_pc_pending_write_chunk_ms != 0) {
    single_pc_pending_write_chunk_ms_ = single_pc_pending_write_chunk_ms;
  }
  LOG_INFO << "single_pc_pending_write_chunk_ms: "
           << single_pc_pending_write_chunk_ms_
           << " single_pc_pending_write_chunk_second: "
           << single_pc_pending_write_chunk_second();

  int single_pc_read_koala_timeout_second =
      parser_.IntValue(kSectionCommon, kSinglePcReadKoalaTimeoutSecond);
  if (single_pc_read_koala_timeout_second != 0) {
    single_pc_read_koala_timeout_second_ = single_pc_read_koala_timeout_second;
  }
  LOG_INFO << "single_pc_read_koala_timeout_second: "
           << single_pc_read_koala_timeout_second_;

  int single_pc_read_chunk_timeout_second =
      parser_.IntValue(kSectionCommon, kSinglePcReadChunkTimeoutSecond);
  if (single_pc_read_chunk_timeout_second != 0) {
    single_pc_read_chunk_timeout_second_ = single_pc_read_chunk_timeout_second;
  }
  LOG_INFO << "single_pc_read_chunk_timeout_second: "
           << single_pc_read_chunk_timeout_second_;

  int single_pc_write_chunk_timeout_second =
      parser_.IntValue(kSectionCommon, kSinglePcWriteChunkTimeoutSecond);
  if (single_pc_write_chunk_timeout_second != 0) {
    single_pc_write_chunk_timeout_second_ =
        single_pc_write_chunk_timeout_second;
  }
  LOG_INFO << "single_pc_write_chunk_timeout_second: "
           << single_pc_write_chunk_timeout_second_;

  int single_pc_huge_image_pending_times =
      parser_.IntValue(kSectionCommon, kSinglePcHugImagePendingTimes);
  if (single_pc_huge_image_pending_times != 0) {
    single_pc_huge_image_pending_times_ = single_pc_huge_image_pending_times;
  }
  LOG_INFO << "single_pc_huge_image_pending_times: "
           << single_pc_huge_image_pending_times_;

  int batch_hela_task_timeout_second =
      parser_.IntValue(kSectionCommon, kBatchHelaTaskTimeoutSecond);
  if (batch_hela_task_timeout_second != 0) {
    batch_hela_task_timeout_second_ = batch_hela_task_timeout_second;
  }
  LOG_INFO << "batch hela task timeout(s): " << batch_hela_task_timeout_second_;

  int batch_hela_task_schedule_second =
      parser_.IntValue(kSectionCommon, kBatchHelaTaskScheduleSecond);
  if (batch_hela_task_schedule_second != 0) {
    batch_hela_task_schedule_second_ = batch_hela_task_schedule_second;
  }
  LOG_INFO << "batch_hela_task_schedule_second(s): "
           << batch_hela_task_schedule_second;

  int batch_hela_task_statistics_on =
      parser_.IntValue(kSectionCommon, kBatchHelaTaskStatisticsOn);
  batch_hela_task_statistics_on_ =
      batch_hela_task_statistics_on == 0 ? false : true;
  LOG_INFO << "batch_hela_task_statistics_on: "
           << batch_hela_task_statistics_on_;

  int batch_hela_taks_schedule_shuffle_on =
      parser_.IntValue(kSectionCommon, kBatchHelaTaskScheduleShufferOn);
  batch_hela_taks_schedule_shuffle_on_ =
      batch_hela_taks_schedule_shuffle_on == 0 ? false : true;
  LOG_INFO << "batch_hela_taks_schedule_shuffle_on: "
           << batch_hela_taks_schedule_shuffle_on_;

  int batch_task_dispatch_interval =
      parser_.IntValue(kSectionCommon, kBatchTaskDispatchInterval);
  if (batch_task_dispatch_interval != 0) {
    batch_task_dispatch_interval_ = batch_task_dispatch_interval;
  }
  LOG_INFO << "batch_task_dispatch_interval: " << batch_task_dispatch_interval_;

  int batch_task_dispatch_limit_num =
      parser_.IntValue(kSectionCommon, kBatchTaskDispatchLimit);
  if (batch_task_dispatch_limit_num != 0) {
    batch_task_dispatch_limit_num_ = batch_task_dispatch_limit_num;
  }
  LOG_INFO << "batch task dispatch limit: " << batch_task_dispatch_limit_num_;

  int total_task_dispatch_limit_num =
      parser_.IntValue(kSectionCommon, kTotalTaskDispatchLimit);
  if (total_task_dispatch_limit_num != 0) {
    total_task_dispatch_limit_num_ = total_task_dispatch_limit_num;
  }
  LOG_INFO << "total task dispatch limit: " << total_task_dispatch_limit_num_;

  int task_merge_threshold =
      parser_.IntValue(kSectionCommon, kTaskMergeThreshold);
  if (task_merge_threshold != 0) {
    task_merge_threshold_ = task_merge_threshold;
  }
  LOG_INFO << "task merge threshold: " << task_merge_threshold_;

  int flow_report_interval =
      parser_.IntValue(kSectionCommon, kFlowReportInterval);
  if (flow_report_interval != 0) {
    flow_report_interval_ = flow_report_interval;
  }
  LOG_INFO << "flow report interval(s): " << flow_report_interval_;

  int mem_check_interval = parser_.IntValue(kSectionCommon, kMemCheckInterval);
  if (mem_check_interval != 0) {
    mem_check_interval_ = mem_check_interval;
  }
  LOG_INFO << "mem_check_interval(s): " << mem_check_interval_;

  int enable_dynamic_concurence_pc_num =
      parser_.IntValue(kSectionCommon, kEnableDynamicConcurencePcNum);
  if (enable_dynamic_concurence_pc_num == 0) {
    enable_dynamic_concurence_pc_num_ = false;
  } else {
    enable_dynamic_concurence_pc_num_ = true;
  }
  LOG_INFO << "enable_dynamic_concurence_pc_num: "
           << enable_dynamic_concurence_pc_num_;

  int max_concurence_pc_num =
      parser_.IntValue(kSectionCommon, kMaxConcurencePcNum);
  if (max_concurence_pc_num != 0) {
    max_concurence_pc_num_ = max_concurence_pc_num;
  }
  LOG_INFO << "max concurence pc num: " << max_concurence_pc_num_;

  int concurence_sectors = parser_.IntValue(kSectionCommon, kConcurenceSectors);
  if (concurence_sectors != 0) {
    concurence_sectors_ = concurence_sectors;
  }
  LOG_INFO << "concurence_sectors: " << concurence_sectors_;

  operate_size_ = concurence_sectors_ * udisk::common::DEFAULT_SECTOR_SIZE;
  LOG_INFO << "operate_size: " << operate_size_;

  int resource_expiration_second =
      parser_.IntValue(kSectionCommon, kResourceExpiration);
  if (resource_expiration_second != 0) {
    resource_expiration_second_ = resource_expiration_second;
  }
  LOG_INFO << "resource_expiration_second(s): " << resource_expiration_second_;

  transmission_listen_port_ =
      parser_.IntValue(kSectionNetworks, kTransmissionListenPort);
  if (transmission_listen_port_ == 0) {
    ret = -1;
    msg = "transmission_listen_port is empty or 0";
    return std::make_tuple(ret, msg);
  }

  zk_get_route_interval_ =
      parser_.IntValue(kSectionZookeeper, kZkGetRouteInterval);
  if (zk_get_route_interval_ == 0) {
    zk_get_route_interval_ = 10;
  }
  LOG_INFO << "zk_get_route_interval: " << zk_get_route_interval_;

  return std::make_tuple(ret, msg);
}

HelaContext::HelaContext(const std::string &conf_file)
    : config_(conf_file),
      flow_monitor_(nullptr),
      manager_handle_(nullptr),
      manager_loop_(nullptr),
      manager_listener_(nullptr),
      transmission_listen_loop_(nullptr),
      transmission_listener_(nullptr),
      nc_(nullptr),
      zk_reg_success_(false) {}

std::tuple<int, std::string> HelaContext::init() { return config_.init(); }

std::pair<std::string, int> HelaContext::GetZkRouteCache(
    const std::string &name) {
  auto iter = cache_zk_route_.find(name);
  if (iter != cache_zk_route_.end()) {
    return iter->second;
  }
  return std::make_pair("", 0);
}

std::pair<std::string, int> HelaContext::GetZkRoute(const std::string &name) {
  assert(nc_ != nullptr);
  int ret = -1;
  ucloud::uns::NameNodeContent namenode;
  if (name == udisk::common::ConfigParser::kMetaServerName) {
    ret =
        nc_->GetNNCForNode(g_context->config().zk_server(),
                           g_context->config().metaserver_zk_path(),
                           udisk::common::ConfigParser::kLeaderNode, namenode);
  } else if (name == HelaConfig::kIdunName) {
    ret = nc_->GetNNCForNode(
        g_context->config().zk_server(), g_context->config().idun_zk_path(),
        udisk::common::ConfigParser::kLeaderNode, namenode);
  } else {
    return std::make_pair("", 0);
  }

  if (ret) {
    return std::make_pair("", 0);
  }
  return std::make_pair(namenode.ip(), namenode.port());
}

void HelaContext::UpdateZkRoute(const std::string &name, const std::string &ip,
                                int port) {
  auto iter = cache_zk_route_.find(name);
  if (iter != cache_zk_route_.end()) {
    std::pair<std::string, int> ip_info = iter->second;
    if (ip != ip_info.first || port != ip_info.second) {
      if (name == udisk::common::ConfigParser::kMetaServerName) {
        LOG_INFO << "ip info has changed, name: " << name
                 << " old ip: " << ip_info.first << " port: " << ip_info.second
                 << " new ip: " << ip << " port: " << port;
      }
      cache_zk_route_[name] = std::make_pair(ip, port);
    }
  } else {
    LOG_INFO << "first update ip info, name: " << name << " ip: " << ip
             << " port: " << port;
    cache_zk_route_[name] = std::make_pair(ip, port);
  }
}

// 在管理线程中更新IP信息
void HelaContext::ZkRouteTimerTask() {
  std::pair<std::string, int> result =
      g_context->GetZkRoute(HelaConfig::kIdunName);
  g_context->manager_handle()->GetLoop()->RunInLoop(
      std::bind(&HelaContext::UpdateZkRoute, this, HelaConfig::kIdunName,
                result.first, result.second));

  result = g_context->GetZkRoute(udisk::common::ConfigParser::kMetaServerName);
  g_context->manager_handle()->GetLoop()->RunInLoop(
      std::bind(&HelaContext::UpdateZkRoute, this,
                udisk::common::ConfigParser::kMetaServerName, result.first,
                result.second));
}

// ZK线程定时拉取IP信息
void HelaContext::AddZkRouteTimer() {
  // 获取nc缓存
  AddZkRouteTask();

  g_context->zk_loop()->RunEvery(
      g_context->config().zk_get_route_interval(),
      std::bind(&HelaContext::ZkRouteTimerTask, this));
}

// 第一次同步获取nc缓存
void HelaContext::AddZkRouteTask() {
  std::pair<std::string, int> result =
      g_context->GetZkRoute(HelaConfig::kIdunName);
  if (result.second == 0) {
    LOG_ERROR << "get ip and port failed for: " << HelaConfig::kIdunName;
    std::exit(-1);
  }
  UpdateZkRoute(HelaConfig::kIdunName, result.first, result.second);

  result = g_context->GetZkRoute(udisk::common::ConfigParser::kMetaServerName);
  if (result.second == 0) {
    LOG_ERROR << "get ip and port failed for: "
              << udisk::common::ConfigParser::kUmongoName;
    std::exit(-1);
  }
  UpdateZkRoute(udisk::common::ConfigParser::kMetaServerName, result.first,
                result.second);
}

};  // end of ns hela
};  // end of ns udisk

// 在进程启动后，初始化成功后，设置
udisk::hela::HelaContext *g_context = nullptr;
