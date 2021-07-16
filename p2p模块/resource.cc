Resource::Resource(const std::string &vdisk_id, uint32_t version,
                   uint32_t vdisk_type)
    : vdisk_id_(vdisk_id),
      version_(version),
      vdisk_type_(vdisk_type),
      src_version_(0),
      src_vdisk_type_(0),
      create_tick_(time(nullptr)),
      finish_tick_(0),
      status_(RESOURCE_STATUS_NORMAL) {
  if (IsUdisk()) {
    info_.lc = new LCInfo;
  }
}




class ResourceSlice {
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

struct ResourceSlicePtrComp {
  bool operator()(const ResourceSlicePtr &lhs,
                  const ResourceSlicePtr &rhs) const {
    return *lhs.get() < *rhs.get();
  }
};
typedef std::set<ResourceSlicePtr, ResourceSlicePtrComp> ResourceSliceSet;

// ==================== Resource ==================== //

enum ResourceStatus {
  RESOURCE_STATUS_NORMAL = 0,
  RESOURCE_STATUS_NEED_RECOVER = 1,  // 待恢复(重启时需要从db恢复分片数据)
  RESOURCE_STATUS_RECOVERING = 2,  // 恢复中
};

class Resource {
  friend class ResourceSlice;

 public:
  Resource(const std::string &vdisk_id, uint32_t version, uint32_t vdisk_type);
  ~Resource();

  ResourceKey GetKey() const {
    return std::make_tuple(vdisk_id_, version_, vdisk_type_);
  }
  ResourceIndexKey GetIndexKey() const {
    return std::make_tuple(src_vdisk_id_, src_version_, src_vdisk_type_,
                           vdisk_type_);
  }

  const std::string &vdisk_id() const { return vdisk_id_; }
  uint32_t version() const { return version_; }
  uint32_t vdisk_type() const { return vdisk_type_; }

  const std::string &src_vdisk_id() const { return src_vdisk_id_; }
  void set_src_vdisk_id(const std::string &vdisk_id) {
    src_vdisk_id_ = vdisk_id;
  }

  uint32_t src_version() const { return src_version_; }
  void set_src_version(uint32_t version) { src_version_ = version; }

  uint32_t src_vdisk_type() const { return src_vdisk_type_; }
  void set_src_vdisk_type(uint32_t type) { src_vdisk_type_ = type; }

  bool IsUdisk() const;

  LCInfo *lc_info() {
    assert(IsUdisk());
    return info_.lc;
  }

  // 根据源创建分片
  ResourceSlicePtr CreateSlice();

  bool AddSlice(const ResourceSlicePtr &slice);
  void RemoveSlice(const ResourceSlicePtr &slice);
  ResourceSliceSet &slices() { return slices_; }

  bool AddTmpSlice(const ResourceSlicePtr &slice);
  void RemoveTmpSlice(const ResourceSlicePtr &slice);
  ResourceSliceSet &tmp_slices() { return tmp_slices_; }

  void set_create_tick(uint32_t tick) { create_tick_ = tick; }
  uint32_t create_tick() const { return create_tick_; }

  void set_finish_tick(uint32_t tick) { finish_tick_ = tick; }
  uint32_t finish_tick() const { return finish_tick_; }
  bool is_finish() const { return finish_tick_ > 0; }

  void set_status(ResourceStatus status) { status_ = status; }
  ResourceStatus status() const { return status_; }

 private:
  // 资源盘标志
  std::string vdisk_id_;
  uint32_t version_;
  uint32_t vdisk_type_;

  // 数据源盘标志
  std::string src_vdisk_id_;
  uint32_t src_version_;
  uint32_t src_vdisk_type_;

  // 资源盘信息
  union {
    LCInfo *lc;
  } info_;

  ResourceSliceSet slices_;  // 所有分片集合
  ResourceSliceSet tmp_slices_;  // 待保存分片集合, 未保存到db的分片会同时存在于
                                 // slices_ 和 tmp_slices_

  uint32_t create_tick_;
  uint32_t finish_tick_;  // 完成的时间

  ResourceStatus status_;
};