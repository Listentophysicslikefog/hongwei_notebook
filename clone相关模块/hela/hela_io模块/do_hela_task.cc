// Copyright (c) 2020 UCloud All rights reserved.
#include "do_hela_task.h"

#include "hela_task.h"
#include "hela_helper.h"
#include "hela_context.h"
#include "do_hela_pc_task.h"
#include "manager_handle.h"
#include "transmission_handle.h"
#include "resource_manager.h"
#include "constanst.h"
#include "message_util.h"
#include "logging.h"
#include "my_uuid.h"
#include <sstream>
#include <random>

namespace udisk {
namespace hela {

using namespace udisk::common;

uint32_t DoHelaTaskHandle::dispatch_loop_idx_ = 0;

DoHelaTaskHandle::DoHelaTaskHandle()
    : timeout_(false),
      lc_size_(0),
      pc_size_(0),
      shard_size_(0),
      cluster_version_(0),
      src_version_(0),
      max_concurence_num_(g_context->config().max_concurence_pc_num()),
      t_total_num_(0),
      t_running_num_(0),
      t_finish_num_(0),
      t_succ_num_(0),
      t_fail_num_(0),
      t_completed_(false),
      resource_manager_(nullptr) {
  resource_manager_ = g_context->manager_handle()->LocalResourceManager();
  assert(resource_manager_ != nullptr);
}

DoHelaTaskHandle::~DoHelaTaskHandle() {
  LOG_TRACE << "DoHelaTaskHandle::~DoHelaTaskHandle";
}

void DoHelaTaskHandle::TaskTimeOut() {
  LOG_ERROR << "desc: DoHelaTaskHandle timeout";
  set_timeout(true);

  // 清除所有的本地资源引用计数
  DecAllResourceRef();

  // 标志所有sub task失败
  SetAllTaskFail();
}

void DoHelaTaskHandle::UpdateIdunTimeout() {
  LOG_ERROR << "desc: update idun timeout";
}

void DoHelaTaskHandle::QueryIdunTimeout(
    const std::vector<HelaPcTaskPtr>& query_task) {
  LOG_ERROR << "desc: query idun timeout";
  for (auto& pc_task : query_task) {
    LOG_INFO << "desc: hit koala resource"
             << " pc_no(" << pc_task->pc() << ")"
             << " src_vdisk_id(" << pc_task->src_vdisk_id() << ")";
    DispatchHelaPcTask(pc_task);
  }
}

void DoHelaTaskHandle::SetTaskTimer() {
  timer_id_ = g_context->manager_handle()->GetLoop()->RunAfter(
      g_context->config().batch_hela_task_timeout_second(),
      std::bind(&DoHelaTaskHandle::TaskTimeOut, shared_from_this()));
}

void DoHelaTaskHandle::CancalTaskTimer() {
  g_context->manager_handle()->GetLoop()->CancelTimer(timer_id_);
}

void DoHelaTaskHandle::SetScheduleTimer() {
  timer_id_2_ = g_context->manager_handle()->GetLoop()->RunEvery(
      g_context->config().batch_hela_task_schedule_second(),
      std::bind(&DoHelaTaskHandle::ScheduleHelaTaskRoutine,
                shared_from_this()));
}

void DoHelaTaskHandle::CancalScheduleTimer() {
  g_context->manager_handle()->GetLoop()->CancelTimer(timer_id_2_);
}

void DoHelaTaskHandle::DumpAllTaskInfo() {
  LOG_DEBUG << "desc: Dump Task Info";
  for (auto& it : task_list_) {
    LOG_DEBUG << " chrono_id(" << it->chrono_id() << ")"
              << " vdisk_id(" << it->vdisk_id() << ")"
              << " src_vdisk_id(" << it->src_vdisk_id() << ")";
  }
}

void DoHelaTaskHandle::SetAllTaskExecuting() {
  for (auto& it : task_list_) {
    it->set_status_executing();
  }
}

void DoHelaTaskHandle::SetAllTaskDone() {
  for (auto& it : task_list_) {
    if (it->need_repair()) {
      LOG_INFO << "desc: need repair"
               << " chrono_id(" << it->chrono_id() << ")"
               << " vdisk_id(" << it->vdisk_id() << ")"
               << " lc_id(" << it->lc_id() << ")"
               << " src_vdisk_id(" << it->src_vdisk_id() << ")";

      it->set_status_ready();
      it->set_need_repair(false);
      it->set_repair_job(true);

      it->set_pc_routers(it->repair_pc_routers());
      continue;
    }
    it->set_status_done();
  }
}

void DoHelaTaskHandle::SetAllTaskFail() {
  for (auto& it : task_list_) {
    it->set_status_fail();
  }
}

void DoHelaTaskHandle::IncResourceRef(const HelaPcTaskPtr& pc_task) {
  if (pc_task->src_koala()) return;

  LOG_TRACE << "desc: inc resource ref"
            << " source_lc_id(" << pc_task->chunk_source_info().lc_id_ << ")"
            << " chrono_id(" << task_list_.front()->chrono_id() << ")";
  resource_manager_->IncResourceRef(pc_task->chunk_source_info().lc_id_);
}

void DoHelaTaskHandle::DecResourceRef(const HelaPcTaskPtr& pc_task) {
  if (pc_task->src_koala()) return;

  LOG_TRACE << "desc: dec resource ref"
            << " source_lc_id(" << pc_task->chunk_source_info().lc_id_ << ")"
            << " chrono_id(" << task_list_.front()->chrono_id() << ")";
  resource_manager_->DecResourceRef(pc_task->chunk_source_info().lc_id_);
}

void DoHelaTaskHandle::DecAllResourceRef() {
  for (auto& it : pc_task_map_) {
    const HelaPcTaskPtr& pc_task = it.second;

    if (pc_task->src_koala()) continue;
    if (!pc_task->status_running()) continue;

    resource_manager_->DecResourceRef(pc_task->chunk_source_info().lc_id_);
  }
}

void DoHelaTaskHandle::UpdateLocalResource(const HelaPcTaskPtr& pc_task) {
  PCResourcePtrList pc_resource_list =
      HelaHelper::AssemblePCResourceList(pc_task);

  resource_manager_->AddResourceList(pc_resource_list);
}

void DoHelaTaskHandle::Start(const HelaTaskPtrList& task_list) {
  task_list_ = task_list;

  if (task_list_.empty()) {
    LOG_ERROR << "desc: hela task list empty";
    return;
  }

  LOG_DEBUG << "desc: DoHelaTaskHandle start";

  SetTaskTimer();

  SetAllTaskExecuting();

  DumpAllTaskInfo();

  CheckTaskListValid();

  CheckClusterVersion();

  ConstructHelaPcTask();

  DumpPcTaskInfo();

  SetScheduleTimer();

  schedule_iter_ = pc_task_map_.begin();

  ScheduleHelaPcTask();
}

// 批量执行的task信息校验
// a. 关键信息src_vdisk_id/lc_size/pc_size/shard_size必须一致
// b.
// [TODO]镜像源信息如果不一致，进行简单的处理，只取最新版本的镜像源信息，并更新所有的task
void DoHelaTaskHandle::CheckTaskListValid() {
  auto it = task_list_.begin();
  HelaTaskPtr& task = *it;

  src_vdisk_id_ = task->src_vdisk_id();
  lc_size_ = task->lc_size();
  pc_size_ = task->pc_size();
  shard_size_ = task->shard_size();
  src_version_ = task->src_version();
  koala_routers_ = task->koala_routers();

  for (it++; it != task_list_.end(); it++) {
    assert((*it)->src_vdisk_id() == src_vdisk_id_);
    assert((*it)->lc_size() == lc_size_);
    assert((*it)->pc_size() == pc_size_);
    assert((*it)->shard_size() == shard_size_);

    if ((*it)->src_version() > src_version_) {
      src_version_ = (*it)->src_version();
      koala_routers_ = (*it)->koala_routers();
    }
  }

  for (auto& it : task_list_) {
    if (it->src_version() != src_version_) {
      it->set_src_version(src_version_);
      it->set_koala_routers(koala_routers_);
    }
  }
}

// 检查cluster version是否变化
void DoHelaTaskHandle::CheckClusterVersion() {
  uint64_t g_cluster_version = g_context->manager_handle()->ClusterVersion();

  for (auto& it : task_list_) {
    if (it->cluster_version() != g_cluster_version) {
      LOG_INFO << "desc: DoHelaTaskHandle task cluster version has changed"
               << " chrono_id(" << it->chrono_id() << ")"
               << " vdisk_id(" << it->vdisk_id() << ")"
               << " src_vdisk_id(" << it->src_vdisk_id() << ")";
      it->set_cluster_version(g_cluster_version);
    }
  }

  cluster_version_ = g_cluster_version;
}

void DoHelaTaskHandle::DumpPcTaskInfo() {
  std::ostringstream oss;
  oss << "\n================Dump Pc Task Info================\n";

  oss << "src_vdisk_id: " << src_vdisk_id_ << "\n";
  oss << "total_task_num: " << pc_task_map_.size() << "\n";
  for (auto& it1 : pc_task_map_) {
    uint32_t pc_no = it1.first;
    HelaPcTaskPtr& pc_task = it1.second;

    oss << "#pc_no(" << pc_no << ")\n";
    oss << "\thas_data:" << pc_task->has_data() << "\n";

    oss << "\ttiny_pc_task_info:\n";
    for (auto& it2 : pc_task->info_list()) {
      oss << "\t\tchrono_id:" << it2->chrono_id() << "\n";
      oss << "\t\tvdisk_id:" << it2->vdisk_id() << "\n";
      oss << "\t\tversion:" << it2->version() << "\n";
      oss << "\t\tvdisk_type:" << it2->vdisk_type() << "\n";
      oss << "\t\tlc_id:" << it2->lc_id() << "\n";
      oss << "\t\tlc_random_id:" << it2->lc_random_id() << "\n";
      oss << "\t\toffline:" << it2->offline() << "\n";
      oss << "\t\tchunk_ip:" << it2->pc_router().copies(0).ip() << "\n";
      oss << "\t\tchunk_port:" << it2->pc_router().copies(0).port() << "\n";
    }

    oss << "\tkoala_route_map:\n";
    for (auto& it2 : pc_task->koala_routers()) {
      const ShardChronoRouter& shard_router = it2.second;
      oss << "\t\tshard_idx:" << shard_router.shard_idx()
          << " ip:" << shard_router.server().ip()
          << " port:" << shard_router.server().port()
          << " has_data:" << shard_router.has_data() << "\n";
    }
  }

  LOG_TRACE << oss.str();
}

// 构建pc_task
// a. pc_no => pc_task
// b. 每个pc_task包含多个tiny_pc_task_info，对应多个目标盘
void DoHelaTaskHandle::ConstructHelaPcTask() {
  for (auto& it1 : task_list_) {
    const HelaTaskPtr& task = it1;
    const PcRouteMap& pc_route_map = it1->pc_routers();

    for (auto& it2 : pc_route_map) {
      uint32_t pc_no = it2.first;
      const PcRouteList& pc_route_list = it2.second;

      for (auto& it3 : pc_route_list) {
        const ucloud::utimemachine::PcRoute& pc_route = it3;
        TinyPcTaskInfoPtr info =
            HelaHelper::ConstructTinyPcTaskInfo(task, pc_route);

        auto it4 = pc_task_map_.find(pc_no);
        if (it4 == pc_task_map_.end()) {
          HelaPcTaskPtr pc_task = HelaHelper::ConstructPcTask(pc_no, task);

          pc_task->info_list().push_back(info);
          pc_task_map_.insert(std::make_pair(pc_no, pc_task));
          continue;
        }

        HelaPcTaskPtr& pc_task = it4->second;
        pc_task->info_list().push_back(info);
      }
    }
  }

  if (g_context->config().batch_hela_taks_schedule_shuffle_on()) {
    LOG_INFO << "hela pc task support shuffle";
    ShuffleHelaPcTask();
  }

  t_total_num_ = pc_task_map_.size();
}

void DoHelaTaskHandle::ShuffleHelaPcTask() {
  // 使用随机函数使分片打散
  // 按照200G/ 4M 最大大约5w个的数量计算还好
  std::vector<HelaPcTaskPtr> pc_task_vec;
  for (const auto& it : pc_task_map_) {
    pc_task_vec.push_back(it.second);
  }

  // auto engine = std::default_random_engine{};
  std::random_device rd;
  std::mt19937 engine(rd());
  std::shuffle(pc_task_vec.begin(), pc_task_vec.end(), engine);

  pc_task_map_.clear();
  for (const auto& task : pc_task_vec) {
    pc_task_map_[task->pc()] = task;
  }

// debug task info
#if 0
  for (const auto& iter : pc_task_map_) {
    LOG_INFO << "hela task pc: " << iter.second->pc();
  }
#endif
}

bool DoHelaTaskHandle::CheckLocalResource(const HelaPcTaskPtr& pc_task) {
  PCResourcePtr pc_resource = resource_manager_->GetResourceNew(
      std::make_pair(src_vdisk_id_, pc_task->pc()));
  if (!pc_resource) {
    return false;
  } else {
    struct ChunkSourceInfo chunk_source;
    HelaHelper::FillChunkSourceInfo(chunk_source, pc_resource);

    pc_task->set_chunk_source_info(chunk_source);
    pc_task->set_src_local_chunk();
    return true;
  }
}

// 查询idun
bool DoHelaTaskHandle::QueryIdunResource(
    const std::vector<HelaPcTaskPtr>& query_task) {
  if (query_task.empty()) {
    LOG_ERROR << "query task empty";
    return false;
  }

  std::pair<std::string, int> result =
      g_context->GetZkRouteCache(HelaConfig::kIdunName);
  std::string ip = result.first;
  uint32_t port = result.second;
  if (port == 0) {
    LOG_ERROR << "can not get idun ip,port";
    return false;
  }

  LOG_DEBUG << "idun ip(" << ip << ")"
            << " idun port(" << port << ")";

  uevent::ConnectionUeventPtr conn =
      g_context->manager_handle()->GetOutConnection(ip, port);
  if (!conn) {
    LOG_ERROR << "can not get idun connection"
              << " ip(" << ip << ")"
              << " port(" << port << ")";
    return false;
  }

  ucloud::UMessage msg;
  uint32_t objid = uevent::MessageUtil::ObjId();
  uint32_t flowno = uevent::MessageUtil::Flowno();
  NewMessage_v2(&msg, flowno, base::MyUuid::NewUuid(),
                ucloud::utimemachine::IDUN_QUERY_RESOURCE_REQUEST, 0, false,
                objid, 0, "IdunQueryResource", NULL, NULL);
  ucloud::utimemachine::IdunQueryResourceRequest* req =
      msg.mutable_body()->MutableExtension(
          ucloud::utimemachine::idun_query_resource_request);

  req->set_src_vdisk_id(src_vdisk_id_);
  req->set_src_version(src_version_);
  req->set_src_vdisk_type(ucloud::utimemachine::PB_VDISK_IMAGE);
  req->set_vdisk_type(query_task[0]->vdisk_type());
  for (auto& pc_task : query_task) {
    req->add_pc_nos(pc_task->pc());
    pc_task->set_status_query();
  }

  LOG_DEBUG << msg.DebugString();
  uevent::MessageUtil::SendPbRequest(
      conn, msg,
      std::bind(&DoHelaTaskHandle::EntryQueryIdunResource, shared_from_this(),
                std::placeholders::_1, query_task),
      std::bind(&DoHelaTaskHandle::QueryIdunTimeout, shared_from_this(),
                query_task),
      10);

  return true;
}

void DebugIdunPcRoute(const std::string& extern_id,
                      const ucloud::utimemachine::IdunPcRoute& pc_route) {
  LOG_DEBUG << "DebugIdunPcRoute: "
            << " pc_no:" << pc_route.pc_no() << " pg_id: " << pc_route.pg_id()
            << " chunk_id: " << pc_route.chunk_id()
            << " chunk_ip: " << pc_route.chunk_ip()
            << " chunk_port: " << pc_route.chunk_port();
}

void DoHelaTaskHandle::EntryQueryIdunResource(
    ucloud::UMessage* um, const std::vector<HelaPcTaskPtr>& query_task) {
  LOG_DEBUG << um->DebugString();
  const ucloud::utimemachine::IdunQueryResourceResponse& res =
      um->body().GetExtension(
          ucloud::utimemachine::idun_query_resource_response);

  if (res.rc().retcode() != ucloud::utimemachine::VDISK_RET_SUCCESS) {
    LOG_WARN << "desc: query idun resource resp: " << res.rc().error_message();

    for (auto& pc_task : query_task) {
      LOG_INFO << "desc: hit koala resource"
               << " pc_no(" << pc_task->pc() << ")"
               << " src_vdisk_id(" << pc_task->src_vdisk_id() << ")";
      DispatchHelaPcTask(pc_task);
    }
    return;
  }

  std::map<uint32_t, RoutePair> idun_route;
  for (int i = 0; i < res.lc_infos_size(); i++) {
    const ucloud::utimemachine::IdunLCInfo& lc_info = res.lc_infos(i);
    for (int j = 0; j < lc_info.pc_routes_size(); j++) {
      const ucloud::utimemachine::IdunPcRoute& pc_route = lc_info.pc_routes(j);
      // DebugIdunPcRoute(lc_info.extern_id(), pc_route);
      idun_route.insert(
          std::make_pair(pc_route.pc_no(), std::make_pair(lc_info, pc_route)));
    }
  }

  for (auto& pc_task : query_task) {
    if (idun_route.empty()) {
      LOG_INFO << "desc: hit koala resource"
               << " pc_no(" << pc_task->pc() << ")"
               << " src_vdisk_id(" << pc_task->src_vdisk_id() << ")";
      DispatchHelaPcTask(pc_task);
      continue;
    }

    auto it = idun_route.find(pc_task->pc());
    if (it == idun_route.end()) {
      LOG_INFO << "desc: hit koala resource"
               << " pc_no(" << pc_task->pc() << ")"
               << " src_vdisk_id(" << pc_task->src_vdisk_id() << ")";
      DispatchHelaPcTask(pc_task);
      continue;
    }

    const ucloud::utimemachine::IdunLCInfo& lc_info = it->second.first;
    const ucloud::utimemachine::IdunPcRoute& pc_route = it->second.second;
    struct ChunkSourceInfo chunk_source;
    HelaHelper::FillChunkSourceInfo(chunk_source, pc_route, lc_info,
                                    cluster_version_);

    pc_task->set_chunk_source_info(chunk_source);
    pc_task->set_src_remote_chunk();

    LOG_INFO << "desc: hit remote chunk resource"
             << " pc_no(" << pc_task->pc() << ")"
             << " chunk_id(" << chunk_source.chunk_id_ << ")"
             << " chunk_ip(" << chunk_source.chunk_ip_ << ")"
             << " chunk_port(" << chunk_source.chunk_port_ << ")"
             << " src_vdisk_id(" << pc_task->src_vdisk_id() << ")";

    // 对于读remote/local chunk源，要保持对其的引用计数，直到完成后再释放
    IncResourceRef(pc_task);

    DispatchHelaPcTask(pc_task);

    idun_route.erase(it);
  }
}

int DoHelaTaskHandle::ExecutingPcTaskNum() {
  int executing_pc_task_num = 0;

  for (auto& it : pc_task_map_) {
    const HelaPcTaskPtr& pc_task = it.second;

    if (pc_task->status_running()) executing_pc_task_num++;

    if (pc_task->status_query()) executing_pc_task_num++;
  }

  return executing_pc_task_num;
}

int DoHelaTaskHandle::ReadyPcTaskNum() {
  int ready_pc_task_num = 0;

  for (auto& it : pc_task_map_) {
    const HelaPcTaskPtr& pc_task = it.second;

    if (pc_task->status_ready()) ready_pc_task_num++;
  }

  return ready_pc_task_num;
}

void DoHelaTaskHandle::ScheduleHelaTaskRoutine() {
  // 调度任务
  ScheduleHelaPcTask();

  // update idun源信息
  UpdateIdunResource();
}

void DoHelaTaskHandle::ScheduleHelaPcTask() {
  if (t_completed_ || timeout_) {
    LOG_DEBUG << "desc: hela task has been completed or timeout";
    return;
  }

  // 解决大盘镜像克隆, 动态增加PC Task调度:
  // 单个任务中并发执行pc job比较少, pc job并发度应该根据盘大小调节
  // 假如线上客户大盘(300G)76800个pc, 并发pc是设置死的480的话, 比例低到0.6%.
  // 总的PC个数: pc_num = lc_size_ / 4M
  // 并发PC比例: max_concurence_proportion = max_concurence_num_ / pc_num
  // 如果是经典的20G和40G镜像, 那么使用以前默认的10%左右的并发
  // 如果超过这个盘大小, 那么根据固定更小的5%比例来增加task
  if (g_context->config().enable_dynamic_concurence_pc_num()) {
    uint64_t lc_size_bytes =
        static_cast<uint64_t>(lc_size_) * 1024 * 1024 * 1024;
    if (UNLIKELY(lc_size_ > kKoalaImageSize40G)) {
      max_concurence_num_ = (lc_size_bytes / pc_size_) / 20;
    } else {
      max_concurence_num_ = (lc_size_bytes / pc_size_) / 10;
    }
    LOG_INFO << "clone lc_size: " << lc_size_ << " pc_size: " << pc_size_
             << " dynamic_concurence_pc_num: " << max_concurence_num_;
    if (max_concurence_num_ < g_context->config().max_concurence_pc_num()) {
      max_concurence_num_ = g_context->config().max_concurence_pc_num();
    }
    LOG_INFO << "clone lc_size: " << lc_size_ << " pc_size: " << pc_size_
             << " dynamic_concurence_pc_num: " << max_concurence_num_;
  } else {
    LOG_INFO << "clone lc_size: " << lc_size_ << " pc_size: " << pc_size_
             << " fixed max_concurence_num: " << max_concurence_num_;
  }

  int max_schedule_num = max_concurence_num_ - ExecutingPcTaskNum();
  int ready_num = ReadyPcTaskNum();
  int can_schedule_num =
      max_schedule_num > ready_num ? ready_num : max_schedule_num;
  int has_schedule_num = 0;

  // LOG_DEBUG << "desc: Debug ScheduleHelaPcTask"
  //          << " max_schedule_num(" << max_schedule_num << ")"
  //          << " ready_num(" << ready_num << ")"
  //          << " can_schedule_num(" << can_schedule_num << ")";
  std::vector<HelaPcTaskPtr> schedule_task;
  while (has_schedule_num < can_schedule_num) {
    const HelaPcTaskPtr& pc_task = schedule_iter_->second;

    if (pc_task->status_ready()) {
      if (pc_task->has_data()) {
        schedule_task.push_back(pc_task);
      } else {
        pc_task->set_status_done();
        t_finish_num_++;
      }

      has_schedule_num++;
    }

    schedule_iter_++;
    if (schedule_iter_ == pc_task_map_.end()) {
      schedule_iter_ = pc_task_map_.begin();
    }
  }

  HandleHelaPcTask(schedule_task);

  CheckHelaTaskFinish();
}

void DoHelaTaskHandle::HandleHelaPcTask(
    const std::vector<HelaPcTaskPtr>& schedule_task) {
  if (schedule_task.empty()) return;

  std::vector<HelaPcTaskPtr> query_idun_task;
  for (auto& pc_task : schedule_task) {
    // 本地有源直接调度
    if (CheckLocalResource(pc_task)) {
      LOG_INFO << "desc: hit local chunk resource"
               << " pc_no(" << pc_task->pc() << ")"
               << " chunk_id(" << pc_task->chunk_source_info().chunk_id_ << ")"
               << " chunk_ip(" << pc_task->chunk_source_info().chunk_ip_ << ")"
               << " chunk_port(" << pc_task->chunk_source_info().chunk_port_
               << ")"
               << " src_vdisk_id(" << pc_task->src_vdisk_id() << ")";

      // 对于读remote/local chunk源，要保持对其的引用计数，直到完成后再释放
      IncResourceRef(pc_task);

      DispatchHelaPcTask(pc_task);
    } else {
      // 本地没有源进行批量query
      query_idun_task.push_back(pc_task);
    }
  }

  if (query_idun_task.empty()) return;

  if (!QueryIdunResource(query_idun_task)) {
    LOG_ERROR << "desc: query idun resource error";
    for (auto& pc_task : query_idun_task) {
      LOG_INFO << "desc: hit koala resource"
               << " pc_no(" << pc_task->pc() << ")";
      DispatchHelaPcTask(pc_task);
    }
  }
}

// 选取一个loop handle执行hela pc task
void DoHelaTaskHandle::DispatchHelaPcTask(const HelaPcTaskPtr& pc_task) {
  uevent::ListenerUevent* transmission_listener =
      g_context->transmission_listener();
  assert(transmission_listener != nullptr);

  std::vector<uevent::UeventLoop*> loops =
      transmission_listener->GetPrimayLoops();

  uevent::UeventLoop* transmission_loop = loops[dispatch_loop_idx_];
  assert(transmission_loop != nullptr);
  TransmissionHandle* loop_handle =
      (TransmissionHandle*)transmission_loop->GetLoopHandle();

  if (++dispatch_loop_idx_ >= loops.size()) dispatch_loop_idx_ = 0;

  LOG_DEBUG << "desc: dispatch hela pc task"
            << " loop_id(" << dispatch_loop_idx_ << ")";

  std::shared_ptr<DoHelaPcTaskHandle> pc_task_handle =
      std::make_shared<DoHelaPcTaskHandle>(
          std::bind(&DoHelaTaskHandle::HandlePcTaskResult, shared_from_this(),
                    std::placeholders::_1, std::placeholders::_2),
          pc_task, loop_handle);

  transmission_loop->RunInLoop(std::bind(&TransmissionHandle::StartHelaPcTask,
                                         loop_handle, pc_task_handle));

  pc_task->set_status_running();
  t_running_num_++;
}

std::string DoHelaTaskHandle::DumpHandleInfo() {
  std::ostringstream oss;

  oss << " src_vdisk_id(" << src_vdisk_id_ << ")"
      << " t_fail_num(" << t_fail_num_ << ")"
      << " t_succ_num(" << t_succ_num_ << ")"
      << " t_running_num(" << t_running_num_ << ")"
      << " t_finish_num(" << t_finish_num_ << ")"
      << " t_total_num(" << t_total_num_ << ")";

  return oss.str();
}

void DoHelaTaskHandle::HandlePcTaskResult(int retcode, uint32_t pc_no) {
  if (timeout()) {
    LOG_ERROR << "desc: hela task has been timeout"
              << " pc_no(" << pc_no << ")";
    return;
  }

  auto it = pc_task_map_.find(pc_no);
  if (it == pc_task_map_.end()) {
    LOG_ERROR << "desc: can not find pc task"
              << " pc_no(" << pc_no << ")";
    return;
  }

  const HelaPcTaskPtr& pc_task = it->second;
  if (!pc_task->status_running()) {
    LOG_ERROR << "desc: pc task is not running"
              << " pc_no(" << pc_no << ")";
    return;
  }

  DecResourceRef(pc_task);

  if (retcode == VDISK_RET_SUCCESS) {
    t_succ_num_++;
    t_running_num_--;
    t_finish_num_++;

    pc_task->set_status_done();

    LOG_DEBUG << "desc: pc task succ"
              << " retcode(" << retcode << ")"
              << " pc_no(" << pc_no << ")" << DumpHandleInfo();
    // 从src读失败，则强制其从镜像源读
  } else if (retcode == VDISK_RET_SOURCE_UNAVAILABLE) {
    pc_task->set_src_koala();
    pc_task->set_status_running();

    LOG_ERROR << "desc: source unavaliable"
              << " retcode(" << retcode << ")"
              << " pc_no(" << pc_no << ")" << DumpHandleInfo();

    return DispatchHelaPcTask(pc_task);
  } else {
    t_fail_num_++;
    t_running_num_--;
    t_finish_num_++;

    pc_task->set_status_fail();

    LOG_ERROR << "desc: pc task fail"
              << " retcode(" << retcode << ")"
              << " pc_no(" << pc_no << ")" << DumpHandleInfo();
  }

  if (pc_task->status_done()) {
    // 更新本地源信息
    UpdateLocalResource(pc_task);

    // 远程源信息暂缓更新
    update_idun_task_.push_back(pc_task);
  }
}

void DoHelaTaskHandle::CheckHelaTaskFinish() {
  if (t_finish_num_ == t_total_num_) {
    if (t_fail_num_) {
      LOG_ERROR << "desc: hela task fail" << DumpHandleInfo();
      SetAllTaskFail();
    } else {
      LOG_INFO << "desc: hela task succ" << DumpHandleInfo();
      SetAllTaskDone();
    }

    t_completed_ = true;

    CancalTaskTimer();
    CancalScheduleTimer();
  }
}

bool DoHelaTaskHandle::UpdateIdunResource() {
  if (update_idun_task_.empty()) return true;

  std::map<std::string, std::vector<TinyPcTaskInfoPtr>> resource_info;
  for (auto& pc_task : update_idun_task_) {
    for (auto& it : pc_task->info_list()) {
      if (it->offline()) continue;

      std::string vdisk_id = it->vdisk_id();
      auto iter = resource_info.find(vdisk_id);
      if (iter == resource_info.end()) {
        std::vector<TinyPcTaskInfoPtr> tiny_infos;
        tiny_infos.push_back(it);
        resource_info.insert(std::make_pair(vdisk_id, tiny_infos));
      } else {
        iter->second.push_back(it);
      }
    }
  }
  // 清除待更新的resource任务
  update_idun_task_.clear();

  std::pair<std::string, int> result =
      g_context->GetZkRouteCache(HelaConfig::kIdunName);
  std::string ip = result.first;
  uint32_t port = result.second;
  if (port == 0) {
    LOG_ERROR << "can not get idun ip,port";
    return false;
  }

  uevent::ConnectionUeventPtr conn =
      g_context->manager_handle()->GetOutConnection(ip, port);
  if (!conn) {
    LOG_ERROR << "can not get idun connection"
              << " ip(" << ip << ")"
              << " port(" << port << ")";
    return false;
  }

  for (auto iter = resource_info.begin(); iter != resource_info.end(); iter++) {
    ucloud::UMessage msg;
    uint32_t objid = uevent::MessageUtil::ObjId();
    uint32_t flowno = uevent::MessageUtil::Flowno();
    NewMessage_v2(&msg, flowno, base::MyUuid::NewUuid(),
                  ucloud::utimemachine::IDUN_UPDATE_RESOURCE_REQUEST, 0, false,
                  objid, 0, "IdunUpdateResource", NULL, NULL);
    ucloud::utimemachine::IdunUpdateResourceRequest* req =
        msg.mutable_body()->MutableExtension(
            ucloud::utimemachine::idun_update_resource_request);

    std::vector<TinyPcTaskInfoPtr>& tiny_infos = iter->second;
    for (auto& it : tiny_infos) {
      HelaHelper::AssembleUpdateIdunResourceReq(req, it);
    }

    LOG_DEBUG << msg.DebugString();
    uevent::MessageUtil::SendPbRequest(
        conn, msg, std::bind(&DoHelaTaskHandle::EntryUpdateIdunResource,
                             shared_from_this(), std::placeholders::_1),
        std::bind(&DoHelaTaskHandle::UpdateIdunTimeout, shared_from_this()), 5);
  }

  return true;
}

void DoHelaTaskHandle::EntryUpdateIdunResource(ucloud::UMessage* um) {
  LOG_DEBUG << um->DebugString();
  const ucloud::utimemachine::IdunUpdateResourceResponse& res =
      um->body().GetExtension(
          ucloud::utimemachine::idun_update_resource_response);

  // 不管update idun成功与否
  if (res.rc().retcode() != 0) {
    LOG_ERROR << "desc: update idun resource error "
              << res.rc().error_message();
  }
}

}  // namespace hela
}  // namespace udisk
