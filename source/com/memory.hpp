#pragma once
#include <memory>
#include <Windows.h>

#include <com/runtime_validation.hpp>

namespace com {

    // Just a basic idea how to change semantics and move away from COM reference counting.
    // For production, you'd have to extend this to be more robust for all use cases.
    // I also suggest completely wrapping COM interfaces on top of that.

    template <typename ComType>
    struct com_deleter {
        void operator()(ComType* ptr) {
            ptr->Release();
        }
    };

    template <typename ComType>
    using unique_ptr = std::unique_ptr<ComType, com_deleter<ComType>>;

    template <typename ComType>
    inline auto make_unique(ComType* com_object) {
        return unique_ptr<ComType> { com_object };
    }

    template <typename DestinationComType, typename SourceComType>
    inline auto make_unique_and_change_interface(SourceComType* com_object) {

        DestinationComType* com_object_with_destination_interface;
        auto result = com_object->QueryInterface(IID_PPV_ARGS(&com_object_with_destination_interface));
        validate_result(result, "Failed to change interface.");
        com_object->Release();

        return make_unique(com_object_with_destination_interface);

    }

}