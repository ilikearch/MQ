# 编译器和标志
CXX = g++
CXXFLAGS = -std=c++17 -pthread
LDFLAGS = -lgtest  -pthread

# 所有目标
TARGETS = suit global assert

# 默认目标
all: $(TARGETS)

# 每个测试可执行文件
suit: suit.cc
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

global: global.cc
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

assert: assert.cc
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# 清理生成的文件
clean:
	rm -f $(TARGETS)

.PHONY: all clean