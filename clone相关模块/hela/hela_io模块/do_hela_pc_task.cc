// Copyright (c) 2020 UCloud All rights reserved.
#include "do_hela_pc_task.h"

#include "hela_context.h"
#include "hela_helper.h"
#include "manager_handle.h"
#include "umessage.h"
#include "message_util.h"
#include "logging.h"
#include "constanst.h"

namespace udisk {
namespace hela {

using namespace uevent;
using namespace udisk::common;

DoHelaPcTaskHandle::DoHelaPcTaskHandle(ResponseHook hook, HelaPcTaskPtr pc_task,
                                       TransmissionHandle *loop_handle)
    : rsp_hook_(hook),
      pc_task_(pc_task),
      loop_handle_(loop_handle),
      retry_read_(false),
      retry_write_(false),
      pending_read_(false),
      pending_write_(false),
      pc_(0),
      src_port_(0),
      src_version_(0),
      shard_size_(0),
      cluster_version_(0),
      lc_size_(0),
      pc_size_(0),
      src_lc_id_(0),
      src_lc_random_id_(0),
      src_pg_id_(0),
      start_sector_(0),
      sector_num_(0),
      next_read_sector_(0),
      end_sector_(0),
      buffer_(nullptr),
      back_off_(std::chrono::milliseconds(
                    g_context->config().single_pc_retry_backoff_initial_ms()),
                std::chrono::milliseconds(
                    g_context->config().single_pc_retry_backoff_maximum_ms()),
                std::chrono::milliseconds(
                    g_context->config().single_pc_retry_backoff_step_ms())) {}

DoHelaPcTaskHandle::~DoHelaPcTaskHandle() {
  LOG_TRACE << "DoHelaPcTaskHandle::~DoHelaPcTaskHandle";

  if (buffer_ != nullptr) {
    SafePutPureStrBuffer(buffer_);
  }
}

void DoHelaPcTaskHandle::set_retry_read(bool retry_read) {
  retry_read_ = retry_read;  
  if (retry_read) {      //判断是否重试
    if (back_off_.CanDiscard()) {  //如果达到最大的退避时间那么认为这个任务失败
      LOG_ERROR << "hela pc task definitely timeout"
                << " pc_no(" << pc_task_->pc() << ")"
                << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
      // 读镜像达到次数认为任务失败,读Chunk认为源不可用
      if (pc_task_->src_koala()) {   //如果是读 koala 那么就把这个失败标记为  读镜像达到次数认为任务失败
        return FinishHelaPcTask(VDISK_RET_FAIL);
      } else {   //表示读chunk  读Chunk认为源不可用
        return FinishHelaPcTask(VDISK_RET_SOURCE_UNAVAILABLE);
      }
    }
    back_off_.ReportStatus(false);   //还没有达到最大的退避时间那么这个任务继续执行
    int timeout_sec = back_off_.GetTimeRemainingUntilRetrySecond();   //设置新的退避时间
    LOG_WARN << "hela pc task will retry read after " << timeout_sec
             << " seconds"
             << " pc_no(" << pc_task_->pc() << ")"
             << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
    timer_id_ = loop_handle_->GetLoop()->RunAfter(                              //  timeout_sec 经过timeout_sec 后重新执行任务
        timeout_sec,
        std::bind(&DoHelaPcTaskHandle::HandleRetryCb, shared_from_this()));   // HandleRetryCb ？？？？？？？？？
  }
}
void DoHelaPcTaskHandle::set_retry_write(bool retry_write) {
  retry_write_ = retry_write;
  if (retry_write) {
    if (back_off_.CanDiscard()) {
      LOG_ERROR << "hela pc task definitely timeout"
                << " pc_no(" << pc_task_->pc() << ")"
                << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
      return FinishHelaPcTask(VDISK_RET_FAIL);
    }
    back_off_.ReportStatus(false);
    int timeout_sec = back_off_.GetTimeRemainingUntilRetrySecond();
    LOG_WARN << "hela pc task will retry write after " << timeout_sec
             << " seconds"
             << " pc_no(" << pc_task_->pc() << ")"
             << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
    timer_id_ = loop_handle_->GetLoop()->RunAfter(
        timeout_sec,
        std::bind(&DoHelaPcTaskHandle::HandleRetryCb, shared_from_this()));
  }
}
void DoHelaPcTaskHandle::set_pending_write(bool pending_write) {
  pending_write_ = pending_write;
  if (pending_write) {  //判断是否需要  pending_write
    LOG_DEBUG << "hela pc task will pending write after "
              << g_context->config().single_pc_pending_write_chunk_second()
              << " seconds"
              << " pc_no(" << pc_task_->pc() << ")"
              << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
    timer_id_ = loop_handle_->GetLoop()->RunAfter(                      //  经过 single_pc_pending_write_chunk_second 后重新执行任务 
        g_context->config().single_pc_pending_write_chunk_second(),
        std::bind(&DoHelaPcTaskHandle::HandlePendingCb, shared_from_this()));
  }
}
void DoHelaPcTaskHandle::set_pending_read(bool pending_read) {  // 设置是否需要 set_pending_read
  pending_read_ = pending_read;
  if (pending_read) {  // 判断是否需要 pending_read
    double timeout_second;
    if (pc_task_->src_koala()) {  //判断本pc是否从 src_koala读取
      timeout_second =
          g_context->config().single_pc_pending_read_koala_second();
    } else {
      timeout_second =
          g_context->config().single_pc_pending_read_chunk_second();
    }
    LOG_DEBUG << "hela pc task will pending read after " << timeout_second
              << " seconds"
              << " pc_no(" << pc_task_->pc() << ")"
              << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
    timer_id_ = loop_handle_->GetLoop()->RunAfter(       // 经过 timeout_second 后重新执行任务 
        timeout_second,
        std::bind(&DoHelaPcTaskHandle::HandlePendingCb, shared_from_this()));
  }
}

void DoHelaPcTaskHandle::HandleRetryCb() {  //重试
  LOG_DEBUG << "DoHelaPcTaskHandle::HandleTimerCb"
            << " status(" << pc_task_->status() << ")"
            << " pc_no(" << pc_task_->pc() << ")"
            << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
  if (pc_task_->status_fail()) {  //如果本pc任务失败 直接返回
    return;
  }
// 理论上重试超时函数是定时器设置的,肯定是可以执行的
#if 0    //代码注释
  if (!back_off_.CanTryNow()) {
    LOG_ERROR << "cannot retry pc task now"
              << " pc_no(" << pc_task_->pc() << ")"
              << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
    timer_id_ = loop_handle_->GetLoop()->RunAfter(
        back_off_.GetTimeRemainingUntilRetrySecond(),
        std::bind(&DoHelaPcTaskHandle::HandleRetryCb, shared_from_this()));
    return;
  }
#endif

  if (retry_write()) {  //判断是否需要  retry_write 如果需要就进入该逻辑
    set_retry_write(false);  //设置 set_retry_write 为 false
    return HandleWriteChunk();   //######
  }

  if (retry_read()) {  //判断是否需要  retry_read 如果需要就进入该逻辑
    set_retry_read(false);  //设置 set_retry_read 为 false
    if (pc_task_->src_koala()) {  // 判断是否是从 src_koala 读数据
      return DecideKoalaSource(false, true);
    } else {
      return ReadDataFromChunk();  //重chunk读数据   ######
    }
  }
}

void DoHelaPcTaskHandle::HandlePendingCb() {
  LOG_DEBUG << "DoHelaPcTaskHandle::HandlePendingCb"
            << " status(" << pc_task_->status() << ")"
            << " pc_no(" << pc_task_->pc() << ")"
            << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
  if (pc_task_->status_fail()) { //如果本pc任务失败 直接返回
    return;
  }
  if (pending_write()) { //判断是否需要  pending_write 如果需要就进入该逻辑
    set_pending_write(false); //设置 set_pending_write 为 false
    return HandleWriteChunk(); // ######
  }

  if (pending_read()) {  //判断是否需要  pending_read 如果需要就进入该逻辑
    set_pending_read(false);  //设置 set_pending_read 为 false
    if (pc_task_->src_koala()) { // 判断是否是从 src_koala 读数据
      if (UNLIKELY(change_ark_immediately_)) {  //将change_ark_immediately_ 与 0 比较判断是否为ture， change_ark_immediately_  不为0 才进入该逻辑
        change_ark_immediately_ = false;  //设置为 0 就是false
        return DecideKoalaSource(false, true);  //######
      }
      int64_t access_num = loop_handle_->GetAccessNumber(src_ip_, src_port_);  // 获取ark机器上面 某个服务(ip port 确定一个服务) 被读取的次数
      if (g_context->config().batch_hela_task_statistics_on()) {  //是否需要统计计数
        LOG_INFO << " koala_ip(" << src_ip_ << ")"
                 << " koala_port(" << src_port_ << ")"
                 << " access_nums: " << access_num << " max access limit: "
                 << g_context->config().single_pc_pending_change_koala_times();
      }
      if (UNLIKELY(
              access_num >
              g_context->config().single_pc_pending_change_koala_times())) {  //与 0 比较判断是否为ture， single_pc_pending_change_koala_times  不为0 才进入该逻辑
        LOG_WARN << " pc_no(" << pc_task_->pc() << ")"
                 << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")"
                 << " koala_ip(" << src_ip_ << ")"
                 << " koala_port(" << src_port_ << ")"
                 << " access_nums: " << access_num << " max access limit: "
                 << g_context->config().single_pc_pending_change_koala_times();
        return DecideKoalaSource(false, true);   // 读取该机器上面的并发度过高，需要切换读取源
      } else {
        return DecideKoalaSource(false, false);  //不需要切换
      }
    } else {
      return ReadDataFromChunk();  //从chunk读
    }
  }
}

void DoHelaPcTaskHandle::ReadKoalaTimeOut() {
  // 可能是自定义镜像, 分片过多允许读取超时
  // 进入pengding而不是进入错误计数导致任务失败
  if ((lc_size_ > kKoalaImageSize40G) &&  //大镜像
      ((huge_image_pending_times_++) <
       g_context->config().single_pc_huge_image_pending_times())) {  //判断pending 次数是否小于配置文件里面的
    LOG_ERROR << "desc: handle read koala timeout"
              << " koala_ip(" << src_ip_ << ")"
              << " koala_port(" << src_port_ << ")"
              << " huge_image_pending_times(" << huge_image_pending_times_
              << ")"
              << " pending pc_no(" << pc_task_->pc() << ")"
              << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
    set_pending_read(true);  //设置set_pending_read为true 
  } else {
    LOG_ERROR << "desc: handle read koala timeout"
              << " koala_ip(" << src_ip_ << ")"
              << " koala_port(" << src_port_ << ")"
              << " retry pc_no(" << pc_task_->pc() << ")"
              << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
    set_retry_read(true); // 设置set_retry_read为true
  }
}

void DoHelaPcTaskHandle::ReadChunkTimeOut() {
  LOG_ERROR << "desc: handle read chunk timeout"
            << " chunk_ip(" << src_ip_ << ")"
            << " chunk_port(" << src_port_ << ")"
            << " pc_no(" << pc_task_->pc() << ")"
            << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";

  set_retry_read(true);   //设置set_retry_read为true
}

void DoHelaPcTaskHandle::WriteChunkTimeOut() {
  LOG_ERROR << "desc: handle write chunk timeout"
            << " pc_no(" << pc_task_->pc() << ")"
            << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";

  //对于write chunk不做最大重试限制，有最大超时保证存在
  set_pending_write(true);  //设置set_pending_write为true
}

void DoHelaPcTaskHandle::Start() {
  LOG_DEBUG << "desc: hela pc task start"
            << " pc_no(" << pc_task_->pc() << ")"
            << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")"
            << " src_type(" << pc_task_->src_type() << ")"
            << " lc_size(" << pc_task_->lc_size() << ")"
            << " pc_size(" << pc_task_->pc_size() << ")"
            << " cluster_version(" << pc_task_->cluster_version() << ")"
            << " src_version(" << pc_task_->src_version() << ")"
            << " shard_size(" << pc_task_->shard_size() << ")";

  pc_ = pc_task_->pc();
  src_vdisk_id_ = pc_task_->src_vdisk_id();
  lc_size_ = pc_task_->lc_size();
  pc_size_ = pc_task_->pc_size();
  cluster_version_ = pc_task_->cluster_version();
  src_version_ = pc_task_->src_version();
  shard_size_ = pc_task_->shard_size();
  info_list_ = pc_task_->info_list();

  chunk_source_info_ = pc_task_->chunk_source_info();
  src_koala_router_ = pc_task_->koala_routers();

  iter_ = info_list_.begin();

  next_read_sector_ = pc_ * pc_size_ / DEFAULT_SECTOR_SIZE;
  end_sector_ = (pc_ + 1) * pc_size_ / DEFAULT_SECTOR_SIZE;

  if (pc_task_->src_koala()) {
    return DecideKoalaSource(true, false);
  } else {
    src_lc_id_ = chunk_source_info_.lc_id_;
    src_lc_random_id_ = chunk_source_info_.lc_random_id_;
    src_pg_id_ = chunk_source_info_.pg_id_;
    src_ip_ = chunk_source_info_.chunk_ip_;
    src_port_ = chunk_source_info_.chunk_port_;

    return ReadDataFromChunk();
  }

  LOG_ERROR << "desc: error hela pc task source type"
            << " pc_no(" << pc_task_->pc() << ")"
            << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
  return FinishHelaPcTask(VDISK_RET_FAIL);
}

// 填充全部的ARK路由信息
// 1. PC任务开始下一个sector填充koala全部的路由信息
// 2. 改变路由一圈后找不到一个可用的路由(全部宕机)
void DoHelaPcTaskHandle::FillKoalaSourceRouter() {
  int shard_index = next_read_sector_ * DEFAULT_SECTOR_SIZE / shard_size_;
  auto it = src_koala_router_.find(shard_index);
  assert(it != src_koala_router_.end());
  ucloud::utimemachine::ShardChronoRouter &shard_chrono_router = it->second;

  // 清理上一个分片的路由
  current_ark_router_.clear();
  current_ark_router_[shard_chrono_router.server().ip()] =
      ARKRouteInfo(shard_chrono_router.server().ip(),
                   shard_chrono_router.server().port() + 3, 0);

  for (int i = 0; i < shard_chrono_router.peer_servers_size(); ++i) {
    current_ark_router_[shard_chrono_router.peer_servers(i).ip()] =
        ARKRouteInfo(shard_chrono_router.peer_servers(i).ip(),
                     shard_chrono_router.peer_servers(i).port() + 3, 0);
  }
}

// 决定koala源,每发生一次读koala源错误，切换到另一个源
// 不是每次都需要去变更PC的读取路由
void DoHelaPcTaskHandle::DecideKoalaSource(bool new_sector,
                                           bool change_koala_source) {
  // 不可能是新的分片, 还要求变路由
  assert(new_sector && change_koala_source);

  // 新的分片开始: 填充每个分片的临时路由信息
  if (new_sector) {
    FillKoalaSourceRouter();
    auto route = loop_handle_->GetIdleARKRoute(current_ark_router_);
    src_ip_ = std::get<0>(route);
    src_port_ = std::get<1>(route);
  }

  // 理论上每个PC只需要获取一个源信息即可, 只有出错了才会切换
  // 如果没有设置koala源信息或者需要改变源信息采取获取路由
  // 当PC Pending的时候不需要切换源信息, Retry重试需要切换
  // TODO: 切换的源不一定时候可靠, 坑爹! 需要动态路由才行(ARKV3不需要)

  // ARK因为宕机路由不能及时感知, 把读取超时的路由从列表中剔除
  // 这样有个前提条件就是活着的ark机器即时不能处理也能够恢复BUSY消息
  // 否则一个网络波动导致某个PC超时就可能导致单个PC无路由可用了
  // 后面ARKV3使用云盘, 只会有一个IP信息, 云盘保证三副本的可用
  // 但是对于镜像静态资源, 均衡到三个副本去读, 性能才会更好, 目前云盘做不到
  if (change_koala_source) {
    // 认为路由有问题, 从当前路由表中摘除
    current_ark_router_.erase(src_ip_);
    if (current_ark_router_.empty()) {
      LOG_ERROR << "pc_no(" << pc_task_->pc() << ")"
                << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")"
                << " no available ark router";
      FillKoalaSourceRouter();
    }
    std::string last_src_ip = src_ip_;
    uint32_t last_src_port = src_port_;

    auto route = loop_handle_->GetIdleARKRoute(current_ark_router_);
    src_ip_ = std::get<0>(route);
    src_port_ = std::get<1>(route);

    LOG_WARN << "pc_no(" << pc_task_->pc() << ")"
             << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")"
             << " change koala from (" << last_src_ip << ":" << last_src_port
             << ") to (" << src_ip_ << ":" << src_port_ << ")";
  }
  return ReadDataFromKoala();
}

//这种处理，要求koala的shard size不能够小于1MB
void DoHelaPcTaskHandle::ReadDataFromKoala() {
  //超过read remote阈值, pending remote read
  if (!g_context->flow_monitor()->GrantReadRemote(
           g_context->config().operate_size())) {
    LOG_TRACE << "desc: trigger pending read koala";
    set_pending_read(true);
    return;
  }

  loop_handle_->ReportAccessInfo(src_ip_, src_port_, true, true);

  // 读Ark还是走V1,等Ark支持V2版本再改
  // 写chunk和读chunk走V2协议版本
  const size_t msg_hdr_len = sizeof(udisk::common::MessageHeaderV1) +
                             sizeof(udisk::common::HelaKoalaRequestHead);
  char msg_hdr[msg_hdr_len];
  ::memset(msg_hdr, 0, msg_hdr_len);
  udisk::common::MessageHeaderV1 *common_hdr =
      (udisk::common::MessageHeaderV1 *)msg_hdr;
  udisk::common::HelaKoalaRequestHead *koala_hdr =
      (udisk::common::
           HelaKoalaRequestHead *)(msg_hdr +
                                   sizeof(udisk::common::MessageHeaderV1));

  start_sector_ = next_read_sector_;
  sector_num_ = g_context->config().operate_size() / DEFAULT_SECTOR_SIZE;

  BuildKoalaReadRequest(common_hdr, koala_hdr);

  int rc = loop_handle_->SendRequest(
      src_ip_, src_port_, koala_hdr->handle_id, msg_hdr, msg_hdr_len,
      std::bind(&DoHelaPcTaskHandle::HandleReadKoalaResponse,
                shared_from_this(), std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3),
      std::bind(&DoHelaPcTaskHandle::ReadKoalaTimeOut, shared_from_this()),
      (double)g_context->config().single_pc_read_koala_timeout_second());

  if (rc) {
    LOG_ERROR << "desc: send koala read req failed"
              << " pc_no(" << koala_hdr->pc_no << ")"
              << " image_id(" << koala_hdr->image_id << ")"
              << " version(" << koala_hdr->version << ")"
              << " sector(" << koala_hdr->sector << ")"
              << " sector_num(" << koala_hdr->sector_num << ")"
              << " sector_size(" << koala_hdr->sector_size << ")"
              << " shard_idx(" << koala_hdr->shard_idx << ")"
              << " handle_id(" << koala_hdr->handle_id << ")"
              << " rc(" << rc << ")";

    set_retry_read(true);
  } else {
    LOG_DEBUG << "desc: send koala read req succ"
              << " pc_no(" << koala_hdr->pc_no << ")"
              << " image_id(" << koala_hdr->image_id << ")"
              << " version(" << koala_hdr->version << ")"
              << " sector(" << koala_hdr->sector << ")"
              << " sector_num(" << koala_hdr->sector_num << ")"
              << " sector_size(" << koala_hdr->sector_size << ")"
              << " shard_idx(" << koala_hdr->shard_idx << ")"
              << " handle_id(" << koala_hdr->handle_id << ")"
              << " rc(" << rc << ")";
  }
}

void DoHelaPcTaskHandle::BuildKoalaReadRequest(
    udisk::common::MessageHeaderV1 *common_hdr,
    udisk::common::HelaKoalaRequestHead *koala_hdr) {
  common_hdr->msg_type = udisk::common::MSG_HELA_KOALA_REQ;
  common_hdr->version = udisk::common::MSG_V1;
  common_hdr->data_len = sizeof(udisk::common::HelaKoalaRequestHead);

  koala_hdr->pc_no = pc_;
  koala_hdr->cmd = udisk::common::HELA_CMD_READ;
  memcpy(koala_hdr->image_id, src_vdisk_id_.c_str(), src_vdisk_id_.size());
  koala_hdr->version = src_version_;
  koala_hdr->sector = start_sector_;
  koala_hdr->sector_num = sector_num_;
  koala_hdr->sector_size = DEFAULT_SECTOR_SIZE;
  koala_hdr->shard_idx = start_sector_ * DEFAULT_SECTOR_SIZE / shard_size_;
  koala_hdr->handle_id = uevent::MessageUtil::ObjId();
}

void DoHelaPcTaskHandle::HandleReadKoalaResponse(
    void *msg_hdr, void *msg_data, const ConnectionUeventPtr &conn) {
  udisk::common::HelaKoalaResponseHead *koala_hdr =
      (udisk::common::
           HelaKoalaResponseHead *)((char *)msg_hdr +
                                    sizeof(udisk::common::MessageHeaderV1));

  if (koala_hdr->retcode == VDISK_RET_SUCCESS) {
    LOG_DEBUG << "desc: read koala succ"
              << " pc_no(" << koala_hdr->pc_no << ")"
              << " sector(" << koala_hdr->sector << ")"
              << " sector_num(" << koala_hdr->sector_num << ")";

    loop_handle_->IncReadRemoteBytes(g_context->config().operate_size());

    buffer_ = (StrBuffer *)msg_data;
    assert(buffer_->Size() == g_context->config().operate_size());

    back_off_.ReportStatus(true);
    return HandleWriteChunk();
    // 如果为空数据, 则不需要写chunk
  } else if (koala_hdr->retcode == (uint32_t)VDISK_RET_EMPTY) {
    LOG_WARN << "desc: read koala data empty"
             << " src_vdisk_idr(" << src_vdisk_id_ << ")"
             << " pc_no(" << koala_hdr->pc_no << ")"
             << " sector(" << koala_hdr->sector << ")"
             << " sector_num(" << koala_hdr->sector_num << ")";
    back_off_.ReportStatus(true);
    return ReadNextPart();
  } else if (koala_hdr->retcode == (uint32_t)VDISK_RET_EIO_ERROR) {
    LOG_WARN << "desc: read koala data eio"
             << " src_vdisk_id(" << src_vdisk_id_ << ")"
             << " pc_no(" << koala_hdr->pc_no << ")"
             << " sector(" << koala_hdr->sector << ")"
             << " sector_num(" << koala_hdr->sector_num << ")";
    return set_retry_read(true);
  } else if (koala_hdr->retcode == (uint32_t)VDISK_RET_BUSY) {
    // 目前返回BUSY码表示镜像处理不过来,比如网卡带宽受限,读取磁盘超时
    LOG_DEBUG << "desc: read koala data busy"
              << " src_vdisk_id(" << src_vdisk_id_ << ")"
              << " pc_no(" << koala_hdr->pc_no << ")"
              << " sector(" << koala_hdr->sector << ")"
              << " sector_num(" << koala_hdr->sector_num << ")";
    return set_pending_read(true);
  } else if (koala_hdr->retcode == (uint32_t)VDISK_RET_SOURCE_UNAVAILABLE) {
    // 主要是ARKER修复的场景，ARKER源可能不可用, 直接切换源
    LOG_DEBUG << "desc: read koala data unavaliable"
              << " src_vdisk_id(" << src_vdisk_id_ << ")"
              << " pc_no(" << koala_hdr->pc_no << ")"
              << " sector(" << koala_hdr->sector << ")"
              << " sector_num(" << koala_hdr->sector_num << ")";
    change_ark_immediately_ = true;
    return set_pending_read(true);
  } else {
    LOG_ERROR << "desc: read koala error"
              << " src_vdisk_id(" << src_vdisk_id_ << ")"
              << " pc_no(" << koala_hdr->pc_no << ")"
              << " sector(" << koala_hdr->sector << ")"
              << " sector_num(" << koala_hdr->sector_num << ")"
              << " retcode(" << (int)koala_hdr->retcode << ")";
    return set_retry_read(true);
  }
}

void DoHelaPcTaskHandle::ReadDataFromChunk() {
  //每次读chunk源之前check一次cluster
  if (CheckCluster()) {
    LOG_ERROR << "desc: cluster change chunk source unavaliable"
              << " pc_no(" << pc_task_->pc() << ")"
              << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
    return FinishHelaPcTask(VDISK_RET_SOURCE_UNAVAILABLE);
  }

  // 限制读远端chunk的带宽
  if (pc_task_->src_remote_chunk()) {
    if (!g_context->flow_monitor()->GrantReadRemote(
             g_context->config().operate_size())) {
      LOG_DEBUG << "desc: trigger pending read remote chunk";
      set_pending_read(true);
      return;
    }
  } else {
    // 限制读本地chunk的带宽
    if (!g_context->flow_monitor()->GrantReadLocal(
             pc_task_->chunk_source_info().chunk_id_,
             g_context->config().operate_size())) {
      LOG_DEBUG << "desc: trigger pending read local chunk";
      set_pending_read(true);
      return;
    }
  }

  // loop_handle_->ReportAccessInfo(src_ip_, src_port_, true, false);

  const size_t msg_hdr_len = sizeof(udisk::common::MessageHeaderV2) +
                             sizeof(udisk::common::HelaChunkRequestHeadV2);

  char msg_hdr[msg_hdr_len];
  ::memset(msg_hdr, 0, msg_hdr_len);
  udisk::common::MessageHeaderV2 *common_hdr =
      (udisk::common::MessageHeaderV2 *)msg_hdr;
  udisk::common::HelaChunkRequestHeadV2 *chunk_hdr =
      (udisk::common::
           HelaChunkRequestHeadV2 *)(msg_hdr +
                                     sizeof(udisk::common::MessageHeaderV2));

  start_sector_ = next_read_sector_;
  sector_num_ = g_context->config().operate_size() / DEFAULT_SECTOR_SIZE;

  BuildChunkReadRequest(common_hdr, chunk_hdr);

  int rc = loop_handle_->SendRequest(
      src_ip_, src_port_, chunk_hdr->handle_id, msg_hdr, msg_hdr_len,
      std::bind(&DoHelaPcTaskHandle::HandleReadChunkResponse,
                shared_from_this(), std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3),
      std::bind(&DoHelaPcTaskHandle::ReadChunkTimeOut, shared_from_this()),
      (double)g_context->config().single_pc_read_chunk_timeout_second());

  if (rc) {
    LOG_ERROR << "desc: send chunk read req failed"
              << " src_ip(" << src_ip_ << ")"
              << " src_port(" << src_port_ << ")"
              << " offset(" << chunk_hdr->offset << ")"
              << " length(" << chunk_hdr->length << ")"
              << " lc_id(" << chunk_hdr->lc_id << ")"
              << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
              << " lc_size(" << chunk_hdr->lc_size << ")"
              << " pc_no(" << chunk_hdr->pc_no << ")"
              << " cluster_version(" << chunk_hdr->route_version << ")"
              << " handle_id(" << chunk_hdr->handle_id << ")";

    set_retry_read(true);
  } else {
    LOG_DEBUG << "desc: send chunk read req succ"
              << " src_ip(" << src_ip_ << ")"
              << " src_port(" << src_port_ << ")"
              << " offset(" << chunk_hdr->offset << ")"
              << " length(" << chunk_hdr->length << ")"
              << " lc_id(" << chunk_hdr->lc_id << ")"
              << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
              << " lc_size(" << chunk_hdr->lc_size << ")"
              << " pc_no(" << chunk_hdr->pc_no << ")"
              << " cluster_version(" << chunk_hdr->route_version << ")"
              << " handle_id(" << chunk_hdr->handle_id << ")";
  }
}

void DoHelaPcTaskHandle::BuildChunkReadRequest(
    udisk::common::MessageHeaderV2 *common_hdr,
    udisk::common::HelaChunkRequestHeadV2 *chunk_hdr) {
  common_hdr->msg_type = udisk::common::MSG_HELA_CHUNK_REQ;
  common_hdr->version = udisk::common::MSG_V2;
  common_hdr->data_len = sizeof(udisk::common::HelaChunkRequestHeadV2);

  chunk_hdr->size = 0;
  chunk_hdr->cmd = udisk::common::HELA_CMD_READ;
  chunk_hdr->offset =
      (start_sector_ % (pc_size_ / DEFAULT_SECTOR_SIZE)) * DEFAULT_SECTOR_SIZE;
  chunk_hdr->length = g_context->config().operate_size();

  chunk_hdr->lc_id = src_lc_id_;
  chunk_hdr->lc_random_id = src_lc_random_id_;
  chunk_hdr->lc_size = lc_size_;
  chunk_hdr->pc_no = pc_;
  chunk_hdr->pg_id = src_pg_id_;
  chunk_hdr->route_version = cluster_version_;
  chunk_hdr->handle_id = uevent::MessageUtil::ObjId();
  chunk_hdr->reserved1 = 0;
  chunk_hdr->reserved2 = 0;
  chunk_hdr->magic = 0xf123987a;
}

void DoHelaPcTaskHandle::HandleReadChunkResponse(
    void *msg_hdr, void *msg_data, const ConnectionUeventPtr &conn) {
  udisk::common::HelaChunkResponseHeadV2 *chunk_hdr =
      (udisk::common::
           HelaChunkResponseHeadV2 *)((char *)msg_hdr +
                                      sizeof(udisk::common::MessageHeaderV2));

  if (!chunk_hdr->retcode) {
    LOG_DEBUG << "desc: read chunk succ"
              << " lc_id(" << chunk_hdr->lc_id << ")"
              << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
              << " pc_no(" << chunk_hdr->pc_no << ")";

    if (pc_task_->src_remote_chunk())
      loop_handle_->IncReadRemoteBytes(g_context->config().operate_size());
    else
      loop_handle_->IncReadLocalBytes(pc_task_->chunk_source_info().chunk_id_,
                                      g_context->config().operate_size());

    buffer_ = (StrBuffer *)msg_data;
    assert(buffer_->Size() == g_context->config().operate_size());

    return HandleWriteChunk();
  } else {
    if (chunk_hdr->retcode == udisk::common::RETCODE_RESET_CONNECTION) {
      loop_handle_->DestroyOutConnection(conn->GetPeerAddress().ToIpString(),
                                         conn->GetPeerAddress().ToPort());
      loop_handle_->ResetConnection();
      set_retry_read(true);
    } else if (chunk_hdr->retcode ==
               udisk::common::RETCODE_CLUSTER_VERSION_ERR) {
      LOG_WARN << "desc: read chunk cluster change"
               << " chunk_ip(" << src_ip_ << ")"
               << " chunk_port(" << src_port_ << ")"
               << " lc_id(" << chunk_hdr->lc_id << ")"
               << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
               << " route_version(" << chunk_hdr->route_version << ")"
               << " pc_no(" << pc_task_->pc() << ")"
               << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
      // 确认远程chunk源已经失效
      // TODO: 可以根据chunk_hdr->route_version更新版本?
      return FinishHelaPcTask(VDISK_RET_SOURCE_UNAVAILABLE);
    } else if (chunk_hdr->retcode == udisk::common::RETCODE_CHUNK_NOT_READY) {
      LOG_WARN << "desc: read chunk not ready"
               << " chunk_ip(" << src_ip_ << ")"
               << " chunk_port(" << src_port_ << ")"
               << " lc_id(" << chunk_hdr->lc_id << ")"
               << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
               << " pc_no(" << chunk_hdr->pc_no << ")"
               << " retcode(" << chunk_hdr->retcode << ")";
      return set_pending_read(true);
    } else if (chunk_hdr->retcode == udisk::common::RETCODE_AIO_READ_TIMEOUT) {
      LOG_WARN << "desc: read chunk aio timeout"
               << " chunk_ip(" << src_ip_ << ")"
               << " chunk_port(" << src_port_ << ")"
               << " lc_id(" << chunk_hdr->lc_id << ")"
               << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
               << " pc_no(" << chunk_hdr->pc_no << ")"
               << " retcode(" << chunk_hdr->retcode << ")";
      return set_pending_read(true);
    } else if (chunk_hdr->retcode == udisk::common::RETCODE_HELA_QUEUE_FULL) {
      LOG_WARN << "desc: read chunk queue full"
               << " chunk_ip(" << src_ip_ << ")"
               << " chunk_port(" << src_port_ << ")"
               << " lc_id(" << chunk_hdr->lc_id << ")"
               << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
               << " pc_no(" << chunk_hdr->pc_no << ")"
               << " retcode(" << chunk_hdr->retcode << ")";
      return set_pending_read(true);
    } else {
      LOG_ERROR << "desc: read chunk error"
                << " chunk_ip(" << src_ip_ << ")"
                << " chunk_port(" << src_port_ << ")"
                << " lc_id(" << chunk_hdr->lc_id << ")"
                << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
                << " pc_no(" << chunk_hdr->pc_no << ")"
                << " retcode(" << chunk_hdr->retcode << ")";
      set_retry_read(true);
    }
  }
}

void DoHelaPcTaskHandle::HandleWriteChunk() {
  if (iter_ == info_list_.end()) {

    //在下一轮读源之前，返还buffer至内存池
    SafePutPureStrBuffer(buffer_);
    buffer_ = nullptr;

    //重置info list迭代器
    iter_ = info_list_.begin();

    return ReadNextPart();
  }

  //每次写chunk之前check一次cluster
  if (CheckCluster()) {
    LOG_ERROR << "desc: cluster change chunk source unavaliable"
              << " pc_no(" << pc_task_->pc() << ")"
              << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
    return FinishHelaPcTask(VDISK_RET_SOURCE_UNAVAILABLE);
  }

  const TinyPcTaskInfoPtr &info = *iter_;

  // chunk已经下线，无需去写
  if (info->offline()) {
    LOG_DEBUG << "desc: chunk offline, no need to write"
              << " pc_no(" << pc_task_->pc() << ")"
              << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")"
              << " lc_id(" << info->lc_id() << ")"
              << " lc_random_id(" << info->lc_random_id() << ")"
              << " vdisk_id(" << info->vdisk_id() << ")"
              << " chrono_id(" << info->chrono_id() << ")";
    iter_++;

    return HandleWriteChunk();
  }

  return WriteDataToChunk(info);
}

void DoHelaPcTaskHandle::WriteDataToChunk(const TinyPcTaskInfoPtr &info) {
  std::string dest_ip = info->pc_router().copies(0).ip();
  uint32_t dest_port = info->pc_router().copies(0).port();
  uint32_t chunk_id = info->pc_router().copies(0).id();

  //超过chunk写阈值, pending write
  if (!g_context->flow_monitor()->GrantWriteLocal(
           chunk_id, g_context->config().operate_size())) {
    LOG_TRACE << "desc: trigger pending write";
    set_pending_write(true);
    return;
  }

  uevent::ConnectionUeventPtr connection =
      loop_handle_->GetOutConnection(dest_ip, dest_port);
  if (!connection) {
    LOG_ERROR << "desc: no connection for chunk"
              << " pc_no(" << pc_ << ")"
              << " dst_ip(" << dest_ip << ")"
              << " dst_port(" << dest_port << ")";
    set_retry_write(true);
  }

  const size_t msg_hdr_len = sizeof(udisk::common::MessageHeaderV2) +
                             sizeof(udisk::common::HelaChunkRequestHeadV2);
  char msg_hdr[msg_hdr_len];
  ::memset(msg_hdr, 0, msg_hdr_len);
  udisk::common::MessageHeaderV2 *common_hdr =
      (udisk::common::MessageHeaderV2 *)msg_hdr;
  udisk::common::HelaChunkRequestHeadV2 *chunk_hdr =
      (udisk::common::
           HelaChunkRequestHeadV2 *)(msg_hdr +
                                     sizeof(udisk::common::MessageHeaderV2));

  BuildChunkWriteRequest(common_hdr, chunk_hdr, info);

  bool ret = (connection->SendData(msg_hdr, msg_hdr_len) == 0) &&
             (connection->SendData(buffer_->data(), buffer_->Size()) == 0);

  if (!ret) {
    LOG_ERROR << "desc: send chunk write req failed"
              << " dest_ip(" << dest_ip << ")"
              << " dest_port(" << dest_port << ")"
              << " offset(" << chunk_hdr->offset << ")"
              << " length(" << chunk_hdr->length << ")"
              << " lc_id(" << chunk_hdr->lc_id << ")"
              << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
              << " lc_size(" << chunk_hdr->lc_size << ")"
              << " pc_no(" << chunk_hdr->pc_no << ")"
              << " cluster_version(" << chunk_hdr->route_version << ")"
              << " handle_id(" << chunk_hdr->handle_id << ")";
    set_retry_write(true);
  } else {
    LOG_DEBUG << "desc: send chunk write req succ"
              << " dest_ip(" << dest_ip << ")"
              << " dest_port(" << dest_port << ")"
              << " offset(" << chunk_hdr->offset << ")"
              << " length(" << chunk_hdr->length << ")"
              << " lc_id(" << chunk_hdr->lc_id << ")"
              << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
              << " lc_size(" << chunk_hdr->lc_size << ")"
              << " pc_no(" << chunk_hdr->pc_no << ")"
              << " cluster_version(" << chunk_hdr->route_version << ")"
              << " handle_id(" << chunk_hdr->handle_id << ")";

    std::string timeout_desc = "obj_id:" + std::to_string(chunk_hdr->handle_id);
    loop_handle_->RegisterResponse(
        chunk_hdr->handle_id,
        std::bind(&DoHelaPcTaskHandle::HandleWriteChunkResponse,
                  shared_from_this(), std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3),
        std::bind(&DoHelaPcTaskHandle::WriteChunkTimeOut, shared_from_this()),
        timeout_desc,
        (double)g_context->config().single_pc_write_chunk_timeout_second());
  }
}

void DoHelaPcTaskHandle::BuildChunkWriteRequest(
    udisk::common::MessageHeaderV2 *common_hdr,
    udisk::common::HelaChunkRequestHeadV2 *chunk_hdr,
    const TinyPcTaskInfoPtr &info) {
  common_hdr->msg_type = udisk::common::MSG_HELA_CHUNK_REQ;
  common_hdr->version = udisk::common::MSG_V2;
  common_hdr->data_len = sizeof(udisk::common::HelaChunkRequestHeadV2) +
                         g_context->config().operate_size();

  chunk_hdr->size = g_context->config().operate_size();
  chunk_hdr->cmd = udisk::common::HELA_CMD_WRITE;
  chunk_hdr->offset =
      (start_sector_ % (pc_size_ / DEFAULT_SECTOR_SIZE)) * DEFAULT_SECTOR_SIZE;
  chunk_hdr->length = g_context->config().operate_size();

  chunk_hdr->lc_id = info->lc_id();
  chunk_hdr->lc_random_id = info->lc_random_id();
  chunk_hdr->lc_size = lc_size_;
  chunk_hdr->pc_no = pc_;
  chunk_hdr->pg_id = info->pc_router().pg_id();
  chunk_hdr->route_version = cluster_version_;
  chunk_hdr->handle_id = uevent::MessageUtil::ObjId();
  chunk_hdr->reserved1 = 0;
  chunk_hdr->reserved2 = 0;
  chunk_hdr->magic = 0xf123987a;
}

void DoHelaPcTaskHandle::HandleWriteChunkResponse(
    void *msg_hdr, void *msg_data, const ConnectionUeventPtr &conn) {
  udisk::common::HelaChunkResponseHeadV2 *chunk_hdr =
      (udisk::common::
           HelaChunkResponseHeadV2 *)((char *)msg_hdr +
                                      sizeof(udisk::common::MessageHeaderV2));

  const TinyPcTaskInfoPtr &info = *iter_;
  uint32_t chunk_id = info->pc_router().copies(0).id();
  loop_handle_->IncWriteLocalBytes(chunk_id,
                                   g_context->config().operate_size());

  if (!chunk_hdr->retcode) {
    LOG_DEBUG << "desc: write chunk succ"
              << " lc_id(" << chunk_hdr->lc_id << ")"
              << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
              << " pc_no(" << chunk_hdr->pc_no << ")";

    iter_++;
    back_off_.ReportStatus(true);
    return HandleWriteChunk();
  } else {
    if (chunk_hdr->retcode == udisk::common::RETCODE_RESET_CONNECTION) {
      loop_handle_->DestroyOutConnection(conn->GetPeerAddress().ToIpString(),
                                         conn->GetPeerAddress().ToPort());
      loop_handle_->ResetConnection();
    } else if (chunk_hdr->retcode ==
               udisk::common::RETCODE_CLUSTER_VERSION_ERR) {
      LOG_WARN << "desc: write chunk cluster change "
               << " lc_id(" << chunk_hdr->lc_id << ")"
               << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
               << " route_version(" << chunk_hdr->route_version << ")"
               << " pc_no(" << pc_task_->pc() << ")"
               << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
      // TODO: 可以根据chunk_hdr->route_version更新版本?
      if (CheckCluster()) {
        LOG_ERROR << "desc: cluster change chunk source unavaliable"
                  << " pc_no(" << pc_task_->pc() << ")"
                  << " src_vdisk_id(" << pc_task_->src_vdisk_id() << ")";
        return FinishHelaPcTask(VDISK_RET_SOURCE_UNAVAILABLE);
      }
    } else if (chunk_hdr->retcode == udisk::common::RETCODE_CHUNK_NOT_READY) {
      LOG_WARN << "desc: write chunk not ready"
               << " lc_id(" << chunk_hdr->lc_id << ")"
               << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
               << " pc_no(" << chunk_hdr->pc_no << ")"
               << " retcode(" << chunk_hdr->retcode << ")";
    } else if (chunk_hdr->retcode == udisk::common::RETCODE_AIO_WRITE_TIMEOUT) {
      LOG_WARN << "desc: write chunk aio timeout"
               << " chunk_ip(" << src_ip_ << ")"
               << " chunk_port(" << src_port_ << ")"
               << " lc_id(" << chunk_hdr->lc_id << ")"
               << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
               << " pc_no(" << chunk_hdr->pc_no << ")"
               << " retcode(" << chunk_hdr->retcode << ")";
    } else if (chunk_hdr->retcode == udisk::common::RETCODE_HELA_QUEUE_FULL) {
      LOG_WARN << "desc: write chunk queue full"
               << " chunk_ip(" << src_ip_ << ")"
               << " chunk_port(" << src_port_ << ")"
               << " lc_id(" << chunk_hdr->lc_id << ")"
               << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
               << " pc_no(" << chunk_hdr->pc_no << ")"
               << " retcode(" << chunk_hdr->retcode << ")";
    } else {
      LOG_ERROR << "desc: write chunk error"
                << " lc_id(" << chunk_hdr->lc_id << ")"
                << " lc_random_id(" << chunk_hdr->lc_random_id << ")"
                << " pc_no(" << chunk_hdr->pc_no << ")"
                << " retcode(" << chunk_hdr->retcode << ")";
    }
    // 对于write chunk不做最大重试限制，有最大超时保证存在
    return set_pending_write(true);
  }
}

void DoHelaPcTaskHandle::ReadNextPart() {
  next_read_sector_ = start_sector_ + sector_num_;

  if (next_read_sector_ < end_sector_) {
    if (pc_task_->src_koala()) {
      return DecideKoalaSource(true, false);
    } else {
      return ReadDataFromChunk();
    }
  }

  return FinishHelaPcTask(VDISK_RET_SUCCESS);
}

bool DoHelaPcTaskHandle::CheckCluster() {
  if (cluster_version_ != loop_handle_->ClusterVersion()) {
    LOG_INFO << "desc: handle cluster change in CheckCluster"
             << " old_cluster_version(" << cluster_version_ << ")"
             << " new_cluster_version(" << loop_handle_->ClusterVersion()
             << ")";

    cluster_version_ = loop_handle_->ClusterVersion();
    pc_task_->set_cluster_version(cluster_version_);

    const ChunkInfoMap &chunks = loop_handle_->ChunkInfo();

    if (!pc_task_->src_koala()) {
      uint32_t chunk_id = chunk_source_info_.chunk_id_;

      if (HelaHelper::IsChunkOffline(chunk_id, chunks)) {
        LOG_INFO << "desc: chunk offline"
                 << " chunk_id(" << chunk_id << ")";
        chunk_source_info_.offline_ = true;
      }
    }

    for (auto &it : info_list_) {
      uint32_t chunk_id = it->pc_router().copies(0).id();

      if (HelaHelper::IsChunkOffline(chunk_id, chunks)) {
        LOG_INFO << "desc: chunk offline"
                 << " chunk_id(" << chunk_id << ")";
        it->set_offline(true);
      }
    }
  }

  return chunk_source_info_.offline_;
}

void DoHelaPcTaskHandle::FinishHelaPcTask(int retcode) {
  LOG_DEBUG << "desc: hela pc task finish"
            << " pc_no(" << pc_ << ")"
            << " retcode: " << retcode;

  back_off_.ReportStatus(true);
  loop_handle_->GetLoop()->CancelTimer(timer_id_);

  g_context->manager_handle()->GetLoop()->RunInLoop(
      std::bind(rsp_hook_, retcode, pc_));
}

}  // namespace hela
}  // namespace udataark
