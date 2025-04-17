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
    
    int leaf_id;//ӳ���Ҷ�ӽ��
    int index;//��ID
    vector<char> data;//����

    Block();
    Block(int leaf_id, int index,const vector<char>& data);

    // ���� == �����
    bool operator==(const Block& other);
    void printBlock();
    virtual ~Block();
};

#endif //PORAM_BLOCK_H

