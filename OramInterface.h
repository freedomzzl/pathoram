//
//
//

#ifndef PORAM_ORAMINTERFACE_H
#define PORAM_ORAMINTERFACE_H
#include "Block.h"
#include <vector>

class OramInterface {
public:
    enum Operation { READ, WRITE };

    // ���ʲ���������һ��ָ�����ݵ�ָ��
    virtual vector<char> access(Operation op, int blockIndex,
        vector<char>& newData) {
        int dummy = 1000;
        int* dummy_ptr = &dummy;
        return vector<char>(); // ����һ���յ� vector<char>
    };

    // ��ȡ·������
    virtual int P(int leaf, int level) { return 0; };

    // ��ȡλ��ӳ��
    virtual int* getPositionMap() {
        int dummy = 1000;
        int* dummy_ptr = &dummy;
        return 0;
    };

    // ��ȡ stash �еĿ�
    virtual vector<Block> getStash() { return vector<Block>(); };
    virtual int getStashSize() { return 0; };// ��ȡ stash �Ĵ�С
    virtual int getNumLeaves() { return 0; };// ��ȡҶ�ӽڵ������
    virtual int getNumLevels() { return 0; };// ��ȡ���Ĳ���
    virtual int getNumBlocks() { return 0; };// ��ȡ�������
    virtual int getNumBuckets() { return 0; };// ��ȡͰ������
};


#endif //PORAM_ORAMINTERFACE_H
