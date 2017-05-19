// The program uses a generated Bond service TestService; however the project
// doesn't contain any .bond file. Instead it includes headers from
// and links to grpc_dll_example.dll which contain definition of TestService and
// pre-built Bond de/serialization code for it. This approach is useful when
// multiple projects use the same Bond types; it allows compiling Bond code
// once and distributing as a binary .lib file. The static library needs to be
// rebuilt only when .bond file (i.e. data schema) changes.  Note that the
// DLL and the programs that consume it have to use the same version
// of Bond.
// This program tests the use of the server-side service implementation.
#include <grpc_dll_apply.h>

// Reflection header needed only for explicit metadata access -- don't
// include by default as it will increase your build times
#include <grpc_dll_reflection.h>

#include <grpc_dll_grpc.h>

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


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::StatusCode;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

#include <iostream>

using namespace examples::grpc_dll;


class TestServiceClient
{
public:
    TestServiceClient(std::shared_ptr<Channel> channel)
        : stub_(TestService::NewStub(channel))
    { }

    // Assembles the client's payload, sends it and presents the response back
    // from the server.

    Item TestMethod(MyStruct &theStruct)
    {
        ClientContext context;

        // The actual RPC.
        bond::comm::message<MyStruct> req(theStruct);
        bond::comm::message<Item> rep;
        Status status = stub_->TestMethod(&context, req, &rep);

        if (!status.ok()) {
           std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
        }

        return rep.value().Deserialize();
    }

private:
    std::unique_ptr<TestService::Stub> stub_;

};

struct TestServiceImpl : TestService::Service
{
    void TestMethod(::bond::ext::gRPC::unary_call<
                        ::bond::comm::message< ::examples::grpc_dll::MyStruct>,
                        ::bond::comm::message< ::examples::grpc_dll::Item>> call) override
    {
        MyStruct request = call.request().value().Deserialize();
        Item response;
        response = request.items[0];
         
        bond::comm::message<Item> resp(response);
        call.Finish(resp, Status::OK);
    }
};

int main()
{
    // Exercise Core facilities

    MyStruct obj;

    // Initialize
    obj.items.resize(1);
    obj.items[0].numbers.push_back(13);

    Item item;

    item.numbers.push_back(11);
    obj.item = bond::bonded<Item>(item);

    // Serialize
    bond::OutputBuffer buffer;
    bond::CompactBinaryWriter<bond::OutputBuffer> writer(buffer);
    bond::Serialize(obj, writer);
        
    bond::blob data = buffer.GetBuffer();

    MyStruct obj2;

    // Deserialize
    bond::CompactBinaryReader<bond::InputBuffer> reader(data);
    bond::Deserialize(reader, obj2);

    Item item2;
    
    obj2.item.Deserialize(item2);

    // Access metadata
    bond::Metadata myMetadata = MyStruct::Schema::GetMetadata();

    std::cout << myMetadata.name << std::endl;

    bond::RuntimeSchema schema = bond::GetRuntimeSchema<MyStruct>();

    std::cout << schema.GetSchema().structs[schema.GetSchema().root.struct_def].fields[0].metadata.name << std::endl;

    // Exersize gRPC facilities

    const std::string server_address("127.0.0.1:50051");

    // Create a service instance
    TestServiceImpl service;
    bond::ext::gRPC::server_builder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<bond::ext::gRPC::server> server(builder.BuildAndStart());

    // Create a proxy
    TestServiceClient stub(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

    return 0;
}
