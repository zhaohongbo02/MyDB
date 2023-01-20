.PHONY: clean clean_load
CXX = g++
CXXFLAGS = --std=c++17
OBJ = parser/SQLLexer.o parser/SQLParser.o \
	  recordmanage/RM_Manager.o recordmanage/RM_FileHandle.o recordmanage/RM_FileScan.o recordmanage/RM_Record.o \
	  sysmanage/SM_Manager.o
INCLUDEANDLIB = -I /usr/local/inclue/antlr4-runtime -L /usr/local/lib -lantlr4-runtime

main: $(OBJ) parser/MyVisitor.h
	$(CXX) -c main.cpp $(INCLUDEANDLIB) $(CXXFLAGS)
	$(CXX) -o main main.o $(OBJ) $(INCLUDEANDLIB) $(CXXFLAGS)

load: $(OBJ) parser/MyVisitor.h
	$(CXX) -c Load.cpp $(CXXFLAGS)
	$(CXX) -o Load Load.o $(OBJ) $(INCLUDEANDLIB) $(CXXFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(INCLUDEANDLIB) $(CXXFLAGS)

clean_load:
	rm Load.o

clean:
	rm main.o parser/*.o sysmanage/*.o recordmanage/*.o