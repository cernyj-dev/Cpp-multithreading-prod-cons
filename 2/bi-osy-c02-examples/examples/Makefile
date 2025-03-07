CC        = gcc
CFLAGS    = -Wall -pedantic -ansi -g
LIBS      = -lpthread  
CXX       = g++
CXXFLAGS  = -Wall -pedantic

TARGETS   = prodcons_cv_01 condvar_pt_01 condvar_01 task1 task1b  

all: $(TARGETS)

prodcons_cv_01: prodcons_cv_01.cpp
	$(CXX) -std=c++11 $(CXXFLAGS) -o $@ $< $(LIBS)

condvar_pt_01: condvar_pt_01.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

condvar_01: condvar_01.cpp
	$(CXX) -std=c++11 $(CXXFLAGS) -o $@ $< $(LIBS)

task1: task1.cpp
	$(CXX) -std=c++11 $(CXXFLAGS) -o $@ $< $(LIBS)

task1b: task1b.cpp
	$(CXX) -std=c++11 $(CXXFLAGS) -o $@ $< $(LIBS)
clean:
	\rm -f $(TARGETS) *~ core

