//
//
//

#ifndef PORAM_UNTRUSTEDSTORAGEINTERFACE_H
#define PORAM_UNTRUSTEDSTORAGEINTERFACE_H
#include "Bucket.h"

class UntrustedStorageInterface {
    //You are required to call setCapacity BEFORE reading or writing any buckets. You are not allowed to change the capacity after you set it.
    //You should only use ONE UntrustedStorage in your ORAM.
public:
    virtual void setCapacity(int totalNumOfBuckets) {};//设置存储系统的总容量（即桶的数量）

    virtual Bucket ReadBucket(int position) { return Bucket(); };//从存储系统中读取指定位置的桶

    virtual void WriteBucket(int position, const Bucket& bucket_to_write) {};//将指定的桶写入存储系统的指定位置。
};


#endif //PORAM_UNTRUSTEDSTORAGEINTERFACE_H


