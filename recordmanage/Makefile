.PHONY: clean
CXX = g++
CXXFLAGS = --std=c++11
DEPS = rm.h
OBJ = RM_Manager.o RM_FileHandle.o RM_FileScan.o RM_Record.o testrecordmanage.o

main: $(OBJ)
	$(CXX) -o test $(OBJ)

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)


clean:
	rm *.o
