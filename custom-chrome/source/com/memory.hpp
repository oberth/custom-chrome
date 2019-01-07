#pragma once
#include <memory>
#include <Windows.h>

#include <com/runtime_validation.hpp>

namespace com {

    // By the same token, you can have com::shared_ptr if you want to 
    // move away from COM reference counting and have something nicer to look at.

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