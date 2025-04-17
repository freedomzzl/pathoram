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
    ServerStorage* storage; //ָ��һ�������ŵĴ洢ϵͳ�ӿڣ����ڶ�д���ݿ顣
    RandomForOram* rand_gen;//ָ��һ��������������ӿڣ��������������Ҷ�ӽڵ�����

    int bucket_size;//ÿ��Ͱ���Դ洢�����ݿ�����
    int num_levels;//ORAM ���Ĳ���
    int num_leaves;//ORAM ����Ҷ�ӽڵ�����
    int num_blocks;//ORAM �д洢�������ݿ�����
    int num_buckets;//ORAM �д洢����Ͱ����

    int* position_map; //array
    vector<Block> stash;

    MultiPathOram(ServerStorage* storage,
        RandomForOram* rand_gen, int bucket_size, int num_blocks);

    vector<vector<char>> multiaccess(vector<pair<Operation, int>>& operations, vector<vector<char>>& newdataList);//ִ�� ORAM �ķ��ʲ�������ȡ��д�룩��һ�η��ʶ��
    int P(int leaf, int level);//����ָ��Ҷ�ӽڵ�Ͳ�����·����
    int* getPositionMap();//��ȡλ��ӳ���
    vector<Block> getStash();//��ȡ���ش洢����ʱ���ݿ鼯�ϣ�stash����
    int getStashSize();
    int getNumLeaves();
    int getNumLevels();
    int getNumBlocks();
    int getNumBuckets();
    virtual ~MultiPathOram();
    void StashClear();
};

#endif //MULTI_PATH_ORAM_H