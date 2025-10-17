#include "bucket.h"

bucket::bucket()
    : Z(0), S(0), blocks()
{
}

bucket::bucket(int Z, int S)
    : Z(Z), S(S), blocks(Z + S, block())  // 初始化为 Z+S 个默认块
{
}