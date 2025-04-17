//
//
//

#ifndef PORAM_RANDFORORAMINTERFACE_H
#define PORAM_RANDFORORAMINTERFACE_H

using namespace std;

//为 ORAM 提供一个抽象的随机数生成器
class RandForOramInterface {
public:
    virtual int getRandomLeaf() { return 0; };//生成一个随机的叶子节点索引。

    virtual void setBound(int num_leaves) {};//设置随机数生成的范围（即叶子节点的总数）。
};


#endif 

