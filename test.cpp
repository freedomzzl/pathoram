#include<iostream>
#include"recursive_pathoram"


void testRecursiveringoram()
{
    ServerStorage storage;
    recursive_pathoram oram(2000, &storage);


    for (int i = 0; i < 200; i++)
    {
        string test_data = "REAL_RINGORAM_TEST";
        test_data.resize(blocksize, 'R');
        vector<char> write_data(test_data.begin(), test_data.end());
        oram.access(i, ringoram::WRITE, write_data);
    }

    for (int i = 0; i < 200; i++)
    {
        vector<char> read_data = oram.access(i, ringoram::READ, {});
        cout << "read!" << endl;
        string result(read_data.begin(), read_data.end());
        std::cout << result << std::endl;
    }
}


int main()
{
       try {
    
    testRecursiveringoram();
}
      catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
}

return 0;



}
