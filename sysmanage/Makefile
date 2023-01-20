.PHONY: clean
CXX = g++
CXXFLAGS = --std=c++17
DEPS = sm.h
OBJ = SM_Manager.o test.o

main: $(OBJ)
	$(CXX) -o test $(OBJ) \
	../recordmanage/RM_Manager.o ../recordmanage/RM_FileHandle.o ../recordmanage/RM_FileScan.o \
	../recordmanage/RM_Record.o

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)


clean:
	rm *.o