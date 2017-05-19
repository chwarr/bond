// The program uses a generated Bond service PingPong; however the project
// doesn't contain any .bond file. Instead it includes
// grpc_static_library_apply.h and grpc_static_library_comm.h
// and links to grpc_static_library.lib which contain definition of PingPong and
// pre-built Bond de/serialization code for it. This approach is useful when
// multiple projects use the same Bond types; it allows compiling Bond code
// once and distributing as a binary .lib file. The static library needs to be
// rebuilt only when .bond file (i.e. data schema) changes.  Note that the
// static library and the programs that consume it have to use the same version
// of Bond.
// This program tests the use of the client-side proxies.
#include <grpc_static_library_apply.h>
#include <grpc_static_library_reflection.h>
#include <grpc_static_library_grpc.h>

// We still need to include Bond headers, however only small inline functions
// will be compiled as part of this file and expensive to build de/serialization
// code will be linked in from static_library.lib.
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4100)
#endif

#include <grpc++/grpc++.h>

#ifdef _MSC_VER
#pragma warning (pop)
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace examples::grpc_static_library;

class PingPongClient
{
public:
    PingPongClient(std::shared_ptr<Channel> channel)
        : stub_(PingPong::NewStub(channel))
    { }

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    std::string Ping(const std::string& message)
    {
        ClientContext context;

        PingRequest request;
        request.Payload = message;

        PingResponse reply;

        // The actual RPC.
        bond::comm::message<PingRequest> req(request);
        bond::comm::message<PingResponse> rep;
        Status status = stub_->Ping(&context, req, &rep);

        if (status.ok()) {
            return rep.value().Deserialize().Payload;
        }
        else {
            std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<PingPong::Stub> stub_;

};

int main()
{
    PingRequest obj;

    obj.Payload = "ping0";

    bond::OutputBuffer buffer;
    bond::CompactBinaryWriter<bond::OutputBuffer> writer(buffer);
    bond::Serialize(obj, writer);

    bond::blob data = buffer.GetBuffer();

    PingRequest obj2;

    // Deserialize
    bond::CompactBinaryReader<bond::InputBuffer> reader(data);
    bond::Deserialize(reader, obj2);

    // Access metadata
    bond::Metadata myMetadata = PingRequest::Schema::GetMetadata();

    // Initialize proxy
    const std::string server_address("127.0.0.1:50051");
    PingPongClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

    return 0;
}
