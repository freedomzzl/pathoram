#pragma once
#include "pathoram.h"
#include "ServerStorage.h"
#include "param.h"
#include <vector>
#include <memory>

class recursive_pathoram {
public:
    std::unique_ptr<pathoram> data_oram;        // 主数据ORAM，同时存储position map
    ServerStorage* storage;                     // 服务器存储
    int data_block_count;                       // 数据块数量
    int recursion_depth;                        // 递归深度

    
    /**
     * @brief 计算position map块的索引
     */
    int get_position_map_block_index(int level, int data_block_index) const;

    /**
     * @brief 编码position map条目
     */
    std::vector<char> encode_position_entry(int block_index, int leaf_id);

    /**
     * @brief 解码position map条目
     */
    void decode_position_entry(const std::vector<char>& data, int& block_index, int& leaf_id);

    /**
     * @brief 递归访问position map（基于ringoram的实现）
     */
    int recursive_access_position_map(int level, int block_index, int new_leaf = -1);


    /**
     * @brief 构造函数
     * @param n 数据块数量
     * @param storage 服务器存储
     */
    recursive_pathoram(int n, ServerStorage* storage);

    /**
     * @brief 访问接口
     */
    std::vector<char> access(int block_index, pathoram::Operation op, std::vector<char> data = {});

    // 获取信息
    int get_recursion_depth() const { return recursion_depth; }
    int get_data_block_count() const { return data_block_count; }
    
    // 获取统计信息
    pathoram::CommunicationStats getStats() const {
        return data_oram ? data_oram->getStats() : pathoram::CommunicationStats();
    }

    // 获取带宽信息的方法
    size_t getTotalBandwidth() const {
        return data_oram ? data_oram->getStats().getTotalBytes() : 0;
    }

    size_t getTotalAccessedBlocks() const {
        return data_oram ? data_oram->getStats().getTotalBlocks() : 0;
    }

    void resetStats() {
        data_oram->resetStats();
    }
};