#include "RecursivePathOram.h"
#include"block.h"
#include <iostream>
#include <cmath>
#include<algorithm>

using namespace std;
RecursivePathOram::RecursivePathOram(ServerStorage* storage, RandomForOram* rand_gen, int bucket_size, int num_blocks,int group_size,int recursive_levels)
    : storage(storage), rand_gen(rand_gen), bucket_size(bucket_size), num_blocks(num_blocks), group_size(group_size),recursive_levels(recursive_levels) {

    // ���� ORAM ���Ĳ�����Ҷ�ӽڵ���
    
    total_blocks = calculateTotalBlocks(num_blocks, group_size,recursive_levels);
    num_levels = std::ceil(std::log2(total_blocks)) + 1;
    num_leaves = std::pow(2, num_levels - 1);
    num_buckets = std::pow(2, num_levels) - 1;

    //���洢ϵͳ���������Ƿ��㹻�洢�������ݿ�
    if (this->bucket_size * this->num_buckets < total_blocks)
    {
        throw new runtime_error("Not enough space for the acutal number of blocks.");
    }

    //����������������ķ�Χ
    this->rand_gen->setBound(num_leaves);

    //���ô洢ϵͳ������
    this->storage->setCapacity(num_buckets);
    
    //��ʼ��stash
    this->stash = vector<Block>();

    //��ʼ���洢ϵͳ�е�ÿ��Ͱ
    for (int i = 0; i < num_buckets; i++) {

        Bucket init_bkt = Bucket();
        for (int j = 0; j < bucket_size; j++) {
            init_bkt.addBlock(Block());
        }
        storage->WriteBucket(i, Bucket(init_bkt));
    }

    //��ʼ��positionmap
    initializePositionMap();
}

RecursivePathOram::~RecursivePathOram() {
    // ������Դ
}


//�ݹ��access
std::vector<char> RecursivePathOram::recursiveaccess(Operation op, int blockIndex, const std::vector<char>& newData, int level) {
    std::cout << "Debug: level = " << level << ", blockIndex = " << blockIndex << std::endl;

    //�ж�blockindex�Ƿ���������Χ��
    if (blockIndex < 0 || blockIndex >= total_blocks) {
        std::cerr << "Error: blockIndex out of range! (" << blockIndex << ")" <<total_blocks<< std::endl;
        return {};
    }
    
    std::vector<char> result, data;
    std::vector<int> map;
    std::vector<Block> blocks;
    int old_leaf = -1, new_leaf = -1;

    
    
    

    // �ݹ������һ��
    if (level < recursive_levels) {
        
        int next_id = getLayerBlockID(blockIndex, num_blocks, group_size, level);//����next_id
        
        result = recursiveaccess(READ, next_id, {}, level + 1);//�ݹ�access
        
        if (result.empty()) {
            std::cerr << "Error: Failed to read next level block!" << std::endl;
            return {};

        }

        //��ȡpositionmap
        map = deserializePositionMap(result);
        if (map.empty()) {
            std::cerr << "Error: Failed to deserialize position map!" << std::endl;
            return {};
        }
       
        old_leaf = map[getPositionMapIndex(blockIndex, group_size, level)];//����oldleaf
        record_OldLeaf.push_back(old_leaf);
        
        map[getPositionMapIndex(blockIndex, group_size, level)] = rand_gen->getRandomLeaf();//ӳ���µ�Ҷ�ӽ��
        new_leaf = map[getPositionMapIndex(blockIndex, group_size, level)];
        
        //�Ѹ��ĺ��positionmap�����
        vector<char> char_map = serializePositionMap(map);
        for (auto& block : stash) {


            if (block.index == next_id) {
                block.data = char_map;
                
                break;
            }
        }

    }
    else if (level == recursive_levels) {
        old_leaf = top_level_position_map[getPositionMapIndex(blockIndex, group_size, level)];//����oldleaf
        record_OldLeaf.push_back(old_leaf);
        
        top_level_position_map[getPositionMapIndex(blockIndex, group_size, level)]= rand_gen->getRandomLeaf();
        new_leaf = top_level_position_map[getPositionMapIndex(blockIndex, group_size, level)];
        
    }
    else {
        std::cerr << "Error: Invalid level (" << level << ")!" << std::endl;
        return {};
    }

    std::cout << "blockindex: " << blockIndex << endl;
    
    // ��ȡ·���ϵ����п鵽 stash
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
    
    // �� stash �в���Ŀ���
    Block* targetBlock = nullptr;
    
    for (auto& block : stash) {

        
        if (block.index == blockIndex) {
            targetBlock = &block;
            std::cout << "find!" << endl;
            break;
        }
    }
    
    // �����/д����
    if (op == Operation::WRITE) {
        if (targetBlock == nullptr) {
            // ���Ŀ��鲻���ڣ������¿鲢��ӵ� stash
            if (blockIndex < 0 || blockIndex >= total_blocks) {
                std::cerr << "Error: blockIndex out of range in position_map!" << std::endl;
                return {};
            }
            Block newBlock(map[getPositionMapIndex(blockIndex, group_size, level)], blockIndex, newData);
            std::cout << "write success" << endl;
            stash.push_back(newBlock);
        }
        else {
            // ���Ŀ�����ڣ���������
            
            targetBlock->data = newData;
            targetBlock->leaf_id = new_leaf;
            
        }
    }
    else {
        if (targetBlock != nullptr) {
            
            // ���Ŀ�����ڣ���������
            data = targetBlock->data;
            targetBlock->leaf_id = new_leaf;
            std::cout << "read success" << endl;
        }
        else {
            data.clear();
        }
    }

    
    if (level == 0) {
        
        vector<int> record_path;//��¼д�ع���Ͱ
        vector<int> bid_evicted;
        for (int l = num_levels - 1; l >= 0; l--) {
            
            
            for (int j = 0; j < recursive_levels; j++)
            {
                
                //�汾1д��
                int Pxl;
                
                 Pxl = P(record_OldLeaf[j], l);
                 auto it = std::find(record_path.begin(), record_path.end(), Pxl);
                 if (it == record_path.end())
                 {
                     Bucket bucket;
                     int counter = 0;
                     record_path.push_back(Pxl);
                     // �� stash �еĿ�д��·��
                     for (auto it = stash.begin(); it != stash.end();) {
                         if (counter >= bucket_size) {
                             break;
                         }
                         if (Pxl == P(it->leaf_id, l)) {
                             bucket.addBlock(*it);
                             bid_evicted.push_back(it->index);
                             it = stash.erase(it); // �� stash ���Ƴ�
                             counter++;
                         }
                         else {
                             ++it;
                         }
                     }

                     // ��������
                     while (counter < bucket_size) {
                         bucket.addBlock(Block()); // �����
                         counter++;
                     }

                     // ��Ͱд�ش洢
                     storage->WriteBucket(Pxl, bucket);
                 }
                 
                 /*�汾2д��
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
                         it = stash.erase(it); // �� stash ���Ƴ�
                         count++;
                     }
                     else {
                         ++it;
                     }
                 }

                 // ��������
                 while (count < bucket_size) {
                     bucket.addBlock(Block()); // �����
                     count++;
                 }

                 // ��Ͱд�ش洢
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

//����Ŀ����Ӧ�Ŀ�ID
int RecursivePathOram::getLayerBlockID(int dataBlockID, int N, int B, int targetLayer)
{
    
        int currentID = dataBlockID ;
        currentID = currentID - calculateTotalBlocks(N, B, targetLayer - 1);
        int start_id = calculateTotalBlocks(N, B, targetLayer);
        
        int result = start_id + currentID / B;
        return result;
    
   
}

// ���ݿ�x �ڵ�i����еľֲ�������
int RecursivePathOram::getPositionMapIndex(int dataBlockID, int B, int targetLayer)
{
    int i = 0;

    int x = dataBlockID-calculateTotalBlocks(num_blocks,B,targetLayer-1);
    
    int positionMapIndex = x % B;
    
    return positionMapIndex;
}

   
//��vector<int>ת��vector<char>
vector<char> RecursivePathOram::serializePositionMap(const vector<int>& positionMap)
{
    vector<char> data(positionMap.size() * sizeof(int));
    for (size_t i = 0; i < positionMap.size(); ++i) {
        memcpy(data.data() + i * sizeof(int), &positionMap[i], sizeof(int));
    }
    return data;
}

//��vector<char>ת��vector<int>
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
    // Ϊÿ�����ݿ����һ�������Ҷ�ӽڵ�
    position_map.resize(num_blocks);
    for (int i = 0; i < num_blocks; ++i) {
        position_map[i] = rand_gen->getRandomLeaf();
    }
    
    // ʹ�õݹ�Path ORAM����λ��ӳ��洢��ORAM��
    if (recursive_levels > 0) {
        storePositionMapInOramRecursive(position_map, recursive_levels);
    }

    // ���ٲ���Ҫ�� position_map
    position_map.clear(); // ֻ�������� p  osition_map
}

//��positionmap�����
void RecursivePathOram::storePositionMapInOramRecursive(const std::vector<int>& pmap_block_ids, int level)
{
    
    // �� pmap_block_ids �ֳɶ���飬ÿ��洢��һ������
    int num_groups = (pmap_block_ids.size() + group_size - 1) / group_size;
    
    std::vector<int> new_pmap_block_ids(num_groups);

    for (int i = 0; i < num_groups; ++i) {
        // ��ȡ��ǰ��� pmap_block_ids
        int start = i * group_size;
        int end = std::min(start + group_size, (int)pmap_block_ids.size());
        std::vector<int> subgroup(pmap_block_ids.begin() + start, pmap_block_ids.begin() + end);

        
        // �� subgroup ���л�Ϊ vector<char>
        std::vector<char> serializedData = serializePositionMap(subgroup);
        
        
        // Ϊ��ǰ�����һ�������Ҷ�ӽڵ�
        int leaf = rand_gen->getRandomLeaf();
        new_pmap_block_ids[i] = leaf;

        // ����һ���鲢�洢 serializedData
        int block_id = calculateTotalBlocks(num_blocks,group_size,recursive_levels-level) + i; // ��� ID
        
        Block block(leaf, block_id, serializedData);
        
        int positon = P(leaf, num_levels-1);
        
        // ���洢positionmap�Ŀ�д�� ServerStorage
        Bucket bucket;
        bucket.setMaxSize(bucket_size);
        bucket.addBlock(block);
        storage->WriteBucket(positon, bucket);

        
    }

    // ����ݹ��δ��� 1�������ݹ�洢
    if (level > 1) {
        storePositionMapInOramRecursive(new_pmap_block_ids, level - 1);
    }
    else {
        // ����ݹ���Ϊ 1����ǰ��� position_map ������˵� position_map
        top_level_position_map = new_pmap_block_ids;
    }
}



//�����ܵĿ���
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

//��stash�е����п�д��Oram
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
                    bucket.addBlock(Block()); // �����
                    count++;
                }

                // ��Ͱд�ش洢
                storage->WriteBucket(path, bucket);
                it = stash.erase(it);
                break;
            }

            else if (l == 0)
            {
                // ���в㼶�����Թ��ˣ��޷�����
                ++it;
            }
        }
    }
}

