DBGFLAGS = -O3 -Wall
DBGFLAGS = -g  -Wall

CXX      = g++
##............................... At this point using C++20 excusively for std::endian feature:
CXXFLAGS = $(DBGFLAGS) -std=c++20

# CPPFLAGS = $(CXXFLAGS)
LDFLAGS  = -lm  -lboost_program_options

stl-bin-to-text.o:  stl-bin-to-text.cpp stlfile.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

stl-bin-to-text:  stl-bin-to-text.o stlfile.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

stl-check.o:  stl-check.cpp stlfile.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

stl-check:  stl-check.o stlfile.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

stl-compute-normals.o:  stl-compute-normals.cpp stlfile.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

stl-compute-normals:  stl-compute-normals.o stlfile.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

stl-scale-xyz.o:  stl-scale-xyz.cpp stlfile.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

stl-scale-xyz:  stl-scale-xyz.o stlfile.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

stl-stats.o:  stl-stats.cpp stlfile.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

stl-stats:  stl-stats.o stlfile.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

stl-text-to-bin.o:  stl-text-to-bin.cpp stlfile.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

stl-text-to-bin:  stl-text-to-bin.o stlfile.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

TARGETS = stl-bin-to-text stl-check stl-compute-normals stl-scale-xyz stl-stats stl-text-to-bin

.PROXY : all clean

all : $(TARGETS)

clean:
	rm -f *~ *.o $(TARGETS)
