typedef std::unordered_map<ResourceKey, Resource *, ResourceKeyHash>
    ResourceMap;  // 避免循环引用, Resource用裸指针

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

  ResourceMap resources_;    // 资源列表, vdisk => Resource
  ResourceIndexMap indexs_;  // 索引列表, src_vdisk => ResourceIndex
};


Resource *ResourceManager::GetResource(const ResourceKey &key) {
  auto itr = resources_.find(key);
  if (itr != resources_.end()) return itr->second;
  return nullptr;
}

bool ResourceManager::AddResource(Resource *resource) {
  ResourceKey key = resource->GetKey();

  // 检查是否已存在
  if (resources_.find(key) != resources_.end()) {
    LOG_ERROR << "resource key exists, key:" << ResourceKeyToString(key);
    return false;
  }

  if (!resources_.insert(std::make_pair(key, resource)).second) {
    LOG_ERROR << "insert resource to map error, key:"
              << ResourceKeyToString(key);
    return false;
  }

  if (!AddResourceIndex(resource)) {
    LOG_ERROR << "add resource index error, key:" << ResourceKeyToString(key);
    resources_.erase(key);
    return false;
  }

  return true;
}

void ResourceManager::RemoveResource(const ResourceKey &key) {
  Resource *resource = GetResource(key);
  if (resource == nullptr) {
    LOG_INFO << "remove resource but not found, key:"
             << ResourceKeyToString(key);
    return;
  }

  // 删除索引
  RemoveResourceIndex(resource);
  // 移除队列
  resources_.erase(key);
  // 释放资源
  delete resource;
}

bool ResourceManager::ResourceIsFinish(Resource *resource) {
  ResourceIndexPtr resource_index = GetResourceIndex(resource->GetIndexKey());
  if (resource_index == nullptr) {
    LOG_ERROR << "desc:critical resource index not exists, index_key:"
              << ResourceIndexKeyToString(resource->GetIndexKey());
    return false;
  }

  // 检查资源在索引的哪个列表里
  if (!resource_index->ResourceIsFinish(resource)) {
    assert(!resource->is_finish());
    return false;
  }

  assert(resource->is_finish());
  return true;
}

bool ResourceManager::FinishResource(Resource *resource, uint32_t finish_tick) {
  if (resource->is_finish()) {
    // TODO: 如果
    LOG_INFO << "finish resource but resource already finish, key:"
             << ResourceKeyToString(resource->GetKey())
             << ", finish_tick:" << resource->finish_tick();
    return true;
  }

  ResourceIndexPtr resource_index = GetResourceIndex(resource->GetIndexKey());
  if (resource_index == nullptr) {
    LOG_ERROR << "desc:critical finish resource but resource index not exists, "
                 "index_key:"
              << ResourceIndexKeyToString(resource->GetIndexKey());
    return false;
  }

  resource->set_finish_tick(finish_tick);

  if (!resource_index->FinishResource(resource)) {
    LOG_ERROR << "desc:finish resource in index error, key:"
              << ResourceKeyToString(resource->GetKey());
    resource->set_finish_tick(0);
    return false;
  }

  return true;
}


