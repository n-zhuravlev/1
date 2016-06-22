SRC = SMP.cpp 

all: tests


sort_test: $(SRC)
	g++ -std=c++11 -fopenmp $(SRC) -o sort_result

tests: sort_test
	./sort_result
    
