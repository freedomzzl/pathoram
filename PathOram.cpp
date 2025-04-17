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


vector<char> OramReadPathEviction::access(Operation op, int blockIndex, const vector<char>& newdata) {

    vector<char> data;

    // 1-2: ��ȡ�ɵ�Ҷ�ӽڵ㲢����λ��ӳ��
    int oldLeaf = position_map[blockIndex];
    position_map[blockIndex] = rand_gen->getRandomLeaf();

    //// ��ӡ�ɵ�Ҷ�ӽڵ���µ�Ҷ�ӽڵ�
    //cout << "Old Leaf: " << oldLeaf << ", New Leaf: " << position_map[blockIndex] << endl;

    // 3-5: ��ȡ·���ϵ����п鵽 stash
    for (int i = 0; i < num_levels; i++) {
        int path = P(oldLeaf, i);
        
        vector<Block> blocks = storage->ReadBucket(path).getBlocks();
        for (Block b : blocks) {
            if (b.index != -1) { // ���������
                stash.push_back(b);
            }
        }
    }

    
   

    // 6: �� stash �в���Ŀ���
    Block* targetBlock = nullptr;
    for (auto& block : stash) {
        if (block.index == blockIndex) {
            targetBlock = &block;
            break;
        }
    }

    // 7-9: �����/д����
    if (op == Operation::WRITE) {
        if (targetBlock == nullptr) {
            // ���Ŀ��鲻���ڣ������¿鲢��ӵ� stash
            Block newBlock(position_map[blockIndex], blockIndex, newdata);
            stash.push_back(newBlock);
            
        }
        else {
            // ���Ŀ�����ڣ���������
            targetBlock->data = newdata;
            
            targetBlock->printBlock();
        }
    }//дģʽ
    else {
        if (targetBlock != nullptr) {
            // ���Ŀ�����ڣ���������
            data = targetBlock->data;
            
            
        }
        else {
            data.clear();
        }
    }//��ģʽ

    // 10-15: ���д��·��
    for (int l = num_levels - 1; l >= 0; l--) {
        vector<int> bid_evicted;
        Bucket bucket;
        int Pxl = P(oldLeaf, l);
        int counter = 0;

        

        // �� stash �еĿ�д��·��
        for (auto it = stash.begin(); it != stash.end();) {
            if (counter >= bucket_size) {
                break;
            }
            if (Pxl == P(position_map[it->index], l)) {
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

    

    return data;
}

//����ָ��Ҷ�ӽڵ�Ͳ�����·��,���ش洢ϵͳ�ж�ӦͰ��λ��
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
