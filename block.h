//
//
//

#ifndef PORAM_BLOCK_H
#define PORAM_BLOCK_H
#include<vector>
#include <algorithm>
using namespace std;

class Block {
public:
    
    int leaf_id;//映射的叶子结点
    int index;//块ID
    vector<char> data;//数据

    Block();
    Block(int leaf_id, int index,const vector<char>& data);

    // 重载 == 运算符
    bool operator==(const Block& other);
    void printBlock();
    virtual ~Block();
};

#endif //PORAM_BLOCK_H

