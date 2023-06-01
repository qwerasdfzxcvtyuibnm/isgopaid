#include "SearchKey.hpp"

#define MAX_GRBMIPGapAbs 8

#define MAX_NUMBER_SEARCHKEY 500

#define TIME_LIMIT_SEARCHKEY 300

#define MAX_GRB_THREAD 8

#define ENFORCE_NONZERO_LASTKEY true 
#define ENFORCE_NONZERO_FIRSTKEY true 

#define WEIGHT_THRESHOLD_STEP1 40 

#define DYNAMIC_STEP1_MIPFOCUS 1 

#define AGRESSIVE_STRATEGY_MINDEGREE false 

#define MAX_NUMBER_SOLCOUNT_FULLCIPHER 100000

#define TIME_LIMIT_COUNTTRAILS_FULLCIPER 60


using namespace std;

typedef unsigned int uint;
typedef pair<uint32_t,uint32_t> pairuint32;
typedef tuple<uint32_t, uint32_t, uint64_t> tup3uint;

std::random_device rd; 
std:: mt19937 prng(rd()); 
std::uniform_int_distribution<uint> randintSeedGurobi(0, 1999999999);
std::uniform_int_distribution<uint> randintCilumn;


tuple<vector<uint8_t>, vector<uint8_t>, vector<vector<uint8_t>>>
imSearch(uint const rMax, uint const indexinput, uint const indexoutput) 
{

	uint blockSize = 64;

	
	GRBEnv gurobiEnv;
	gurobiEnv.set(GRB_IntParam_OutputFlag, 0);
	

	vector<uint8_t> output(blockSize, 0);
	vector<uint8_t> input(blockSize, 1);
	output[indexoutput] = 1;
	input[indexinput] = 0;

	int rMax_minus_1 = (int)(rMax) - 1;
	int rMax_minus_2 = (int)(rMax) - 2;




	int rMiddle = rMax_minus_2;

	GRBModel m(gurobiEnv, "./modelp/" + to_string(rMax)+"_present.mps");
	m.set(GRB_IntParam_LazyConstraints, 1);
	m.set(GRB_IntParam_MIPFocus, 1);

	for(uint i = 0; i < blockSize; i++)
	{
		m.addConstr(m.getVarByName("x_0_" + to_string(i)) == input[i]);
		m.addConstr(m.getVarByName("y_" + to_string(rMax_minus_1) +"_" +to_string(i)) == output[i]);
	}

	GRBLinExpr sumlastkey(0);
	for(uint i = 0; i < blockSize; i++)
	{
		sumlastkey += m.getVarByName("k_" + to_string(rMax_minus_2) + "_" + to_string(i));
		sumlastkey += m.getVarByName("k_" + to_string(rMax_minus_2 - 1) + "_" + to_string(i));
	}
	m.addConstr(sumlastkey >= 1);  

	GRBLinExpr sumlastkey1(0);
	for(uint i = 0; i < blockSize; i++)
	{
		sumlastkey1 += m.getVarByName("k_0_" + to_string(i));
		sumlastkey1 += m.getVarByName("k_1_" + to_string(i));
	}
	m.addConstr(sumlastkey1 >= 1); 

	vector<GRBVar> xmidvar(blockSize);
	vector<GRBVar> kmidvar(blockSize);
	vector<GRBVar> ymidvar(blockSize);
	for(uint i = 0; i < blockSize; i++)
	{
		xmidvar[i] = m.getVarByName("x_"+to_string(rMiddle)+"_"+to_string(i));
		kmidvar[i] = m.getVarByName("k_" + to_string(rMiddle) + "_" + to_string(i));
		ymidvar[i] = m.getVarByName("y_" + to_string(rMiddle+1) + "_" + to_string(i));
	}

	GRBLinExpr objExpr(0);
	for(uint i = 0; i < blockSize; i++)
	{
		objExpr += kmidvar[i];
	}
	m.setObjective(objExpr, GRB_MAXIMIZE);

	auto seedGurobi = randintSeedGurobi(prng);
	m.set(GRB_IntParam_Seed, seedGurobi);

	vector<vector<GRBVar>> invar(rMax-rMiddle-1, vector<GRBVar>(blockSize));
	vector<vector<GRBVar>> outvar(rMax-rMiddle-1, vector<GRBVar>(blockSize));
	vector<vector<GRBVar>> keyvar(rMax-rMiddle-1, vector<GRBVar>(blockSize));
	for(uint r = 0; r < rMax - rMiddle -1; r++)
	{
		auto & invar_r = invar[r];
		auto & outvar_r = outvar[r];
		auto & keyvar_r = keyvar[r];

		for(uint i = 0; i < blockSize; i++)
		{
			invar_r[i] = m.getVarByName("x_"+to_string(r+rMiddle)+"_"+to_string(i));
			outvar_r[i] = m.getVarByName("y_"+to_string(r+rMiddle+1)+"_"+to_string(i));
			keyvar_r[i] = m.getVarByName("k_"+to_string(r+rMiddle)+"_"+to_string(i));
		}
	}
	

	m.set(GRB_IntParam_Threads, 8);
	m.set(GRB_IntParam_PoolSearchMode, 2);
	m.set(GRB_IntParam_PoolSolutions, 2000000000);
	m.set(GRB_DoubleParam_TimeLimit, 60);
	m.set(GRB_IntParam_SolutionLimit, 50);


	callbackimSearch cb (invar, outvar, keyvar);
	m.setCallback(&cb);
	m.update();
	m.optimize();

	
	if(cb.allsolution.size() == 0)
	{
		return make_tuple(vector<uint8_t>(), vector<uint8_t>(), vector<vector<uint8_t>>());
	}



	

	stack<tuple<uint, vector<uint8_t>, vector<uint8_t>>> tree;
	for(pair<vector<uint8_t>, vector<uint8_t>> it : cb.allsolution)
	{
		for(uint i = 0; i < 64; i++)
		{
			cout << int(it.first[i]);
		}
		cout << "  ";
		for(uint i = 0; i < 64; i++)
		{
			cout << int(it.second[i]);
		}
		cout << endl;
	}	
	for(pair<vector<uint8_t>, vector<uint8_t>> it : cb.allsolution)
	{
		tree.push(make_tuple(rMiddle, it.first, it.second));
	}	


	vector<vector<uint8_t>> valk(rMax_minus_1, vector<uint8_t>(blockSize, 0));
	vector<vector<uint8_t>> valx(rMax_minus_1, vector<uint8_t>(blockSize, 0));

	uint i = 0;
	while(!tree.empty())
	{
		auto element = tree.top();  
		tree.pop();
		
		int hw = 0;
		for(uint i = 0; i < blockSize; i++)
		{
			hw += int(get<2>(element)[i]);
		}	


		if (hw >= 40)
		{	
			for(uint i = 0; i < blockSize; i++)
			{
				valk[get<0>(element)][i] = get<1>(element)[i];
				cout << int(get<1>(element)[i]);
				valx[get<0>(element)][i] = get<2>(element)[i];
			}
			cout << endl;

			uint round_index = get<0>(element) - 1;
			auto keypattern_front = searchKeyall(rMax, round_index, indexinput, indexoutput, valk, valx);

			if (keypattern_front.size() > 0)
				return make_tuple(vector<uint8_t>(), vector<uint8_t>(), keypattern_front);
			else
				continue;

		}

		else
		{
			cout << i++  << endl;
			for(uint i = 0; i < blockSize; i++)
			{
				valk[get<0>(element)][i] = get<1>(element)[i];
				valx[get<0>(element)][i] = get<2>(element)[i];
			}
			uint round_index = get<0>(element) - 1;
			auto solution = backsearch(rMax, round_index, indexinput, indexoutput, valk, valx);

			if(solution.size() > 0)
			{
				if(round_index == 0)
				{
					for(pair<vector<uint8_t>, vector<uint8_t>> it : solution)
					{
						for(uint i = 0; i < blockSize; i++)
						{
							valk[0][i] = it.first[i];
							valx[0][i] = it.second[i];
						}
						break;
					}
					return make_tuple(vector<uint8_t>(), vector<uint8_t>(), valk);
				}
				else
				{


					for(pair<vector<uint8_t>, vector<uint8_t>> it : solution)
					{
						tree.push(make_tuple(round_index, it.first, it.second));
					}	


				}
			}
			else
			{
				continue;
			}
		}
	}

	
	return make_tuple(vector<uint8_t>(), vector<uint8_t>(), vector<vector<uint8_t>>());
}


vector<vector<uint8_t>>
searchKeyall(uint rMax, uint rMiddle, uint indexinput, uint indexoutput, vector<vector<uint8_t>> & valk, vector<vector<uint8_t>> & valx)
{


	uint blockSize = 64;
	vector<uint8_t> input(blockSize, 1);
	vector<uint8_t> output(blockSize, 0);
	
	input[indexinput] = 0;
	output[indexoutput] = 1;

	uint rMax_minus_1 = rMax-1;
	uint rMax_minus_2 = rMax -2;

	GRBEnv gurobiEnv;
	gurobiEnv.set(GRB_IntParam_OutputFlag, 0);
	GRBModel m(gurobiEnv, "./modelp/" + to_string(rMax) +"_present.mps");

	m.set(GRB_IntParam_LazyConstraints, 1);
	m.set(GRB_IntParam_MIPFocus, 1);

	for(uint i = 0; i < blockSize; i++)
	{
		m.addConstr(m.getVarByName("x_0_" + to_string(i)) == input[i]);
		m.addConstr(m.getVarByName("y_" + to_string(rMax_minus_1) +"_" +to_string(i)) == output[i]);
	}

	for(uint r = rMiddle+1; r < rMax_minus_1; r++)
	{
		auto const & valx_r = valx[r];
		for(uint i = 0; i < blockSize; i++)
			m.addConstr(m.getVarByName("x_" + to_string(r) + "_" +to_string(i)) == valx_r[i]);
	}


	for(uint r = rMiddle+1; r < rMax_minus_1; r++)
	{
		auto const & valk_r = valk[r];
		for(uint i = 0; i < blockSize; i++)
			m.addConstr(m.getVarByName("k_" + to_string(r) + "_" +to_string(i)) == valk_r[i]);
	}
	vector<vector<GRBVar>> allkey(rMax - 1, vector<GRBVar>(blockSize));

	for(uint r = 0; r < rMax-1; r++)
	{
		for(uint i = 0; i < blockSize; i++)
		{
			allkey[r][i] = m.getVarByName("k_"+to_string(r)+"_"+to_string(i));
		}
	}
	vector<GRBVar> invar(blockSize);
	vector<GRBVar> outvar(blockSize);
	for(uint i = 0; i < blockSize; i++)
	{
		invar[i] = m.getVarByName("x_0_"+to_string(i));
		outvar[i] = m.getVarByName("y_"+to_string(rMax_minus_1)+"_"+to_string(i));
	}



	GRBLinExpr sumlastkey1(0);
	for(uint i = 0; i < blockSize; i++)
	{
		sumlastkey1 += m.getVarByName("k_0_" + to_string(i));
		sumlastkey1 += m.getVarByName("k_1_" + to_string(i));
	}
	m.addConstr(sumlastkey1 >= 1);  


	GRBLinExpr sumlastkey(0);
	for(uint i = 0; i < blockSize; i++)
	{
		sumlastkey += m.getVarByName("k_" + to_string(rMax_minus_2) + "_" + to_string(i));
		sumlastkey += m.getVarByName("k_" + to_string(rMax_minus_2 - 1) + "_" + to_string(i));
	}
	m.addConstr(sumlastkey >= 1);  




	GRBLinExpr objExpr(0);
	for(uint i = 0; i < blockSize; i++)
	{
		objExpr += invar[i];
	}
	m.setObjective(objExpr, GRB_MAXIMIZE);

	auto seedGurobi = randintSeedGurobi(prng);
	m.set(GRB_IntParam_Seed, seedGurobi);
    	m.set(GRB_IntParam_Threads, 8);
	m.set(GRB_IntParam_PoolSearchMode, 2);
	m.set(GRB_IntParam_PoolSolutions, 2000000000);
	m.set(GRB_DoubleParam_TimeLimit, 60);
	m.set(GRB_IntParam_SolutionLimit, 1);


	callbacksearchKeyall cb (rMax, invar, outvar, allkey);
	m.setCallback(&cb);
	m.update();
	m.optimize();

        uint const nbsol = m.get(GRB_IntAttr_SolCount);

	if(nbsol > 0)
	{
		vector<vector<uint8_t>> Allkeyval(rMax-1, vector<uint8_t>(blockSize));

		m.set(GRB_IntParam_SolutionNumber, 0);
		for(uint r = 0; r < uint(rMax-1); r++){
			for(uint i = 0; i < blockSize; i++)
				Allkeyval[r][i] = uint8_t(round(allkey[r][i].get(GRB_DoubleAttr_Xn)));
			}
		
		return Allkeyval;
	 }
	 else{

		return vector<vector<uint8_t>>();
	}
	return vector<vector<uint8_t>>();;
}



set<pair<vector<uint8_t>, vector<uint8_t>>>
backsearch(uint rMax, uint rMiddle, uint indexinput, uint indexoutput, vector<vector<uint8_t>> & valk, vector<vector<uint8_t>> & valx)
{


	uint blockSize = 64;
	vector<uint8_t> input(blockSize, 1);
	vector<uint8_t> output(blockSize, 0);
	
	input[indexinput] = 0;
	output[indexoutput] = 1;

	uint rMax_minus_1 = rMax-1;
	uint rMax_minus_2 = rMax-2;

	GRBEnv gurobiEnv;
	gurobiEnv.set(GRB_IntParam_OutputFlag, 0);
	GRBModel m(gurobiEnv, "./modelp/" + to_string(rMax) +"_present.mps");

	m.set(GRB_IntParam_LazyConstraints, 1);
	m.set(GRB_IntParam_MIPFocus, 1);

	for(uint i = 0; i < blockSize; i++)
	{
		m.addConstr(m.getVarByName("x_0_" + to_string(i)) == input[i]);
		m.addConstr(m.getVarByName("y_" + to_string(rMax_minus_1) +"_" +to_string(i)) == output[i]);
	}

	for(uint r = rMiddle+1; r < rMax_minus_1; r++)
	{
		auto const & valx_r = valx[r];
		for(uint i = 0; i < blockSize; i++)
			m.addConstr(m.getVarByName("x_" + to_string(r) + "_" +to_string(i)) == valx_r[i]);
	}

	for(uint r = rMiddle+1; r < rMax_minus_1; r++)
	{
		auto const & valk_r = valk[r];
		for(uint i = 0; i < blockSize; i++)
			m.addConstr(m.getVarByName("k_" + to_string(r) + "_" +to_string(i)) == valk_r[i]);
	}

	vector<GRBVar> xmidvar(blockSize);
	vector<GRBVar> kmidvar(blockSize);
	vector<GRBVar> ymidvar(blockSize);
	vector<GRBVar> ymidvar1(blockSize);
	for(uint i = 0; i < blockSize; i++)
	{
		xmidvar[i] = m.getVarByName("x_"+to_string(rMiddle)+"_"+to_string(i));
		kmidvar[i] = m.getVarByName("k_" + to_string(rMiddle) + "_" + to_string(i));
		ymidvar[i] = m.getVarByName("y_" + to_string(rMiddle+1) + "_" + to_string(i));
		ymidvar1[i] = m.getVarByName("y_" + to_string(rMiddle) + "_" +to_string(i));
	}
	
	
	GRBLinExpr sumlastkey1(0);
	for(uint i = 0; i < blockSize; i++)
	{
		sumlastkey1 += m.getVarByName("k_0_" + to_string(i));
		sumlastkey1 += m.getVarByName("k_1_" + to_string(i));
	}
	m.addConstr(sumlastkey1 >= 1);  


	GRBLinExpr sumlastkey(0);
	for(uint i = 0; i < blockSize; i++)
	{
		sumlastkey += m.getVarByName("k_" + to_string(rMax_minus_2) + "_" + to_string(i));
		sumlastkey += m.getVarByName("k_" + to_string(rMax_minus_2 - 1) + "_" + to_string(i));
	}
	m.addConstr(sumlastkey >= 1);  






	GRBLinExpr objExpr(0);
	for(uint i = 0; i < blockSize; i++)
	{
		objExpr += kmidvar[i];
	}
	m.setObjective(objExpr, GRB_MAXIMIZE);

	auto seedGurobi = randintSeedGurobi(prng);
	m.set(GRB_IntParam_Seed, seedGurobi);

	vector<vector<GRBVar>> invar(rMax-rMiddle-1, vector<GRBVar>(blockSize));
	vector<vector<GRBVar>> outvar(rMax-rMiddle-1, vector<GRBVar>(blockSize));
	vector<vector<GRBVar>> keyvar(rMax-rMiddle-1, vector<GRBVar>(blockSize));
	for(uint r = 0; r < rMax - rMiddle -1; r++)
	{
		auto & invar_r = invar[r];
		auto & outvar_r = outvar[r];
		auto & keyvar_r = keyvar[r];
	
		for(uint i = 0; i < blockSize; i++)
		{
			invar_r[i] = m.getVarByName("x_"+to_string(r+rMiddle)+"_"+to_string(i));
			outvar_r[i] = m.getVarByName("y_"+to_string(r+rMiddle+1)+"_"+to_string(i));
			keyvar_r[i] = m.getVarByName("k_"+to_string(r+rMiddle)+"_"+to_string(i));
		}
	}
		
    	m.set(GRB_IntParam_Threads, 8);
	m.set(GRB_IntParam_PoolSearchMode, 2);
	m.set(GRB_IntParam_PoolSolutions, 2000000000);
	m.set(GRB_DoubleParam_TimeLimit, 60);
	m.set(GRB_IntParam_SolutionLimit, 10);


	callbackimSearch cb (invar, outvar, keyvar);
	m.setCallback(&cb);
	m.update();
	m.optimize();
	return cb.allsolution;
}


pair<uint64_t,int> counttrail(uint rounds,
		    vector<uint8_t> & u,
                    vector<uint8_t> & v,
                    vector<vector<uint8_t>> & k)
{
    GRBEnv gurobiEnv;
    gurobiEnv.set(GRB_IntParam_OutputFlag, 0);
    GRBModel m(gurobiEnv, "./modelp/" + to_string(rounds)+".mps");
    m.set(GRB_IntParam_Threads, 8);

    vector<GRBVar> invar(64);
    vector<GRBVar> outvar(64);
    vector<vector<GRBVar>> keyvar(rounds-1, vector<GRBVar>(64));
    for (uint i = 0; i < 64; i++)
    {
        invar[i] = m.getVarByName("x_0_"+to_string(i));
        outvar[i] = m.getVarByName("y_"+to_string(rounds-1)+"_" + to_string(i));

        m.addConstr(invar[i] == u[i]);
        m.addConstr(outvar[i] == v[i]);

    }
    for(uint r = 0; r < rounds - 1;r++)
    { 
	    for (uint i = 0; i < 64; i++)
	    {
		keyvar[r][i] = m.getVarByName("k_"+to_string(r)+"_"+to_string(i));

		m.addConstr(keyvar[r][i] == k[r][i]);
	    }
    }

    m.set(GRB_IntParam_PoolSearchMode, 2);
    m.set(GRB_IntParam_PoolSolutions , 2000000000);
    m.set(GRB_IntParam_SolutionLimit, 2000000000);
    m.set(GRB_DoubleParam_TimeLimit, 60);
    m.optimize();
    int optimized;
    uint64_t nbtrail = 0;
    if(m.get(GRB_IntAttr_Status) == GRB_OPTIMAL)
    {
        nbtrail = m.get(GRB_IntAttr_SolCount);
	optimized = int(m.get(GRB_IntAttr_Status));
    }

    else
    {
	    cout << "__reason:" << m.get(GRB_IntAttr_Status) << endl;
	    optimized = int(m.get(GRB_IntAttr_Status));
    }
    return make_pair(nbtrail, optimized);
}


