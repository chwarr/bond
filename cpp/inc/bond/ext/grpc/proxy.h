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

        ~proxy()
        {
            Shutdown();
            Wait();
        }

        proxy(const proxy&) = delete;
        proxy& operator=(const proxy&) = delete;

        proxy(proxy&&) = default;
        proxy& operator=(proxy&&) = default;

        /// Shutdown the proxy, waiting for all rpc processing to finish.
        void Shutdown()
        {
            _ioManager->shutdown();
        }

        /// @brief Block waiting for all work to complete.
        ///
        /// @warning The proxy must be either shutting down or some other
        /// thread must call \p Shutdown for this function to ever return.
        void Wait()
        {
            _ioManager->wait();
        }

        void start()
        {
            _ioManager->start();
        }

    private:
        std::shared_ptr<io_manager> _ioManager;
    };

} } } //namespace bond::ext::gRPC
