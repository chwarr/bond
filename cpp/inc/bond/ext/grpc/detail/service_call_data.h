// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#ifdef _MSC_VER
    #pragma warning (push)
    #pragma warning (disable: 4100 4702)
#endif

#include <grpc++/grpc++.h>
#include <grpc++/impl/codegen/rpc_method.h>
#include <grpc++/impl/codegen/service_type.h>
#include <grpc++/impl/codegen/status.h>

#ifdef _MSC_VER
    #pragma warning (pop)
#endif

#include <bond/ext/grpc/io_mgr.h>
#include <bond/ext/grpc/service.h>
#include <bond/ext/grpc/unary_call.h>

#include <boost/assert.hpp>
#include <functional>
#include <memory>
#include <thread>

namespace bond { namespace ext { namespace gRPC { namespace detail {

template <typename TRequest, typename TResponse>
struct service_unary_call_data : io_mgr_tag
{
    typedef std::function<void(unary_call<TRequest, TResponse> call)> CBType;

    service* _service;
    int _methodIndex;
    grpc::ServerCompletionQueue* _cq;
    CBType _cb;
    std::unique_ptr<unary_call_impl<TRequest, TResponse>> _receivedCall;

    service_unary_call_data(
        service* service,
        int methodIndex,
        grpc::ServerCompletionQueue* cq,
        CBType cb)
        : _service(service),
          _methodIndex(methodIndex),
          _cq(cq),
          _cb(cb),
          _receivedCall(new unary_call_impl<TRequest, TResponse>)
    {
        BOOST_ASSERT(service);
        BOOST_ASSERT(cq);
        BOOST_ASSERT(cb);
    }

    void invoke(bool ok) override
    {
        if (ok)
        {
            // unary_call_impl::invoke will delete itself after it's posted
            // back to the completion queue as the result of sending a
            // response. The UnaryCall wrapper that we create inside the
            // thread guarantees that some response will always be sent.
            unary_call_impl<TRequest, TResponse>* receivedCall = _receivedCall.release();

            // TODO: switch to thread pool
            // TODO: use queuing policy here after switching to thread pool
            std::thread([this, receivedCall]()
            {
                _cb(unary_call<TRequest, TResponse> { receivedCall });
            }).detach();

            _receivedCall.reset(new unary_call_impl<TRequest, TResponse>);
            _service->queue_receive(
                _methodIndex,
                &_receivedCall->_context,
                &_receivedCall->_request,
                &_receivedCall->_responder,
                _cq,
                this);
        }
        else
        {
            // we're shutting down, so don't requeue
        }
    }
};

} } } } //namespace bond::ext::gRPC::detail
