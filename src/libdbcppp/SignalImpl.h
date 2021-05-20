#pragma once

#include <string>
#include <memory>

#include <dbcppp/Signal.h>
#include <dbcppp/Node.h>
#include "AttributeImpl.h"
#include "SignalMultiplexerValueImpl.h"

namespace dbcppp
{
    class SignalImpl final
        : public Signal
    {
    public:
        SignalImpl(
              uint64_t message_size
            , std::string&& name
            , Multiplexer multiplexer_indicator
            , uint64_t multiplexer_switch_value
            , uint64_t start_bit
            , uint64_t bit_size
            , ByteOrder byte_order
            , ValueType value_type
            , double factor
            , double offset
            , double minimum
            , double maximum
            , std::string&& unit
            , std::vector<std::string>&& receivers
            , std::vector<AttributeImpl>&& attribute_values
            , std::vector<std::tuple<int64_t, std::string>>&& value_descriptions
            , std::string&& comment
            , Signal::ExtendedValueType extended_value_type
            , std::vector<SignalMultiplexerValueImpl>&& signal_multiplexer_values);
            
        virtual std::unique_ptr<Signal> clone() const override;

        virtual const std::string& getName() const override;
        virtual Multiplexer getMultiplexerIndicator() const override;
        virtual uint64_t getMultiplexerSwitchValue() const override;
        virtual uint64_t getStartBit() const override;
        virtual uint64_t getBitSize() const override;
        virtual ByteOrder getByteOrder() const override;
        virtual ValueType getValueType() const override;
        virtual double getFactor() const override;
        virtual double getOffset() const override;
        virtual double getMinimum() const override;
        virtual double getMaximum() const override;
        virtual std::string getUnit() const override;
        virtual bool hasReceiver(const std::string& name) const override;
        virtual void forEachReceiver(std::function<void(const std::string&)>&& cb) const override;

        virtual const std::string* getValueDescriptionByValue(int64_t value) const override;
        virtual void forEachValueDescription(std::function<void(int64_t, const std::string&)>&& cb) const override;
    
        virtual const Attribute* getAttributeValueByName(const std::string& name) const override;
        virtual const Attribute* findAttributeValue(std::function<bool(const Attribute&)>&& pred) const override;
        virtual void forEachAttributeValue(std::function<void(const Attribute&)>&& cb) const override;
        virtual const std::string& getComment() const override;
        virtual ExtendedValueType getExtendedValueType() const override;
        virtual void forEachSignalMultiplexerValue(std::function<void(const SignalMultiplexerValue&)> cb) const override;
        virtual bool getError(ErrorCode code) const override;
        
        virtual bool operator==(const Signal& rhs) const override;
        virtual bool operator!=(const Signal& rhs) const override;

    private:
        void setError(ErrorCode code);

        std::string _name;
        Multiplexer _multiplexer_indicator;
        uint64_t _multiplexer_switch_value;
        uint64_t _start_bit;
        uint64_t _bit_size;
        ByteOrder _byte_order;
        ValueType _value_type;
        double _factor;
        double _offset;
        double _minimum;
        double _maximum;
        std::string _unit;
        std::vector<std::string> _receivers;
        std::vector<AttributeImpl> _attribute_values;
        std::vector<std::tuple<int64_t, std::string>> _value_descriptions;
        std::string _comment;
        ExtendedValueType _extended_value_type;
        std::vector<SignalMultiplexerValueImpl> _signal_multiplexer_values;

    public:
        // for performance
        uint64_t _mask;
        uint64_t _mask_signed;
        uint64_t _fixed_start_bit_0;
        uint64_t _fixed_start_bit_1;
        uint64_t _byte_pos;

        Signal::ErrorCode _error;
    };
}