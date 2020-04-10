all:mto10_server_d mto10_read

mto10_server_d: server.cpp MD5.h MD5.cpp db.h db.cpp public.h public.cpp
	c++ -std=c++11 -o mto10_server_d server.cpp MD5.cpp db.cpp public.cpp $(shell mysql_config --cflags --libs)

mto10_read: mto10_read.cpp db.h db.cpp public.h public.cpp
	c++ -std=c++11 -o mto10_read mto10_read.cpp db.cpp public.cpp $(shell mysql_config --cflags --libs)

clean:
	rm -rf mto10_server_d mto10_read



