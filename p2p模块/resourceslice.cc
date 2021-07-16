



class ResourceSlice {  //一个pc的信息   一个shard有多个pc
  friend class Resource;

 public:
  ~ResourceSlice();

  bool operator==(const ResourceSlice &rhs) const;
  bool operator!=(const ResourceSlice &rhs) const { return !operator==(rhs); }

  bool operator<(const ResourceSlice &rhs) const;

  bool IsUdisk() const;
  uint32_t GetIndex() const;  // 获取分片索引序号

  Resource *resource() { return resource_; }
  PcRoute *pc_route() {
    assert(IsUdisk());
    return ptr_.pc;
  }

 private:
  ResourceSlice(Resource *r);

  Resource *resource_;
  uint32_t type_;  // 与所属资源type一致
  union {
    PcRoute *pc;
  } ptr_;

  uint32_t create_tick_;
};

ResourceSlice;   pc_no pg_id chunk_id   //一个pc的信息   一个shard有多个pc  ResourceSlice只可以由Resource 创建对象 
Resource;      // 一块盘的信息 以及镜像源信息
ResourceIndex;  // 管理 一个Resource  的所有的pc源分片?? 管理一块盘的所有源的pc信息
ResourceManager; // 管理 所有 ResourceIndexMap indexs_   管理所有  ResourceMap resources_

typedef std::unordered_map<ResourceIndexKey, ResourceIndexPtr,
                           ResourceIndexKeyHash> ResourceIndexMap; --->value ResourceIndexPtr

###################################################

1. fryer向idun注册    fryer login idun 每一块盘都要注册一次
message IdunResourceLoginRequest {
    required string vdisk_id = 1;         // 资源盘id
    //required uint64 vdisk_cap = 2;        // 资源盘容量
    optional uint32 version = 3;          // 资源盘版本号
    required VDiskType vdisk_type = 4;    // 资源盘类型
    required uint32 set_id = 8;  //集群set

    required string src_vdisk_id = 5;     // 资源源数据盘id
    optional uint32 src_version = 6;      // 资源源数据版本号
    required VDiskType src_vdisk_type = 7;// 

    optional IdunLCInfo lc_info = 11;     // udisk必填
}


message IdunLCInfo {
    optional string extern_id = 1;
    required uint32 lc_id = 2;
    required uint32 lc_random_id = 3;
    required uint32 lc_size = 4;            // 云盘大小
    //required uint64 pc_size = 5;            // 分片大小
    //required uint64 cluster_version = 6;    // 集群版本号
    repeated IdunPcRoute pc_routes = 7;     // 查询时填写
}

message IdunPcRoute {
    required uint32 pc_no = 1;              // 分片序号
    required uint32 pg_id = 2;

    required uint32 chunk_id = 3;           // idun只取chunk_id
    optional string chunk_ip = 4;
    optional uint32 chunk_port = 5;
}

uint32_t objid = uevent::MessageUtil::ObjId();
  uint32_t flowno = uevent::MessageUtil::Flowno();
  NewMessage_v2(&msg, flowno, base::MyUuid::NewUuid(),
                ucloud::utimemachine::IDUN_RESOURCE_LOGIN_REQUEST, 0, false,
                objid, 0, "LoginIdunResource", NULL, NULL);
  ucloud::utimemachine::IdunResourceLoginRequest *req =
      msg.mutable_body()->MutableExtension(
          ucloud::utimemachine::idun_resource_login_request);

  req->set_vdisk_id(hela_session_->vdisk_id);
  req->set_version(hela_session_->version);
  req->set_vdisk_type(
      (ucloud::utimemachine::VDiskType)hela_session_->vdisk_type);
  req->set_src_vdisk_id(hela_session_->src_vdisk_id);
  req->set_src_version(hela_session_->src_version);
  req->set_src_vdisk_type(
      (ucloud::utimemachine::VDiskType)hela_session_->src_vdisk_type);

  ucloud::utimemachine::IdunLCInfo *lc_info = req->mutable_lc_info();
  lc_info->set_extern_id(hela_session_->vdisk_id);
  lc_info->set_lc_id(hela_session_->lc_id);
  lc_info->set_lc_random_id(hela_session_->lc_random_id);
  lc_info->set_lc_size(hela_session_->lc_size);


2. idun 接受 fryer login


const ucloud::utimemachine::IdunResourceLoginRequest &req =
      um->body().GetExtension(
          ucloud::utimemachine::idun_resource_login_request);

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

  if (!IdunHelper::ParseLoginResource(req, &resource_, slices_)) {
    LOG_ERROR << "parse login resource fail";
    ret_code_ = ucloud::utimemachine::VDISK_RET_FAIL;
    ret_msg_ = "param error";
    DoResponse();
    return;
  }bool IdunHelper::ParseLoginResource(
    const ucloud::utimemachine::IdunResourceLoginRequest &req,
    Resource **resource, ResourceSliceList &slices) {
  Resource *new_resource = new Resource(
      req.vdisk_id(), req.has_version() ? req.version() : DEFAULT_VDISK_VERSION,
      req.vdisk_type());

  // src_vdisk_id
  new_resource->set_src_vdisk_id(req.src_vdisk_id());
  // src_version
  new_resource->set_src_version(req.has_src_version() ? req.src_version()
                                                      : DEFAULT_VDISK_VERSION);
  // src_vdisk_type
  new_resource->set_src_vdisk_type(req.src_vdisk_type());

  // 解析资源盘信息
  if (new_resource->IsUdisk()) {
    ParseUdiskResourceInfo(req.lc_info(), new_resource->lc_info());
    bool IdunHelper::ParseUdiskResourceInfo(
    const ucloud::utimemachine::IdunLCInfo &lc_info, LCInfo *lc) {
  // extern_id
  if (lc_info.has_extern_id()) lc->extern_id = lc_info.extern_id();
  // lc_id
  lc->lc_id = lc_info.lc_id();
  // lc_random_id
  lc->lc_random_id = lc_info.lc_random_id();
  // lc_size
  lc->lc_size = lc_info.lc_size();
  // cluster_version
  // lc->cluster_version = lc_info.cluster_version();

  return true;
}
    ParseUdiskResourceSlices(req.lc_info().pc_routes(), new_resource, slices);
    bool IdunHelper::ParseUdiskResourceSlices(
    const PB_ARRAY(ucloud::utimemachine::IdunPcRoute) & pc_routes,
    Resource *resource, ResourceSliceList &slices) {
  if (pc_routes.size() == 0) {
    LOG_INFO << "udisk resource slice empty";
    return false;
  }

  for (int i = 0; i < pc_routes.size(); ++i) {
    const ucloud::utimemachine::IdunPcRoute &pc_route = pc_routes.Get(i);
    ResourceSlicePtr slice = resource->CreateSlice();
    PcRoute *pc = slice->pc_route();

    pc->pc_no = pc_route.pc_no();
    pc->pg_id = pc_route.pg_id();
    pc->chunk_id = pc_route.chunk_id();
    // pc->chunk_ip = pc_route.chunk_ip();
    // pc->chunk_port = pc_route.chunk_port();

    slices.push_back(slice);
  }

  return true;
}
  }

  *resource = new_resource;
  return true;
}

  ResourceKey key = resource_->GetKey();

  LOG_INFO << "login resource, key:"
           << ResourceKeyToString(resource_->GetKey());

  // 检查资源是否已存在
  if (resource_manager_->GetResource(resource_->GetKey()) != nullptr) {
    LOG_INFO << "resource already exists, key:"
             << ResourceKeyToString(resource_->GetKey());
    ret_code_ = ucloud::utimemachine::VDISK_RET_YET_EXIST;
    ret_msg_ = "yet exists";
    DoResponse();
    return;
  }

  InsertResource();
}

void ResourceLoginHandle::InsertResource() {
  uevent::ConnectionUeventPtr conn =
      GetZKConnection(udisk::common::ConfigParser::kUmongoName);
  if (conn == nullptr) {
    LOG_ERROR << "get umongo connection fail";
    ret_code_ = ucloud::utimemachine::VDISK_RET_FAIL;
    ret_msg_ = "umongo conn error";
    DoResponse();
    return;
  }

  ucloud::UMessage msg;
  uint32_t objid = uevent::MessageUtil::ObjId();
  uint32_t flowno = uevent::MessageUtil::Flowno();
  NewMessage_v2(&msg, flowno, base::MyUuid::NewUuid(),
                ucloud::umgogate::EXECUTE_MGO_REQUEST, 0, false, objid, 0,
                "IdunInsertResource", NULL, NULL);
  ucloud::umgogate::ExecuteMgoRequest *req =
      msg.mutable_body()->MutableExtension(
          ucloud::umgogate::execute_mgo_request);

  req->set_db(g_context->config().db_name());
  req->set_collection(DB_IDUN_RESOURCE_TABLE_NAME);
  req->set_optype(ucloud::umgogate::OP_INSERT);

  ucloud::umgogate::OpInsertReq *insert_req = req->mutable_op_insert_req();
  insert_req->add_doc(IdunHelper::AssembleResource(resource_));

  uevent::MessageUtil::SendPbRequest(
      conn, msg, std::bind(&ResourceLoginHandle::EntryInsertResource,
                           this_ptr(), std::placeholders::_1),
      std::bind(&ResourceLoginHandle::Timeout, this_ptr()), 5);
}

void ResourceLoginHandle::EntryInsertResource(ucloud::UMessage *um) {
  const ucloud::umgogate::ExecuteMgoResponse &res =
      um->body().GetExtension(ucloud::umgogate::execute_mgo_response);
  if (res.rc().retcode() != 0) {
    LOG_ERROR << "insert resource failed, " << res.rc().error_message();
    ret_code_ = ucloud::utimemachine::VDISK_RET_FAIL;
    ret_msg_ = "insert resource error";
    return;
  }

  if (!resource_manager_->AddResource(resource_)) {
    LOG_ERROR << "desc:critical add resource error, key:"
              << ResourceKeyToString(resource_->GetKey());
    ret_code_ = ucloud::utimemachine::VDISK_RET_FAIL;
    ret_msg_ = "add resource error";
    return;
  }

  DoResponse();
}

#############################################################################

3. hela 向 idun更新源

message IdunUpdateResourceRequest {
    required string vdisk_id = 1;
    optional uint32 version = 3;
    required VDiskType vdisk_type = 4;
    //optional bool offline = 5 [default = false];  // 是否下线

    repeated IdunPcRoute pc_routes = 11;          // udisk必填
}

message IdunPcRoute {
    required uint32 pc_no = 1;              // 分片序号
    required uint32 pg_id = 2;

    required uint32 chunk_id = 3;           // idun只取chunk_id
    optional string chunk_ip = 4;
    optional uint32 chunk_port = 5;
}

void HelaHelper::AssembleUpdateIdunResourceReq(
    ucloud::utimemachine::IdunUpdateResourceRequest* req,
    const TinyPcTaskInfoPtr& info) {
  ucloud::utimemachine::IdunPcRoute* router = req->add_pc_routes();

  req->set_vdisk_id(info->vdisk_id());
  req->set_version(info->version());
  req->set_vdisk_type(info->vdisk_type());

  router->set_pc_no(info->pc_router().pc_no());
  router->set_pg_id(info->pc_router().pg_id());
  router->set_chunk_ip(info->pc_router().copies(0).ip());
  router->set_chunk_port(info->pc_router().copies(0).port());
  router->set_chunk_id(info->pc_router().copies(0).id());
}
//增加一个set id





3.1 idun 更新源
key_ = IdunHelper::ParseUpdateResourceKey(req);
ResourceKey IdunHelper::ParseUpdateResourceKey(
    const ucloud::utimemachine::IdunUpdateResourceRequest &req) {
  return ResourceKey(req.vdisk_id(),
                     req.has_version() ? req.version() : DEFAULT_VDISK_VERSION,
                     (uint32_t)req.vdisk_type());
}
Resource *resource = resource_manager_->GetResource(key_);
ResourceSliceList slices;
  if (!IdunHelper::ParseUpdateResourceSlices(req, resource, slices)) {
    LOG_ERROR << "parse resource slice fail, key:" << ResourceKeyToString(key_);
    ret_code_ = ucloud::utimemachine::VDISK_RET_FAIL;
    ret_msg_ = "param error";
    DoResponse();
    return;
  }
bool IdunHelper::ParseUpdateResourceSlices(
    const ucloud::utimemachine::IdunUpdateResourceRequest &req,
    Resource *resource, ResourceSliceList &slices) {

  if (resource->IsUdisk()) {
    return ParseUdiskResourceSlices(req.pc_routes(), resource, slices);
  }

  LOG_ERROR << "idun resource type unknow, " << req.vdisk_type();
  return false;
}


bool IdunHelper::ParseUdiskResourceSlices(
    const PB_ARRAY(ucloud::utimemachine::IdunPcRoute) & pc_routes,
    Resource *resource, ResourceSliceList &slices) {
  if (pc_routes.size() == 0) {
    LOG_INFO << "udisk resource slice empty";
    return false;
  }

  for (int i = 0; i < pc_routes.size(); ++i) {
    const ucloud::utimemachine::IdunPcRoute &pc_route = pc_routes.Get(i);
    ResourceSlicePtr slice = resource->CreateSlice();
    PcRoute *pc = slice->pc_route();

    pc->pc_no = pc_route.pc_no();
    pc->pg_id = pc_route.pg_id();
    pc->chunk_id = pc_route.chunk_id();
    // pc->chunk_ip = pc_route.chunk_ip();
    // pc->chunk_port = pc_route.chunk_port();

    slices.push_back(slice);
  }

  return true;
}

 for (auto &slice : slices) {
    // TODO: 检查添加失败, 部分失败如何处理
    resource_manager_->AddResourceSlice(slice);

    // 添加到待保存集合
    // resource->AddTmpSlice(slice);
  }

  ResourceSliceSet &tmp_slices = resource->tmp_slices();
  if (tmp_slices.size() <
      (size_t)g_context->config().slice_merge_insert_num()) {
    DoResponse();
    return;
  }

  // 取出tmp_slices集合
  save_slices_.insert(save_slices_.end(), tmp_slices.begin(), tmp_slices.end());
  tmp_slices.clear();




4. hela 向idun查源

message IdunQueryResourceRequest {
    required string src_vdisk_id = 1;
    optional uint32 src_version = 2;
    required VDiskType src_vdisk_type = 3;    // 数据源 盘类型

    required VDiskType vdisk_type = 4;        // 目标 盘类型

    repeated uint32 pc_nos = 11;              // udisk必填  pc_number多个     repeated
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

hela向idun批量查询，查寻到后计算机器间的时差获取实际的租约时间长度。然后下发task任务进行clone










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


4.2 idun端查源


message IdunQueryResourceResponse {
    required ResponseCode rc = 10;
    //required VDiskType vdisk_type = 20;

    repeated IdunLCInfo lc_infos = 30;  // udisk
}

message IdunLCInfo {
    optional string extern_id = 1;
    required uint32 lc_id = 2;
    required uint32 lc_random_id = 3;
    required uint32 lc_size = 4;            // 云盘大小
    //required uint64 pc_size = 5;            // 分片大小
    //required uint64 cluster_version = 6;    // 集群版本号
    repeated IdunPcRoute pc_routes = 7;     // 查询时填写
}

message IdunPcRoute {
    required uint32 pc_no = 1;              // 分片序号
    required uint32 pg_id = 2;

    required uint32 chunk_id = 3;           // idun只取chunk_id
    optional string chunk_ip = 4;
    optional uint32 chunk_port = 5;
}



ParseQueryResource(req, index_key_, shard_idxs_)) 
bool IdunHelper::ParseQueryResource(
    const ucloud::utimemachine::IdunQueryResourceRequest &req,
    ResourceIndexKey &key, std::set<uint32_t> &shard_idxs) {
  key = ResourceIndexKey(
      req.src_vdisk_id(),
      req.has_src_version() ? req.src_version() : DEFAULT_VDISK_VERSION,
      (uint32_t)req.src_vdisk_type(), (uint32_t)req.vdisk_type());
  if (IsUdisk(req.vdisk_type())) {
    for (int i = 0; i < req.pc_nos_size(); ++i)
      shard_idxs.insert(req.pc_nos(i));
  }

  return true;
}

 ResourceIndexPtr resource_index=
      g_context->resource_manager()->GetResourceIndex(index_key_);
      for (const auto &share_idx : shard_idxs_) {  // shard_idxs_ 应该是pcno的集合
        auto itr = resource_index->slice_index().find(share_idx);
        ResourceSliceSet &slices = itr->second;
        Metaserver *metaserver = g_context->metaserver();
    metaserver->LockNowTick();

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

hela端获取到源后的处理

void HelaHelper::FillChunkSourceInfo(struct ChunkSourceInfo& chunk_source,
                                     const PCResourcePtr& pc_resource) {
  chunk_source.pc_no_ = pc_resource->pc_no;
  chunk_source.pg_id_ = pc_resource->pg_id;
  chunk_source.chunk_ip_ = pc_resource->chunk_ip;
  chunk_source.chunk_id_ = pc_resource->chunk_id;
  chunk_source.chunk_port_ = pc_resource->chunk_port;

  chunk_source.lc_id_ = pc_resource->lc_id;
  chunk_source.lc_random_id_ = pc_resource->lc_random_id;
  chunk_source.cluster_version_ = pc_resource->cluster_version;
}



5. fryer发来idun_resource_expired_request

message IdunResourceExpiredRequest {
    required string vdisk_id = 1;
    required uint32 version = 2;
    required VDiskType vdisk_type = 3;
}


ucloud::UMessage msg;
  uint32_t objid = uevent::MessageUtil::ObjId();
  uint32_t flowno = uevent::MessageUtil::Flowno();
  NewMessage_v2(&msg, flowno, base::MyUuid::NewUuid(),
                ucloud::utimemachine::IDUN_RESOURCE_EXPIRED_REQUEST, 0, false,
                objid, 0, "IudnResourceExpired", NULL, NULL);
  ucloud::utimemachine::IdunResourceExpiredRequest *req =
      msg.mutable_body()->MutableExtension(
          ucloud::utimemachine::idun_resource_expired_request);

  req->set_vdisk_id(hela_session_->vdisk_id);
  req->set_version(hela_session_->version);
  req->set_vdisk_type(
      (ucloud::utimemachine::VDiskType)hela_session_->vdisk_type);


idun 处理 idun_resource_expired_request


key_ = IdunHelper::ParseExpiredResourceKey(req);
ResourceKey IdunHelper::ParseExpiredResourceKey(
    const ucloud::utimemachine::IdunResourceExpiredRequest &req) {
  return ResourceKey(req.vdisk_id(), req.version(), req.vdisk_type());
}


resource_ = resource_manager_->GetResource(key_);
// 资源还未完成，freyr不应该发送expired请求
  if (!resource_manager_->ResourceIsFinish(resource_)) {
    LOG_ERROR << "resource not finished, key:" << ResourceKeyToString(key_);
    ret_code_ = ucloud::utimemachine::VDISK_RET_FAIL;
    ret_msg_ = "not finished";
    DoResponse();
    return;
  }


void ResourceExpiredHandle::QueryResourceExpired() {
  ResourceIndexPtr resource_index =
      resource_manager_->GetResourceIndex(resource_->GetIndexKey());
  if (resource_index == nullptr) {
    LOG_ERROR << "resource index not exists, key:" << ResourceKeyToString(key_);
    ret_code_ = ucloud::utimemachine::VDISK_RET_NO_EXIST;
    ret_msg_ = "no resource index";
    DoResponse();
    return;
  }

  std::list<Resource *> &unfinish_resources =
      resource_index->unfinish_resources();
  std::list<Resource *> &finish_resources = resource_index->finish_resources();

  if (unfinish_resources.size() > 0) {
    // 计算需要保留的源个数
    uint32_t reserve = (uint32_t)g_context->config().max_reserve_resource_num();
    // 计算当前源在队列中的位置
    auto pos =
        std::find(finish_resources.begin(), finish_resources.end(), resource_);

    // TODO: 资源不存在于已完成队列
    /*
    if (pos == finish_resources.end()) {
      PB_ERROR << "resource not exists in finish list, key:"
               << ResourceKeyToString(key_);
      DoResponse();
      return;
    }
    */

    // 当前资源与队列末尾的距离
    uint32_t pos_dis_end = std::distance(pos, finish_resources.end());
    if (pos_dis_end <= reserve) {
      ret_code_ = ucloud::utimemachine::VDISK_RET_NEED_WAIT;
      ret_msg_ = "need wait";
      LOG_ERROR << "expired resource but need wait, key:"
                << ResourceKeyToString(key_) << ", index:" << pos_dis_end
                << " unfinish_resources: " << unfinish_resources.size()
                << " finish_resources: " << finish_resources.size()
                << " pos_dis_end: " << pos_dis_end << " reserve:" << reserve;
    }
  }

  DoResponse();
}
























typedef std::shared_ptr<ResourceIndex> ResourceIndexPtr;
typedef std::unordered_map<ResourceIndexKey, ResourceIndexPtr,
                           ResourceIndexKeyHash> ResourceIndexMap;


  class ResourceManager {
 public:
  ResourceManager();
  ~ResourceManager();

  ResourceMap &resources() { return resources_; }
  ResourceIndexMap &indexs() { return indexs_; }

  Resource *GetResource(const ResourceKey &key);
  bool AddResource(Resource *resource);         // 添加资源后添加索引
  void RemoveResource(const ResourceKey &key);  // 删除资源前先删索引

  bool ResourceIsFinish(Resource *resource);
  bool FinishResource(Resource *resource, uint32_t finish_tick);

  bool AddResourceSlice(const ResourceSlicePtr &slice);
  void RemoveResourceSlice(const ResourceSlicePtr &slice);

  ResourceIndexPtr GetResourceIndex(const ResourceIndexKey &key);

  void ClearAllResources();

 private:
  bool AddResourceIndex(Resource *resource);  // 计数+1, 不存在时创建
  void RemoveResourceIndex(Resource *resource);  // 计数-1, 计数为0时删除索引

  ResourceMap resources_;    // 资源列表, vdisk => Resource   管理所有源的盘相关信息
  ResourceIndexMap indexs_;  // 索引列表, src_vdisk => ResourceIndex  管理所有源的pc相关信息
};
                         


idun切主   新上线一个set












################################################
hela 构建pc task

task_list_ 是存放 指向HelaTask的自能指针 列表

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

  std::string src_vdisk_id_;
  uint32_t src_version_;
  VDiskType src_vdisk_type_;
  uint64_t shard_size_;
  KoalaRouteMap koala_routers_;

  bool need_repair_;
  PcRouteMap repair_pc_routers_;
  bool repair_job_;
};


############ hela 构建 pc_task #########
第1.  hela_task_login.cc 279 BuildNormalHelaTask()

manager_handle_->AddHelaTask(hela_task);

第2.  void ManagerHandle::AddHelaTask(const HelaTaskPtr& hela_task) {
  auto iter = hela_tasks_.find(hela_task->src_vdisk_id());

  if (iter == hela_tasks_.end()) {
    HelaTaskPtrList tasks;
    tasks.push_back(hela_task);

    hela_tasks_.insert(std::make_pair(hela_task->src_vdisk_id(), tasks));
    return;
  }

  iter->second.push_back(hela_task);
}
hela_tasks_ 是管理多种镜像源的 task任务


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
hela的pc task 获取

第3. 

manager_handle.cc  DispatchHelaTask(dispatch_list); 440 
  HelaTaskPtrList dispatch_list;
  uint32_t total_dispatch_num = 0;

// 这里下发task时是下发同一种镜像源的task 给hela  相同镜像源的盘的部分task(属于该hela所在ip的pc) $$$$$$$$$$$$$$$$$$$$$$$$$$$$$
  for (auto& it : hela_tasks_) {     // hela_tasks_  key:image_id  value:task
    const HelaTaskPtrList& task_list = it.second;
    uint32_t dispatch_num = 0;

    for (auto& iter : task_list) {
      const HelaTaskPtr& task = iter;

      if (total_dispatch_num >= total_task_dispatch_limit_num) {
        break;
      }

      if (dispatch_num >= once_dispatch_limit) {
        break;
      }

      if (task->status_ready()) {
        if (task->repair_job()) {
          LOG_INFO << "desc: dispatch repair job"
                   << " chrono_id(" << task->chrono_id() << ")"
                   << " vdisk_id(" << task->vdisk_id() << ")"
                   << " lc_id(" << task->lc_id() << ")"
                   << " src_vdisk_id(" << task->src_vdisk_id() << ")";
        }

        dispatch_list.push_back(task);
        dispatch_num++;
        total_dispatch_num++;
        if (dispatch_list.size() >= merge_threshold) {
          DispatchHelaTask(dispatch_list);  // 这里下发task时是下发同一种镜像源的task
          dispatch_list.clear();
        }
      } else if (task->status_executing()) {
        dispatch_num++;
      } else {
        continue;
      }
    }

    if (dispatch_list.size() > 0) {
      DispatchHelaTask(dispatch_list);  // 这里下发task时是下发同一种镜像源的task
      dispatch_list.clear();
    }
  }


manager_handle.cc  DispatchHelaTask(dispatch_list); 440 --> void DoHelaTaskHandle::Start(const HelaTaskPtrList& task_list) do_hela_task.cc 172
// 构建pc_task
// a. pc_no => pc_task
// b. 每个pc_task包含多个tiny_pc_task_info，对应多个目标盘
第4.  void DoHelaTaskHandle::ConstructHelaPcTask() {
  for (auto& it1 : task_list_) {
    const HelaTaskPtr& task = it1;
    const PcRouteMap& pc_route_map = it1->pc_routers();  //std::map<uint32_t, PcRouteList> PcRouteMap  // pc_no => PcRouteList

    for (auto& it2 : pc_route_map) {  // pc_no => PcRouteList
      uint32_t pc_no = it2.first;   //是pc_no 
      const PcRouteList& pc_route_list = it2.second;  // PcRouteList 

      for (auto& it3 : pc_route_list) {   // it3 是 pc_route 
           /*//每个pc的路由
    message PcRoute {
    required uint32 pc_no = 1;
    required uint32 pg_id = 2;
    repeated ChunkRoute copies = 3;            //每个pg包含至多三个副本
         }   
           */
        const ucloud::utimemachine::PcRoute& pc_route = it3;
        TinyPcTaskInfoPtr info =
            HelaHelper::ConstructTinyPcTaskInfo(task, pc_route);
                                                // pc_no => HelaPcTaskPtr
        auto it4 = pc_task_map_.find(pc_no);   //pc_task_map_ 是类型 typedef std::unordered_map<uint32_t, HelaPcTaskPtr> HelaPcTaskPtrMap; 
        if (it4 == pc_task_map_.end()) {
          HelaPcTaskPtr pc_task = HelaHelper::ConstructPcTask(pc_no, task);

          pc_task->info_list().push_back(info);   // info_list().push_back(info) 是clone的盘的一些信息，每一个pc都要存放    info 应该是fryer带来的 $$$$$$$$$
          pc_task_map_.insert(std::make_pair(pc_no, pc_task));  // pc_no HelaPcTaskPtr
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

########################################################


TinyPcTaskInfoPtr HelaHelper::ConstructTinyPcTaskInfo(
    const HelaTaskPtr& task, const ucloud::utimemachine::PcRoute& pc_route) {
  TinyPcTaskInfoPtr info =
      TinyPcTaskInfoPtr(new TinyPcTaskInfo(task->chrono_id()));

  info->set_vdisk_id(task->vdisk_id());
  info->set_version(task->version());
  info->set_vdisk_type(task->vdisk_type());
  info->set_lc_id(task->lc_id());
  info->set_lc_random_id(task->lc_random_id());
  info->set_pc_router(pc_route);

  uint32_t chunk_id = pc_route.copies(0).id();
  const ChunkInfoMap& chunks = g_context->manager_handle()->ChunkInfo();
  if (IsChunkOffline(chunk_id, chunks)) {
    info->set_offline(true);
  }

  return info;
}




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




HelaPcTaskPtr HelaHelper::ConstructPcTask(uint32_t pc_no,
                                          const HelaTaskPtr& task) {
  HelaPcTaskPtr pc_task = HelaPcTaskPtr(new HelaPcTask());

  pc_task->set_pc(pc_no);
  pc_task->set_src_vdisk_id(task->src_vdisk_id());
  pc_task->set_lc_size(task->lc_size());
  pc_task->set_pc_size(task->pc_size());
  pc_task->set_shard_size(task->shard_size());
  pc_task->set_cluster_version(task->cluster_version());
  pc_task->set_src_version(task->src_version());
  pc_task->set_vdisk_type(task->vdisk_type());

  pc_task->set_status_ready();
  pc_task->set_src_koala();

  ConstructPcKoalaRouter(pc_task, task);

  return pc_task;
}



void HelaHelper::ConstructPcKoalaRouter(HelaPcTaskPtr& pc_task,
                                        const HelaTaskPtr& task) {
  bool pc_larger = false;
  uint32_t shards_per_pc = 0;
  uint32_t pcs_per_shard = 0;
  uint32_t pc_size = task->pc_size();
  uint32_t shard_size = task->shard_size();

  //计算pc和shard的映射关系
  if (pc_size > shard_size) {
    pc_larger = true;
    shards_per_pc = pc_size / shard_size;
    pcs_per_shard = 1;
  } else {
    pc_larger = false;
    shards_per_pc = 1;
    pcs_per_shard = shard_size / pc_size;
  }

  KoalaRouteMap koala_routers;
  if (pc_larger) {
    int start_index = pc_task->pc() * shards_per_pc;
    int end_index = start_index + shards_per_pc;

    bool has_data = false;
    for (int idx = start_index; idx < end_index; idx++) {
      ucloud::utimemachine::ShardChronoRouter& shard_chrono_router =
          task->koala_routers()[idx];

      if (shard_chrono_router.has_data()) has_data = true;

      koala_routers.insert(std::make_pair(idx, shard_chrono_router));
    }

    pc_task->set_has_data(has_data);
  } else {
    int idx = pc_task->pc() / pcs_per_shard;
    ucloud::utimemachine::ShardChronoRouter& shard_chrono_router =
        task->koala_routers()[idx];

    pc_task->set_has_data(shard_chrono_router.has_data());

    koala_routers.insert(std::make_pair(idx, shard_chrono_router));
  }

  pc_task->set_koala_routers(koala_routers);
}







clone 盘的信息来源

hela hela_task_login.cc 235   该信息是由fryer 下发clone任务带来的
void HelaTaskLoginHandle::BuildNormalHelaTask() {
  HelaTaskPtr hela_task = HelaTaskPtr(new HelaTask(req_.chrono_id()));

  //初始化task状态为ready
  hela_task->set_status_ready();

  //目标盘信息
  hela_task->set_vdisk_id(req_.vdisk_id());
  hela_task->set_version(req_.version());
  hela_task->set_vdisk_type(req_.vdisk_type());
  hela_task->set_lc_id(req_.lc_id());
  hela_task->set_lc_random_id(req_.lc_random_id());
  hela_task->set_lc_size(req_.lc_size());
  hela_task->set_pc_size(req_.pc_size());
  hela_task->set_cluster_version(manager_handle_->ClusterVersion());