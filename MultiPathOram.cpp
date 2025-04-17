#include "MultiPathOram.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <unordered_set>

using namespace std;

MultiPathOram::MultiPathOram(ServerStorage* storage, RandomForOram* rand_gen, int bucket_size, int num_blocks)
{
   
        this->storage = storage;
        this->rand_gen = rand_gen;
        this->bucket_size = bucket_size;
        this->num_blocks = num_blocks;

        //计算 ORAM 树的层数 num_levels 和总桶数量 num_buckets。
        this->num_levels = ceil(log10(num_blocks) / log10(2)) + 1;
        this->num_buckets = pow(2, num_levels) - 1;

        //检查存储系统的总容量是否足够存储所有数据块
        if (this->num_buckets * this->bucket_size < this->num_blocks) //deal with precision loss
        {
            throw new runtime_error("Not enough space for the acutal number of blocks.");
        }

        //计算叶子节点数量
        this->num_leaves = pow(2, num_levels - 1);

        //设置随机数生成器的范围
        this->rand_gen->setBound(num_leaves);

        //设置存储系统的容量
        this->storage->setCapacity(num_buckets);

        //分配位置映射表的内存空间。
        this->position_map = new int[this->num_blocks];
        this->stash = vector<Block>();

        //为每个数据块随机分配一个叶子节点索引
        for (int i = 0; i < this->num_blocks; i++) {
            position_map[i] = rand_gen->getRandomLeaf();
        }

        //初始化存储系统中的每个桶
        for (int i = 0; i < num_buckets; i++) {

            Bucket init_bkt = Bucket();
            for (int j = 0; j < bucket_size; j++) {
                init_bkt.addBlock(Block());
            }
            storage->WriteBucket(i, Bucket(init_bkt));
        }

}

vector<vector<char>> MultiPathOram::multiaccess(vector<pair<Operation, int>>& operations, vector<vector<char>>& newdataList)
{
    vector<vector<char>> results;
    results.reserve(operations.size());

    // 1. 收集所有需要访问的块的信息
    vector<int> blockIndices;
    vector<int> oldLeaves;
    vector<int> newLeaves;

    for (const auto& op : operations) {
        int blockIndex = op.second;
        blockIndices.push_back(blockIndex);
        oldLeaves.push_back(position_map[blockIndex]);
        newLeaves.push_back(rand_gen->getRandomLeaf());
    }
    
    // 2. 更新位置映射表
    for (size_t i = 0; i < blockIndices.size(); i++) {
        position_map[blockIndices[i]] = newLeaves[i];
    }

    // 3. 收集所有需要读取的路径（去重）
    unordered_set<int> pathsToRead;
    for (int oldLeaf : oldLeaves) {
        for (int l = 0; l < num_levels; l++) {
            pathsToRead.insert(P(oldLeaf, l));
        }
    }
    
    // 4. 读取所有路径上的块到stash
    for (int path : pathsToRead) {
        vector<Block> blocks = storage->ReadBucket(path).getBlocks();
        for (Block b : blocks) {
            if (b.index != -1) { // 忽略虚拟块
                stash.push_back(b);
            }
        }
    }
    
    // 5. 处理每个操作
    for (size_t i = 0; i < operations.size(); i++) {
        Operation op = operations[i].first;
        int blockIndex = blockIndices[i];
        const vector<char>& newdata = newdataList[i];

        // 在stash中查找目标块
        Block* targetBlock = nullptr;
        for (auto& block : stash) {
            if (block.index == blockIndex) {
                targetBlock = &block;
                
                break;
            }
        }
        
        // 处理读/写操作
        if (op == Operation::WRITE) {
            if (targetBlock == nullptr) {
                
                // 如果目标块不存在，创建新块并添加到stash
                Block newBlock(newLeaves[i], blockIndex, newdata);
                stash.push_back(newBlock);
            }
            else {
                // 如果目标块存在，更新数据
                targetBlock->data = newdata;
                targetBlock->leaf_id = newLeaves[i];
                
            }
            results.push_back(vector<char>()); // 写操作不返回数据
        }
        else {
            if (targetBlock != nullptr) {
                // 如果目标块存在，返回数据
                results.push_back(targetBlock->data);
            }
            else {
                cout << "hi" << endl;
                results.push_back(vector<char>()); // 返回空数据
            }
        }
    }
    
    // 6. 收集所有需要写回的路径（去重）
    unordered_set<int> pathsToWrite;
    for (int oldLeaf : oldLeaves) {
        for (int l = 0; l < num_levels; l++) {
            pathsToWrite.insert(P(oldLeaf, l));
        }
    }
    
    // 7. 逐层写回所有路径
    for (int path : pathsToWrite) {

        vector<int> bid_evicted;
        Bucket bucket;
        int counter = 0;
        for (int l = num_levels - 1; l >= 0; l--) {
            

            

            // 将stash中的块写回路径
            for (auto it = stash.begin(); it != stash.end();) {
                if (counter >= bucket_size) {
                    break;
                }
                if (path == P(position_map[it->index], l)) {
                    bucket.addBlock(*it);
                    bid_evicted.push_back(it->index);
                    it = stash.erase(it); // 从stash中移除
                    counter++;
                }
                else {
                    ++it;
                }
            }
        }
        // 填充虚拟块
        while (counter < bucket_size) {
            bucket.addBlock(Block()); // 虚拟块
            counter++;
        }

        // 将桶写回存储
        storage->WriteBucket(path, bucket);
    }
    cout << "stash size: " << stash.size() << endl;
    return results;
}

int MultiPathOram::P(int leaf, int level)
{
    return (1 << level) - 1 + (leaf >> (this->num_levels - level - 1));
}

int* MultiPathOram::getPositionMap()
{
    return this->position_map;
}

vector<Block> MultiPathOram::getStash()
{
    return this->stash;
}

int MultiPathOram::getStashSize()
{
    return (this->stash).size();
}

int MultiPathOram::getNumLeaves()
{
    return this->num_leaves;
}

int MultiPathOram::getNumLevels()
{
    return this->num_levels;
}

int MultiPathOram::getNumBlocks()
{
    return this->num_blocks;
}

int MultiPathOram::getNumBuckets()
{
    return this->num_buckets;
}

MultiPathOram::~MultiPathOram()
{
    delete[] position_map;
}

void MultiPathOram::StashClear()
{
    for (auto it = stash.begin(); it != stash.end();)
    {


        for (int l = num_levels - 1; l >= 0; l--)
        {

            int path, count = 0;
            Bucket bucket;
            path = P(it->leaf_id, l);
            vector<Block> blocks = storage->ReadBucket(path).getBlocks();
            for (auto b = blocks.begin(); b != blocks.end();)
            {
                if (b->index != -1)
                {
                    bucket.addBlock(*b);
                    count++;
                }
                ++b;
            }

            if (count < bucket_size)
            {
                bucket.addBlock(*it);
                count++;

                while (count < bucket_size)
                {
                    bucket.addBlock(Block()); // 虚拟块
                    count++;
                }

                // 将桶写回存储
                storage->WriteBucket(path, bucket);
                it = stash.erase(it);
                break;
            }

            else if (l == 0)
            {
                // 所有层级都尝试过了，无法放置
                ++it;
            }
        }
    }
}

