//
//
//
#include "Block.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

Block::Block() {//dummy index
	this->leaf_id = -1;
	this->index = -1;
}

Block::Block(int leaf_id, int index,const vector<char>& data):leaf_id(leaf_id), index(index), data(data)
{
}




Block::~Block()
{
	//dtor
}

bool Block::operator==(const Block& other)
{
	return index == other.index;
}

// ¥Ú”°øÈ–≈œ¢
void Block::printBlock() {
	cout << "index: " << index << " leaf id: " << leaf_id << " data: ";
	for (char c : data) {
		cout << c;
	}
	cout << endl;
}