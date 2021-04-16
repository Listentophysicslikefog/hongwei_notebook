all: server client

server:
	g++ -Iinclude/ server.cc zk_cpp.cc -lzookeeper_mt -o server

client:
	g++ -Iinclude/ client.cc zk_cpp.cc -lzookeeper_mt -o client

clean:
	rm -f server client
