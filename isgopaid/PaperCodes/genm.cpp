#include<vector>
#include<iostream>
#include<fstream>
#include<string>

#include "SearchKey.hpp"

typedef unsigned int uint;

using namespace std;

int main(int argc, char* argv[])
{
    uint round = 0;

    for(int i = 0; i < argc; i++)
    {
	    if(!strcmp(argv[i], "-r")) round = atoi(argv[i+1]);
    }


    uint round_1 = round - 1; 
    vector<vector<vector<uint8_t>>> keypat(4096, vector<vector<uint8_t>>(round_1, vector<uint8_t>(64)));
    fstream rd;
    fstream wd; 
    string line;

    for(uint i = 0; i < 64; i++)
    {
        for(uint j = 0; j < 64; j++) 
        {
            rd.open("./"+ to_string(round) + "round_allkey/k"+to_string(i)+"_"+to_string(j)+".txt", ios::in);

            uint count = 0;

            while(getline(rd, line) && count != round_1)
            {
                if(line == "")
                    cout << "error" << endl;
 		if(int(count) >= 0)
                {
                    for(uint ind = 0; ind < 64; ind++)
                    {
                        keypat[64*i + j][count-0][ind] = char(int(line[ind])-48);
                    }
                }
                
                count++;

            }            
            
            rd.close();
        }
    }

    
    vector<vector<uint8_t>> input(64, vector<uint8_t>(64, 1));
    vector<vector<uint8_t>> output(64, vector<uint8_t>(64, 0));
    for (uint i = 0; i < 64; i++)
    {
        input[i][i] = 0;
        output[i][i] = 1;
    }
    

    //for(uint i = 0; i < 4096; i++)
    //{
    //    for(uint j = 0; j < round_1; j++)
    //    {
    //        for(uint k =0; k < 64; k++)
    //        {
    //            cout << int(keypat[i][j][k]);
    //        }
    //        cout << endl;
    //    }
    //    cout << endl;
    //}
    
    


    vector<vector<uint8_t>> matrix(4096, vector<uint8_t>(4096));    
    for(uint i = 0; i < 4096; i++)
    {
        for(uint j = 0; j < 64; j++)
        {
            for(uint k = 0; k < 64; k++)
            {
                cout << i << "  " << j << "  " << k << "  " << endl;
                auto nbtrail = counttrail(round, input[j], output[k], keypat[i]);
                if(nbtrail.second == 2)
                {
                    cout << "trail number： " << nbtrail.first << endl;
                
                    if(nbtrail.first % 2 == 1)
                    	matrix[i][64*j+k] = '1';
                    else
                        matrix[i][64*j+k] = '0';
                }
                else if(nbtrail.second == 9 || nbtrail.second == 10)
                    matrix[i][64*j+k] = '*';
		else if(nbtrail.second == 3)
                    matrix[i][64*j+k] = '0';
		else
                    matrix[i][64*j+k] = '#';

            }
        }
    }

    
    wd.open(to_string(round) + "matrix.txt", ios::out | ios::app);
    for(uint i = 0; i < 4096; i++)
    {
        for(uint j = 0; j < 4096; j++)
        {
            wd << matrix[i][j];
        }
        wd << endl;
    }
    wd.close();
    
    return 0;

}
