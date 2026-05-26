CXX=g++
CXXFLAGS=-std=c++17

OBJS = main.o Calendar.o Calculator.o Tasks.o Notepad.o

pm: $(OBJS)
	$(CXX) $(OBJS) -o pm

clean:
	rm -f *.o pm
