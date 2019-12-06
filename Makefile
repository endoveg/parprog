LAB2_DIR=build/lab2
LAB2D_SRC=Cycles.cpp Inner.cpp OuterEight.cpp OuterFour.cpp OuterTwo.cpp Single.cpp main.cpp
FCR_DIR=build/FCR
all: lab2 FCR
lab2:
	mkdir -p $(LAB2_DIR)
	mpicc -o $(LAB2_DIR)/reference ./src/reference.c -lm
	mpicc -o $(LAB2_DIR)/2g ./src/lab2_2g.c -lm
	mpic++ -o $(LAB2_DIR)/2d $(addprefix ./src/lab2_2d/,$(LAB2D_SRC)) -lm
FCR:
	mkdir -p $(FCR_DIR)
	g++ -std=c++17 -o $(FCR_DIR)/fcr ./src/full\ cycle\ reduction/tridiag.cpp ./src/full\ cycle\ reduction/partridiag.cpp -fopenmp
clean:
	rm -rf build
