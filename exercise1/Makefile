# Compiler and linker options
CXX = mpic++
CXXFLAGS = -fopenmp -O3
LDFLAGS = -fopenmp -O3

# Source files and object files
SRCS = Conway.cpp game.h utilities.h read_write.h
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = Conway.x

# Rules for building the executable
$(EXEC): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

# Rules for building object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f *.o $(EXEC)
