The version of Gurobi we utilized is "Version 9.5.0".
The fold "modelp" is  the MILP model  for PRESENT.
Given a r-round PRESENT, Our modeling sequence is as follows: S--P--K--S--P--k ······ S--P--K--S 

Type "make" in the console to generate executable program "main.run,  genm.run"

run "./main.run -r 12 -s i - j" to Search a middle-round  key monomial corresponding to a (n-1)-degree monomial u_i( only not contain x[i]) 
and a 1-degree monomial e_j(only contain y[j]) for 12-round PRESENT, where x is input and y is output.

If all middle-round key monomial have been obtained , run "python genallkey_12.py" to generate 4096 key monomials for all rounds.

run "./genm.run -r 12" to generate 12-round integral-resistance matrix. 


run "sage determine_rank.sage 12" to solve the rank of 12-round integral-resistance matrix.


