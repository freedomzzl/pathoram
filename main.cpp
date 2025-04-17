#include"block.h"
#include<vector>
#include<iostream>
#include<string>
#include<fstream>
#include"Bucket.h"
#include"PathOram.h"
#include"RandForOramInterface.h"
#include"UntrustedStorageInterface.h"
#include"OramInterface.h"
#include"ServerStorage.h"
#include "RandomForOram.h"
#include"RecursivePathOram.h"
#include"MultiPathOram.h"


// 将数据写入文件
void writeToFile(const string& filename, const vector<char>& data) {
	ofstream outFile(filename, ios::out | ios::binary);
	if (!outFile) {
		cerr << "无法打开文件: " << filename << endl;
		return;
	}
	outFile.write(data.data(), data.size());
	outFile.close();
}

// 从文件中读取数据
vector<char> readFromFile(const string& filename) {
	ifstream inFile(filename, ios::in | ios::binary);
	if (!inFile) {
		cerr << "无法打开文件: " << filename << endl;
		return {};
	}
	inFile.seekg(0, ios::end);
	size_t size = inFile.tellg();
	inFile.seekg(0, ios::beg);
	vector<char> data(size);
	inFile.read(data.data(), size);
	inFile.close();
	return data;
}
using namespace std;
int main()
{
    
	// 1. 初始化ORAM参数
	const int bucket_size = 4;
	const int num_blocks = 10000;
	const int test_blocks = 1000; // 要测试的块数量

	// 2. 创建ORAM实例
	ServerStorage* storage = new ServerStorage();
	RandomForOram* rand_gen = new RandomForOram();
	MultiPathOram oram(storage, rand_gen, bucket_size, num_blocks);

	cout << "ORAM初始化完成" << endl;
	cout << "树层数: " << oram.getNumLevels() << ", 叶子节点数: " << oram.getNumLeaves() << endl;
	
	// 3. 准备测试数据 - 批量写入
	vector<pair<OramInterface::Operation, int>> write_ops;
	vector<vector<char>> write_data;

	cout << "\n准备写入测试数据..." << endl;
	for (int i = 0; i < test_blocks; i++) {
		write_ops.push_back({ OramInterface::Operation::WRITE, i });

		// 生成测试数据，每个块数据不同
		vector<char> data;
		string data_str = "block_" + to_string(i) + "_data";
		data.assign(data_str.begin(), data_str.end());
		write_data.push_back(data);

		
	}

	// 4. 执行批量写入
	auto write_results = oram.multiaccess(write_ops, write_data);
	cout << "\n批量写入完成，写入结果数量: " << write_results.size() << endl;

	// 5. 准备读取操作 - 读取刚才写入的块
	vector<pair<OramInterface::Operation, int>> read_ops;
	vector<vector<char>> read_data; // 读取不需要数据，全部为空

	cout << "\n准备读取测试数据..." << endl;
	for (int i = 0; i < test_blocks; i++) {
		read_ops.push_back({ OramInterface::Operation::READ, i }); // 读取块0到块4
		read_data.push_back({}); // 对应空数据
	}

	// 6. 执行批量读取
	auto read_results = oram.multiaccess(read_ops, read_data);
	cout << "\n批量读取完成，读取结果数量: " << read_results.size() << endl;

	// 7. 验证读取结果
	cout << "\n验证读取结果..." << endl;
	int success_count = 0;
	for (int i = 0; i < test_blocks; i++) {
		string expected(write_data[i].begin(), write_data[i].end());
		string actual(read_results[i].begin(), read_results[i].end());

		cout << "块" << i << ": ";
		if (actual == expected) {
			cout << "验证成功! 数据: " << actual << endl;
			success_count++;
		}
		else {
			cout << "验证失败! 期望: " << expected << " 实际: " << actual << endl;
		}
	}

	// 8. 测试总结
	cout << "\n测试总结:" << endl;
	cout << "总测试块数: " << test_blocks << endl;
	cout << "成功验证块数: " << success_count << endl;
	cout << "失败块数: " << (test_blocks - success_count) << endl;
	cout << "当前stash大小: " << oram.getStashSize() << endl;

	// 9. 清理资源
	delete storage;
	delete rand_gen;

	return 0;
}