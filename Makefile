TARGET	:= smart_pointer
SOURCE  := $(wildcard *.cpp)
OBJS=$(patsubst %.cpp, %.o, $(SOURCE))

CC		:= gcc
CXX		:= g++
LIBS 	:=
LDFLAGS	:=
INCLUDE	:= -I.
CFLAGS	:= -g -Wall -O3 $(INCLUDE)
CXXFLAGS:= $(CFLAGS)

.PHONY: all objs clean rebuild

all : $(TARGET)

objs : $(OBJS)

rebuild : distclean all

clean :
	rm -rf *.o
	rm -rf *.so

distclean: clean
	rm -rf $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)