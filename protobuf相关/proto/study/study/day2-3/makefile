all: server client

server:
	g++ server.cc ./myproto.cc ./lib_json/*.cpp -I ./ -o server

client:
	g++ client.cc ./myproto.cc ./lib_json/*.cpp -I ./ -o client

clean:
	rm -f server client