import numpy as np

def DTWDistance(signalA, signalB):
    n,m = len(signalA), len(signalB)
    DTW = np.zeros(shape=(n, m))
    for i in range(0,n):
        DTW[i,0] = np.finfo(dtype=float).max
    for i in range(0,m):
        DTW[0,i] = np.finfo(dtype=float).max
    DTW[0,0] = 0
    
    for i in range(1,n):
        for j in range(1,m):
            const = abs(signalA[i] - signalB[j])
            DTW[i,j] = const + min(DTW[i-1, j], DTW[i, j-1], DTW[i-1, j-1])
    
    return DTW[n - 1, m - 1]
        
    
    