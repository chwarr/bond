// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#ifdef _MSC_VER
    #pragma warning (push)
    #pragma warning (disable: 4100 4702)
#endif


#include <grpc++/grpc++.h>
#include <grpc++/impl/codegen/status.h>

#ifdef _MSC_VER
    #pragma warning (pop)
#endif

#include <bond/ext/grpc/io_mgr.h>

#include <boost/assert.hpp>
#include <atomic>

namespace bond { namespace ext { namespace gRPC {

namespace detail {

    /// @brief Implementation class that holds the state associated with a single async, unary
    /// call. It manages its own lifetime: to send a response, it enques
    /// itself in the completion queue. When that sending of the response is
    /// done and it is dequed from the completion queue, it deletes itself.
    template <typename TRequest, typename TResponse>
    struct unary_call_impl final : detail::io_mgr_tag
    {
        grpc::ServerContext _context;
        TRequest _request;
        grpc::ServerAsyncResponseWriter<TResponse> _responder;
        std::atomic_flag _responseSentFlag;

        unary_call_impl()
            : _context(),
            _request(),
            _responder(&_context),
            _responseSentFlag()
        { }

        unary_call_impl(const unary_call_impl&) = delete;
        unary_call_impl& operator=(const unary_call_impl&) = delete;

        void Finish(const TResponse& msg, const grpc::Status& status)
        {
            bool wasResponseSent = _responseSentFlag.test_and_set();
            if (!wasResponseSent)
            {
                _responder.Finish(msg, status, this);
            }
        }

        void FinishWithError(const grpc::Status& status)
        {
            bool wasResponseSent = _responseSentFlag.test_and_set();
            if (!wasResponseSent)
            {
                _responder.FinishWithError(status, this);
            }
        }

        void invoke(bool /* ok */) override
        {
            delete this;
        }
    };

}

/// @brief The details of a single async, unary call. Call
/// Finish/FinishWithError to send a response back to the cleint. This class
/// can only be moved. If no explicit call to Finish/FinishWithError has
/// been made, a generic internal server error.
template <typename TRequest, typename TResponse>
class unary_call final
{
public:
    unary_call() noexcept : _impl(nullptr) {}

    explicit unary_call(detail::unary_call_impl<TRequest, TResponse>* impl) noexcept
        : _impl(impl)
    {
        BOOST_ASSERT(impl);
    }

    unary_call(unary_call&& other) noexcept
        : _impl(other._impl)
    {
        other._impl = nullptr;
    }

    unary_call& operator=(unary_call&& rhs)
    {
        if (this != &rhs)
        {
            reset(rhs.impl);
            rhs._impl = nullptr;
        }

        return *this;
    }

    // unary_call is move-only
    unary_call(const unary_call&) = delete;
    unary_call& operator=(const unary_call&) = delete;

    ~unary_call()
    {
        reset(nullptr);
    }

    void swap(unary_call& rhs) noexcept
    {
        using std::swap;
        swap(_impl, rhs._impl);
    }

    const grpc::ServerContext& context() const
    {
        return _impl->_context;
    }

    grpc::ServerContext& context()
    {
        return _impl->_context;
    }

    const TRequest& request() const
    {
        return _impl->_request;
    }

    TRequest& request()
    {
        return _impl->_request;
    }

    /// @brief Responds to the client with the given message and status.
    void Finish(const TResponse& msg, const grpc::Status& status)
    {
        _impl->Finish(msg, status);
    }

    /// @brienf Responds to the client with the given status and no message.
    void FinishWithError(const grpc::Status& status)
    {
        _impl->FinishWithError(status);
    }

private:
    detail::unary_call_impl<TRequest, TResponse>* _impl;

    void reset(detail::unary_call_impl<TRequest, TResponse>* newValue)
    {
        auto oldValue = _impl;
        _impl = newValue;

        if (oldValue != newValue)
        {
            if (oldValue)
            {
                // the current impl is being destroyed, so send an error
                // response. Relies on unary_call_impl to only send on
                // actual response.
                oldValue->FinishWithError(
                    grpc::Status{
                        grpc::StatusCode::INTERNAL,
                        "An internal server error has occurred." });
            }
        }
    }
};

template <typename TRequest, typename TResponse>
void swap(unary_call<TRequest, TResponse>& lhs, unary_call<TRequest, TResponse>& rhs) noexcept
{
    lhs.swap(rhs);
}

} } } //namespace bond::ext::gRPC
