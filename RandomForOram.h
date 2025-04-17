#ifndef PORAM_RANDOMFORORAM_H
#define PORAM_RANDOMFORORAM_H

#include <vector>
#include <random>
#include <stdexcept>

class RandomForOram {
public:
    static bool is_initialized; // 静态变量，用于标记是否已初始化
    static int bound;           // 静态变量，用于存储随机数的上限

    RandomForOram();            // 构造函数
    int getRandomLeaf();        // 获取随机叶子节点
    int getRandomLeafMT();      // 使用 Mersenne Twister 生成随机叶子节点
    int getRandomLeafLCG();     // 使用线性同余生成器生成随机叶子节点
    void setBound(int totalNumOfLeaves); // 设置随机数的上限
    void resetState();          // 重置随机数生成器状态
    void clearHistory();        // 清除随机数历史记录
    std::vector<int> getHistory(); // 获取随机数历史记录

private:
    std::vector<int> rand_history; // 存储随机数历史记录
    std::mt19937 mt_generator;     // Mersenne Twister 随机数生成器
    std::linear_congruential_engine<unsigned long, 0x5DEECE66DL, 11, 281474976710656> lcg_generator; // 线性同余生成器
    long seed;                     // 随机数种子
};

#endif