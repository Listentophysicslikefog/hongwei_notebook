// Copyright (c) 2020 UCloud All rights reserved.
#include <uevent/base/logging.h>
#include <algorithm>

#include "resource_query.h"

#include "idun_context.h"
#include "idun_helper.h"
#include "message_util.h"
#include "metaserver.h"
#include "umessage.h"

namespace udataark {
namespace idun {

uint32_t ResourceQueryHandle::ID =
    ucloud::utimemachine::IDUN_QUERY_RESOURCE_REQUEST;

ResourceQueryHandle::ResourceQueryHandle(uevent::UeventLoop *loop) {}

ResourceQueryHandle::~ResourceQueryHandle() {}

void ResourceQueryHandle::Timeout() {
  LOG_ERROR << "query resource timeout";
  ret_code_ = ucloud::utimemachine::VDISK_RET_TIME_OUT;
  ret_msg_ = "timeout";
  DoResponse();
}

void ResourceQueryHandle::EntryInit(const uevent::ConnectionUeventPtr &conn,
                                    ucloud::UMessage *um) {
  conn_ = conn;
  MakeResponse(um, ucloud::utimemachine::IDUN_QUERY_RESOURCE_RESPONSE,
               &response_);

  const ucloud::utimemachine::IdunQueryResourceRequest &req =
      um->body().GetExtension(
          ucloud::utimemachine::idun_query_resource_request);

  if (!g_context->is_leader()) {
    ret_code_ = ucloud::utimemachine::VDISK_RET_FAIL;
    ret_msg_ = "I am not leader";
    LOG_ERROR << ret_msg_;
    DoResponse();
    return;
  }
  if (!g_context->manager_handle()->is_init()) {
    ret_code_ = ucloud::utimemachine::VDISK_RET_FAIL;
    ret_msg_ = "idun is not ready";
    LOG_ERROR << ret_msg_;
    DoResponse();
    return;
  }

  if (!IdunHelper::ParseQueryResource(req, index_key_, shard_idxs_)) {
    LOG_ERROR << "parse query resource fail";
    ret_code_ = ucloud::utimemachine::VDISK_RET_FAIL;
    ret_msg_ = "resource error";
    DoResponse();
    return;
  }

  LOG_DEBUG << "query resource, index_key:"
            << ResourceIndexKeyToString(index_key_);

  if (shard_idxs_.empty()) {
    ret_code_ = ucloud::utimemachine::VDISK_RET_FAIL;
    ret_msg_ = "pc no empty";
    DoResponse();
    return;
  }

  QueryResource();
}

void ResourceQueryHandle::QueryResource() {
  ResourceIndexPtr resource_index =
      g_context->resource_manager()->GetResourceIndex(index_key_);
  if (resource_index == nullptr) {
    // 相关资源没有注册, 开始克隆的时候就应该注册了
    ret_code_ = ucloud::utimemachine::VDISK_RET_NO_EXIST;
    ret_msg_ = "no resource";
    DoResponse();
    return;
  }

  ucloud::utimemachine::IdunQueryResourceResponse *res =
      response_.mutable_body()->MutableExtension(
          ucloud::utimemachine::idun_query_resource_response);

  // 支持批量查询
  for (const auto &share_idx : shard_idxs_) {
    // 获取同源分片集合
    auto itr = resource_index->slice_index().find(share_idx);
    if (itr == resource_index->slice_index().end()) {
      LOG_DEBUG << "slice index not exists, share_idx" << share_idx;
      continue;
    }
    ResourceSliceSet &slices = itr->second;
    if (slices.empty()) {
      LOG_DEBUG << "slice index empty, share_idx: " << share_idx;
      continue;
    }
    LOG_DEBUG << "slice index, share_idx: " << share_idx
              << ", size:" << slices.size();

    Metaserver *metaserver = g_context->metaserver();
    metaserver->LockNowTick();

    // 获取按取出次数排序的chunk
    ChunkUseCountSet online_chunks;
    std::set<uint32_t> online_chunk_ids;
    for (auto &itr : metaserver->chunk_infos()) {
      const ChunkInfo &chunk = itr.second;
      if (chunk.state == CHUNK_STATE_ONLINE) {
        uint32_t host_value = metaserver->GetHostUseCount(chunk.ip);
        uint32_t chunk_value = metaserver->GetChunkUseCount(chunk.id);
        online_chunks.insert(
            std::make_tuple(chunk.id, host_value, chunk_value));
        online_chunk_ids.insert(chunk.id);
      }
    }

    // 将分片对应到chunk上
    std::map<uint32_t, ResourceSliceList> online_slices;
    for (auto &slice : slices) {
      // TODO: 目前只考虑chunk分片
      PcRoute *pc = slice->pc_route();
      if (online_chunk_ids.find(pc->chunk_id) != online_chunk_ids.end()) {
        online_slices[pc->chunk_id].push_back(slice);
      }
    }

    if (online_slices.empty()) {
      LOG_DEBUG << "slice online empty, share_idx" << share_idx;
      continue;
    }

    // 按照使用次数遍历chunk
    for (auto &chunk_use_count : online_chunks) {
      uint32_t chunk_id = std::get<0>(chunk_use_count);

      // 判断chunk上是否有分片
      if (online_slices.find(chunk_id) != online_slices.end()) {
        // 判断当前chunk是否繁忙
        uint32_t host_value = std::get<1>(chunk_use_count);
        uint32_t chunk_value = std::get<2>(chunk_use_count);
        if (host_value >=
                (uint32_t)g_context->config().max_return_host_pc_per_second() ||
            chunk_value >= (uint32_t)g_context->config()
                               .max_return_chunk_pc_per_second()) {
          LOG_DEBUG << "slice busy, share_idx:" << share_idx;
          ret_code_ = ucloud::utimemachine::VDISK_RET_BUSY;
          ret_msg_ = "resource busy";
          continue;
        }

        // 选择一个最新创建的盘上的分片
        ResourceSliceList &chunk_slices = online_slices[chunk_id];
        assert(!chunk_slices.empty());

        auto newest_slice_itr = std::max_element(
            chunk_slices.begin(), chunk_slices.end(),
            [](const ResourceSlicePtr &a, const ResourceSlicePtr &b) {
              return a->resource()->create_tick() <
                     b->resource()->create_tick();
            });

        const ResourceSlicePtr &newest_slice = *newest_slice_itr;
        slices_.push_back(newest_slice);

        // 添加chunk及host的使用次数
        const ChunkInfo &chunk = metaserver->chunk(chunk_id);
        metaserver->IncChunkUseCount(chunk_id);
        metaserver->IncHostUseCount(chunk.ip);
        break;
      }
    }

    // 当前只返回一个分片
    IdunHelper::PopulateQueryResource(res, slices_.front()->resource(),
                                      slices_);
    slices_.clear();
  }
  LOG_DEBUG << "lc info size: " << res->lc_infos_size();

  // 批量查询有一个查询到也算成功
  if (res->lc_infos_size() > 0) {
    ret_code_ = ucloud::utimemachine::VDISK_RET_SUCCESS;
    ret_msg_ = "successs";
  } else {
    ret_code_ = ucloud::utimemachine::VDISK_RET_EMPTY;
    ret_msg_ = "resource empty";
  }
  DoResponse();
}

void ResourceQueryHandle::DoResponse() {
  ucloud::utimemachine::IdunQueryResourceResponse *res =
      response_.mutable_body()->MutableExtension(
          ucloud::utimemachine::idun_query_resource_response);

  res->mutable_rc()->set_retcode(ret_code_);
  res->mutable_rc()->set_error_message(ret_msg_);

  LOG_DEBUG << "query resource end, index_key:"
            << ResourceIndexKeyToString(index_key_) << ", retcode:" << ret_code_
            << ", msg:" << ret_msg_;

  uevent::MessageUtil::SendPbResponse(conn_, response_);
}

}  // idun
}  // udataark
