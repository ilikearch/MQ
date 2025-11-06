#ifndef __M_BROKER_H__
#define __M_BROKER_H__
#include "muduo/proto/codec.h"
#include "muduo/proto/dispatcher.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "../mqcommon/mq_threadpool.hpp"
#include "../mqcommon/mq_msg.pb.h"
#include "../mqcommon/mq_proto.pb.h"
#include "../mqcommon/mq_logger.hpp"
#include "mq_connection.hpp"
#include "mq_consumer.hpp"
#include "mq_host.hpp"

namespace mq
{
#define DBFILE "/meta.db"
#define HOSTNAME "MyVirtualHost"
    class Server
    {
    public:
        typedef std::shared_ptr<google::protobuf::Message> MessagePtr;
        Server(int port, const std::string &basedir)
            : _server(&_baseloop, muduo::net::InetAddress("0.0.0.0", port),
                      "Server", muduo::net::TcpServer::kReusePort),
              _dispatcher(std::bind(&Server::onUnknownMessage, this, std::placeholders::_1,
                                    std::placeholders::_2, std::placeholders::_3)),
              _codec(std::make_shared<ProtobufCodec>(std::bind(&ProtobufDispatcher::onProtobufMessage, &_dispatcher,
                                                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))),
              _virtual_host(std::make_shared<VirtualHost>(HOSTNAME, basedir, basedir + DBFILE)),
              _consumer_manager(std::make_shared<ConsumerManager>()),
              _connection_manager(std::make_shared<ConnectionManager>()),
              _threadpool(std::make_shared<threadpool>())
        {
            // 针对历史消息中的所有队列，别忘了，初始化队列的消费者管理结构
            QueueMap qm = _virtual_host->allQueues();
            for (auto &q : qm)
            {
                _consumer_manager->initQueueConsumer(q.first);
            }
            // 注册业务请求处理函数
            _dispatcher.registerMessageCallback<mq::openChannelRequest>(std::bind(&Server::onOpenChannel, this,
                                                                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispatcher.registerMessageCallback<mq::closeChannelRequest>(std::bind(&Server::onCloseChannel, this,
                                                                                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispatcher.registerMessageCallback<mq::declareExchangeRequest>(std::bind(&Server::onDeclareExchange, this,
                                                                                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispatcher.registerMessageCallback<mq::deleteExchangeRequest>(std::bind(&Server::onDeleteExchange, this,
                                                                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispatcher.registerMessageCallback<mq::declareQueueRequest>(std::bind(&Server::onDeclareQueue, this,
                                                                                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispatcher.registerMessageCallback<mq::deleteQueueRequest>(std::bind(&Server::onDeleteQueue, this,
                                                                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispatcher.registerMessageCallback<mq::queueBindRequest>(std::bind(&Server::onQueueBind, this,
                                                                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispatcher.registerMessageCallback<mq::queueUnBindRequest>(std::bind(&Server::onQueueUnBind, this,
                                                                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispatcher.registerMessageCallback<mq::basicPublishRequest>(std::bind(&Server::onBasicPublish, this,
                                                                                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispatcher.registerMessageCallback<mq::basicAckRequest>(std::bind(&Server::onBasicAck, this,
                                                                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispatcher.registerMessageCallback<mq::basicConsumeRequest>(std::bind(&Server::onBasicConsume, this,
                                                                                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _dispatcher.registerMessageCallback<mq::basicCancelRequest>(std::bind(&Server::onBasicCancel, this,
                                                                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

            _server.setMessageCallback(std::bind(&ProtobufCodec::onMessage, _codec.get(),
                                                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            _server.setConnectionCallback(std::bind(&Server::onConnection, this, std::placeholders::_1));
        }
        void start(){
            _server.start();
            _baseloop.loop();
        }
    private:
        
        void onUnknownMessage() {}

    private:
        muduo::net::EventLoop _baseloop;
        muduo::net::TcpServer _server;
        ProtobufDispatcher _dispatcher;
        ProtobufCodecPtr _codec;
        VirtualHost::ptr _virtual_host;
        ConsumerManager::ptr _consumer_manager;
        ConnectionManager::ptr _connection_manager;
        threadpool::ptr _threadpool;
    };
}

#endif