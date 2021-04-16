all: server client

protoc_middleman: udisk.proto
	protoc --cpp_out=. udisk.proto
	@touch protoc_middleman

server: server.cc protoc_middleman
	pkg-config --cflags protobuf  # fails if protobuf is not installed
	c++ server.cc udisk.pb.cc ./myproto.cc -o server `pkg-config --cflags --libs protobuf`

client: client.cc protoc_middleman
	pkg-config --cflags protobuf  # fails if protobuf is not installed
	c++ client.cc udisk.pb.cc ./myproto.cc -o client `pkg-config --cflags --libs protobuf`

clean:
	rm -f server client
	rm -f protoc_middleman udisk.pb.cc udisk.pb.h