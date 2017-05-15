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

#include <bond/ext/grpc/detail/cq_poller.h>

#include <boost/assert.hpp>
#include <boost/optional/optional.hpp>
#include <memory>
#include <thread>

namespace bond { namespace ext { namespace gRPC {

    /// @brief Models a gRPC proxy powered by Bond services.
    ///
    class proxy
    {
    public:
        proxy(std::unique_ptr<grpc::CompletionQueue> cq)
            : _cqPoller(std::move(cq))
        {
        }

        ~proxy()
        {
            Shutdown();
            Wait();
        }

        /*
        proxy(const proxy&) = delete;
        proxy& operator=(const proxy&) = delete;

        proxy(proxy&&) = default;
        proxy& operator=(proxy&&) = default;
        */

        /// Shutdown the proxy, waiting for all rpc processing to finish.
        void Shutdown()
        {
            _cqPoller.shutdown();
        }

        /// @brief Block waiting for all work to complete.
        ///
        /// @warning The proxy must be either shutting down or some other
        /// thread must call \p Shutdown for this function to ever return.
        void Wait()
        {
            _cqPoller.wait();
        }

        void start()
        {
            _cqPoller.start();
        }

    private:

        detail::cq_poller _cqPoller;
    };

} } } //namespace bond::ext::gRPC
