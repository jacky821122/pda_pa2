CPP = pa2.cpp block.cpp block.h
EXE = PA2

all: $(EXE)

PA2: $(CPP)
	g++ -O3 -o $@ $(CPP)

a: $(EXE)
	./$(EXE) 1 testcases/ami33.block

e: $(EXE)
	./$(EXE) 1 testcases/example1.block

clean:
	rm -rf $(EXE)
