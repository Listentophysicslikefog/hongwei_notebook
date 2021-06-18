
1. 

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
  hela_task->set_src_vdisk_id(req_.src_vdisk_id());   //镜像源id
  hela_task->set_src_version(req_.src_version());
  hela_task->set_src_vdisk_type(req_.src_vdisk_type());
  hela_task->set_shard_size(req_.shard_size());

  AddKoalaRoute(hela_task);

  // hela_task入队列
  manager_handle_->AddHelaTask(hela_task);

2. 

  void ManagerHandle::AddHelaTask(const HelaTaskPtr& hela_task) {
  auto iter = hela_tasks_.find(hela_task->src_vdisk_id());

  if (iter == hela_tasks_.end()) {
    HelaTaskPtrList tasks;
    tasks.push_back(hela_task);

    hela_tasks_.insert(std::make_pair(hela_task->src_vdisk_id(), tasks));   //hela_tasks_ 有多种镜像源的task
  }

  iter->second.push_back(hela_task);
}

3.   hela_tasks_  //根据不同的镜像源来存放clone的task任务   key: 镜像源id   value: clone task

  
HelaTaskPtrList dispatch_list;
  uint32_t total_dispatch_num = 0;

  for (auto& it : hela_tasks_) {   // it 是其中一种镜像源的task任务
    const HelaTaskPtrList& task_list = it.second;  // it这种镜像源的 所有task任务
    uint32_t dispatch_num = 0;

    for (auto& iter : task_list) {   // iter 是it这种镜像源的其中的一个task任务
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

        dispatch_list.push_back(task);  //task 是相同镜像源的task 
        dispatch_num++;
        total_dispatch_num++;
        if (dispatch_list.size() >= merge_threshold) {
          DispatchHelaTask(dispatch_list);
          dispatch_list.clear();
        }
      } else if (task->status_executing()) {
        dispatch_num++;
      } else {
        continue;
      }
    }

    if (dispatch_list.size() > 0) {
      DispatchHelaTask(dispatch_list);
      dispatch_list.clear();
    }
  }
}

4. 

void DoHelaTaskHandle::Start(const HelaTaskPtrList& task_list) {
  task_list_ = task_list;

  void DoHelaTaskHandle::ConstructHelaPcTask() {
  for (auto& it1 : task_list_) {   //相同镜像源的一个task
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
