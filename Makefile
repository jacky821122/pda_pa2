CPP = pa2.cpp structure.cpp structure.h
EXE = PA2

all: $(EXE)

PA2: $(CPP)
	g++ -O3 -o $@ $(CPP)

a: $(EXE)
	./$(EXE) 0.5 testcases/ami33.block testcases/ami33.nets output/ami33.rpt

e: $(EXE)
	./$(EXE) 0.5 testcases/example1.block testcases/example1.nets output/example1.rpt

clean:
	rm -rf $(EXE)
