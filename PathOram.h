//
//
//

#ifndef PORAM_ORAMREADPATHEVICTION_H
#define PORAM_ORAMREADPATHEVICTION_H
#include "OramInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"
#include <cmath>
#include"ServerStorage.h"
#include"RandomForOram.h"

class OramReadPathEviction : public OramInterface {
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

    OramReadPathEviction(ServerStorage* storage,
        RandomForOram* rand_gen, int bucket_size, int num_blocks);

    vector<char> access(Operation op, int blockIndex, const vector<char>& newdata);//ִ�� ORAM �ķ��ʲ�������ȡ��д�룩
    int P(int leaf, int level);//����ָ��Ҷ�ӽڵ�Ͳ�����·����
    int* getPositionMap();//��ȡλ��ӳ���
    vector<Block> getStash();//��ȡ���ش洢����ʱ���ݿ鼯�ϣ�stash����
    int getStashSize();
    int getNumLeaves();
    int getNumLevels();
    int getNumBlocks();
    int getNumBuckets();
    virtual ~OramReadPathEviction();
    void StashClear();
};


#endif 

