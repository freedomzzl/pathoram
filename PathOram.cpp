#include "PathOram.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;
OramReadPathEviction::OramReadPathEviction(ServerStorage* storage, RandomForOram* rand_gen,
    int bucket_size, int num_blocks) {
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


vector<char> OramReadPathEviction::access(Operation op, int blockIndex, const vector<char>& newdata) {

    vector<char> data;

    // 1-2: 获取旧的叶子节点并更新位置映射
    int oldLeaf = position_map[blockIndex];
    position_map[blockIndex] = rand_gen->getRandomLeaf();

    //// 打印旧的叶子节点和新的叶子节点
    //cout << "Old Leaf: " << oldLeaf << ", New Leaf: " << position_map[blockIndex] << endl;

    // 3-5: 读取路径上的所有块到 stash
    for (int i = 0; i < num_levels; i++) {
        int path = P(oldLeaf, i);
        
        vector<Block> blocks = storage->ReadBucket(path).getBlocks();
        for (Block b : blocks) {
            if (b.index != -1) { // 忽略虚拟块
                stash.push_back(b);
            }
        }
    }

    
   

    // 6: 在 stash 中查找目标块
    Block* targetBlock = nullptr;
    for (auto& block : stash) {
        if (block.index == blockIndex) {
            targetBlock = &block;
            break;
        }
    }

    // 7-9: 处理读/写操作
    if (op == Operation::WRITE) {
        if (targetBlock == nullptr) {
            // 如果目标块不存在，创建新块并添加到 stash
            Block newBlock(position_map[blockIndex], blockIndex, newdata);
            stash.push_back(newBlock);
            
        }
        else {
            // 如果目标块存在，更新数据
            targetBlock->data = newdata;
            
            targetBlock->printBlock();
        }
    }//写模式
    else {
        if (targetBlock != nullptr) {
            // 如果目标块存在，返回数据
            data = targetBlock->data;
            
            
        }
        else {
            data.clear();
        }
    }//读模式

    // 10-15: 逐层写回路径
    for (int l = num_levels - 1; l >= 0; l--) {
        vector<int> bid_evicted;
        Bucket bucket;
        int Pxl = P(oldLeaf, l);
        int counter = 0;

        

        // 将 stash 中的块写回路径
        for (auto it = stash.begin(); it != stash.end();) {
            if (counter >= bucket_size) {
                break;
            }
            if (Pxl == P(position_map[it->index], l)) {
                bucket.addBlock(*it);
                bid_evicted.push_back(it->index);
                it = stash.erase(it); // 从 stash 中移除
                counter++;
            }
            else {
                ++it;
            }
        }

        // 填充虚拟块
        while (counter < bucket_size) {
            bucket.addBlock(Block()); // 虚拟块
            counter++;
        }

        // 将桶写回存储
        storage->WriteBucket(Pxl, bucket);

        
    }

    

    return data;
}

//计算指定叶子节点和层数的路径,返回存储系统中对应桶的位置
int OramReadPathEviction::P(int leaf, int level) {
    /*
    * This function should be deterministic.
    * INPUT: leaf in range 0 to num_leaves - 1, level in range 0 to num_levels - 1.
    * OUTPUT: Returns the location in the storage of the bucket which is at the input level and leaf.
    */
    return (1 << level) - 1 + (leaf >> (this->num_levels - level - 1));
}


int* OramReadPathEviction::getPositionMap() {
    return this->position_map;
}

vector<Block> OramReadPathEviction::getStash() {
    return this->stash;
}

int OramReadPathEviction::getStashSize() {
    return (this->stash).size();
}

int OramReadPathEviction::getNumLeaves() {
    return this->num_leaves;

}

int OramReadPathEviction::getNumLevels() {
    return this->num_levels;

}

int OramReadPathEviction::getNumBlocks() {
    return this->num_blocks;

}

int OramReadPathEviction::getNumBuckets() {
    return this->num_buckets;

}

OramReadPathEviction::~OramReadPathEviction() {
    delete[] position_map;
}

void OramReadPathEviction::StashClear()
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
