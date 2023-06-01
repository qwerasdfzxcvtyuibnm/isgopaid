reset()

from sage.crypto.boolean_function import BooleanFunction
import sys
load("aux_function.sage")
load("MILP_function.sage")



def applySbox(m, xr, yr, ineq, sboxSize, nbSbox):
    #Add the constraints for the Sbox layer from xr to yr, using the inequalities in ineq
    #Constraints for Sbox

    for j in range(nbSbox):
        inputvar = [xr[sboxSize*j+i] for i in range(sboxSize)]
        outputvar = [yr[sboxSize*j+i] for i in range(sboxSize)]
        c = addSboxConstr(m, ineq, inputvar, outputvar)
        #print("##############")
        #print(c)
        #print("#################")



rMax = int(sys.argv[1])
Sfirst = [0x8, 0x5, 0x0, 0x2, 0xa, 0x9, 0x4, 0xc, 0xe, 0x7, 0xb, 0xd, 0x6, 0x1, 0xf, 0x3]
S    = [0xc,0x5,0x6,0xb,0x9,0x0,0xa,0xd,0x3,0xe,0xf,0x8,0x4,0x7,0x1,0x2] #normal Sbox
Slast  = [0x6,0x7,0xe,0x3,0x1,0x0,0x8,0xd,0x9,0x4,0xf,0xa,0xc,0x5,0xb,0x2] #normal Sbox

#P = [0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15]
P = [0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54, 7, 23, 39, 55, 8, 24, 40, 56, 9, 25, 41, 57, 10, 26, 42, 58, 11, 27, 43, 59, 12, 28, 44, 60, 13, 29, 45, 61, 14, 30, 46, 62, 15, 31, 47, 63]

name = "minipresent_{}r".format(rMax)
nolastMC= True


#precomputer inequalities for the sboxes
(BPR, anf) = anfSbox(S)
#print(anf)
#print("\n\n")
T = divPropANFBinTable(anf)
#print(T)
ineqSbox = sboxReducedInequalities(T)
#print(ineqSbox)

#precomputer inequalities for the last sboxes
(BPRlast, anflast) = anfSbox(Slast)
#print(anf)
#print("\n\n")
Tlast = divPropANFBinTable(anflast)
#print(T)
ineqSboxlast = sboxReducedInequalities(Tlast)
print(ineqSboxlast)
print(len(ineqSboxlast))

#precomputer inequalities for the last sboxes
(BPRfirst, anffirst) = anfSbox(Sfirst)
#print(anf)
#print("\n\n")
Tfirst = divPropANFBinTable(anffirst)
#print(T)
ineqSboxfirst = sboxReducedInequalities(Tfirst)
print(ineqSboxfirst)
print(len(ineqSboxfirst))



sboxSize = 4 #size of sbox
blockSize = 64 #block size
nbSbox = 16 #number of sboxes



#Create the models and the Variables
m = Model(name)

x = [[m.addVar(vtype=GRB.BINARY, name="x_"+str(i)+"_"+str(j)) for j in range(blockSize)] for i in range(rMax)]
y = [[m.addVar(vtype=GRB.BINARY, name="y_"+str(i)+"_"+str(j)) for j in range(blockSize)] for i in range(rMax)]
z = [[m.addVar(vtype=GRB.BINARY, name="z_"+str(i)+"_"+str(j)) for j in range(blockSize)] for i in range(rMax-1)]
k = [[m.addVar(vtype=GRB.BINARY, name="k_"+str(i)+"_"+str(j)) for j in range(blockSize)] for i in range(rMax-1)]

m.update()


for r in range(rMax -1):

    if r == 0:
        ayypplySbox(m, x[r], y[r], ineqSboxfirst, sboxSize, nbSbox)
    else:
        ayypplySbox(m, x[r], y[r], ineqSbox, sboxSize, nbSbox)


    #bit permutation
    for j in range(blockSize):
        m.addConstr(z[r][P[j]] == y[r][j])

    #add roundkey
    for j in range(blockSize):
        m.addConstr(z[r][j] + k[r][j] == x[r+1][j])


#last round
r = rMax -1

#s layer
applySbox(m, x[r], y[r], ineqSboxlast, sboxSize, nbSbox)

m.update()

m.write(str(rMax)+".mps")



