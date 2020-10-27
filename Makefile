CXX = g++
CXXFLAGS = -g -Wall
SRC_PATH = ./src/m0

.PHONY: all m0 clean cleanall

all: m0

m0: SRC_PATH = ./src/m0
m0: wam compiler

main.o: $(SRC_PATH)/machine/main.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

DataCell.o: $(SRC_PATH)/machine/DataCell.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

MemoryBloc.o: $(SRC_PATH)/machine/MemoryBloc.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

Address.o: $(SRC_PATH)/machine/Address.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

wam: main.o DataCell.o MemoryBloc.o Address.o
	$(CXX) $(CXXFLAGS) $? -o $@

compiler: $(SRC_PATH)/compiler/compiler.l $(SRC_PATH)/compiler/compiler.y
	bison -o compiler_y.cpp -d $(SRC_PATH)/compiler/compiler.y
	flex -o compiler_l.cpp $(SRC_PATH)/compiler/compiler.l
	g++ -o compiler compiler_y.cpp compiler_l.cpp

# %.o: %.cpp
# 	$(CXX) $(DFLAGS) -c $(CXXFLAGS) $< -o $@

clean:
	rm -f *.o compiler_y.cpp compiler_l.cpp compiler_y.hpp

cleanall: clean
	rm -f wam compiler