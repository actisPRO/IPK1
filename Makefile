std=c++14
src=main.cpp
out=hinfosvc

all:
	g++ -std=$(std) -o $(out) $(src)