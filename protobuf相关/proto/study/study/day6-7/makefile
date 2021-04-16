OPTIONS          = --std=c++11
LINKER_OPT       = -lzookeeper_mt `pkg-config --cflags --libs libmongocxx`

all: server client

server:
	g++ $(OPTIONS) -Iinclude/ server.cc zk_cpp.cc $(LINKER_OPT) -o server

client:
	g++ $(OPTIONS) -Iinclude/ client.cc zk_cpp.cc $(LINKER_OPT) -o client

clean:
	rm -f server client
