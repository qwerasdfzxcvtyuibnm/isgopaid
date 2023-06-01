#ifndef H_CALLBACK                                                                                                                                                   
#define H_CALLBACK

#include<vector>
#include<iostream>
#include<cmath>
#include<cstdint>


#include "gurobi_c++.h"
#include "SearchKey.hpp"



class callbackimSearch: public GRBCallback
{
    public:
        std::vector<std::vector<GRBVar>> invar;
        std::vector<std::vector<GRBVar>> outvar;
        std::vector<std::vector<GRBVar>> keyvar;
	std::set<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> allsolution;
	int count;

        callbackimSearch( std::vector<std::vector<GRBVar>> const & xinvar, std::vector<std::vector<GRBVar>> const & xoutvar, std::vector<std::vector<GRBVar>> const & xkeyvar);

    protected:
        void callback();
};


class callbacksearchKeyall: public GRBCallback

{
    public:
	uint rMax;
        std::vector<GRBVar> invar;
        std::vector<GRBVar> outvar;
	std::vector<std::vector<GRBVar>> keyvar;
        //uint64_t ctrsolution;
        //std::vector<std::vector<boost::dynamic_bitset<>>> allSolution;

        callbacksearchKeyall( uint const xrMax, std::vector<GRBVar> const & xinvar, std::vector<GRBVar> const & xoutvar, std::vector<std::vector<GRBVar>> const & xkeyvar);

    protected:
        void callback();
};



#endif

