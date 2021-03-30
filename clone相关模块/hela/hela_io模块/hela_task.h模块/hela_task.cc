// Copyright (c) 2020 UCloud All rights reserved.
#include "hela_task.h"
#include "constanst.h"

namespace udisk {
namespace hela {

// ==================== HelaTask ===================== //

HelaTask::HelaTask(const std::string& chrono_id)
    : chrono_id_(chrono_id),
      status_(PB_HELA_TASK_READY),
      version_(0),
      vdisk_type_(VDIKS_TYPE_NONE),
      lc_id_(0),
      lc_random_id_(0),
      lc_size_(0),
      pc_size_(0),
      cluster_version_(0),
      src_version_(0),
      src_vdisk_type_(VDIKS_TYPE_NONE),
      shard_size_(common::IMAGE_SHARD_SIZE),
      need_repair_(false),
      repair_job_(false) {}

HelaTask::~HelaTask() {}

// ==================== HelaPcTask ===================== //

HelaPcTask::HelaPcTask()
    : pc_(0),
      status_(READY),
      lc_size_(0),
      pc_size_(0),
      cluster_version_(0),
      vdisk_type_(VDIKS_TYPE_NONE),
      src_type_(KOALA),
      shard_size_(common::IMAGE_SHARD_SIZE),
      src_version_(0),
      has_data_(false),
      record_active_(false) {}

HelaPcTask::~HelaPcTask() {}

// ==================== TinyPcTaskInfo ==================== //

TinyPcTaskInfo::TinyPcTaskInfo(const std::string& chrono_id)
    : chrono_id_(chrono_id), offline_(false), lc_id_(0), lc_random_id_(0) {}

TinyPcTaskInfo::~TinyPcTaskInfo() {}

};  // namespace hela
};  // namespace udisk
