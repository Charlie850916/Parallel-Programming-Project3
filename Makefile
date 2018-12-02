CC = mpicc
CXX = mpicxx 
CFLAGS = -O3 -fopenmp 
CXXFLAGS = -O3 -fopenmp 
TARGETS = sssp apsp partition

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(TARGETS:=.o)
