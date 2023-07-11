#include "Callback.hpp"

using namespace std;

typedef unsigned int uint;


callbackimSearch::callbackimSearch(vector<vector<GRBVar>> const & xinvar,
				vector<vector<GRBVar>> const & xoutvar,
				vector<vector<GRBVar>> const & xkeyvar):
	invar(xinvar),
	outvar(xoutvar),
	keyvar(xkeyvar),
	allsolution(),
	count(0)
{}

void callbackimSearch::callback()
{

	
	try{
	
		if(where == GRB_CB_MIPSOL)
		{
			count++;

			uint blockSize = 64;
			
			GRBEnv gurobiEnv;
			gurobiEnv.set(GRB_IntParam_OutputFlag, 0);

			GRBModel m(gurobiEnv, "./modelp/2_present.mps");


			vector<uint8_t> keysolution (blockSize);
			vector<uint8_t> insolution (blockSize);
			vector<uint8_t> outsolution (blockSize);
			for(uint i = 0; i < blockSize; i++)
			{
				keysolution[i] = uint(round(getSolution(keyvar[0][i])));
				insolution[i] = uint(round(getSolution(invar[0][i])));
				outsolution[i] = uint(round(getSolution(outvar[0][i])));
			}
			

                        vector<vector<uint8_t>> allkey(int(invar.size()), vector<uint8_t>(blockSize));
                        for(uint r =0; r < invar.size(); r++)
                        {
                                for(uint i = 0; i < blockSize; i++)
                                {
                                        allkey[r][i] = uint(round(getSolution(keyvar[r][i])));
                                }
                        }




			for(uint i = 0; i < blockSize; i++)
			{
				m.addConstr(m.getVarByName("x_0_"+to_string(i)) == uint(round(getSolution(invar[0][i]))));
				m.addConstr(m.getVarByName("y_1_"+to_string(i)) == uint(round(getSolution(outvar[0][i]))));
				m.addConstr(m.getVarByName("k_0_"+to_string(i)) == uint(round(getSolution(keyvar[0][i]))));
			}

			m.set(GRB_IntParam_PoolSearchMode, 2);
			m.set(GRB_IntParam_PoolSolutions, 200000000);
			m.set(GRB_IntParam_SolutionLimit,10);
			m.set(GRB_DoubleParam_TimeLimit, 60);

			GRBLinExpr objExpr(0);
			for(uint i = 0; i < blockSize; i++)
			{
				objExpr += m.getVarByName("x_0_"+to_string(i));
			}
			m.setObjective(objExpr);

			m.set(GRB_IntParam_Threads,8);
			m.update();
			m.optimize();

			bool oddtra = true;

			if(m.get(GRB_IntAttr_Status) == GRB_OPTIMAL)
			{
				uint nbTrail = m.get(GRB_IntAttr_SolCount);
				cout << endl << nbTrail << "trails " << endl;

				if(nbTrail % 2== 0)
				{
					oddtra = false;
					GRBLinExpr cutExpr(0);
					for(uint i = 0; i < blockSize; i++)
					{
						if(keysolution[i] == 0) cutExpr += keyvar[0][i];
						else cutExpr += (1-keyvar[0][i]);
						if(insolution[i] == 0) cutExpr += invar[0][i];
						else cutExpr += (1-invar[0][i]);
						if(outsolution[i] == 0) cutExpr += outvar[0][i];
						else cutExpr += (1-outvar[0][i]);
					}
					addLazy( cutExpr >= 1);
				}
				else
				{

					for(uint i = 0; i < blockSize; i++)
					{
						cout << int(keysolution[i]);
					}
					cout << endl;
				}
			}
			else
			{
				oddtra = false;
			}
			
			if(oddtra)
			{
				auto lastround = invar.size();
				printf("%ld round \n", lastround);

			
				
				GRBModel m(gurobiEnv, "./modelp/" + to_string(lastround + 1) +"_present.mps");

			        for(uint i = 0; i < blockSize; i++)
				{
					m.addConstr(m.getVarByName("x_0_"+to_string(i)) == int(round(getSolution(invar[0][i]))));
					m.addConstr(m.getVarByName("y_"+to_string(lastround)+"_"+to_string(i)) == int(round(getSolution(outvar[lastround-1][i]))));
				}

				for(uint r = 0; r < lastround; r++)
				{
					for(uint i = 0; i < blockSize; i++)
					{
						m.addConstr(m.getVarByName("k_"+to_string(r)+"_"+to_string(i)) == int(round(getSolution(keyvar[r][i]))));
					}
				}

				m.set(GRB_IntParam_PoolSearchMode, 2);
				m.set(GRB_IntParam_PoolSolutions, 200000000);
				m.set(GRB_IntParam_SolutionLimit, 50000);
				m.set(GRB_DoubleParam_TimeLimit, 10);

				GRBLinExpr objExpr(0);
				for(uint i = 0; i < blockSize; i++)
				{
					objExpr += m.getVarByName("x_0_"+to_string(i));
				}
				m.setObjective(objExpr);

				m.set(GRB_IntParam_Threads,8);
				m.update();
				m.optimize();
				if(m.get(GRB_IntAttr_Status) == GRB_OPTIMAL)
				{
					uint lastnbTrail = m.get(GRB_IntAttr_SolCount);
					cout << endl << lastnbTrail << "trails " << endl;

					if(lastnbTrail % 2== 0)
						oddtra = false;
					else
					{
						allsolution.insert(make_pair(keysolution, insolution));
						GRBLinExpr cutExpr(0);
							for(uint i = 0; i < blockSize; i++)
							{
								if(allkey[0][i] == 0) cutExpr += keyvar[0][i];
								else cutExpr += (1-keyvar[0][i]);
								if(insolution[i] == 0) cutExpr += invar[0][i];
								else cutExpr += (1-invar[0][i]);
								if(outsolution[i] == 0) cutExpr += outvar[0][i];
								else cutExpr += (1-outvar[0][i]);
							}
						addLazy( cutExpr >= 1);
					}
				
				}
				else
					oddtra = false;
			}
			
			if(!oddtra)
			{
				GRBLinExpr cutExpr(0);
					for(uint i = 0; i < blockSize; i++)
					{
						if(allkey[0][i] == 0) cutExpr += keyvar[0][i];
						else cutExpr += (1-keyvar[0][i]);
						if(insolution[i] == 0) cutExpr += invar[0][i];
						else cutExpr += (1-invar[0][i]);
						if(outsolution[i] == 0) cutExpr += outvar[0][i];
						else cutExpr += (1-outvar[0][i]);
					}
				addLazy( cutExpr >= 1);
			}

			
		}
	}catch (GRBException e) {
		cout << "Error number: " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl; 
	}catch(...) {
	cout << "Error during callback" << endl;
	}
}




callbacksearchKeyall::callbacksearchKeyall(uint xrMax,
			     vector<GRBVar> const & xinvar,
                             vector<GRBVar> const & xoutvar,
                             vector<vector<GRBVar>> const & xkeyvar) :
    rMax(xrMax),
    invar(xinvar),
    outvar(xoutvar),
    keyvar(xkeyvar)
{}

void callbacksearchKeyall::callback()
{
    try{
        if(where == GRB_CB_MIPSOL)
        {
            uint blockSize = 64;

            
            GRBEnv gurobiEnv;
            gurobiEnv.set(GRB_IntParam_OutputFlag, 0);

            GRBModel m(gurobiEnv, "./modelp/" + to_string(rMax)+"_present.mps");
	    m.set(GRB_IntParam_Threads,16);

            for(uint i = 0; i < 64; i++)
            {
                    m.addConstr(m.getVarByName("x_0_"+to_string(i)) == uint(round(getSolution(invar[i]))));
                    m.addConstr(m.getVarByName("y_"+to_string(rMax-1)+"_"+to_string(i)) == uint(round(getSolution(outvar[i]))));
            }
	    for(uint r = 0; r < rMax-1; r++)
	    {
		    for(uint i = 0; i < blockSize; i++)
		    {
			    m.addConstr(m.getVarByName("k_"+to_string(r)+"_"+to_string(i)) == uint(round(getSolution(keyvar[r][i]))));
		    }
	    }
                
	    
	    vector<vector<uint8_t>> keysolution(rMax - 1, vector<uint8_t> (64));
	    vector<uint8_t> insolution (64);
            vector<uint8_t> outsolution (64);
	    for(uint i = 0; i < blockSize; i++)
	    {
		insolution[i] = uint(round(getSolution(invar[i])));
		outsolution[i] = uint(round(getSolution(outvar[i])));
	    }
	    for(uint r = 0; r < rMax - 1; r++)
	    {
		    for(uint i = 0; i < blockSize; i++)
		    {
			    keysolution[r][i] = uint(round(getSolution(keyvar[r][i])));
		    }
	    }
            

            GRBLinExpr objExpr(0);
            for(uint i = 0; i < blockSize; i++)
	    {
			objExpr += m.getVarByName("x_0_"+to_string(i));
	    }
	    m.setObjective(objExpr);

	    m.set(GRB_IntParam_PoolSearchMode, 2);
      	    m.set(GRB_IntParam_PoolSolutions, 200000000);
	    m.set(GRB_IntParam_SolutionLimit,100000);
	    m.set(GRB_DoubleParam_TimeLimit, 30);


	    m.update();			
	    m.optimize();
            
            bool oddSSB = true;
            if(m.get(GRB_IntAttr_Status) == GRB_OPTIMAL)
            {
                uint nbTrail = m.get(GRB_IntAttr_SolCount);

                if(nbTrail % 2 == 0) oddSSB = false;
                else
                {
		}
	    }
			
	    else
            {
				oddSSB = false;
     	    }

	    uint indexin;
	    for(uint i = 0; i < 64; i++)
	    {
		    if(insolution[i] == 0)
			    indexin = i;
	    }

	    cout << "input index:i  " << indexin << endl;
	/*
	    if(oddSSB)
	    {
		    vector<uint8_t> insolution16(64, 1);
		    vector<uint8_t> insolution32(64, 1);
		    insolution16[indexin - 32] = 0;
	    	    cout << "input index:i  " << indexin << endl;

		    insolution32[indexin - 32] = 0;
		    auto result_16 = counttrail_middle(rMax, insolution16, outsolution, keysolution);
		    if(result_16.second == true && result_16.first % 2 == 1)
			    oddSSB = false;
		    auto result_32 = counttrail_middle(rMax, insolution32, outsolution, keysolution);
		    if(result_32.second == true && result_32.first % 2 == 1)
			    oddSSB = false;
	    }
	*/



	    
            if(!oddSSB)
            {
                GRBLinExpr cutExpr(0);
		for(uint r = 0; r < rMax -1; r++)
		{
			for(uint i = 0; i < blockSize; i++)
			{
				if(keysolution[r][i] == 0) cutExpr += keyvar[r][i];
				else cutExpr += (1 - keyvar[r][i]);
			}
		}
                addLazy( cutExpr >= 1);
            }
        }
    } catch (GRBException e) {
        cout << "Error number: " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch (...) {
        cout << "Error during callback" << endl;
    }
}


	
