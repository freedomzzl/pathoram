//
//
//

#ifndef PORAM_RANDFORORAMINTERFACE_H
#define PORAM_RANDFORORAMINTERFACE_H

using namespace std;

//Ϊ ORAM �ṩһ������������������
class RandForOramInterface {
public:
    virtual int getRandomLeaf() { return 0; };//����һ�������Ҷ�ӽڵ�������

    virtual void setBound(int num_leaves) {};//������������ɵķ�Χ����Ҷ�ӽڵ����������
};


#endif 

