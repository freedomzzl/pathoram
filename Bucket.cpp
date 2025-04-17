//
//
//
#include "Bucket.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

Bucket::Bucket()
{
    this->max_size = -1;
    this->blocks = vector<Block>();
    
}







//Get block object with matching index
Block Bucket::getBlockByIndex(int index) {
    for (Block& b : blocks) {
        if (b.index == index) {
            return b;
        }
    }
    throw runtime_error("Block not found");
}

//Ìí¼Ó¿é
void Bucket::addBlock(Block new_blk) {
    if (blocks.size() < max_size) {
        Block toAdd = Block(new_blk);
        blocks.push_back(toAdd);
    }

}

//ÒÆ³ý¿é
bool Bucket::removeBlock(int ID) {
    bool removed = false;
    for (int i = 0; i < blocks.size(); i++) {
        if (blocks[i].index == ID) {
            blocks.erase(blocks.begin() + i);
            removed = true;
            break;
        }
    }
    return removed;
}

// Return a shallow copy.
vector<Block> Bucket::getBlocks() {
    return this->blocks;
}

//ÉèÖÃÈÝÁ¿
void Bucket::setMaxSize(int maximumSize) {
    
    this->max_size = maximumSize;
    
}





void Bucket::printBlocks() {
    for (Block b : blocks) {
        b.printBlock();
    }
}

