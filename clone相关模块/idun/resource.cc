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