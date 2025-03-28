# Compiler and compiler flags
CXX = clang++
CXXFLAGS = -O3 -std=c++17

# Directories
SRCDIR = ./src
THIRDPARTYDIR = ./ThirdParty
INCLUDEDIR = -I$(THIRDPARTYDIR) -I$(SRCDIR) -I$(THIRDPARTYDIR)/SDL2 -I$(THIRDPARTYDIR)/SDL2_image -I$(THIRDPARTYDIR)/SDL2_mixer -I$(THIRDPARTYDIR)/SDL2_ttf -I$(THIRDPARTYDIR)/lua -I$(THIRDPARTYDIR)/LuaBridge -I$(THIRDPARTYDIR)/box2d

LIBDIR = -L$(THIRDPARTYDIR) -L$(THIRDPARTYDIR)/SDL2/lib -L$(THIRDPARTYDIR)/SDL2_image/lib -L$(THIRDPARTYDIR)/SDL2_mixer/lib -L$(THIRDPARTYDIR)/SDL2_ttf/lib
LIBS = -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -llua5.4#-lYourLibrary1 -lYourLibrary2  # Add the library names you need

# Source files and object files
BOX2D_SRCS := \
    $(wildcard $(THIRDPARTYDIR)/box2d/common/*.cpp) \
    $(wildcard $(THIRDPARTYDIR)/box2d/collision/*.cpp) \
    $(wildcard $(THIRDPARTYDIR)/box2d/dynamics/*.cpp) \
    $(wildcard $(THIRDPARTYDIR)/box2d/rope/*.cpp)

SRCS = $(wildcard $(SRCDIR)/*.cpp) $(BOX2D_SRCS)
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