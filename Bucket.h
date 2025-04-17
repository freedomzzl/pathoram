//
//
//

#ifndef PORAM_BUCKET_H
#define PORAM_BUCKET_H


#include "Block.h"
#include <vector>
#include <stdexcept>


class Bucket {

public:
    int max_size;//容量
    vector<Block> blocks;//桶中存放的数据块
    

    Bucket();
   
    
    Block getBlockByIndex(int index);
    void addBlock(Block new_blk);
    bool removeBlock(int ID);
    vector<Block> getBlocks();
    void setMaxSize(int maximumSize);
    
    
    void printBlocks();
   

    
    
};


#endif //PORAM_BUCKET_H


