// block.h
#pragma once
#include<vector>

using namespace std;

class block
{
private:
    int leaf_id;
    int blockindex;
    vector<char> data;

public:
    block();
    block(int leaf_id, int blockindex, vector<char> data);
    int GetBlockindex();
    void SetBlockindex(int blockindex);
    int GetLeafid();
    void SetLeafid(int lead_id);
    vector<char> GetData();
    vector<char> GetData() const;
    void SetData(vector<char> data);
};