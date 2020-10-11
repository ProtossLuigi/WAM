CXX = g++
CXXFLAGS = -Wall
DFLAGS = -g
SRC_PATH = ./src/m0

m0: SRC_PATH = ./src/m0
m0: wam

main.o: $(SRC_PATH)/main.cpp
	$(CXX) $(DFLAGS) -c $(CXXFLAGS) $< -o $@

FunctorCell.o: $(SRC_PATH)/FunctorCell.cpp
	$(CXX) $(DFLAGS) -c $(CXXFLAGS) $< -o $@

RefCell.o: $(SRC_PATH)/RefCell.cpp
	$(CXX) $(DFLAGS) -c $(CXXFLAGS) $< -o $@

wam: main.o FunctorCell.o RefCell.o
	$(CXX) $(CXXFLAGS) $? -o $@

# %.o: %.cpp
# 	$(CXX) $(DFLAGS) -c $(CXXFLAGS) $< -o $@

.PHONY: clean cleanall
clean:
	rm -f *.o

cleanall:
	rm -f *.o wam