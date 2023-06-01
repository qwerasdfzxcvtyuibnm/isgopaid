#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <tuple>
#include <algorithm>
#include <utility>
#include <stdlib.h>     /* atoi */
#include <fstream>

#include "SearchKey.hpp"


using namespace std;
typedef unsigned int uint;

int main(int argc, char* argv[])
{

    int nbRounds = 0;
    uint end_start = 0;
    uint begin_start = 0;
    
    for(int i = 0; i < argc; i++)
    {
	    if(!strcmp(argv[i], "-r")) nbRounds = atoi(argv[i+1]);
	    if(!strcmp(argv[i], "-e")) end_start = atoi(argv[i+1]);
	    if(!strcmp(argv[i], "-b")) begin_start = atoi(argv[i+1]);
    }

    for(uint i = begin_start; i < (begin_start+1); i++)
    {
	   for(uint j = end_start; j < (end_start + 1); j++)
	   {
    		   fstream fd;
		   fd.open("./"+ to_string(nbRounds) +"result/k"+to_string(i)+"__"+to_string(j)+".txt", ios::out);
		   auto solution = imSearch(nbRounds, i, j);
		   cout << get<2>(solution).size() << endl;
		   if(get<2>(solution).size() > 0)
		   {
			fd << i  << "    " << j << "    has solution" << endl;
			cout << i  << "    " << j << "    has solution" << endl;
			for(uint index = 0; index < uint(nbRounds-1); index++)
			{
				for(uint index1 = 0; index1 < 64; index1++)
				{
					fd << int(get<2>(solution)[index][index1]);
					cout << int(get<2>(solution)[index][index1]);
				}
				fd << endl;
				cout << endl;
			}
		    }
		    else
		    {
			fd << i  << "    " << j << "    no solution" << endl;
			cout << i << "    " << j << "    no solution" << endl;
		    }

		    fd << endl;
		    fd.close();
	   }
    } 
    return 0;
}
