// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#ifdef _MSC_VER
    #pragma warning (push)
    #pragma warning (disable: 4100 4702)
#endif

#include <grpc++/grpc++.h>

#ifdef _MSC_VER
    #pragma warning (pop)
#endif

#include <bond/ext/grpc/io_manager.h>
#include <memory>

namespace bond { namespace ext { namespace gRPC {

    /// @brief Models a gRPC proxy powered by Bond services.
    ///
    class proxy
    {
    public:
        proxy(std::shared_ptr<io_manager> ioManager)
        {
            _ioManager = ioManager;
        }

        proxy(const proxy&) = delete;
        proxy& operator=(const proxy&) = delete;

        proxy(proxy&&) = default;
        proxy& operator=(proxy&&) = default;

    protected:
        std::shared_ptr<io_manager> _ioManager;
    };

} } } //namespace bond::ext::gRPC
