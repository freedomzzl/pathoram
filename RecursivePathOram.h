#ifndef RECURSIVE_PATH_ORAM_H
#define RECURSIVE_PATH_ORAM_H

#include "OramInterface.h"
#include "ServerStorage.h"
#include "RandomForOram.h"
#include"Bucket.h"
#include"block.h"
#include <vector>
#include <unordered_map>
#include <cmath>

class RecursivePathOram : public OramInterface {
public:
    RecursivePathOram(ServerStorage* storage, RandomForOram* rand_gen, int bucket_size, int num_blocks, int group_size,int recursive_levels);
    ~RecursivePathOram();

    
    std::vector<char> recursiveaccess(Operation op, int blockIndex, const std::vector<char>& newData,int level) ;


    ServerStorage* storage; // 不可信存储
    RandomForOram* rand_gen; // 随机数生成器

    int bucket_size; // 每个桶的大小
    int num_blocks; // 数据块的数量
    int group_size; // 每组 position map 的大小

    int recursive_levels;//递归层数
    int num_levels; // ORAM 树的层数
    int num_leaves; // 叶子节点的数量
    int num_buckets; // 桶的数量
    int total_blocks;

    vector<Block> stash;
    vector<int> position_map; 
    vector<int> top_level_position_map; // 最顶端的 position_map

    vector<int> record_OldLeaf;//记录访问的块曾经的叶子结点

    int P(int leaf, int level);//计算指定叶子节点和层数的路径。

    int getLayerBlockID(int dataBlockID, int N, int B,  int targetLayer);//计算目标层对应的块ID
    int getPositionMapIndex(int dataBlockID, int B, int targetLayer);// 数据块x 在第i层块中的局部索引。
   

    vector<char> serializePositionMap(const vector<int>& positionMap);//把vector<int>转成vector<char>
    vector<int> deserializePositionMap(const std::vector<char>& data);//把vector<char>转成vector<int>
    void initializePositionMap();//初始化positionmap
    
    void storePositionMapInOramRecursive(const std::vector<int>& pmap_block_ids, int level);//把positionmap存入块
    
    int calculateTotalBlocks(int num_blocks, int group_size,int recursive_levels);//数据块+存储positionmap的块的数量
    void StashClear();
};

#endif // RECURSIVE_PATH_ORAM_H