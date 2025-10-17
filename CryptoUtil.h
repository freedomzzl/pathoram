// CryptoUtils.h
#pragma once
#include <vector>
#include <cstdint>
#include <string>

class CryptoUtils {
private:
    std::vector<uint8_t> key;
    std::vector<uint8_t> iv; // 对于CBC模式需要IV

public:
    CryptoUtils(const std::vector<uint8_t>& key);
    CryptoUtils(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);

    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plaintext);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext);

    static std::vector<uint8_t> generateRandomKey(size_t key_size = 16);
    static std::vector<uint8_t> generateRandomIV(size_t iv_size = 16);
    static std::vector<uint8_t> padData(const std::vector<uint8_t>& data, size_t block_size);
    static std::vector<uint8_t> unpadData(const std::vector<uint8_t>& data);

    // 工具函数
    static std::string bytesToHex(const std::vector<uint8_t>& bytes);
    static std::vector<uint8_t> hexToBytes(const std::string& hex);
};