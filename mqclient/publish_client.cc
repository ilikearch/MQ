#include "mq_connection.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    std::cout << "启动发布者..." << std::endl;
    
    //1. 实例化异步工作线程对象
    mq::AsyncWorker::ptr awp = std::make_shared<mq::AsyncWorker>();
    //2. 实例化连接对象
    mq::Connection::ptr conn = std::make_shared<mq::Connection>("127.0.0.1", 8085, awp);
    //3. 通过连接创建信道
    mq::Channel::ptr channel = conn->openChannel();
    
    std::cout << "连接建立成功，等待连接稳定..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    //4. 通过信道提供的服务完成所需
    google::protobuf::Map<std::string, std::string> tmp_map;
    
    channel->declareExchange("exchange1", mq::ExchangeType::FANOUT, true, false, tmp_map);
    
    std::cout << "声明队列..." << std::endl;
    channel->declareQueue("queue1", true, false, false, tmp_map);
    channel->declareQueue("queue2", true, false, false, tmp_map);
    
    std::cout << "绑定队列..." << std::endl;
    channel->queueBind("exchange1", "queue1", "queue1");
    channel->queueBind("exchange1", "queue2", "news.music.#");
    
    // 等待声明完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    //5. 循环向交换机发布消息
    std::cout << "开始发布消息..." << std::endl;
    
    for (int i = 0; i < 10; i++) {
        mq::BasicProperties bp;
        bp.set_id(mq::UUIDHelper::uuid());
        bp.set_delivery_mode(mq::DeliveryMode::DURABLE);
        //bp.set_routing_key("news.music.pop");  // 这个应该匹配 queue2 的绑定键 news.music.#
        bp.set_routing_key("queue1");//只匹配queue1


        std::string message = "Hello World-" + std::to_string(i);
        channel->basicPublish("exchange1", &bp, message);
        std::cout << "发布: " << message << std::endl;
        
        // 小延迟，避免发送过快
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    mq::BasicProperties bp;
    bp.set_id(mq::UUIDHelper::uuid());
    bp.set_delivery_mode(mq::DeliveryMode::DURABLE);
    
    bp.set_routing_key("news.music.sport");  // 这个应该匹配 queue2 的绑定键 news.music.#
    channel->basicPublish("exchange1", &bp, "Hello");
    std::cout << "发布: Hello " << std::endl;
    
    bp.set_routing_key("news.sport");  // 这个不应该匹配任何绑定键
    channel->basicPublish("exchange1", &bp, "Hello chileme?");
    std::cout << "发布: Hello chileme?" << std::endl;
    
    // 关键：等待所有消息发送完成
    std::cout << "等待消息发送完成..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    //6. 关闭信道
    std::cout << "关闭连接..." << std::endl;
    conn->closeChannel(channel);
    
    std::cout << "发布者退出" << std::endl;
    return 0;
}