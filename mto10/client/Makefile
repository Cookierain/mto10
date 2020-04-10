all:mto10_client

mto10_client: client.cpp MD5.h MD5.cpp public.h public.cpp
	c++ -std=c++11 -o mto10_client client.cpp MD5.cpp public.cpp

clean:
	rm -rf mto10_client