#include "param.h"

int totalnumRealblock = 20000;
int OramL = static_cast<int>(ceil(log2(totalnumRealblock)));
int blocksize = 4096;
int realBlockEachbkt = 5;
int dummyBlockEachbkt = 6;
block dummyBlock(-1, -1, {
    'd', 'u', 'm', 'm', 'y', 'b', 'l', 'o',
    'c', 'k', 'd', 'a', 't', 'a', '0', '0'
    });
int maxblockEachbkt = realBlockEachbkt + dummyBlockEachbkt;

