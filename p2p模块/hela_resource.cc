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

  PcRouteMap pcRouters;
  for (int i = 0; i < req_.pc_routes_size(); i++) {
    const ucloud::utimemachine::PcRoute& pc_route = req_.pc_routes(i);

    if (IsPcCompleted(pc_route)) {
      LOG_INFO << "desc: HelaTaskLoginHandle pc is already completed"
               << " chrono_id(" << req_.chrono_id() << ")"
               << " vdisk_id(" << req_.vdisk_id() << ")"
               << " pc(" << pc_route.pc_no() << ")";

      UpdateResource(pc_route);
      continue;
    }

    //在测试环境上，不同的pc副本会落在一台机器上
    HelaHelper::InsertPcRouteMap(pc_route, pcRouters);
  }
  hela_task->set_pc_routers(pcRouters);

  //源盘信息
  hela_task->set_src_vdisk_id(req_.src_vdisk_id());
  hela_task->set_src_version(req_.src_version());
  hela_task->set_src_vdisk_type(req_.src_vdisk_type());
  hela_task->set_shard_size(req_.shard_size());

  AddKoalaRoute(hela_task);

  // hela_task入队列
  manager_handle_->AddHelaTask(hela_task);

  DumpHelaTask(hela_task);

  Finish(VDISK_RET_SUCCESS, "HelaTaskLogin succ");
}



void HelaTaskLoginHandle::UpdateResource(
    const ucloud::utimemachine::PcRoute& pc_route) {
  ResourceManager* resource_manager = manager_handle_->LocalResourceManager();
  assert(resource_manager != nullptr);

  PCResourcePtr pc_resource = PCResourcePtr(new PCResource());
  pc_resource->src_vdisk_id = req_.vdisk_id();

  pc_resource->lc_id = req_.lc_id();
  pc_resource->lc_random_id = req_.lc_random_id();
  pc_resource->lc_size = req_.lc_size();
  pc_resource->pc_size = req_.pc_size();
  pc_resource->cluster_version = manager_handle_->ClusterVersion();

  pc_resource->pc_no = pc_route.pc_no();
  pc_resource->pg_id = pc_route.pg_id();
  pc_resource->chunk_id = pc_route.copies(0).id();
  pc_resource->chunk_ip = pc_route.copies(0).ip();
  pc_resource->chunk_port = pc_route.copies(0).port();

  pc_resource->valid = true;

  resource_manager->AddResource(pc_resource);
}



void ResourceManager::AddResource(const PCResourcePtr& pc_resource) {
  std::string src_vdisk_id = pc_resource->src_vdisk_id;
  uint32_t pc_no = pc_resource->pc_no;
  uint32_t lc_id = pc_resource->lc_id;

  ResourceKey key = std::make_pair(src_vdisk_id, pc_no);
  LOG_DEBUG << "add pc resource,"
            << " src_vdisk_id: " << key.first << " pc_no: " << key.second;

  auto it = resource_map_.find(key);
  if (it == resource_map_.end()) {
    PCResourceMap pc_resource_map;
    pc_resource_map.insert(std::make_pair(lc_id, pc_resource));
    resource_map_.insert(std::make_pair(key, pc_resource_map));
  } else {
    PCResourceMap& pc_resource_map = it->second;
    auto iter = pc_resource_map.find(lc_id);
    if (iter == pc_resource_map.end()) {
      pc_resource_map.insert(std::make_pair(lc_id, pc_resource));
    }
  }
}



