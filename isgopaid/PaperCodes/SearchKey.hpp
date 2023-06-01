#ifndef H_SEARCHKEY
#define H_SEARCHKEY

#include <cstdint>
#include <cstring>
#include <vector>
#include <set>
#include <stack>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <cmath>
#include <algorithm>
#include <omp.h>
#include <bitset>
#include <random>
#include <tuple>
#include <map>


#include "gurobi_c++.h"
#include "Callback.hpp"






std::tuple<std::vector<uint8_t>, std::vector<uint8_t>, std::vector<std::vector<uint8_t>>>
imSearch(uint const rMax, uint const indexoutput, uint const indexinput); 


std::vector<std::vector<uint8_t>>
searchKeyall(uint rMax, uint rMiddle, uint indexinput, uint indexoutput, std::vector<std::vector<uint8_t>> & valk, std::vector<std::vector<uint8_t>> & valx); 

std::set<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>>
backsearch(uint rMax, uint rMiddle, uint indexinput, uint indexoutput, std::vector<std::vector<uint8_t>> & valk, std::vector<std::vector<uint8_t>> & valx); 


std::pair<uint64_t, int> counttrail(uint rounds, std::vector<uint8_t> & u, std::vector<uint8_t> & v, std::vector<std::vector<uint8_t>> & k);

#endif
