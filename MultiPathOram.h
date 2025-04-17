#ifndef MULTI_PATH_ORAM_H
#define MULTI_PATH_ORAM_H

#include "OramInterface.h"
#include "ServerStorage.h"
#include "RandomForOram.h"
#include"Bucket.h"
#include"block.h"
#include <vector>
#include <unordered_map>
#include <cmath>


class MultiPathOram : public OramInterface
{
public:
    ServerStorage* storage; //指向一个不可信的存储系统接口，用于读写数据块。
    RandomForOram* rand_gen;//指向一个随机数生成器接口，用于生成随机的叶子节点索引

    int bucket_size;//每个桶可以存储的数据块数量
    int num_levels;//ORAM 树的层数
    int num_leaves;//ORAM 树的叶子节点数量
    int num_blocks;//ORAM 中存储的总数据块数量
    int num_buckets;//ORAM 中存储的总桶数量

    int* position_map; //array
    vector<Block> stash;

    MultiPathOram(ServerStorage* storage,
        RandomForOram* rand_gen, int bucket_size, int num_blocks);

    vector<vector<char>> multiaccess(vector<pair<Operation, int>>& operations, vector<vector<char>>& newdataList);//执行 ORAM 的访问操作（读取或写入），一次访问多个
    int P(int leaf, int level);//计算指定叶子节点和层数的路径。
    int* getPositionMap();//获取位置映射表。
    vector<Block> getStash();//获取本地存储的临时数据块集合（stash）。
    int getStashSize();
    int getNumLeaves();
    int getNumLevels();
    int getNumBlocks();
    int getNumBuckets();
    virtual ~MultiPathOram();
    void StashClear();
};

#endif //MULTI_PATH_ORAM_H