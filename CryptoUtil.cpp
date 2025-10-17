// CryptoUtils.cpp
#include "CryptoUtil.h"
#include <cryptopp/osrng.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/secblock.h>
#include <stdexcept>
#include <iostream>

using namespace CryptoPP;

CryptoUtils::CryptoUtils(const std::vector<uint8_t>& key) : key(key) {
    // 默认使用16字节IV
    this->iv = generateRandomIV(16);
}

CryptoUtils::CryptoUtils(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv)
    : key(key), iv(iv) {
    if (key.size() != 16 && key.size() != 24 && key.size() != 32) {
        throw std::invalid_argument("AES key must be 16, 24, or 32 bytes");
    }
    if (iv.size() != AES::BLOCKSIZE) {
        throw std::invalid_argument("IV must be 16 bytes for AES");
    }
}

std::vector<uint8_t> CryptoUtils::encrypt(const std::vector<uint8_t>& plaintext) {
    if (plaintext.empty()) {
        return {};
    }

    try {
        // 填充数据
        auto padded_data = padData(plaintext, AES::BLOCKSIZE);

        // 准备输出缓冲区
        std::vector<uint8_t> ciphertext(padded_data.size() + AES::BLOCKSIZE);

        // 使用AES CBC模式加密
        CBC_Mode<AES>::Encryption encryptor;
        encryptor.SetKeyWithIV(key.data(), key.size(), iv.data());

        // 执行加密
        ArraySource as(padded_data.data(), padded_data.size(), true,
            new StreamTransformationFilter(encryptor,
                new ArraySink(ciphertext.data(), ciphertext.size())
            )
        );

        // 调整大小到实际加密后的大小
        ciphertext.resize(ciphertext.size() - AES::BLOCKSIZE); // StreamTransformationFilter可能会添加额外块

        return ciphertext;
    }
    catch (const Exception& e) {
        std::cerr << "Crypto++ encryption error: " << e.what() << std::endl;
        return {};
    }
}

std::vector<uint8_t> CryptoUtils::decrypt(const std::vector<uint8_t>& ciphertext) {
    if (ciphertext.empty()) {
        return {};
    }

    try {
        // 准备输出缓冲区
        std::vector<uint8_t> plaintext(ciphertext.size() + AES::BLOCKSIZE);

        // 使用AES CBC模式解密
        CBC_Mode<AES>::Decryption decryptor;
        decryptor.SetKeyWithIV(key.data(), key.size(), iv.data());

        // 执行解密
        ArraySource as(ciphertext.data(), ciphertext.size(), true,
            new StreamTransformationFilter(decryptor,
                new ArraySink(plaintext.data(), plaintext.size())
            )
        );

        // 调整大小到实际解密后的大小
        size_t actual_size = 0;
        for (size_t i = 0; i < plaintext.size(); ++i) {
            if (plaintext[i] != 0) actual_size = i + 1;
        }
        plaintext.resize(actual_size);

        // 去除填充
        return unpadData(plaintext);
    }
    catch (const Exception& e) {
        std::cerr << "Crypto++ decryption error: " << e.what() << std::endl;
        return {};
    }
}

std::vector<uint8_t> CryptoUtils::generateRandomKey(size_t key_size) {
    if (key_size != 16 && key_size != 24 && key_size != 32) {
        throw std::invalid_argument("Key size must be 16, 24, or 32 bytes");
    }

    AutoSeededRandomPool rng;
    std::vector<uint8_t> key(key_size);
    rng.GenerateBlock(key.data(), key_size);
    return key;
}

std::vector<uint8_t> CryptoUtils::generateRandomIV(size_t iv_size) {
    AutoSeededRandomPool rng;
    std::vector<uint8_t> iv(iv_size);
    rng.GenerateBlock(iv.data(), iv_size);
    return iv;
}

std::vector<uint8_t> CryptoUtils::padData(const std::vector<uint8_t>& data, size_t block_size) {
    if (data.empty()) {
        return std::vector<uint8_t>(block_size, static_cast<uint8_t>(block_size));
    }

    size_t padding_needed = block_size - (data.size() % block_size);
    if (padding_needed == 0) {
        padding_needed = block_size;
    }

    std::vector<uint8_t> padded_data = data;
    padded_data.insert(padded_data.end(), padding_needed, static_cast<uint8_t>(padding_needed));
    return padded_data;
}

std::vector<uint8_t> CryptoUtils::unpadData(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return {};
    }

    uint8_t padding_value = data.back();
    if (padding_value == 0 || padding_value > data.size()) {
        return data; // 无效填充，返回原数据
    }

    // 检查填充是否有效
    for (size_t i = data.size() - padding_value; i < data.size(); ++i) {
        if (data[i] != padding_value) {
            return data; // 无效填充，返回原数据
        }
    }

    return std::vector<uint8_t>(data.begin(), data.end() - padding_value);
}

std::string CryptoUtils::bytesToHex(const std::vector<uint8_t>& bytes) {
    std::string hex;
    HexEncoder encoder(new StringSink(hex));
    encoder.Put(bytes.data(), bytes.size());
    encoder.MessageEnd();
    return hex;
}

std::vector<uint8_t> CryptoUtils::hexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    HexDecoder decoder(new VectorSink(bytes));
    decoder.Put(reinterpret_cast<const uint8_t*>(hex.data()), hex.size());
    decoder.MessageEnd();
    return bytes;
}
