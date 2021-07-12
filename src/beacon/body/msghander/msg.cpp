//https://github.com/khuttun/PolyM

#include "msg.hpp"
#include <atomic>

namespace PolyM {

namespace {

MsgUID generateUniqueId()
{
    static std::atomic<MsgUID> i(0);
    return ++i;
}

}

Msg::Msg(int msgId)
  : msgId_(msgId), uniqueId_(generateUniqueId())
{
}

std::unique_ptr<Msg> Msg::move()
{
    return std::unique_ptr<Msg>(new Msg(std::move(*this)));
}

int Msg::getMsgId() const
{
    return msgId_;
}

MsgUID Msg::getUniqueId() const
{
    return uniqueId_;
}

}
