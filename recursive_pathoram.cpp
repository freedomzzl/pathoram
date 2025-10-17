// recursive_pathoram.cpp 
#include "recursive_pathoram.h"
#include <iostream>
#include <cmath>
#include <cstring>

using namespace std;

recursive_pathoram::recursive_pathoram(int n, ServerStorage* storage)
    : data_block_count(n), storage(storage), recursion_depth(0) {

    cout << "Initializing Recursive Path ORAM with " << n << " data blocks..." << endl;

    // 计算position map位数
    int position_map_bits = static_cast<int>(ceil(log2(n)));

    // 计算递归深度
    recursion_depth = 0;
    int current_size = n * position_map_bits; // position map总大小(bits)
    int current_n = n;

    while (current_size > blocksize * 8) {
        recursion_depth++;
        int leaves = 1 << static_cast<int>(ceil(log2(current_n))); // 叶子数量
        current_size = leaves * position_map_bits; // 下一级position map大小
        current_n = (leaves * position_map_bits) / (blocksize * 8);
        if (current_n < 16) current_n = 16;
    }

    cout << "Recursion depth: " << recursion_depth << endl;

    // 计算总块数：数据块 + 所有position map块
    // 基于ringoram的逻辑：每层分配 data_block_count 个额外的块
    int total_blocks = data_block_count * (recursion_depth + 1);

  

    // 创建单个ORAM实例（包含数据和所有position map）
    data_oram = make_unique<pathoram>(total_blocks, storage);

  
}

int recursive_pathoram::get_position_map_block_index(int level, int data_block_index) const {
    // ：为不同层级的position map分配不同的block索引范围
    int base_index = data_block_count * (level + 1);
    return base_index + data_block_index;
}

vector<char> recursive_pathoram::encode_position_entry(int block_index, int leaf_id) {
    // 确保数据大小与ORAM块大小匹配
    vector<char> data(blocksize, 0); // 用0填充整个块

    // 写入有效载荷
    memcpy(data.data(), &block_index, sizeof(int));
    memcpy(data.data() + sizeof(int), &leaf_id, sizeof(int));

    return data;
}

void recursive_pathoram::decode_position_entry(const vector<char>& data, int& block_index, int& leaf_id) {
    if (data.size() >= sizeof(int) * 2) {
        memcpy(&block_index, data.data(), sizeof(int));
        memcpy(&leaf_id, data.data() + sizeof(int), sizeof(int));
    }
    else {
        block_index = -1;
        leaf_id = -1;
    }
}

int recursive_pathoram::recursive_access_position_map(int level, int block_index, int new_leaf) {
    if (level >= recursion_depth) {
        // 最底层：返回一个有效的随机leaf
        return data_oram->get_random_leaf();
    }

    // 计算这个层级的position map块的索引
    int pm_block_index = get_position_map_block_index(level, block_index);

    // 检查索引是否在数据ORAM的范围内
    if (pm_block_index >= data_oram->N) {
        pm_block_index = pm_block_index % data_oram->N;
    }

    vector<char> encoded_data;
    if (new_leaf != -1) {
        // WRITE操作：更新position map
        encoded_data = encode_position_entry(block_index, new_leaf);
    }
    else {
        // READ操作：读取当前值
        encoded_data = encode_position_entry(block_index, 0); // dummy data for read
    }

    // 使用数据ORAM来访问position map块
    vector<char> result;
    if (new_leaf != -1) {
        result = data_oram->access(pm_block_index, pathoram::WRITE, encoded_data);
    }
    else {
        result = data_oram->access(pm_block_index, pathoram::READ);
    }

    if (result.empty()) {
        return data_oram->get_random_leaf();
    }

    int old_position, old_leaf;
    decode_position_entry(result, old_position, old_leaf);

    // 确保leaf值在有效范围内
    if (old_leaf < 0 || old_leaf >= data_oram->num_leaves) {
        old_leaf = data_oram->get_random_leaf();
    }

    // 递归访问下一级position map
    return recursive_access_position_map(level + 1, old_leaf, new_leaf);
}

vector<char> recursive_pathoram::access(int block_index, pathoram::Operation op, vector<char> data) {
    if (block_index < 0 || block_index >= data_block_count) {
        cerr << "ERROR: Block index out of range" << endl;
        return {};
    }



    int current_leaf = recursive_access_position_map(0, block_index, -1);

    // 生成新的随机leaf
    int new_leaf = data_oram->get_random_leaf();

    // 2. 更新position map
 
    recursive_access_position_map(0, block_index, new_leaf);


    vector<char> result = data_oram->access(block_index, op, data);

  
    return result;
}

