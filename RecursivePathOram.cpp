#include "RecursivePathOram.h"
#include"block.h"
#include <iostream>
#include <cmath>
#include<algorithm>

using namespace std;
RecursivePathOram::RecursivePathOram(ServerStorage* storage, RandomForOram* rand_gen, int bucket_size, int num_blocks,int group_size,int recursive_levels)
    : storage(storage), rand_gen(rand_gen), bucket_size(bucket_size), num_blocks(num_blocks), group_size(group_size),recursive_levels(recursive_levels) {

    // 计算 ORAM 树的层数和叶子节点数
    
    total_blocks = calculateTotalBlocks(num_blocks, group_size,recursive_levels);
    num_levels = std::ceil(std::log2(total_blocks)) + 1;
    num_leaves = std::pow(2, num_levels - 1);
    num_buckets = std::pow(2, num_levels) - 1;

    //检查存储系统的总容量是否足够存储所有数据块
    if (this->bucket_size * this->num_buckets < total_blocks)
    {
        throw new runtime_error("Not enough space for the acutal number of blocks.");
    }

    //设置随机数生成器的范围
    this->rand_gen->setBound(num_leaves);

    //设置存储系统的容量
    this->storage->setCapacity(num_buckets);
    
    //初始化stash
    this->stash = vector<Block>();

    //初始化存储系统中的每个桶
    for (int i = 0; i < num_buckets; i++) {

        Bucket init_bkt = Bucket();
        for (int j = 0; j < bucket_size; j++) {
            init_bkt.addBlock(Block());
        }
        storage->WriteBucket(i, Bucket(init_bkt));
    }

    //初始化positionmap
    initializePositionMap();
}

RecursivePathOram::~RecursivePathOram() {
    // 清理资源
}


//递归的access
std::vector<char> RecursivePathOram::recursiveaccess(Operation op, int blockIndex, const std::vector<char>& newData, int level) {
    std::cout << "Debug: level = " << level << ", blockIndex = " << blockIndex << std::endl;

    //判断blockindex是否处于正常范围内
    if (blockIndex < 0 || blockIndex >= total_blocks) {
        std::cerr << "Error: blockIndex out of range! (" << blockIndex << ")" <<total_blocks<< std::endl;
        return {};
    }
    
    std::vector<char> result, data;
    std::vector<int> map;
    std::vector<Block> blocks;
    int old_leaf = -1, new_leaf = -1;

    
    
    

    // 递归访问下一层
    if (level < recursive_levels) {
        
        int next_id = getLayerBlockID(blockIndex, num_blocks, group_size, level);//计算next_id
        
        result = recursiveaccess(READ, next_id, {}, level + 1);//递归access
        
        if (result.empty()) {
            std::cerr << "Error: Failed to read next level block!" << std::endl;
            return {};

        }

        //获取positionmap
        map = deserializePositionMap(result);
        if (map.empty()) {
            std::cerr << "Error: Failed to deserialize position map!" << std::endl;
            return {};
        }
       
        old_leaf = map[getPositionMapIndex(blockIndex, group_size, level)];//计算oldleaf
        record_OldLeaf.push_back(old_leaf);
        
        map[getPositionMapIndex(blockIndex, group_size, level)] = rand_gen->getRandomLeaf();//映射新的叶子结点
        new_leaf = map[getPositionMapIndex(blockIndex, group_size, level)];
        
        //把更改后的positionmap存入块
        vector<char> char_map = serializePositionMap(map);
        for (auto& block : stash) {


            if (block.index == next_id) {
                block.data = char_map;
                
                break;
            }
        }

    }
    else if (level == recursive_levels) {
        old_leaf = top_level_position_map[getPositionMapIndex(blockIndex, group_size, level)];//计算oldleaf
        record_OldLeaf.push_back(old_leaf);
        
        top_level_position_map[getPositionMapIndex(blockIndex, group_size, level)]= rand_gen->getRandomLeaf();
        new_leaf = top_level_position_map[getPositionMapIndex(blockIndex, group_size, level)];
        
    }
    else {
        std::cerr << "Error: Invalid level (" << level << ")!" << std::endl;
        return {};
    }

    std::cout << "blockindex: " << blockIndex << endl;
    
    // 读取路径上的所有块到 stash
    for (int i = 0; i < num_levels; i++) {
        
        int path = P(old_leaf, i);
        
        std::vector<Block> blocks = storage->ReadBucket(path).getBlocks();
        for (Block b : blocks) {
            auto it = std::find(stash.begin(), stash.end(), b);
            if (it.operator==(stash.end()) && b.index != -1)
            {
                
                stash.push_back(b);
            }
            
        }
    }
    
    // 在 stash 中查找目标块
    Block* targetBlock = nullptr;
    
    for (auto& block : stash) {

        
        if (block.index == blockIndex) {
            targetBlock = &block;
            std::cout << "find!" << endl;
            break;
        }
    }
    
    // 处理读/写操作
    if (op == Operation::WRITE) {
        if (targetBlock == nullptr) {
            // 如果目标块不存在，创建新块并添加到 stash
            if (blockIndex < 0 || blockIndex >= total_blocks) {
                std::cerr << "Error: blockIndex out of range in position_map!" << std::endl;
                return {};
            }
            Block newBlock(map[getPositionMapIndex(blockIndex, group_size, level)], blockIndex, newData);
            std::cout << "write success" << endl;
            stash.push_back(newBlock);
        }
        else {
            // 如果目标块存在，更新数据
            
            targetBlock->data = newData;
            targetBlock->leaf_id = new_leaf;
            
        }
    }
    else {
        if (targetBlock != nullptr) {
            
            // 如果目标块存在，返回数据
            data = targetBlock->data;
            targetBlock->leaf_id = new_leaf;
            std::cout << "read success" << endl;
        }
        else {
            data.clear();
        }
    }

    
    if (level == 0) {
        
        vector<int> record_path;//记录写回过的桶
        vector<int> bid_evicted;
        for (int l = num_levels - 1; l >= 0; l--) {
            
            
            for (int j = 0; j < recursive_levels; j++)
            {
                
                //版本1写回
                int Pxl;
                
                 Pxl = P(record_OldLeaf[j], l);
                 auto it = std::find(record_path.begin(), record_path.end(), Pxl);
                 if (it == record_path.end())
                 {
                     Bucket bucket;
                     int counter = 0;
                     record_path.push_back(Pxl);
                     // 将 stash 中的块写回路径
                     for (auto it = stash.begin(); it != stash.end();) {
                         if (counter >= bucket_size) {
                             break;
                         }
                         if (Pxl == P(it->leaf_id, l)) {
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
                 
                 /*版本2写回
                 vector<Block> blocks;
                 int Pxl,count=0;
                 Bucket bucket;
                 Pxl = P(record_OldLeaf[j], l);
                 blocks = storage->ReadBucket(Pxl).getBlocks();

                 for (auto b = blocks.begin(); b != blocks.end();)
                 {
                     if (b->index != -1)
                     {
                         bucket.addBlock(*b);
                         count++;
                     }
                     ++b;
                 }

                 for(auto it = stash.begin(); it != stash.end();)
                 {
                     if (count < bucket_size && Pxl == P(it->leaf_id, l))
                     {
                         bucket.addBlock(*it);
                         bid_evicted.push_back(it->index);
                         it = stash.erase(it); // 从 stash 中移除
                         count++;
                     }
                     else {
                         ++it;
                     }
                 }

                 // 填充虚拟块
                 while (count < bucket_size) {
                     bucket.addBlock(Block()); // 虚拟块
                     count++;
                 }

                 // 将桶写回存储
                 storage->WriteBucket(Pxl, bucket);*/
            }
  
        }
        
        /*if (stash.size() > 50) StashClear();*/

        record_OldLeaf.clear();
        record_path.clear();
    }
    
    std::cout << "stash size: " << stash.size() << endl;
    for (auto& block : stash)
    {
        std::cout << block.index << " ";
    }
    std::cout << endl;
    std::cout << endl;
    return data;
}

int RecursivePathOram::P(int leaf, int level)
{
    return (1 << level) - 1 + (leaf >> (this->num_levels - level - 1));
}

//计算目标层对应的块ID
int RecursivePathOram::getLayerBlockID(int dataBlockID, int N, int B, int targetLayer)
{
    
        int currentID = dataBlockID ;
        currentID = currentID - calculateTotalBlocks(N, B, targetLayer - 1);
        int start_id = calculateTotalBlocks(N, B, targetLayer);
        
        int result = start_id + currentID / B;
        return result;
    
   
}

// 数据块x 在第i层块中的局部索引。
int RecursivePathOram::getPositionMapIndex(int dataBlockID, int B, int targetLayer)
{
    int i = 0;

    int x = dataBlockID-calculateTotalBlocks(num_blocks,B,targetLayer-1);
    
    int positionMapIndex = x % B;
    
    return positionMapIndex;
}

   
//把vector<int>转成vector<char>
vector<char> RecursivePathOram::serializePositionMap(const vector<int>& positionMap)
{
    vector<char> data(positionMap.size() * sizeof(int));
    for (size_t i = 0; i < positionMap.size(); ++i) {
        memcpy(data.data() + i * sizeof(int), &positionMap[i], sizeof(int));
    }
    return data;
}

//把vector<char>转成vector<int>
vector<int> RecursivePathOram::deserializePositionMap(const std::vector<char>& data)
{
    std::vector<int> positionMap(data.size() / sizeof(int));
    for (size_t i = 0; i < positionMap.size(); ++i) {
        memcpy(&positionMap[i], data.data() + i * sizeof(int), sizeof(int));
    }
    return positionMap;
}

void RecursivePathOram::initializePositionMap()
{
    // 为每个数据块分配一个随机的叶子节点
    position_map.resize(num_blocks);
    for (int i = 0; i < num_blocks; ++i) {
        position_map[i] = rand_gen->getRandomLeaf();
    }
    
    // 使用递归Path ORAM，将位置映射存储在ORAM中
    if (recursive_levels > 0) {
        storePositionMapInOramRecursive(position_map, recursive_levels);
    }

    // 销毁不需要的 position_map
    position_map.clear(); // 只保留最顶层的 p  osition_map
}

//把positionmap存入块
void RecursivePathOram::storePositionMapInOramRecursive(const std::vector<int>& pmap_block_ids, int level)
{
    
    // 将 pmap_block_ids 分成多个组，每组存储到一个块中
    int num_groups = (pmap_block_ids.size() + group_size - 1) / group_size;
    
    std::vector<int> new_pmap_block_ids(num_groups);

    for (int i = 0; i < num_groups; ++i) {
        // 获取当前组的 pmap_block_ids
        int start = i * group_size;
        int end = std::min(start + group_size, (int)pmap_block_ids.size());
        std::vector<int> subgroup(pmap_block_ids.begin() + start, pmap_block_ids.begin() + end);

        
        // 将 subgroup 序列化为 vector<char>
        std::vector<char> serializedData = serializePositionMap(subgroup);
        
        
        // 为当前块分配一个随机的叶子节点
        int leaf = rand_gen->getRandomLeaf();
        new_pmap_block_ids[i] = leaf;

        // 创建一个块并存储 serializedData
        int block_id = calculateTotalBlocks(num_blocks,group_size,recursive_levels-level) + i; // 块的 ID
        
        Block block(leaf, block_id, serializedData);
        
        int positon = P(leaf, num_levels-1);
        
        // 将存储positionmap的块写入 ServerStorage
        Bucket bucket;
        bucket.setMaxSize(bucket_size);
        bucket.addBlock(block);
        storage->WriteBucket(positon, bucket);

        
    }

    // 如果递归层次大于 1，继续递归存储
    if (level > 1) {
        storePositionMapInOramRecursive(new_pmap_block_ids, level - 1);
    }
    else {
        // 如果递归层次为 1，当前层的 position_map 就是最顶端的 position_map
        top_level_position_map = new_pmap_block_ids;
    }
}



//计算总的块数
int RecursivePathOram::calculateTotalBlocks(int num_blocks, int group_size,int recursive_levels)
{
    if (recursive_levels < 0) return 0;
    else
    {
        int total_blocks = num_blocks;
        int current_level_blocks = num_blocks;
        int a = 0;
        while (a < recursive_levels)
        {
            current_level_blocks = (current_level_blocks + group_size - 1) / group_size;
            total_blocks += current_level_blocks;
            a++;
        }

        return total_blocks;
    }
}

//把stash中的所有块写回Oram
void RecursivePathOram::StashClear()
{
    

    for (auto it = stash.begin(); it != stash.end();)
    {
        
        
        for (int l = num_levels - 1; l >= 0; l--)
        {
            
            int path,count=0;
            Bucket bucket;
            path = P(it->leaf_id, l);
            vector<Block> blocks = storage->ReadBucket(path).getBlocks();
            for (auto b = blocks.begin(); b != blocks.end();)
            {
                if (b->index!=-1)
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

