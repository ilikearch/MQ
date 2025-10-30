.PHONY: mq_connectiontest
mq_filetest:mq_filetest.cc
	g++ -g -std=c++17 $^ -o $@
mq_routetest:mq_routetest.cpp ../mqcommon/mq_msg.pb.cc
	g++ -g -std=c++17 $^ -o $@ -lgtest -lprotobuf -lsqlite3
clean:
	rm -rf mq_filetest mq_exchangetest mq_routetest