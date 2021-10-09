
// set chunk_id

// pc_list: disk_id1 set1 ip chunk1 、disk_id6 set1 ip chunk2、disk_id2 set2 ip chunk1
set
1. 

for (pc_list){
   resourse->LockNowTick();
   chunk = map<"ip_chunk_id">;
   chunk.get_use_count();
   ip_use_count =  map<"ip">;
   ip.get_use_count();
}

map<"1_29">

2.   //可能批量创建的盘，一个set里面的pc 分布在一个set的所有chunk上面，导致chunk 过多，增加遍历时间
for (pc_list){

    chunk_id = pc_list.chunk_id
    chunk_ip = pc_list.chunk_ip
    set_id = pc_list.set_id
    online_slices[pc->chunk_id_ip].push_back(slice);
    // 将在线的chunk放入，不在线的chunk可以直接移除map
    // 到map里面查询是否有对应的chunk，如果没有就添加，如果有就获取所有的chunk，获取按取出次数排序的chunk id集合以及 chunk id use_count集合
    
    // 到map里面查询是否有对应的chunk

}
for(pc_list){

    map<"chunk_id_ip",ResourceSliceList>  online_slices
    PcRoute *pc = slice->pc_route();
      if (online_chunk_ids.find(pc->chunk_id) != online_chunk_ids.end()) {
        online_slices[pc->chunk_id].push_back(slice);  // 将对应的pc_源分配给对应的chunk

      }
}
//按照使用次数遍历chunk
for(auto &chunk_use_count : online_chunks){
// 判断该chunk上是否有可以作为源的该分片以及判断是否繁忙
if (online_slices.find(chunk_id) != online_slices.end()) {
    // 选择最新的那个pc源
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
}




}



3. 

for(metaserver){
    online_chunks.insert(
            std::make_tuple(chunk.id, host_value, chunk_value));
        online_chunk_ids.insert(chunk.id);

}

for(pc_list){
    online_slices[pc->chunk_id].push_back(slice);
}

for (auto &chunk_use_count : online_chunks) {

uint32_t chunk_id = std::get<0>(chunk_use_count);

if (online_slices.find(chunk_id) != online_slices.end()) {


}
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

}




1    2    3    4    5    6    7    8    9    10

12  34   

// SourceManager管理源的类

class SourceManager {



   private: 
   std::map<std::string, std::pair<uint32_t, uint32_t> > chunk_use_count_per_second; 
   std::map<std::string, std::pair<uint32_t, uint32_t> > host_use_count_per_second;

}




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













