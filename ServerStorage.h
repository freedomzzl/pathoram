//
//
//

#ifndef PORAM_SERVERSTORAGE_H
#define PORAM_SERVERSTORAGE_H
#include "OramInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"
#include <cmath>

class ServerStorage : public UntrustedStorageInterface {
public:

    static bool is_initialized;//用于标记 ServerStorage 是否已初始化
    static bool is_capacity_set;//用于标记存储系统的容量是否已设置。

    //Bucket* buckets;
    std::vector<Bucket> buckets;

    ServerStorage();
    void setCapacity(int totalNumOfBuckets);//设置存储系统的总容量（桶的数量）
    Bucket ReadBucket(int position);
    void WriteBucket(int position, const Bucket& bucket_to_write);

private:

    int capacity;

};


#endif //PORAM_ORAMREADPATHEVICTION_H

