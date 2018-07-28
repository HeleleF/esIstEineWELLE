# makefile for mp program

# The name of the main executable
TARGET=myWaveMP

# Flags and stuff, change as required
OPTIMIZE=-O3
# Flags for all languages
CPPFLAGS=-ggdb $(OPTIMIZE) -Wall -MMD -MP `pkg-config --cflags --libs sdl2`
# Flags for C only
CFLAGS=-Wmissing-prototypes -fopenmp
# Flags for C++ only
CXXFLAGS=-std=c++11
# Flags for the linker
LDFLAGS= -fopenmp
# Additional linker libs
LDLIBS=-lm -lSDL2 -lSDL2_image -lSDL2_ttf

# Compilers
CC=gcc
CPP=g++

##############################################################
# Normally there shouldn't be a need to modify the following
# lines, except for bug fixing and new features.

# The sources that make up the main executable. These are
# all files that end with an .c but do not end with _test.c
SRCS=$(filter-out %_test.c,$(wildcard *.c))
CPPSRCS=$(filter-out %_test.cpp,$(wildcard *.cpp))

# We make up the objects by replacing the .c and .cpp suffixes
# with a .o suffix
OBJS=$(patsubst %.c,%.o,$(SRCS)) $(patsubst %.cpp,%.o,$(CPPSRCS))

# The test sources are assumed to end with _test
TEST_SRCS=$(wildcard *_test.c)
TEST_CPPSRCS=$(wildcard *_test.cpp)

# The test objects
TEST_OBJS=$(patsubst %.c,%.o,$(TEST_SRCS)) $(patsubst %.cpp,%.o,$(TEST_CPPSRCS))

# The test executables (without any suffix)
TESTS=$(patsubst %.c,%,$(TEST_SRCS)) $(patsubst %.cpp,%,$(TEST_CPPSRCS))

# The dependency files
DEPS=$(SRCS:.c=.d) $(CPPSRCS:.cpp=.d) $(TEST_SRCS:.c=.d) $(TEST_CPPSRCS:.cpp=.d)

# If we have any C++ sources, we link with CPP
# For this redfine the internal LINK.o macro
ifneq ($(strip $(CPPSRCS)),)
LINK.o = $(CPP) $(LDFLAGS) $(TARGET_ARCH)
endif

# The first target (all) is always the default target
.PHONY: all
all: clean build

# Our build target depends on the real target
.PHONY: clean build
build: $(TARGET)

# Our target is built up from the objects
$(TARGET): $(OBJS)

# valgrind checker target
.PHONY: test
test: 
	valgrind ./$(TARGET) -u 0

# Cleanup all generated files
.PHONY: clean
clean:
	rm -Rf $(TEST_OBJS) $(TESTS) $(OBJS) $(TARGET) $(DEPS)

-include $(DEPS)
