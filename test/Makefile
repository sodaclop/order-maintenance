#co.exe: co.cpp lib/dart_order.hpp ../src/order_maintenance.h ../src/libordmaint.so Makefile
#	g++  -O0 -W -Wall -ggdb3 -L../src -I../src co.cpp -lordmaint -o co.exe
co.exe: co.cpp lib/dart_order.hpp ../src/order_maintenance.h ../src/baseamort.o Makefile
	g++  -O0 -W -Wall -ggdb3 -I../src co.cpp ../src/baseamort.o -o co.exe
