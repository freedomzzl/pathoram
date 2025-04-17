#include "RandomForOram.h"
#include <iostream>
#include <chrono>

// 静态成员变量初始化
bool RandomForOram::is_initialized = false;
int RandomForOram::bound = -1;

// 构造函数
RandomForOram::RandomForOram() {
    if (is_initialized) {
        throw std::runtime_error("ONLY ONE RANDOM INSTANCE CAN BE USED AT A TIME");
    }
    // 使用当前时间作为种子
    seed = static_cast<long>(std::chrono::system_clock::now().time_since_epoch().count());
    mt_generator.seed(seed);
    lcg_generator.seed(seed);
    rand_history = std::vector<int>();
    is_initialized = true;
}

// 获取随机叶子节点（默认使用 Mersenne Twister）
int RandomForOram::getRandomLeaf() {
    if (bound == -1) {
        throw std::runtime_error("Bound is not set. Call setBound() first.");
    }
    int randVal = mt_generator() % bound;
    rand_history.push_back(randVal); // 记录历史
    return randVal;
}

// 使用 Mersenne Twister 生成随机叶子节点
int RandomForOram::getRandomLeafMT() {
    if (bound == -1) {
        throw std::runtime_error("Bound is not set. Call setBound() first.");
    }
    int randVal = mt_generator() % bound;
    rand_history.push_back(randVal); // 记录历史
    return randVal;
}

// 使用线性同余生成器生成随机叶子节点
int RandomForOram::getRandomLeafLCG() {
    if (bound == -1) {
        throw std::runtime_error("Bound is not set. Call setBound() first.");
    }
    int randVal = lcg_generator() % bound;
    rand_history.push_back(randVal); // 记录历史
    return randVal;
}

// 设置随机数的上限
void RandomForOram::setBound(int totalNumOfLeaves) {
    if (totalNumOfLeaves <= 0) {
        throw std::runtime_error("Bound must be a positive integer.");
    }
    bound = totalNumOfLeaves;
}

// 重置随机数生成器状态
void RandomForOram::resetState() {
    mt_generator.seed(seed);
    lcg_generator.seed(seed);
    rand_history.clear();
    is_initialized = false;
}

// 清除随机数历史记录
void RandomForOram::clearHistory() {
    rand_history.clear();
}

// 获取随机数历史记录
std::vector<int> RandomForOram::getHistory() {
    return rand_history;
}