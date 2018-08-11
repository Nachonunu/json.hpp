CODE=main.cpp
FLAG=-static
CPP=g++

Test: $(CODE)
	$(CPP) -g -Wall -O2 -std=c++11 $(FLAG) -I./ -o Test $(CODE)

