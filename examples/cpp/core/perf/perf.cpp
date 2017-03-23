#include "allocator.h"
#include "compat_apply.h"
#include "compat_reflection.h"

#include <bond/core/bond.h>
#include <bond/stream/output_buffer.h>

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_array.hpp>

#include <chrono>
#include <errno.h>
#include <exception>
#include <iostream>
#include <memory>
#include <stdarg.h>
#include <stdio.h>
#include <vector>

using namespace unittest::compat;

static const size_t MAX_SIZE = 6 * 1024 * 1024;

void die(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    exit(1);
}

std::vector<char> read_from_file(const char* const inputPath)
{
    std::vector<char> inputBuffer(MAX_SIZE);

    FILE* file;
    int err = fopen_s(&file, inputPath, "rb");
    if (err != 0)
    {
        die("\nCan't open %s: %d\n", inputPath, err);
    }

    char* readPos = &inputBuffer[0];

    do
    {
        size_t inputBufferSpaceLeft = inputBuffer.size() - static_cast<size_t>(readPos - &inputBuffer[0]);

        size_t readCount = fread(readPos, 1, inputBufferSpaceLeft, file);
        readPos += readCount;

        if (readCount == 0)
        {
            if (ferror(file))
            {
                die("Error while reading %s\n", inputPath);
            }
        }
    } while (!feof(file));

    const size_t finalSize = static_cast<size_t>(readPos - &inputBuffer[0]);

    inputBuffer.resize(finalSize);
    return inputBuffer;
}

std::chrono::high_resolution_clock::duration
perf_test(const std::vector<char>& inputBuffer, size_t iterations)
{
    typedef PoolAlloc::rebind<Compat>::other CompatAlloc;

    // Deserialize the object to serialize from the provided buffer
    bond::blob input(&inputBuffer[0], static_cast<uint32_t>(inputBuffer.size()));

    {
        // Do one deserialization outside of timing in case anything needs
        // to be warmed up. (e.g., page in deserialization code)
        bond::CompactBinaryReader<bond::InputBuffer> reader(input);

        Compat* pObj = CompatAlloc::allocate(1);
        ::new (static_cast<void*>(pObj)) Compat();

        Deserialize(reader, *pObj);

        pObj->~Compat();
        CompatAlloc::deallocate(pObj, 1);
    }

    const auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < iterations; ++i)
    {
        bond::CompactBinaryReader<bond::InputBuffer> reader(input);

        Compat* pObj = CompatAlloc::allocate(1);
        ::new (static_cast<void*>(pObj)) Compat();

        Deserialize(reader, *pObj);

        pObj->~Compat();
        CompatAlloc::deallocate(pObj, 1);
    }

    const auto end = std::chrono::high_resolution_clock::now();
    return end - start;
}

int main(int argc, const char* argv[])
{
    try
    {
        if (argc != 2)
        {
            die("Usage: %s input-file\n", argv[0]);
        }

        const char* inputPath = argv[1];

        std::vector<char> inputBuffer = read_from_file(inputPath);

        auto testDuration = perf_test(inputBuffer, 100);
        auto testDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(testDuration);

        std::cout << "Took " << testDurationMs.count() << "ms" << std::endl;
    }
    catch (const std::exception& ex)
    {
        die("Unhandled exception: %s\n", ex.what());
    }

    return 0;
}
