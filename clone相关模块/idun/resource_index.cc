
class ResourceIndex {
 public:
  ResourceIndex();
  ~ResourceIndex();

  void AddResource(Resource *resource);
  void RemoveResource(Resource *resource);
  bool ResourceIsFinish(Resource *resource);
  bool FinishResource(Resource *resource);

  bool AddSlice(const ResourceSlicePtr &slice);
  void RemoveSlice(const ResourceSlicePtr &slice);

  // ResourceSlicePtr GetResourceSlice(uint32_t idx);

  uint32_t ref_count() { return ref_count_; }
  uint32_t inc_ref_count() { return ++ref_count_; }
  uint32_t dec_ref_count() { return --ref_count_; }

  std::map<uint32_t, ResourceSliceSet> &slice_index() { return slice_index_; }

  std::list<Resource *> &unfinish_resources() { return unfinish_resources_; }
  std::list<Resource *> &finish_resources() { return finish_resources_; }

 private:
  void InsertFinishResource(Resource *resource);

  std::map<uint32_t, ResourceSliceSet> slice_index_;

  std::list<Resource *> unfinish_resources_;  // 未完成的资源,
  // 按创建时间排序(不保证)
  std::list<Resource *> finish_resources_;  // 已完成的资源, 按完成时间排序

  uint32_t ref_count_;  // 被资源引用的计数, = unfinish + finish
};




ResourceIndex::ResourceIndex() : ref_count_(0) {}

ResourceIndex::~ResourceIndex() {}

void ResourceIndex::InsertFinishResource(Resource *resource) {
  // 从队列末尾开始遍历，找到插入位置
  std::list<Resource *>::reverse_iterator ritr;
  for (ritr = finish_resources_.rbegin(); ritr != finish_resources_.rend();
       ++ritr)
    if ((*ritr)->finish_tick() < resource->finish_tick()) break;

  std::list<Resource *>::iterator pos = ritr.base();
  finish_resources_.insert(pos, resource);
}
