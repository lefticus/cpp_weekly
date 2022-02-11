CXX=g++
CXXFLAGS=-std=c++17
RM=rm -f

SRCS=FA.cpp convertor.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: convertor

convertor: $(OBJS)
	$(CXX) -o convertor $(OBJS)

FA.o: FA.h FA.cpp

convertor.o: FA.o convertor.cpp

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) tool
