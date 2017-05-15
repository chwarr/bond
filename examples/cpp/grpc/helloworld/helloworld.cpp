#include "helloworld_types.h"
#include "helloworld_grpc.h"

#include "helloworld_proxy.h"

#ifdef _MSC_VER
    #pragma warning (push)
    #pragma warning (disable: 4100)
#endif

#include <grpc++/grpc++.h>

#ifdef _MSC_VER
    #pragma warning (pop)
#endif

#include <bond/ext/grpc/server.h>
#include <bond/ext/grpc/server_builder.h>
#include <bond/ext/grpc/unary_call.h>

#include <bond/ext/detail/event.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

using bond::ext::detail::event;

using namespace helloworld;

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {
    void SayHello(
        bond::ext::gRPC::unary_call<
            bond::comm::message<::helloworld::HelloRequest>,
            bond::comm::message<::helloworld::HelloReply>> call) override
    {
        HelloReply real_reply;
        real_reply.message = "hello " + call.request().value().Deserialize().name;

        bond::comm::message<HelloReply> rep(real_reply);

        call.Finish(rep, Status::OK);
    }
};

void printAndSet(event* print_event, ::bond::comm::message< ::helloworld::HelloReply> response) {
    std::string message = response.value().Deserialize().message;

    if (message.compare(std::string("hello world")) == 0)
    {
        std::cout << "Correct response: " << message;
        print_event->set();
    }
    else
    {
        std::cout << "Wrong response";
    }
}

int main()
{
    const std::string server_address("127.0.0.1:50051");
    GreeterServiceImpl service;

    bond::ext::gRPC::server_builder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<bond::ext::gRPC::server> server(builder.BuildAndStart());

    std::unique_ptr<grpc::CompletionQueue> cq_(new grpc::CompletionQueue());
    grpc::CompletionQueue* cq_ptr = cq_.get();

    helloworld::Greeter2::GreeterClient greeter(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()), std::move(cq_));
    greeter.start();

    ClientContext context;

    std::string user("world");
    HelloRequest request;
    request.name = user;
    bond::comm::message<HelloRequest> req(request);
    event print_event;

    std::function<void(::bond::comm::message< ::helloworld::HelloReply>)> f_print = [&print_event](::bond::comm::message< ::helloworld::HelloReply> response) { printAndSet(&print_event, response); };

    greeter.AsyncSayHello(&context, req, cq_ptr, f_print);

    bool waitResult = print_event.wait(std::chrono::seconds(10));

    if (waitResult)
        return 0;
    else
        return 1;
}
