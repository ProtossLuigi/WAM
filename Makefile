CXX = g++
CXXFLAGS = -Wall
DFLAGS =

all: wam

debug: DFLAGS = -g
debug: wam

wam.o: ./src/main.cpp
	$(CXX) $(DFLAGS) -c $(CXXFLAGS) $< -o $@

wam: wam.o
	$(CXX) $(CXXFLAGS) $< -o $@

# %.o: %.cpp
# 	$(CXX) $(DFLAGS) -c $(CXXFLAGS) $< -o $@

.PHONY: clean cleanall
clean:
	rm -f *.o

cleanall:
	rm -f *.o wam