from math import *

Fs=100
f=0.2
N=500
a=[0]*N

for i in range(N):
    sample = a[i]=40*sin(2*pi*f*i/Fs) + 40
    print(f"{sample}")

