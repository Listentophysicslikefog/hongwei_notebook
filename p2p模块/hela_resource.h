// Copyright (c) 2020 UCloud All rights reserved.
#ifndef UDISK_HELA_RESOURCE_MANAGER_H
#define UDISK_HELA_RESOURCE_MANAGER_H

#include "hela_dec.h"
#include "message_util.h"
#include "umessage_common.h"
#include "pb_request_handle.h"
#include "uevent.h"
#include <memory>

namespace udisk {
namespace hela {

// ==================== ResourceKey ==================== //

typedef std::pair<std::string, uint32_t> ResourceKey;  // src_vdisk_id, pc
struct ResourceKeyHash {
  std::size_t operator()(const ResourceKey& item) const {
    std::size_t h1 = std::hash<std::string>()(item.first);
    std::size_t h2 = std::hash<uint32_t>()(item.second);
    return h1 ^ (h2 << 1);
  }
};

// ==================== PCResource ==================== //

struct PCResource {
  PCResource()
      : lc_id(0),
        lc_random_id(0),
        lc_size(0),
        pc_size(0),
        cluster_version(0),
        pc_no(0),
        pg_id(0),
        chunk_id(0),
        chunk_port(0),
        ref_count(0),
        create_tick(time(NULL)),
        valid(false) {}

  std::string src_vdisk_id;

  // LCInfo
  uint32_t lc_id;
  uint32_t lc_random_id;
  uint32_t lc_size;
  uint64_t pc_size;
  uint64_t cluster_version;

  // PCRoute
  uint32_t pc_no;
  uint32_t pg_id;
  uint32_t chunk_id;
  std::string chunk_ip;
  uint32_t chunk_port;

  // 源被使用的次数
  uint32_t ref_count;
  uint32_t create_tick;
  bool valid;
};

// ==================== Typedef ==================== //

typedef std::shared_ptr<PCResource> PCResourcePtr;
typedef std::list<PCResourcePtr> PCResourcePtrList;

// lc_id => pc_resource
typedef std::map<uint32_t, PCResourcePtr> PCResourceMap;

typedef std::unordered_map<ResourceKey, PCResourceMap, ResourceKeyHash>
    ResourceMap;
typedef std::map<uint32_t, uint32_t> ResourceRefMap;  // lc_id =>
                                                      // resource_reference
typedef std::map<std::string, std::set<uint32_t>> ActiveMap;  // src_vdisk_id =>
                                                              // pc_no set

// ==================== ResourceManager ==================== //

class ResourceManager {
 public:
  ResourceManager();
  ~ResourceManager();

  void DebugResource();
  PCResourcePtr GetResourceNew(const ResourceKey& key);
  const PCResourcePtr& GetResource(const ResourceKey& key);
  void AddResource(const PCResourcePtr& resource);
  void AddResourceList(const PCResourcePtrList& resource_list);
  void ExpireResource(uint32_t lc_id);
  void RemoveResource(uint32_t lc_id);

  bool ResourceBusy(uint32_t lc_id);
  void IncResourceRef(uint32_t lc_id);
  void DecResourceRef(uint32_t lc_id);
  void RemoveResourceRef(uint32_t lc_id);
  uint32_t ResourceRef(uint32_t lc_id);

  bool IsActive(std::string src_vdisk_id, uint32_t pc_no);
  void InsertActive(std::string src_vdisk_id, uint32_t pc_no);
  void RemoveActive(std::string src_vdisk_id, uint32_t pc_no);

  void HandleClusterChange(uint64_t cluster_version,
                           const ChunkInfoMap& chunks);

 private:
  ResourceMap resource_map_;
  ResourceRefMap resource_ref_map_;
  ActiveMap active_map_;
  uint32_t resource_expiration_;
};

};  // namespace hela
};  // namespace udisk

#endif
