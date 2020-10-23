CXX = g++
CXXFLAGS = -g -Wall
SRC_PATH = ./src/m0

m0: SRC_PATH = ./src/m0
m0: wam

main.o: $(SRC_PATH)/main.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

DataCell.o: $(SRC_PATH)/DataCell.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

MemoryBloc.o: $(SRC_PATH)/MemoryBloc.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

Address.o: $(SRC_PATH)/Address.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

wam: main.o DataCell.o MemoryBloc.o Address.o
	$(CXX) $(CXXFLAGS) $? -o $@

# %.o: %.cpp
# 	$(CXX) $(DFLAGS) -c $(CXXFLAGS) $< -o $@

.PHONY: clean cleanall
clean:
	rm -f *.o

cleanall:
	rm -f *.o wam