#include "RandomForOram.h"
#include <iostream>
#include <chrono>

// ��̬��Ա������ʼ��
bool RandomForOram::is_initialized = false;
int RandomForOram::bound = -1;

// ���캯��
RandomForOram::RandomForOram() {
    if (is_initialized) {
        throw std::runtime_error("ONLY ONE RANDOM INSTANCE CAN BE USED AT A TIME");
    }
    // ʹ�õ�ǰʱ����Ϊ����
    seed = static_cast<long>(std::chrono::system_clock::now().time_since_epoch().count());
    mt_generator.seed(seed);
    lcg_generator.seed(seed);
    rand_history = std::vector<int>();
    is_initialized = true;
}

// ��ȡ���Ҷ�ӽڵ㣨Ĭ��ʹ�� Mersenne Twister��
int RandomForOram::getRandomLeaf() {
    if (bound == -1) {
        throw std::runtime_error("Bound is not set. Call setBound() first.");
    }
    int randVal = mt_generator() % bound;
    rand_history.push_back(randVal); // ��¼��ʷ
    return randVal;
}

// ʹ�� Mersenne Twister �������Ҷ�ӽڵ�
int RandomForOram::getRandomLeafMT() {
    if (bound == -1) {
        throw std::runtime_error("Bound is not set. Call setBound() first.");
    }
    int randVal = mt_generator() % bound;
    rand_history.push_back(randVal); // ��¼��ʷ
    return randVal;
}

// ʹ������ͬ���������������Ҷ�ӽڵ�
int RandomForOram::getRandomLeafLCG() {
    if (bound == -1) {
        throw std::runtime_error("Bound is not set. Call setBound() first.");
    }
    int randVal = lcg_generator() % bound;
    rand_history.push_back(randVal); // ��¼��ʷ
    return randVal;
}

// ���������������
void RandomForOram::setBound(int totalNumOfLeaves) {
    if (totalNumOfLeaves <= 0) {
        throw std::runtime_error("Bound must be a positive integer.");
    }
    bound = totalNumOfLeaves;
}

// ���������������״̬
void RandomForOram::resetState() {
    mt_generator.seed(seed);
    lcg_generator.seed(seed);
    rand_history.clear();
    is_initialized = false;
}

// ����������ʷ��¼
void RandomForOram::clearHistory() {
    rand_history.clear();
}

// ��ȡ�������ʷ��¼
std::vector<int> RandomForOram::getHistory() {
    return rand_history;
}