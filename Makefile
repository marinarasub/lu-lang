###############################################################################
# Makefile for lu-lang
# Copyright © 2023 David Lu
# 
# BUILDING
# If you have make and a g++ installed, it's as easy as
# >		make
# You can switch compilers (i.e. clang++) by specifying 
# >		make CXX=<c++ compiler> LD=<c++ linker>
#
# TARGETS
# To build a specific target, specify the target to the command make
# >		make <target>
# - sstest - build the core SSTest static library
# - sstest_main - build the library containing default main() function, dependent on sstest
# 	- unless you want to use a custom main() function, you'll want to build this too
# - all (default) - build all targets
# - test - build tests
# - example - build example executables
# - clean - delete build output files
#
# CONFIGURING
# To use a CONFIGuration, use an argument:
# >		make CONFIG=<CONFIG>
# *note these are case sensitive*
# - Release (Default) - build for use with your project 
# - RelWithDebInfo - build for release with debug information
# - MinSizeRel - build release with minimum code size
# - Debug - build for development
# By default, the output directory is build/$(config), specify with BUILD_DIR=<your build dir>
#
###############################################################################

CXX = clang++
LD = clang++

CXXFLAGS = -std=c++11 -pedantic-errors -Wall -Werror -Wfatal-errors -Wextra -Wdangling-else -Wconversion -fPIE
LD = g++
LDFLAGS = -std=c++11 -pedantic-errors -Wall -Werror -Wfatal-errors -Wextra -Wdangling-else -Wconversion
AR = ar

DEBUG_FLAGS = -O0 -g -Wno-unused-parameter -Wno-unused-variable -Wno-unused-const-variable -fstack-protector -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope 
RELEASE_FLAGS = -O2 -DNDEBUG
MIN_SIZE_RELEASE_FLAGS = -Os -DNDEBUG

CONFIG = Debug

# add additonal compiler options per CONFIGuration 
ifeq ($(CONFIG), Debug)
CXXFLAGS += $(DEBUG_FLAGS)
LDFLAGS += $(DEBUG_FLAGS) 
endif

ifeq ($(CONFIG), Release)
CXXFLAGS += $(RELEASE_FLAGS)
LDFLAGS += $(RELEASE_FLAGS)
endif

ifeq ($(CONFIG), RelWithDebInfo)
CXXFLAGS += $(RELEASE_FLAGS) -g
LDFLAGS += $(RELEASE_FLAGS) -g
endif

ifeq ($(CONFIG), MinSizeRel)
CXXFLAGS += $(MIN_SIZE_RELEASE_FLAGS)
LDFLAGS += $(MIN_SIZE_RELEASE_FLAGS)
endif

BUILD_DIR = build/$(CONFIG)

OBJS = string.o print.o source.o token.o lex.o parse.o diag.o analyze.o type.o expr.o timer.o csv.o profile.o main.o symbol.o scope.o intrinsic.o intermediate.o interpreter.o value.o cast.o# TODO main shouldn't be object
OBJS := $(addprefix $(BUILD_DIR)/, $(OBJS))
LIBS = lu.a
LIBS := $(addprefix $(BUILD_DIR)/, $(LIBS))
EXES = main
EXES := $(addprefix $(BUILD_DIR)/, $(EXES))

all: mkdirs $(EXES) complete

$(OBJS) : $(BUILD_DIR)/%.o : %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

$(LIBS) : $(OBJS)
	$(AR) rcs -o $@ $^

$(EXES): $(LIBS)
	$(LD) $(LDFLAGS) -o $@ $^

complete:
	$(info *** Build output to $(BUILD_DIR) ***)

mkdirs:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(EXES) $(OBJS) $(LIBS)

rebuild : clean all

.PHONY: all clean mkdirs complete