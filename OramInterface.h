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

    // 访问操作，返回一个指向数据的指针
    virtual vector<char> access(Operation op, int blockIndex,
        vector<char>& newData) {
        int dummy = 1000;
        int* dummy_ptr = &dummy;
        return vector<char>(); // 返回一个空的 vector<char>
    };

    // 获取路径函数
    virtual int P(int leaf, int level) { return 0; };

    // 获取位置映射
    virtual int* getPositionMap() {
        int dummy = 1000;
        int* dummy_ptr = &dummy;
        return 0;
    };

    // 获取 stash 中的块
    virtual vector<Block> getStash() { return vector<Block>(); };
    virtual int getStashSize() { return 0; };// 获取 stash 的大小
    virtual int getNumLeaves() { return 0; };// 获取叶子节点的数量
    virtual int getNumLevels() { return 0; };// 获取树的层数
    virtual int getNumBlocks() { return 0; };// 获取块的数量
    virtual int getNumBuckets() { return 0; };// 获取桶的数量
};


#endif //PORAM_ORAMINTERFACE_H
