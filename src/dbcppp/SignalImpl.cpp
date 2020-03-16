
#include <limits>
#include <boost/endian/conversion.hpp>
#include "SignalImpl.h"

using namespace dbcppp;

enum class Alignment
{
	size_inbetween_first_64_bit,
	signal_exceeds_64_bit_size_but_signal_fits_into_64_bit,
	signal_exceeds_64_bit_size_and_signal_does_not_fit_into_64_bit
};

template <Alignment aAlignment, Signal::ByteOrder aByteOrder, Signal::ValueType aValueType, Signal::ExtendedValueType aExtendedValueType>
double template_decode(const Signal* sig, const void* nbytes) noexcept
{
	const SignalImpl* sigi = static_cast<const SignalImpl*>(sig);
	union
	{
		uint64_t ui;
		int64_t i;
		float f;
		double d;
	} data;
	if constexpr (aAlignment == Alignment::signal_exceeds_64_bit_size_and_signal_does_not_fit_into_64_bit)
	{
		data.ui = *reinterpret_cast<const uint64_t*>(&reinterpret_cast<const uint8_t*>(nbytes)[sigi->_byte_pos]);
		uint64_t data1 = reinterpret_cast<const uint8_t*>(nbytes)[sigi->_byte_pos + 8];
		if constexpr (aByteOrder == Signal::ByteOrder::BigEndian)
		{
			boost::endian::native_to_big_inplace(data.ui);
			data.ui &= sigi->_mask;
			data.ui <<= sigi->_fixed_start_bit_0;
			data1 >>= sigi->_fixed_start_bit_1;
			data.ui |= data1;
		}
		else
		{
			boost::endian::native_to_little_inplace(data.ui);
			data.ui >>= sigi->_fixed_start_bit_0;
			data1 &= sigi->_mask;
			data1 <<= sigi->_fixed_start_bit_1;
			data.ui |= data1;
		}
		if constexpr (aExtendedValueType == Signal::ExtendedValueType::Double)
		{
			return data.d;
		}
		if constexpr (aExtendedValueType == Signal::ExtendedValueType::Float)
		{
			return data.f;
		}
		if constexpr (aValueType == Signal::ValueType::Signed)
		{
			if (data.ui & sigi->_mask_signed)
			{
				data.ui |= sigi->_mask_signed;
			}
			return double(data.i);
		}
		return double(data.ui);
	}
	else
	{
		if constexpr (aAlignment == Alignment::size_inbetween_first_64_bit)
		{
			data.ui = *reinterpret_cast<const uint64_t*>(nbytes);
		}
		else
		{
			data.ui = *reinterpret_cast<const uint64_t*>(&reinterpret_cast<const uint8_t*>(nbytes)[sigi->_byte_pos]);
		}
		if constexpr (aByteOrder == Signal::ByteOrder::BigEndian)
		{
			boost::endian::native_to_big_inplace(data.ui);
		}
		else
		{
			boost::endian::native_to_little_inplace(data.ui);
		}
		if constexpr (aExtendedValueType == Signal::ExtendedValueType::Double)
		{
			return data.d;
		}
		data.ui >>= sigi->_fixed_start_bit_0;
	}
	if constexpr (aExtendedValueType == Signal::ExtendedValueType::Float)
	{
		return data.f;
	}
	data.ui &= sigi->_mask;
	if constexpr (aValueType == Signal::ValueType::Signed)
	{
		// bit extending
		// trust the compiler to optimize this
		if (data.ui & sigi->_mask_signed)
		{
			data.ui |= sigi->_mask_signed;
		}
		return double(data.i);
	}
	return double(data.ui);
}

constexpr uint64_t enum_mask(Alignment a, Signal::ByteOrder bo, Signal::ValueType vt, Signal::ExtendedValueType evt)
{
	uint64_t result = 0;
	switch (a)
	{
	case Alignment::size_inbetween_first_64_bit:									result |= 0b1; break;
	case Alignment::signal_exceeds_64_bit_size_but_signal_fits_into_64_bit:			result |= 0b10; break;
	case Alignment::signal_exceeds_64_bit_size_and_signal_does_not_fit_into_64_bit: result |= 0b100; break;
	}
	switch (bo)
	{
	case Signal::ByteOrder::LittleEndian:											result |= 0b1000; break;
	case Signal::ByteOrder::BigEndian:												result |= 0b10000; break;
	}
	switch (vt)
	{
	case Signal::ValueType::Signed:													result |= 0b100000; break;
	case Signal::ValueType::Unsigned:												result |= 0b1000000; break;
	}
	switch (evt)
	{
	case Signal::ExtendedValueType::Integer:										result |= 0b10000000; break;
	case Signal::ExtendedValueType::Float:											result |= 0b100000000; break;
	case Signal::ExtendedValueType::Double:											result |= 0b1000000000; break;
	}
	return result;
}
using decode_func_t = double (*)(const Signal*, const void*) noexcept;
decode_func_t make_decode(Alignment a, Signal::ByteOrder bo, Signal::ValueType vt, Signal::ExtendedValueType evt)
{
	constexpr auto si64b			= Alignment::size_inbetween_first_64_bit;
	constexpr auto se64bsbsfi64b	= Alignment::signal_exceeds_64_bit_size_but_signal_fits_into_64_bit;
	constexpr auto se64bsasdnfi64b	= Alignment::signal_exceeds_64_bit_size_and_signal_does_not_fit_into_64_bit;
	constexpr auto le				= Signal::ByteOrder::LittleEndian;
	constexpr auto be				= Signal::ByteOrder::BigEndian;
	constexpr auto sig				= Signal::ValueType::Signed;
	constexpr auto usig				= Signal::ValueType::Unsigned;
	constexpr auto i				= Signal::ExtendedValueType::Integer;
	constexpr auto f				= Signal::ExtendedValueType::Float;
	constexpr auto d				= Signal::ExtendedValueType::Double;
	switch (enum_mask(a, bo, vt, evt))
	{
	case enum_mask(si64b, le, sig, i):				return template_decode<si64b, le, sig, i>;
	case enum_mask(si64b, le, sig, f):				return template_decode<si64b, le, sig, f>;
	case enum_mask(si64b, le, sig, d):				return template_decode<si64b, le, sig, d>;
	case enum_mask(si64b, le, usig, i):				return template_decode<si64b, le, usig, i>;
	case enum_mask(si64b, le, usig, f):				return template_decode<si64b, le, usig, f>;
	case enum_mask(si64b, le, usig, d):				return template_decode<si64b, le, usig, d>;
	case enum_mask(si64b, be, sig, i):				return template_decode<si64b, be, sig, i>;
	case enum_mask(si64b, be, sig, f):				return template_decode<si64b, be, sig, f>;
	case enum_mask(si64b, be, sig, d):				return template_decode<si64b, be, sig, d>;
	case enum_mask(si64b, be, usig, i):				return template_decode<si64b, be, usig, i>;
	case enum_mask(si64b, be, usig, f):				return template_decode<si64b, be, usig, f>;
	case enum_mask(si64b, be, usig, d):				return template_decode<si64b, be, usig, d>;
	case enum_mask(se64bsbsfi64b, le, sig, i):		return template_decode<se64bsbsfi64b, le, sig, i>;
	case enum_mask(se64bsbsfi64b, le, sig, f):		return template_decode<se64bsbsfi64b, le, sig, f>;
	case enum_mask(se64bsbsfi64b, le, sig, d):		return template_decode<se64bsbsfi64b, le, sig, d>;
	case enum_mask(se64bsbsfi64b, le, usig, i):		return template_decode<se64bsbsfi64b, le, usig, i>;
	case enum_mask(se64bsbsfi64b, le, usig, f):		return template_decode<se64bsbsfi64b, le, usig, f>;
	case enum_mask(se64bsbsfi64b, le, usig, d):		return template_decode<se64bsbsfi64b, le, usig, d>;
	case enum_mask(se64bsbsfi64b, be, sig, i):		return template_decode<se64bsbsfi64b, be, sig, i>;
	case enum_mask(se64bsbsfi64b, be, sig, f):		return template_decode<se64bsbsfi64b, be, sig, f>;
	case enum_mask(se64bsbsfi64b, be, sig, d):		return template_decode<se64bsbsfi64b, be, sig, d>;
	case enum_mask(se64bsbsfi64b, be, usig, i):		return template_decode<se64bsbsfi64b, be, usig, i>;
	case enum_mask(se64bsbsfi64b, be, usig, f):		return template_decode<se64bsbsfi64b, be, usig, f>;
	case enum_mask(se64bsbsfi64b, be, usig, d):		return template_decode<se64bsbsfi64b, be, usig, d>;
	case enum_mask(se64bsasdnfi64b, le, sig, i):	return template_decode<se64bsasdnfi64b, le, sig, i>;
	case enum_mask(se64bsasdnfi64b, le, sig, f):	return template_decode<se64bsasdnfi64b, le, sig, f>;
	case enum_mask(se64bsasdnfi64b, le, sig, d):	return template_decode<se64bsasdnfi64b, le, sig, d>;
	case enum_mask(se64bsasdnfi64b, le, usig, i):	return template_decode<se64bsasdnfi64b, le, usig, i>;
	case enum_mask(se64bsasdnfi64b, le, usig, f):	return template_decode<se64bsasdnfi64b, le, usig, f>;
	case enum_mask(se64bsasdnfi64b, le, usig, d):	return template_decode<se64bsasdnfi64b, le, usig, d>;
	case enum_mask(se64bsasdnfi64b, be, sig, i):	return template_decode<se64bsasdnfi64b, be, sig, i>;
	case enum_mask(se64bsasdnfi64b, be, sig, f):	return template_decode<se64bsasdnfi64b, be, sig, f>;
	case enum_mask(se64bsasdnfi64b, be, sig, d):	return template_decode<se64bsasdnfi64b, be, sig, d>;
	case enum_mask(se64bsasdnfi64b, be, usig, i):	return template_decode<se64bsasdnfi64b, be, usig, i>;
	case enum_mask(se64bsasdnfi64b, be, usig, f):	return template_decode<se64bsasdnfi64b, be, usig, f>;
	case enum_mask(se64bsasdnfi64b, be, usig, d):	return template_decode<se64bsasdnfi64b, be, usig, d>;
	}
	return nullptr;
}

double raw_to_phys(const Signal* sig, double raw) noexcept
{
	const SignalImpl* sigi = static_cast<const SignalImpl*>(sig);
	return raw * sigi->getFactor() + sigi->getOffset();
}
double phys_to_raw(const Signal* sig, double phys) noexcept
{
	const SignalImpl* sigi = static_cast<const SignalImpl*>(sig);
	return (phys - sigi->getOffset()) / sigi->getFactor();
}
std::unique_ptr<Signal> Signal::create(
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
	, std::set<std::string>&& receivers
	, std::map<std::string, std::unique_ptr<Attribute>>&& attribute_values
	, std::map<double, std::string>&& value_descriptions
	, std::string&& comment
	, Signal::ExtendedValueType extended_value_type)
{
	std::unique_ptr<SignalImpl> result;
	std::map<std::string, AttributeImpl> avs;
	for (auto& av : attribute_values)
	{
		avs.insert(std::make_pair(av.first, std::move(*static_cast<AttributeImpl*>(av.second.get()))));
		av.second.reset(nullptr);
	}
	result = std::make_unique<SignalImpl>(
		  message_size
		, std::move(name)
		, multiplexer_indicator
		, multiplexer_switch_value
		, start_bit
		, bit_size
		, byte_order
		, value_type
		, factor
		, offset
		, minimum
		, maximum
		, std::move(unit)
		, std::move(receivers)
		, std::move(avs)
		, std::move(value_descriptions)
		, std::move(comment)
		, extended_value_type);
	if (result->getError() != SignalImpl::ErrorCode::NoError)
	{
		result = nullptr;
	}
	return result;
}
SignalImpl::SignalImpl(
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
	, std::set<std::string>&& receivers
	, std::map<std::string, AttributeImpl>&& attribute_values
	, std::map<double, std::string>&& value_descriptions
	, std::string&& comment
	, ExtendedValueType extended_value_type)
	
	: _name(std::move(name))
	, _multiplexer_indicator(std::move(multiplexer_indicator))
	, _multiplexer_switch_value(std::move(multiplexer_switch_value))
	, _start_bit(std::move(start_bit))
	, _bit_size(std::move(bit_size))
	, _byte_order(std::move(byte_order))
	, _value_type(std::move(value_type))
	, _factor(std::move(factor))
	, _offset(std::move(offset))
	, _minimum(std::move(minimum))
	, _maximum(std::move(maximum))
	, _unit(std::move(unit))
	, _receivers(std::move(receivers))
	, _attribute_values(std::move(attribute_values))
	, _value_descriptions(std::move(value_descriptions))
	, _comment(std::move(comment))
	, _extended_value_type(std::move(extended_value_type))
	, _error(Signal::ErrorCode::NoError)
{
	message_size = message_size < 8 ? 8 : message_size;
	// check for out of frame size error
	switch (byte_order)
	{
	case ByteOrder::LittleEndian:
		if ((start_bit + bit_size) > message_size * 8)
		{
			_error = ErrorCode::SignalExceedsMessageSize;
		}
		break;
	case ByteOrder::BigEndian:
		uint64_t fsize = bit_size + (7 - (start_bit % 8));
		int64_t fstart = int64_t(start_bit) - (start_bit % 8);
		if (fstart + ((fsize - 1) / 8) * 8 >= message_size * 8)
		{
			_error = ErrorCode::SignalExceedsMessageSize;
		}
		break;
	}
	switch (extended_value_type)
	{
	case Signal::ExtendedValueType::Float:
		if (bit_size != 32)
		{
			_error = ErrorCode::WrongBitSizeForExtendedDataType;
		}
		break;
	case Signal::ExtendedValueType::Double:
		if (bit_size != 64)
		{
			_error = ErrorCode::WrongBitSizeForExtendedDataType;
		}
		break;
	}
	if (extended_value_type == ExtendedValueType::Float && !std::numeric_limits<float>::is_iec559)
	{
		_error = ErrorCode::MaschinesFloatEncodingNotSupported;
	}
	if (extended_value_type == ExtendedValueType::Double && !std::numeric_limits<double>::is_iec559)
	{
		_error = ErrorCode::MaschinesDoubleEncodingNotSupported;
	}

	// save some additional values to speed up decoding
	_mask =  (1ull << (_bit_size - 1ull) << 1ull) - 1;
	_mask_signed = ~((1ull << (_bit_size - 1ull)) - 1);

	_byte_pos = _start_bit / 8;

	uint64_t nbytes;
	if (_byte_order == ByteOrder::LittleEndian)
	{
		nbytes = (_start_bit % 8 + _bit_size + 7) / 8;
	}
	else
	{
		nbytes = (_bit_size + (7 - _start_bit % 8) + 7) / 8;
	}
	Alignment alignment = Alignment::size_inbetween_first_64_bit;
	// check whether the data is in the first 8 bytes
	// so we can optimize out one memory access
	if (_byte_pos + nbytes <= 8)
	{
		alignment = Alignment::size_inbetween_first_64_bit;
		if (_byte_order == ByteOrder::LittleEndian)
		{
			_fixed_start_bit_0 = _start_bit;
		}
		else
		{
			_fixed_start_bit_0 = (8 * (7 - (_start_bit / 8))) + (_start_bit % 8) - (_bit_size - 1);
		}
	}
	// check whether we can align the data on 64 bit
	else if (_byte_pos  % 8 + nbytes <= 8)
	{
		alignment = Alignment::signal_exceeds_64_bit_size_but_signal_fits_into_64_bit;
		// align the byte pos on 64 bit
		_byte_pos -= _byte_pos % 8;
		_fixed_start_bit_0 = _start_bit - _byte_pos * 8;
		if (_byte_order == ByteOrder::BigEndian)
		{
			_fixed_start_bit_0 = (8 * (7 - (_fixed_start_bit_0 / 8))) + (_fixed_start_bit_0 % 8) - (_bit_size - 1);
		}
	}
	// we aren't able to align the data on 64 bit, so check whether the data fits into on uint64_t
	else if (nbytes <= 8)
	{
		alignment = Alignment::signal_exceeds_64_bit_size_but_signal_fits_into_64_bit;
		_fixed_start_bit_0 = _start_bit - _byte_pos * 8;
		if (_byte_order == ByteOrder::BigEndian)
		{
			_fixed_start_bit_0 = (8 * (7 - (_fixed_start_bit_0 / 8))) + (_fixed_start_bit_0 % 8) - (_bit_size - 1);
		}
	}
	// we aren't able to align the data on 64 bit, and we aren't able to fit the data into one uint64_t
	// so we have to compose the resulting value
	else
	{
		alignment = Alignment::signal_exceeds_64_bit_size_and_signal_does_not_fit_into_64_bit;
		if (_byte_order == ByteOrder::BigEndian)
		{
			uint64_t nbits_last_byte = (7 - _start_bit % 8) + _bit_size - 64;
			_fixed_start_bit_0 = nbits_last_byte;
			_fixed_start_bit_1 = 8 - nbits_last_byte;
			_mask = (1ull << (_start_bit % 8 + 57)) - 1;
		}
		else
		{
			_fixed_start_bit_0 = _start_bit - _byte_pos * 8;
			_fixed_start_bit_1 = 64 - _start_bit % 8;
			uint64_t nbits_last_byte = _bit_size + _start_bit % 8 - 64;
			_mask = (1ull << nbits_last_byte) - 1ull;
		}
	}

	_decode = ::make_decode(alignment, _byte_order, _value_type, _extended_value_type);

	_raw_to_phys = ::raw_to_phys;
	_phys_to_raw = ::phys_to_raw;
}
const std::string& SignalImpl::getName() const
{
	return _name;
}
Signal::Multiplexer SignalImpl::getMultiplexerIndicator() const
{
	return _multiplexer_indicator;
}
uint64_t SignalImpl::getMultiplexerSwitchValue() const
{
	return _multiplexer_switch_value;
}
uint64_t SignalImpl::getStartBit() const
{
	return _start_bit;
}
uint64_t SignalImpl::getBitSize() const
{
	return _bit_size;
}
Signal::ByteOrder SignalImpl::getByteOrder() const
{
	return _byte_order;
}
Signal::ValueType SignalImpl::getValueType() const
{
	return _value_type;
}
double SignalImpl::getFactor() const
{
	return _factor;
}
double SignalImpl::getOffset() const
{
	return _offset;
}
double SignalImpl::getMinimum() const
{
	return _minimum;
}
double SignalImpl::getMaximum() const
{
	return _maximum;
}
std::string SignalImpl::getUnit() const
{
	return _unit;
}
bool SignalImpl::hasReceiver(const std::string& name) const
{
	return _receivers.find(name) != _receivers.end();
}
void SignalImpl::forEachReceiver(std::function<void(const std::string&)>&& cb) const
{
	for (const auto& n : _receivers)
	{
		cb(n);
	}
}
const std::string* SignalImpl::getValueDescriptionById(double id) const
{
	const std::string* result = nullptr;
	auto iter = _value_descriptions.find(id);
	if (iter != _value_descriptions.end())
	{
		result = &iter->second;
	}
	return result;
}
void SignalImpl::forEachValueDescription(std::function<void(double, const std::string&)>&& cb) const
{
	for (auto& av : _value_descriptions)
	{
		cb(av.first, av.second);
	}
}
const Attribute* SignalImpl::getAttributeValueByName(const std::string& name) const
{
	const Attribute* result = nullptr;
	auto iter = _attribute_values.find(name);
	if (iter != _attribute_values.end())
	{
		result = &iter->second;
	}
	return result;
}
const Attribute* SignalImpl::findAttributeValue(std::function<bool(const Attribute&)>&& pred) const
{
	const Attribute* result = nullptr;
	for (const auto& av : _attribute_values)
	{
		if (pred(av.second))
		{
			result = &av.second;
			break;
		}
	}
	return result;
}
const void SignalImpl::forEachAttributeValue(std::function<void(const Attribute&)>&& cb) const
{
	for (const auto& av : _attribute_values)
	{
		cb(av.second);
	}
}
const std::string& SignalImpl::getComment() const
{
	return _comment;
}
Signal::ExtendedValueType SignalImpl::getExtendedValueType() const
{
	return _extended_value_type;
}
Signal::ErrorCode SignalImpl::getError() const
{
	return _error;
}

void Signal::serializeToStream(std::ostream& os) const
{
	os << "SG_ " << getName() << " ";
	switch (getMultiplexerIndicator())
	{
	case Signal::Multiplexer::MuxSwitch: os << "M "; break;
	case Signal::Multiplexer::MuxValue: os << "m" << getMultiplexerSwitchValue() << " "; break;
	}
	os << ": " << getStartBit() << "|" << getBitSize() << "@";
	switch (getByteOrder())
	{
	case Signal::ByteOrder::BigEndian: os << "0"; break;
	case Signal::ByteOrder::LittleEndian: os << "1"; break;
	}
	switch (getValueType())
	{
	case Signal::ValueType::Unsigned: os << "+ "; break;
	case Signal::ValueType::Signed: os << "- "; break;
	}
	os << "(" << getFactor() << "," << getOffset() << ") ";
	os << "[" << getMinimum() << "|" << getMaximum() << "] ";
	os << "\"" << getUnit() << "\"";
	forEachReceiver(
		[&](const std::string& n)
		{
			os << " " << n;
		});
}
