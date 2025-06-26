#!/usr/bin/make

src_dir		= src
bin_dir		= bin
build_dir	= $(src_dir)/build

CXX		= g++
CXXFLAGS	= -O2 -Wall
LDFLAGS		= -lpthread -ldl -lcaenhvwrapper

FLAGS	= $(CXXFLAGS)

sources	= $(wildcard $(src_dir)/*.cc)
objects	= $(sources:$(src_dir)/%.cc=$(build_dir)/%.o)
#target	= $(sources:$(src_dir)/%.cc=$(bin_dir)/%)
target	= $(bin_dir)/caen-test

all: $(target)

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
