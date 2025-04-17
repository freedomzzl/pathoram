#ifndef PORAM_RANDOMFORORAM_H
#define PORAM_RANDOMFORORAM_H

#include <vector>
#include <random>
#include <stdexcept>

class RandomForOram {
public:
    static bool is_initialized; // ��̬���������ڱ���Ƿ��ѳ�ʼ��
    static int bound;           // ��̬���������ڴ洢�����������

    RandomForOram();            // ���캯��
    int getRandomLeaf();        // ��ȡ���Ҷ�ӽڵ�
    int getRandomLeafMT();      // ʹ�� Mersenne Twister �������Ҷ�ӽڵ�
    int getRandomLeafLCG();     // ʹ������ͬ���������������Ҷ�ӽڵ�
    void setBound(int totalNumOfLeaves); // ���������������
    void resetState();          // ���������������״̬
    void clearHistory();        // ����������ʷ��¼
    std::vector<int> getHistory(); // ��ȡ�������ʷ��¼

private:
    std::vector<int> rand_history; // �洢�������ʷ��¼
    std::mt19937 mt_generator;     // Mersenne Twister �����������
    std::linear_congruential_engine<unsigned long, 0x5DEECE66DL, 11, 281474976710656> lcg_generator; // ����ͬ��������
    long seed;                     // ���������
};

#endif