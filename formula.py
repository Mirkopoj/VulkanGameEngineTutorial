#17op? 0desperdicio
def xy1(i:int, xn:int):
    r = int(i/2)
    s = int(r/xn % 2)
    m = (1-2*s)
    y = int(m*(i%2)+int(r/xn)*2)
    x = int(s*(xn-1)+m*(int((i+s)/2)%xn))
    return (x, y)

def clamp(v, min, max):
    if v<min:
        return min
    if v>max:
        return max
    return v

#6op? 3triangulos/fila
def xy2(i:int, xn: int):
    r = i%xn
    ci = clamp(r-1, 0, xn-3)
    x = int(ci/2)
    y = (ci%2) + int(i/xn)
    return (x, y)

xn = 5
for i in range(19):
    (x, y) = xy1(i,xn)
    print("i = ", i, "(", x, ", ", y, ")")

print("")

xn = xn*2+2
for i in range(19):
    (x, y) = xy2(i,xn)
    print("i = ", i, "(", x, ", ", y, ")")

