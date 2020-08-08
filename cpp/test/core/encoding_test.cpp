#include "precompiled.h"

#include <bond/protocol/encoding.h>

#include <limits>
#include <stdexcept>

BOOST_AUTO_TEST_SUITE(EncodingTests)

// A minimal implementation of the Input Stream concept that does not
// implement an optimized ReadVariableUnsigned.
//
// Exposes the current offset for assertion purposes.
//
// This is missing large parts of the Input Stream concept, but the test
// doesn't use those...
class BasicInputStream
{
public:
    explicit BasicInputStream(const bond::blob& blob)
        : _blob(blob),
          _offset()
    {}

    void Read(uint8_t& value)
    {
        if (_blob.length() == _offset)
        {
            throw std::out_of_range("Attempting to read beyond end of buffer.");
        }

        value = static_cast<const uint8_t>(_blob.content()[_offset++]);
    }

    size_t Offset() const
    {
        return _offset;
    }

private:
    bond::blob _blob;
    size_t _offset;
};

BOOST_AUTO_TEST_CASE(GenericReadWontReadTooMuch)
{
    // A buffer that only has var int continuation bytes. When reading a var
    // int, the reader should only read up to the maximum number of bytes
    // for a given width.
    std::vector<uint8_t> allContinuationBytes(10, 0x80);
    bond::blob blob(
        allContinuationBytes.data(),
        allContinuationBytes.size());

    {
        BasicInputStream bis(blob);

        uint16_t result;
        BOOST_CHECK_THROW(
            bond::GenericReadVariableUnsigned(bis, result),
            bond::StreamException);
        BOOST_CHECK_EQUAL(3, bis.Offset());
    }

    {
        BasicInputStream bis(blob);

        uint32_t result;
        BOOST_CHECK_THROW(
            bond::GenericReadVariableUnsigned(bis, result),
            bond::StreamException);
        BOOST_CHECK_EQUAL(5, bis.Offset());
    }

    {
        BasicInputStream bis(blob);

        uint64_t result;
        BOOST_CHECK_THROW(
            bond::GenericReadVariableUnsigned(bis, result),
            bond::StreamException);
        BOOST_CHECK_EQUAL(10, bis.Offset());
    }
}

BOOST_AUTO_TEST_CASE(GenericReadMaxValues)
{
    {
        std::array<uint8_t, 3> maxValueEncoded = { 0xFF, 0xFF, 0x03 };
        bond::blob blob(maxValueEncoded.data(), maxValueEncoded.size());
        BasicInputStream bis(blob);

        uint16_t result;
        bond::GenericReadVariableUnsigned(bis, result);
        BOOST_CHECK_EQUAL(std::numeric_limits<uint16_t>::max(), result);
        BOOST_CHECK_EQUAL(3, bis.Offset());
    }

    {
        std::array<uint8_t, 5> maxValueEncoded = { 0xFF, 0xFF, 0xFF, 0xFF, 0x0F };
        bond::blob blob(maxValueEncoded.data(), maxValueEncoded.size());
        BasicInputStream bis(blob);

        uint32_t result;
        bond::GenericReadVariableUnsigned(bis, result);
        BOOST_CHECK_EQUAL(std::numeric_limits<uint32_t>::max(), result);
        BOOST_CHECK_EQUAL(5, bis.Offset());
    }

    {
        std::array<uint8_t, 10> maxValueEncoded = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01 };
        bond::blob blob(maxValueEncoded.data(), maxValueEncoded.size());
        BasicInputStream bis(blob);

        uint64_t result;
        bond::GenericReadVariableUnsigned(bis, result);
        BOOST_CHECK_EQUAL(std::numeric_limits<uint64_t>::max(), result);
        BOOST_CHECK_EQUAL(10, bis.Offset());
    }
}

BOOST_AUTO_TEST_SUITE_END()

bool init_unit_test()
{
    return true;
}
