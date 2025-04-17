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


    ServerStorage* storage; // �����Ŵ洢
    RandomForOram* rand_gen; // �����������

    int bucket_size; // ÿ��Ͱ�Ĵ�С
    int num_blocks; // ���ݿ������
    int group_size; // ÿ�� position map �Ĵ�С

    int recursive_levels;//�ݹ����
    int num_levels; // ORAM ���Ĳ���
    int num_leaves; // Ҷ�ӽڵ������
    int num_buckets; // Ͱ������
    int total_blocks;

    vector<Block> stash;
    vector<int> position_map; 
    vector<int> top_level_position_map; // ��˵� position_map

    vector<int> record_OldLeaf;//��¼���ʵĿ�������Ҷ�ӽ��

    int P(int leaf, int level);//����ָ��Ҷ�ӽڵ�Ͳ�����·����

    int getLayerBlockID(int dataBlockID, int N, int B,  int targetLayer);//����Ŀ����Ӧ�Ŀ�ID
    int getPositionMapIndex(int dataBlockID, int B, int targetLayer);// ���ݿ�x �ڵ�i����еľֲ�������
   

    vector<char> serializePositionMap(const vector<int>& positionMap);//��vector<int>ת��vector<char>
    vector<int> deserializePositionMap(const std::vector<char>& data);//��vector<char>ת��vector<int>
    void initializePositionMap();//��ʼ��positionmap
    
    void storePositionMapInOramRecursive(const std::vector<int>& pmap_block_ids, int level);//��positionmap�����
    
    int calculateTotalBlocks(int num_blocks, int group_size,int recursive_levels);//���ݿ�+�洢positionmap�Ŀ������
    void StashClear();
};

#endif // RECURSIVE_PATH_ORAM_H