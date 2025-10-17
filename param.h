#ifndef PARAM_H
#define PARAM_H

#include"block.h"
#include<vector>


/*
 * param.h
 * ----------------------------------------
 * 本文件定义了 ORAM 系统中的全局参数。
 * 这些参数会在 param.cpp 中被初始化。
 */

 // ORAM 系统中真实数据块的总数量
extern int totalnumRealblock;

extern int OramL;

extern int blocksize;


// 每个 bucket 中真实数据块的数量
extern int realBlockEachbkt;

// 每个 bucket 中虚拟（dummy）数据块的数量
extern int dummyBlockEachbkt;


extern block dummyBlock;

//每个桶的capacity
extern int maxblockEachbkt;



#endif