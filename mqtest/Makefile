.PHONY: mq_hosttest
mq_hosttest: mq_hosttest.cpp ../mqcommon/mq_msg.pb.cc
	g++ -g -std=c++17 mq_hosttest.cpp ../mqcommon/mq_msg.pb.cc -o mq_hosttest -lgtest -lprotobuf -lsqlite3 -pthread
mq_msgtest: mq_msgtest.cpp ../mqcommon/mq_msg.pb.cc
	g++ -g -std=c++17 mq_msgtest.cpp ../mqcommon/mq_msg.pb.cc -o mq_msgtest -lgtest -lprotobuf -lsqlite3 -pthread
mq_bindingtest:mq_bindingtest.cpp
	g++ -g -std=c++17 $^ -o $@ -lgtest -lprotobuf -lsqlite3
mq_queuetest:mq_queuetest.cpp
	g++ -g -std=c++17 $^ -o $@ -lgtest -lprotobuf -lsqlite3
mq_routetest:mq_routetest.cpp ../mqcommon/mq_msg.pb.cc
	g++ -g -std=c++11 $^ -o $@ -lgtest -lprotobuf -lsqlite3
mq_exchangetest:mq_exchangetest.cpp
	g++ -g -std=c++17 $^ -o $@ -lgtest -lprotobuf -lsqlite3
mq_filetest:mq_filetest.cpp
	g++ -g -std=c++17 $^ -o $@
clean:
	rm -rf mq_hosttest