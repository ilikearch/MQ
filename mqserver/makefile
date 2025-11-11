.PHONY: mqserver
CFLAGS = -I../mqthird/include -g -std=c++17 -D_GLIBCXX_USE_CXX11_ABI=1
LDFLAGS = -L../mqthird/lib -pthread -lmuduo_net -lmuduo_base -lmuduo_http -lmuduo_inspect -lprotobuf -lsqlite3 -lz -lgtest

mqserver: mq_server.cc ../mqcommon/mq_msg.pb.cc ../mqcommon/mq_proto.pb.cc ../mqthird/include/muduo/proto/codec.cc
	g++ -no-pie $(CFLAGS) $^ -o $@ $(LDFLAGS)
clean:
	rm -rf data mqserver