// pathoram.cpp - 修复版本
#include "pathoram.h"
#include "param.h"
#include <iostream>
#include <random>
#include <algorithm>

using namespace std;

pathoram::pathoram(int n, ServerStorage* storage)
    : N(n), L(static_cast<int>(ceil(log2(N)))), num_buckets((1 << (L + 1)) - 1), num_leaves(1 << L),
    storage(storage) {
   
    c = 0;
    position_map.resize(N);
    for (int i = 0; i < N; i++) {
        position_map[i] = get_random_leaf();
    }

    storage->setCapacity(num_buckets);
    encryption_key = CryptoUtils::generateRandomKey(16);
    crypto = make_shared<CryptoUtils>(encryption_key);
    stash.clear();

  
}

int pathoram::get_random_leaf() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, num_leaves - 1);
    return dist(gen);
}

int pathoram::get_bucket_in_path(int leaf, int level) {
    return (1 << level) - 1 + (leaf >> (L - level));
}

void pathoram::read_path(int leaf) {
    size_t blocks_this_read = 0;

    for (int level = 0; level <= L; level++) {
        int bucket_pos = get_bucket_in_path(leaf, level);
        bucket bkt = storage->GetBucket(bucket_pos);

        blocks_this_read += bkt.blocks.size();
        
        // 将所有块（加密状态）添加到stash
        for (auto& blk : bkt.blocks) {
            if (blk.GetBlockindex() != -1) {  // 不是dummy块
                stash.push_back(blk);
            }
        }
    }
    
    comm_stats.total_blocks_read += blocks_this_read;
}


void pathoram::write_path(int leaf) {
 
    for (int level = L; level >= 0; level--) {
        int bucket_pos = get_bucket_in_path(leaf, level);
        vector<block> blocks_for_bucket;

        for (auto it = stash.begin(); it != stash.end(); ) {
            int block_leaf = it->GetLeafid();
            int block_bucket_pos = get_bucket_in_path(block_leaf, level);

            if (block_bucket_pos == bucket_pos) {
                vector<char> block_data = it->GetData();
                vector<char> encrypted_data;

                // 修复比较逻辑：先获取dummy block的数据
                vector<char> dummy_data = dummyBlock.GetData();

                // 像ringoram一样检查是否已加密
                // 如果数据大小不是16的倍数，或者看起来像未加密数据，需要加密
                if (block_data.size() % 16 != 0 &&
                    block_data.size() == blocksize &&
                    !block_data.empty() &&
                    block_data != dummy_data) {
                    // 未加密数据，需要加密
                    encrypted_data = encrypt_data(block_data);
                }
                else {
                    // 已加密数据或dummy块，直接使用
                    encrypted_data = block_data;
                }

                block encrypted_block(it->GetLeafid(), it->GetBlockindex(), encrypted_data);
                blocks_for_bucket.push_back(encrypted_block);
                it = stash.erase(it);
            }
            else {
                ++it;
            }

            if (blocks_for_bucket.size() >= realBlockEachbkt) break;
        }

        while (blocks_for_bucket.size() < realBlockEachbkt + dummyBlockEachbkt) {
            blocks_for_bucket.push_back(dummyBlock);
        }

        random_device rd;
        mt19937 g(rd());
        shuffle(blocks_for_bucket.begin(), blocks_for_bucket.end(), g);

        bucket new_bucket(realBlockEachbkt, dummyBlockEachbkt);
        new_bucket.blocks = blocks_for_bucket;
        storage->SetBucket(bucket_pos, new_bucket);
    }
   
}

block pathoram::find_block_in_stash(int block_index) {
    for (auto& blk : stash) {
        if (blk.GetBlockindex() == block_index) {
            return blk;
        }
    }
    return dummyBlock;
}

void pathoram::remove_block_from_stash(int block_index) {
    for (auto it = stash.begin(); it != stash.end(); ++it) {
        if (it->GetBlockindex() == block_index) {
            stash.erase(it);
            break;
        }
    }
}

vector<char> pathoram::encrypt_data(const vector<char>& data) {
    if (!crypto || data.empty()) return data;

    // 像ringoram一样直接转换和加密
    vector<uint8_t> data_u8(data.begin(), data.end());
    auto encrypted_u8 = crypto->encrypt(data_u8);
    return vector<char>(encrypted_u8.begin(), encrypted_u8.end());
}

vector<char> pathoram::decrypt_data(const vector<char>& encrypted_data) {
    if (!crypto || encrypted_data.empty()) return encrypted_data;

    // 像ringoram一样检查数据大小
    if (encrypted_data.size() % 16 != 0) {
        cerr << "[DECRYPT] ERROR: Size " << encrypted_data.size() << " not multiple of 16" << endl;
        return encrypted_data;
    }

    try {
        vector<uint8_t> encrypted_u8(encrypted_data.begin(), encrypted_data.end());
        auto decrypted_u8 = crypto->decrypt(encrypted_u8);
        return vector<char>(decrypted_u8.begin(), decrypted_u8.end());
    }
    catch (const exception& e) {
        cerr << "[DECRYPT] ERROR: " << e.what() << endl;
        return encrypted_data;
    }
}

vector<char> pathoram::access(int block_index, Operation op, vector<char> data) {
    if (block_index < 0 || block_index >= N) {
        cerr << "ERROR: Block index " << block_index << " out of range [0, " << N - 1 << "]" << endl;
        return {};
    }


    // 1. 重新映射
    int old_leaf = position_map[block_index];
    int new_leaf = get_random_leaf();
    position_map[block_index] = new_leaf;

    // 2. 读取路径 - 计算通信开销
  
    for (int level = 0; level <= L; level++) {
        int bucket_pos = get_bucket_in_path(old_leaf, level);
        // 每个桶有 Z 个块，每个块大小是 param::block_size
      
    }
  

    read_path(old_leaf);
    

    // 3. 在stash中查找目标块并解密
    block target_block = find_block_in_stash(block_index);
    vector<char> result_data;

    if (target_block.GetBlockindex() != -1) {
        // 找到块，解密数据
        vector<char> encrypted_data = target_block.GetData();
        result_data = decrypt_data(encrypted_data);
        remove_block_from_stash(block_index);

        if (op == WRITE) {
            result_data = data;
        }

        // 重新加密并添加到stash
        vector<char> reencrypted_data = encrypt_data(result_data);
        stash.emplace_back(new_leaf, block_index, reencrypted_data);
    }
    else {
        // 未找到块
        if (op == READ) {
            result_data = vector<char>(blocksize, 0);
        }
        else {
            result_data = data;
            vector<char> encrypted_data = encrypt_data(result_data);
            stash.emplace_back(new_leaf, block_index, encrypted_data);
        }
    }

    // 4. 写回路径 - 计算通信开销
   
    for (int level = L; level >= 0; level--) {
        int bucket_pos = get_bucket_in_path(old_leaf, level);
    }
  

    write_path(old_leaf);


    return result_data;
}

