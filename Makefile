CXX = g++
CXXFLAGS = -ggdb -Wall
SRC_PATH = ./src/m3
WAM_REQS = main.o DataCell.o MemoryBloc.o Address.o Enviroment.o

.PHONY: all m0 m1 m2 m3 clean cleanall

all: m3

m0: SRC_PATH = ./src/m0
m0: WAM_REQS = main.o DataCell.o MemoryBloc.o Address.o
m0: wam compiler

m1: SRC_PATH = ./src/m1
m1: WAM_REQS = main.o DataCell.o MemoryBloc.o Address.o
m1: wam compiler

m2: SRC_PATH = ./src/m2
m2: wam compiler

m3: SRC_PATH = ./src/m3
m3: wam compiler

main.o: $(SRC_PATH)/machine/main.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

DataCell.o: $(SRC_PATH)/machine/DataCell.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

MemoryBloc.o: $(SRC_PATH)/machine/MemoryBloc.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

Address.o: $(SRC_PATH)/machine/Address.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

Enviroment.o: $(SRC_PATH)/machine/Enviroment.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

wam: $(WAM_REQS)
	$(CXX) $(CXXFLAGS) $? -o $@

compiler_y.cpp compiler_y.hpp: $(SRC_PATH)/compiler/compiler.y
	bison -o compiler_y.cpp -d $?

compiler_l.cpp: $(SRC_PATH)/compiler/compiler.l
	flex -o $@ $<

compiler_y.o: compiler_y.cpp
	$(CXX) -c $(CXXFLAGS) $? -o $@

compiler_l.o: compiler_l.cpp compiler_y.hpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

Term.o: $(SRC_PATH)/compiler/Term.cpp
	$(CXX) -c $(CXXFLAGS) $? -o $@

compiler: compiler_y.o compiler_l.o Term.o
	$(CXX) $(CXXFLAGS) $? -o $@

# %.o: %.cpp
# 	$(CXX) $(DFLAGS) -c $(CXXFLAGS) $< -o $@

clean:
	rm -f *.o compiler_y.cpp compiler_l.cpp compiler_y.hpp

cleanall: clean
	rm -f wam compiler