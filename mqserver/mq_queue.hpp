#ifndef __M_QUEUE_H__
#define __M_QUEUE_H__
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <memory>
#include "../mqcommon/mq_logger.hpp"
#include "../mqcommon/mq_helper.hpp"
#include "../mqcommon/mq_msg.pb.h"

namespace mq
{
    struct MsgQueue
    {
        using ptr = std::shared_ptr<MsgQueue>;
        std::string name;
        bool durable;
        bool exclusive;
        bool auto_delete;
        google::protobuf::Map<std::string, std::string> args;

        MsgQueue() {}
        MsgQueue(const std::string &qname, bool qdurable, bool qexclusive, bool qauto_delete, const google::protobuf::Map<std::string, std::string> &qargs)
            : name(qname),
              durable(qdurable),
              exclusive(qexclusive),
              auto_delete(qauto_delete),
              args(qargs) {}
        void setArgs(const std::string &str_args)
        {
            std::vector<std::string> sub_args;
            StrHelper::split(str_args, "&", sub_args);
            for (auto &str : sub_args)
            {
                size_t pos = str.find("=");
                std::string key = str.substr(0, pos);
                std::string value = str.substr(pos + 1);
                args[key] = value;
            }
        }
        std::string getArgs()
        {
            std::string result;
            for (auto start = args.begin(); start != args.end(); start++)
            {
                result += start->first + "=" + start->second + "&";
            }
            return result;
        }
    };

    using QueueMap = std::unordered_map<std::string, MsgQueue::ptr>;
    class MsgQueueMapper
    {
    public:
        MsgQueueMapper(const std::string &dbfile) : _sql_helper(dbfile)
        {
            std::string path = FileHelper::parentDirectory(dbfile);
            FileHelper::createDirectory(path);
            assert(_sql_helper.open());
            creatTable();
        }
        void creatTable()
        {
            std::stringstream sql;
            sql << "creat table if not exists queue_table(";
            sql << "name varchar(32) primary key, ";
            sql << "durable int, ";
            sql << "exclusive int, ";
            sql << "auto_delete int, ";
            sql << "args varchar(128));";
            assert(_sql_helper.exec(sql.str(), nullptr, nullptr));
        }
        void removeTable()
        {
            std::string sql = "drop table if exists queue_table;";
            _sql_helper.exec(sql, nullptr, nullptr);
        }
        bool insert(MsgQueue::ptr &queue)
        {
            std::stringstream ss;
            ss << "insert into queue_table value(";
            ss << "'" << queue->name << "', ";
            ss << queue->durable << ", ";
            ss << queue->exclusive << ", ";
            ss << queue->auto_delete << ", ";
            ss << "'" << queue->getArgs() << "');";
            return _sql_helper.exec(ss.str(), nullptr, nullptr);
        }
        void remove(const std::string &name)
        {
            // delete from queue_table where name='queue1';
            std::stringstream ss;
            ss << "delate from queue_table where name=";
            ss << "'" << name << "';";
            _sql_helper.exec(ss.str(), nullptr, nullptr);
        }
        QueueMap recovery()
        {
            QueueMap result;
            std::string sql = "select name, durable, exclusive, auto_delete, args from queue_table;";
            _sql_helper.exec(sql, selectCallback, &result);
            return result;
        }

    private:
        static int selectCallback(void *arg, int numcol, char **row, char **fields)
        {
            QueueMap *result = (QueueMap *)arg;
            auto mqp = std::make_shared<MsgQueue>();
            mqp->name = row[0];
            mqp->durable = (bool)std::stoi(row[1]);
            mqp->exclusive = (bool)std::stoi(row[2]);
            mqp->auto_delete = (bool)std::stoi(row[3]);
            if (row[4])
                mqp->setArgs(row[4]);
            result->insert(std::make_pair(mqp->name, mqp));
            return 0;
        }

    private:
        SqliteHelper _sql_helper;
    };

    class MsgqueueManager
    {
    public:
        using ptr = std::shared_ptr<MsgqueueManager>;
        MsgqueueManager(const std::string &dbfile) : _mapper(dbfile)
        {
            _msg_queues = _mapper.recovery();
        }

        bool declareQueue(const std::string &qname,
                          bool qdurable, bool qexclusive, bool qauto_delete,
                          const google::protobuf::Map<std::string, std::string> &qargs)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _msg_queues.find(qname);
            if (it != _msg_queues.end())
            {
                return true;
            }
            MsgQueue::ptr mqp = std::make_shared<MsgQueue>();
            mqp->name = qname;
            mqp->durable = qdurable;
            mqp->exclusive = qexclusive;
            mqp->auto_delete = qauto_delete;
            mqp->args = qargs;
            if (qdurable == true)
            {
                bool ret = _mapper.insert(mqp);
                if (ret == false)
                    return false;
            }
            _msg_queues.insert(std::make_pair(qname, mqp));
            return true;
        }
        void deleteQueue(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _msg_queues.find(name);
            if (it == _msg_queues.end())
            {
                return;
            }
            if (it->second->durable == true)
                _mapper.remove(name);
            _msg_queues.erase(name);
        }

        MsgQueue::ptr selectQueue(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _msg_queues.find(name);
            if (it == _msg_queues.end())
                return MsgQueue::ptr();
            return it->second;
        }

        bool exists(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _msg_queues.find(name);
            if (it == _msg_queues.end())
                return false;
            return true;
        }
        size_t size()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _msg_queues.size();
        }

        void clear()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _mapper.removeTable();
            _msg_queues.clear();
        }

    private:
        std::mutex _mutex;
        MsgQueueMapper _mapper;
        QueueMap _msg_queues;
    };
}
#endif