// Copyright (c) 2020 UCloud All rights reserved.
#ifndef UDISK_HELA_DO_HELA_TASK_H_
#define UDISK_HELA_DO_HELA_TASK_H_

#include "hela_dec.h"
#include "manager_handle.h"
#include "resource_manager.h"
#include "umessage_common.h"
#include "uevent.h"
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <iostream>

namespace udisk {
namespace hela {

using namespace uevent;

class DoHelaTaskHandle : public std::enable_shared_from_this<DoHelaTaskHandle> {
 public:
  DoHelaTaskHandle();
  ~DoHelaTaskHandle();

  void set_timeout(bool timeout) { timeout_ = timeout; }

  bool timeout() const { return timeout_; }

  void TaskTimeOut();
  void UpdateIdunTimeout();
  void QueryIdunTimeout(const std::vector<HelaPcTaskPtr>& query_task);

  void SetTaskTimer();
  void CancalTaskTimer();
  void SetScheduleTimer();
  void CancalScheduleTimer();

  void DumpAllTaskInfo();
  void SetAllTaskExecuting();
  void SetAllTaskDone();
  void SetAllTaskFail();

  void IncResourceRef(const HelaPcTaskPtr& pc_task);
  void DecResourceRef(const HelaPcTaskPtr& pc_task);
  void DecAllResourceRef();
  void UpdateLocalResource(const HelaPcTaskPtr& pc_task);

  void Start(const HelaTaskPtrList& task_list_);

  void CheckTaskListValid();

  void CheckClusterVersion();

  void DumpPcTaskInfo();
  void ConstructHelaPcTask();
  void ShuffleHelaPcTask();

  bool CheckLocalResource(const HelaPcTaskPtr& pc_task);
  bool QueryIdunResource(const std::vector<HelaPcTaskPtr>& query_task);
  void EntryQueryIdunResource(ucloud::UMessage* um,
                              const std::vector<HelaPcTaskPtr>& query_task);

  int ExecutingPcTaskNum();
  int ReadyPcTaskNum();

  void ScheduleHelaTaskRoutine();
  void ScheduleHelaPcTask();
  void HandleHelaPcTask(const std::vector<HelaPcTaskPtr>& schedule_task);
  void DispatchHelaPcTask(const HelaPcTaskPtr& pc_task);
  std::string DumpHandleInfo();
  void HandlePcTaskResult(int retcode, uint32_t pc_no);

  void CheckHelaTaskFinish();
  bool UpdateIdunResource();
  void EntryUpdateIdunResource(ucloud::UMessage* um);

 private:
  HelaTaskPtrList task_list_;
  HelaPcTaskPtrMap pc_task_map_;
  HelaPcTaskPtrMapIter schedule_iter_;
  KoalaRouteMap koala_routers_;

  uevent::TimerId timer_id_;
  uevent::TimerId timer_id_2_;
  bool timeout_;

  std::string src_vdisk_id_;  // 镜像源 disk id
  uint32_t lc_size_;  // lc盘的大小
  uint64_t pc_size_;   // pc分片的大小
  uint64_t shard_size_;
  uint32_t cluster_version_;  //应该是chunk集群版本
  uint32_t src_version_;  //镜像源集群版本

  int max_concurence_num_;  //最大并发量
  int t_total_num_;     //总共的数量
  int t_running_num_;  //正在进行的数量
  int t_finish_num_;  //结束的数量
  int t_succ_num_;    // 成功的数量
  int t_fail_num_;    //失败的数量
  bool t_completed_;  //完成的数量

  std::vector<HelaPcTaskPtr> update_idun_task_;   //typedef std::shared_ptr<HelaPcTask> HelaPcTaskPtr;

  ResourceManager* resource_manager_;

  static uint32_t dispatch_loop_idx_;
};

}  // namespace hela
}  // namespace udisk

#endif
