#pragma once
#include "block.h"

class bucket
{
public:
    int Z;   // 真实块数量
    int S;   // 虚拟块数量

    // 桶中存储的块（真实块 + 虚拟块）
    vector<block> blocks;

    bucket();
    bucket(int Z, int S);
};