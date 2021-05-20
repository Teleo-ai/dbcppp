#pragma once

#include <cstddef>
#include <string>
#include <memory>

#include "Export.h"
#include "Node.h"
#include "Signal.h"
#include "Attribute.h"
#include "SignalGroup.h"

namespace dbcppp
{
    class DBCPPP_API Message
    {
    public:
        enum class ErrorCode
            : uint64_t
        {
            NoError,
            MuxValeWithoutMuxSignal
        };

        static std::unique_ptr<Message> create(
              uint64_t id
            , std::string&& name
            , uint64_t message_size
            , std::string&& transmitter
            , std::vector<std::string>&& message_transmitters
            , std::vector<std::unique_ptr<Signal>>&& signals
            , std::vector<std::unique_ptr<Attribute>>&& attribute_values
            , std::string&& comment
            , std::vector<std::unique_ptr<SignalGroup>>&& signal_groups);

        virtual std::unique_ptr<Message> clone() const = 0;

        virtual ~Message() = default;
        virtual uint64_t getId() const = 0;
        virtual const std::string& getName() const = 0;
        virtual uint64_t getMessageSize() const = 0;
        virtual const std::string& getTransmitter() const = 0;
        virtual bool hasMessageTransmitter(const std::string& name) const = 0;
        virtual void forEachMessageTransmitter(std::function<void(const std::string&)> cb) const = 0;
        virtual const Signal* getSignalByName(const std::string& name) const = 0;
        virtual const Signal* findSignal(std::function<bool(const Signal&)> pred) const = 0;
        virtual void forEachSignal(std::function<void(const Signal&)> cb) const = 0;
        virtual const Attribute* getAttributeValueByName(const std::string& name) const = 0;
        virtual const Attribute* findAttributeValue(std::function<bool(const Attribute&)> pred) const = 0;
        virtual void forEachAttributeValue(std::function<void(const Attribute&)> cb) const = 0;
        virtual const std::string& getComment() const = 0;
        virtual void forEachSignalGroup(std::function<void(const SignalGroup&)> cb) const = 0;
        virtual const Signal* getMuxSignal() const = 0;
        
        virtual bool operator==(const Message& message) const = 0;
        virtual bool operator!=(const Message& message) const = 0;

        virtual ErrorCode getError() const = 0;
    };
}