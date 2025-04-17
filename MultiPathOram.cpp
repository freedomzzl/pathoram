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

        //���� ORAM ���Ĳ��� num_levels ����Ͱ���� num_buckets��
        this->num_levels = ceil(log10(num_blocks) / log10(2)) + 1;
        this->num_buckets = pow(2, num_levels) - 1;

        //���洢ϵͳ���������Ƿ��㹻�洢�������ݿ�
        if (this->num_buckets * this->bucket_size < this->num_blocks) //deal with precision loss
        {
            throw new runtime_error("Not enough space for the acutal number of blocks.");
        }

        //����Ҷ�ӽڵ�����
        this->num_leaves = pow(2, num_levels - 1);

        //����������������ķ�Χ
        this->rand_gen->setBound(num_leaves);

        //���ô洢ϵͳ������
        this->storage->setCapacity(num_buckets);

        //����λ��ӳ�����ڴ�ռ䡣
        this->position_map = new int[this->num_blocks];
        this->stash = vector<Block>();

        //Ϊÿ�����ݿ��������һ��Ҷ�ӽڵ�����
        for (int i = 0; i < this->num_blocks; i++) {
            position_map[i] = rand_gen->getRandomLeaf();
        }

        //��ʼ���洢ϵͳ�е�ÿ��Ͱ
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

    // 1. �ռ�������Ҫ���ʵĿ����Ϣ
    vector<int> blockIndices;
    vector<int> oldLeaves;
    vector<int> newLeaves;

    for (const auto& op : operations) {
        int blockIndex = op.second;
        blockIndices.push_back(blockIndex);
        oldLeaves.push_back(position_map[blockIndex]);
        newLeaves.push_back(rand_gen->getRandomLeaf());
    }
    
    // 2. ����λ��ӳ���
    for (size_t i = 0; i < blockIndices.size(); i++) {
        position_map[blockIndices[i]] = newLeaves[i];
    }

    // 3. �ռ�������Ҫ��ȡ��·����ȥ�أ�
    unordered_set<int> pathsToRead;
    for (int oldLeaf : oldLeaves) {
        for (int l = 0; l < num_levels; l++) {
            pathsToRead.insert(P(oldLeaf, l));
        }
    }
    
    // 4. ��ȡ����·���ϵĿ鵽stash
    for (int path : pathsToRead) {
        vector<Block> blocks = storage->ReadBucket(path).getBlocks();
        for (Block b : blocks) {
            if (b.index != -1) { // ���������
                stash.push_back(b);
            }
        }
    }
    
    // 5. ����ÿ������
    for (size_t i = 0; i < operations.size(); i++) {
        Operation op = operations[i].first;
        int blockIndex = blockIndices[i];
        const vector<char>& newdata = newdataList[i];

        // ��stash�в���Ŀ���
        Block* targetBlock = nullptr;
        for (auto& block : stash) {
            if (block.index == blockIndex) {
                targetBlock = &block;
                
                break;
            }
        }
        
        // �����/д����
        if (op == Operation::WRITE) {
            if (targetBlock == nullptr) {
                
                // ���Ŀ��鲻���ڣ������¿鲢��ӵ�stash
                Block newBlock(newLeaves[i], blockIndex, newdata);
                stash.push_back(newBlock);
            }
            else {
                // ���Ŀ�����ڣ���������
                targetBlock->data = newdata;
                targetBlock->leaf_id = newLeaves[i];
                
            }
            results.push_back(vector<char>()); // д��������������
        }
        else {
            if (targetBlock != nullptr) {
                // ���Ŀ�����ڣ���������
                results.push_back(targetBlock->data);
            }
            else {
                cout << "hi" << endl;
                results.push_back(vector<char>()); // ���ؿ�����
            }
        }
    }
    
    // 6. �ռ�������Ҫд�ص�·����ȥ�أ�
    unordered_set<int> pathsToWrite;
    for (int oldLeaf : oldLeaves) {
        for (int l = 0; l < num_levels; l++) {
            pathsToWrite.insert(P(oldLeaf, l));
        }
    }
    
    // 7. ���д������·��
    for (int path : pathsToWrite) {

        vector<int> bid_evicted;
        Bucket bucket;
        int counter = 0;
        for (int l = num_levels - 1; l >= 0; l--) {
            

            

            // ��stash�еĿ�д��·��
            for (auto it = stash.begin(); it != stash.end();) {
                if (counter >= bucket_size) {
                    break;
                }
                if (path == P(position_map[it->index], l)) {
                    bucket.addBlock(*it);
                    bid_evicted.push_back(it->index);
                    it = stash.erase(it); // ��stash���Ƴ�
                    counter++;
                }
                else {
                    ++it;
                }
            }
        }
        // ��������
        while (counter < bucket_size) {
            bucket.addBlock(Block()); // �����
            counter++;
        }

        // ��Ͱд�ش洢
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

