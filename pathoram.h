#pragma once
#include "block.h"
#include "bucket.h"
#include "CryptoUtil.h"
#include "ServerStorage.h"
#include "param.h"
#include <vector>
#include <cmath>
#include <memory>
#include <random>
#include<iostream>

using namespace std;

/**
 * @class pathoram
 * @brief Path ORAM实现类，基于Path ORAM论文的简单ORAM协议
 *
 * 核心特性：
 * - 每次访问只读取和写入一条从根到叶子的路径
 * - 使用stash作为客户端缓存
 * - 每次访问后重新映射块到新的随机叶子
 */
class pathoram
{
public:
    // 操作类型枚举
    enum Operation { READ, WRITE };
    int c;
    // === ORAM结构参数 ===
    int N;             // 数据块总数
    int L;             // 树的高度
    int num_buckets;   // 树中桶的总数量
    int num_leaves;    // 叶子节点数量

    // === 存储与状态 ===
    ServerStorage* storage;     // 底层服务器存储
    vector<int> position_map;   // Position Map：记录每个数据块所在的叶节点位置
    vector<block> stash;        // 客户端缓存（Stash）

    // === 加密支持 ===
    std::shared_ptr<CryptoUtils> crypto;      // 加密工具类
    std::vector<uint8_t> encryption_key;      // 加密密钥

    struct CommunicationStats {
        size_t total_blocks_read = 0;    // 累计读取的块数
        size_t total_blocks_written = 0; // 累计写入的块数

        void reset() {
            total_blocks_read = 0;
            total_blocks_written = 0;
        }

        // 计算字节数
        size_t getBytesRead() const { return total_blocks_read * blocksize; }
        size_t getBytesWritten() const { return total_blocks_written * blocksize; }
        size_t getTotalBytes() const { return getBytesRead() + getBytesWritten(); }

        // 添加获取总块数的方法
        size_t getTotalBlocks() const { return total_blocks_read + total_blocks_written; }
        size_t getBlocksRead () const {return total_blocks_read; }

        void print() const {
            std::cout << "=== Communication Statistics ===" << std::endl;
            std::cout << "Blocks Read: " << total_blocks_read << std::endl;
            std::cout << "Blocks Written: " << total_blocks_written << std::endl;
            std::cout << "Total Blocks: " << getTotalBlocks() << std::endl;
            std::cout << "Total Bytes: " << getTotalBytes() << std::endl;
        }
    };

    CommunicationStats comm_stats;

    // 获取统计信息
    CommunicationStats getStats() const { return comm_stats; }
    void resetStats() { comm_stats.reset(); }

    /**
     * @brief 构造函数
     * @param n 数据块数量
     * @param storage 指向底层 ServerStorage 的指针
     */
    pathoram(int n, ServerStorage* storage);

    /**
     * @brief 获取随机叶节点编号
     * @return 随机叶节点索引 [0, num_leaves-1]
     */
    int get_random_leaf();

    /**
     * @brief 计算给定叶节点在某一层的bucket位置
     * @param leaf 叶节点编号
     * @param level 树层编号（根为0，叶子为L）
     * @return 对应的bucket索引
     */
    int get_bucket_in_path(int leaf, int level);

    /**
     * @brief 读取整个路径到stash
     * @param leaf 叶节点编号
     */
    void read_path(int leaf);

    /**
     * @brief 写回整个路径，并尽可能将stash中的块推回路径
     * @param leaf 叶节点编号
     */
    void write_path(int leaf);

    /**
     * @brief 在stash中查找指定块
     * @param block_index 块索引
     * @return 找到的块，如果未找到返回dummy block
     */
    block find_block_in_stash(int block_index);

    /**
     * @brief 从stash中移除指定块
     * @param block_index 块索引
     */
    void remove_block_from_stash(int block_index);

    // === 数据加密与解密 ===
    std::vector<char> encrypt_data(const std::vector<char>& data);
    std::vector<char> decrypt_data(const std::vector<char>& encrypted_data);

    /**
     * @brief Path ORAM访问接口
     * @param block_index 数据块索引
     * @param op 操作类型（READ/WRITE）
     * @param data 写操作时提供的数据
     * @return 若为读操作，返回读取的数据内容
     */
    vector<char> access(int block_index, Operation op, vector<char> data = {});

};