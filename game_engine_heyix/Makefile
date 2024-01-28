# Compiler and compiler flags
CXX = clang++
CXXFLAGS = -O2 -std=c++17

# Directories
SRCDIR = ./src
THIRDPARTYDIR = ./ThirdParty
INCLUDEDIR = -I$(THIRDPARTYDIR)/glm -I$(SRCDIR)
LIBDIR = -L$(THIRDPARTYDIR)
LIBS = #-lYourLibrary1 -lYourLibrary2  # Add the library names you need

# Source files and object files
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(SRCS:.cpp=.o)

# Output executable
EXECUTABLE = game_engine_linux

# Build the executable
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDEDIR) $(LIBDIR) -o $@ $(OBJS) $(LIBS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDEDIR) -c -o $@ $<

clean:
	rm -f $(OBJS) $(EXECUTABLE)