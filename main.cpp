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


// ������д���ļ�
void writeToFile(const string& filename, const vector<char>& data) {
	ofstream outFile(filename, ios::out | ios::binary);
	if (!outFile) {
		cerr << "�޷����ļ�: " << filename << endl;
		return;
	}
	outFile.write(data.data(), data.size());
	outFile.close();
}

// ���ļ��ж�ȡ����
vector<char> readFromFile(const string& filename) {
	ifstream inFile(filename, ios::in | ios::binary);
	if (!inFile) {
		cerr << "�޷����ļ�: " << filename << endl;
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
    
	// 1. ��ʼ��ORAM����
	const int bucket_size = 4;
	const int num_blocks = 10000;
	const int test_blocks = 1000; // Ҫ���ԵĿ�����

	// 2. ����ORAMʵ��
	ServerStorage* storage = new ServerStorage();
	RandomForOram* rand_gen = new RandomForOram();
	MultiPathOram oram(storage, rand_gen, bucket_size, num_blocks);

	cout << "ORAM��ʼ�����" << endl;
	cout << "������: " << oram.getNumLevels() << ", Ҷ�ӽڵ���: " << oram.getNumLeaves() << endl;
	
	// 3. ׼���������� - ����д��
	vector<pair<OramInterface::Operation, int>> write_ops;
	vector<vector<char>> write_data;

	cout << "\n׼��д���������..." << endl;
	for (int i = 0; i < test_blocks; i++) {
		write_ops.push_back({ OramInterface::Operation::WRITE, i });

		// ���ɲ������ݣ�ÿ�������ݲ�ͬ
		vector<char> data;
		string data_str = "block_" + to_string(i) + "_data";
		data.assign(data_str.begin(), data_str.end());
		write_data.push_back(data);

		
	}

	// 4. ִ������д��
	auto write_results = oram.multiaccess(write_ops, write_data);
	cout << "\n����д����ɣ�д��������: " << write_results.size() << endl;

	// 5. ׼����ȡ���� - ��ȡ�ղ�д��Ŀ�
	vector<pair<OramInterface::Operation, int>> read_ops;
	vector<vector<char>> read_data; // ��ȡ����Ҫ���ݣ�ȫ��Ϊ��

	cout << "\n׼����ȡ��������..." << endl;
	for (int i = 0; i < test_blocks; i++) {
		read_ops.push_back({ OramInterface::Operation::READ, i }); // ��ȡ��0����4
		read_data.push_back({}); // ��Ӧ������
	}

	// 6. ִ��������ȡ
	auto read_results = oram.multiaccess(read_ops, read_data);
	cout << "\n������ȡ��ɣ���ȡ�������: " << read_results.size() << endl;

	// 7. ��֤��ȡ���
	cout << "\n��֤��ȡ���..." << endl;
	int success_count = 0;
	for (int i = 0; i < test_blocks; i++) {
		string expected(write_data[i].begin(), write_data[i].end());
		string actual(read_results[i].begin(), read_results[i].end());

		cout << "��" << i << ": ";
		if (actual == expected) {
			cout << "��֤�ɹ�! ����: " << actual << endl;
			success_count++;
		}
		else {
			cout << "��֤ʧ��! ����: " << expected << " ʵ��: " << actual << endl;
		}
	}

	// 8. �����ܽ�
	cout << "\n�����ܽ�:" << endl;
	cout << "�ܲ��Կ���: " << test_blocks << endl;
	cout << "�ɹ���֤����: " << success_count << endl;
	cout << "ʧ�ܿ���: " << (test_blocks - success_count) << endl;
	cout << "��ǰstash��С: " << oram.getStashSize() << endl;

	// 9. ������Դ
	delete storage;
	delete rand_gen;

	return 0;
}