#include <bond/ext/grpc/detail/service.h>
#include <bond/ext/grpc/detail/service_call_data.h>
#include <bond/ext/grpc/server.h>
#include <bond/ext/grpc/server_builder.h>
#include <bond/ext/grpc/unary_call.h>

#include <bond/ext/grpc/bond_utils.h>


// TODO: generate this code instead of writing it by hand
class PingPongServiceAsync : public bond::ext::gRPC::detail::service
{
public:
    PingPongServiceAsync()
    {
        // code gen a call to AddMethod for each method with the right name. Order will matter.
        AddMethod("/examples/grpc_static_library.PingPong/Ping");
    }

    // code gen a virtual method with a signature like this
    virtual void Ping(
        bond::ext::gRPC::unary_call<
            bond::comm::message<::examples::grpc_static_library::PingRequest>,
            bond::comm::message<::examples::grpc_static_library::PingResponse>> call) = 0;

    void start(grpc::ServerCompletionQueue* cq) override
    {
        BOOST_ASSERT(cq);

        // for each method, actually initialize the service_unary_call_data
        _receivePingPong.emplace(
            this,
            0,
            cq,
            std::bind(
                &PingPongServiceAsync::Ping,
                this,
                std::placeholders::_1));

        // code gen enqueueing a receive to start processing the method
        queue_receive(
            0,
            &_receivePingPong->_receivedCall->_context,
            &_receivePingPong->_receivedCall->_request,
            &_receivePingPong->_receivedCall->_responder,
            cq,
            &_receivePingPong.get());
    }

private:
    // code gen a data member for each method
    boost::optional<bond::ext::gRPC::detail::service_unary_call_data<
        bond::comm::message<::examples::grpc_static_library::PingRequest>,
        bond::comm::message<::examples::grpc_static_library::PingResponse>>> _receivePingPong;
};
