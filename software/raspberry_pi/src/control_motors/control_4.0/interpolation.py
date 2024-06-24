import numpy as np

def spline(x, y, derivada=[]):
    # Armazena o tamanho do vetor X
    n = len(x)
    # Verifica se o tamanho de Y e igual ao de X (se cada X possui um Y correspondente)
    assert len(y) == n 

    # Recebe os valores de Y no vetor de coeficientes a
    a = y
    h = np.zeros(n)

    for i in range(n - 1): 
        h[i] = x[i + 1] - x[i]
    
    A = np.zeros((n, n))
    B = np.zeros(n)

    for i in range(1, n - 1):
        A[i][i - 1] = h[i - 1]
        A[i][i] = 2 * (h[i - 1] + h[i])
        A[i][i + 1] = h[i]

        B[i] = ((3 / float(h[i])) * (a[i + 1] - a[i])) - ((3 / float(h[i - 1])) * (a[i] - a[i - 1]))
    
    try:
        der1 = derivada[0]
        der2 = derivada[1]

        A[0][0] = 2 * h[0]
        A[0][1] = h[0]
        A[n - 1][n - 1] = 2 * h[n - 2]
        A[n - 1][n - 2] = h[n - 2]

        B[0] = ((3 / float(h[0])) * (a[1] - a[0])) - (3 * der1)
        B[n - 1] = (3 * der2) - ((3 / float(h[n - 2])) * (a[n - 1] - a[n - 2]))
    except:
        A[0][0] = 1.0
        A[n - 1][n - 1] = 1.0

    c = np.linalg.solve(A, B)
    b = np.zeros(n - 1)
    d = np.zeros(n - 1)

    for i in range(n - 1):
        b[i] = ((a[i + 1] - a[i]) / h[i]) - ((h[i] / 3) * ((2 * c[i]) + c[i + 1]))
        d[i] = (c[i + 1] - c[i]) / (3 * h[i])

    coef = np.zeros((n - 1, 4))
    for i in range(n - 1):
        coef[i][0] = a[i]
        coef[i][1] = b[i]
        coef[i][2] = c[i]
        coef[i][3] = d[i]

    return coef

def calc_spline(x, X, coef):
    y = 0
    
    try:
        n = len(X)
        
        y = np.zeros(n)
        for i in range(21):
            print(x)
            y[i] = calc_spline(x, X[i], coef)
    
    except:
        H = X - x
        ak = coef[0]
        bk = coef[1]
        ck = coef[2] 
        dk = coef[3]
        y = ak + H * (bk + H * ( ck + H * dk ))
    
    return y