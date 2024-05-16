import matplotlib.pyplot as plt
import numpy as np

import matplotlib.tri as mtri

x = np.asarray([0, 1, 2, 3, 4, 
                0, 1, 2, 3, 4, 
                0, 1, 2, 3, 4, 
                0, 1, 2, 3, 4, 
                0, 1, 2, 3, 4])
y = np.asarray([0, 0, 0, 0, 0,
                1, 1, 1, 1, 1,
                2, 2, 2, 2, 2,
                3, 3, 3, 3, 3,
                4, 4, 4, 4, 4])

#0 triangulos de mas
def xy(i:int, xn:int):
    n = 4*xn-2
    r = i%n
    c = int(r/2)
    d = int(c/xn) % 2
    s = 1-2*d
    y = s*(i%2) + int(c/xn)*2 + int(i/n)*2
    x = d*(xn-1)+s * ( int((r+d)/2) % xn )
    return (x, y)

xn = 5
total= xn*2*(xn-1)-(xn-2)
verteces = []
for i in range(total):
    verteces.append(xy(i,xn))

def xy2i(xy: tuple[int,int], xn:int):
    (x, y) = xy
    return y*xn + x

triangles = [];
for i in range(len(verteces) - 2):
    triangles.append([
        xy2i(verteces[i+0], xn),
        xy2i(verteces[i+1], xn),
        xy2i(verteces[i+2], xn),
                ])

triang = mtri.Triangulation(x, y, triangles)
plt.triplot(triang, 'ko-')
for (i, (x,y)) in enumerate(verteces):
    ofset = (-0.08, 0.1)
    if  i < 10:
        ofset = (-0.05, -0.1)
    if  10 <= i and i < 20 and i%2 == 0:
        ofset = (0.03, -0.1)
    if  19 <= i and i < 27 and i%2 == 1:
        ofset = (-0.08, -0.1)
    (xo, yo) = ofset
    plt.text(x+xo, y+yo, f'{i}', fontsize=12)
plt.show()

exit()
def clamp(v, min, max):
    if v<min:
        return min
    if v>max:
        return max
    return v

#3 triangulos/fila de mas
def xy2(i:int, xn: int):
    r = i%xn
    ci = clamp(r-1, 0, xn-3)
    x = int(ci/2)
    y = (ci%2) + int(i/xn)
    return (x, y)

print("")
xn = xn*2+2
for i in range(19):
    (x, y) = xy2(i,xn)
    print("i = ", i, "(", x, ", ", y, ")")

