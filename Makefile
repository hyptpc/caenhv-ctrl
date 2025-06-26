#!/usr/bin/make

top_dir		:= $(CURDIR)
bin_dir		:= $(top_dir)/bin
src_dir		:= $(top_dir)/src
include_dir	:= $(top_dir)/include
build_dir	:= $(src_dir)/build

# root_config	:= root-config
# root_include	:= $(shell $(root_config) --cflags)
# root_libs	:= $(shell $(root_config) --glibs)

CXX		:= g++
CXXFLAGS	+= -O2 -Wall -std=c++17 -MMD
CXXFLAGS	+= -I$(include_dir) $(root_include)
CXXFLAGS	+= -DUNIX
CXXFLAGS	+= -DDEBUG
LDFLAGS		= -lpthread -ldl -lncurses -lcaenhvwrapper $(root_libs)

FLAGS	= $(CXXFLAGS)

sources	= $(wildcard $(src_dir)/*.cc)
headers	= $(wildcard $(include_dir)/*.hh)
dependencies	= $(sources:$(src_dir)/%.cc=$(build_dir)/%.d)
objects	= $(sources:$(src_dir)/%.cc=$(build_dir)/%.o)
#target	= $(sources:$(src_dir)/%.cc=$(bin_dir)/%)
target	= $(bin_dir)/caenhv-hyptpc

#______________________________________________________________________________
.PHONY: all clean
.SECONDARY:

all: $(target)

-include $(dependencies)

$(target): $(objects)
	@ echo === Linking $@ ...
	@ mkdir -p $(bin_dir)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(build_dir)/%.o: $(src_dir)/%.cc
	@ echo === Compiling $< ...
	@ mkdir -p $(build_dir)
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	@ echo === Cleaning up ...
	@ rm -rfv $(build_dir) $(target)
