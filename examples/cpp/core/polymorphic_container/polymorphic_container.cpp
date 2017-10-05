#include "polymorphic_container_reflection.h"

#include <cassert>
#include <bond/core/bond.h>
#include <bond/stream/output_buffer.h>

using namespace examples::polymorphic_containers;


template <typename T>
bond::bonded<T> Serialize(const T& obj)
{
    bond::OutputBuffer buffer;
    bond::CompactBinaryWriter<bond::OutputBuffer> writer(buffer);
    bond::Serialize(obj, writer);

    bond::CompactBinaryReader<bond::InputBuffer> reader(buffer.GetBuffer());
    return bond::bonded<T>(reader);
}

int main()
{
    bond::bonded<bond::Box<std::vector<bond::bonded<Base>>>> bonded;

    {
        bond::Box<std::vector<bond::bonded<Base>>> box;

        {
            Base b1, b2;
            b1.f = 1;
            b2.f = 2;

            box.value.emplace_back(Serialize(b1));
            box.value.emplace_back(Serialize(b2));
        }

        bonded = Serialize(box);
    }

    {
        bond::Box<std::vector<bond::bonded<Base>>> obj1;
        bonded.Deserialize(obj1);

        assert(obj1.value.size() == 2);

        bond::bonded<Derived> bondedD(obj1.value[0]);
        Derived d;
        bondedD.Deserialize(d);

        assert(d.f == 1);
        // the default value for df is 0, so we sort of "expect" that.
        // Really, though, I think that deserialization should fail at
        // runtime. There could be an argument made for deserialization
        // working if there are no required fields in any of the child
        // parts...
        assert(d.df == 0);
    }

    return 0;
}
