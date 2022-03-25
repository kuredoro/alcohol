#pragma once

#include <memory>

namespace util
{

template <class T, class... Args>
auto make_unique_from_ref(Args&&... args)
{
    return std::make_unique<std::remove_reference_t<T>>(std::forward<Args>(args)...);
}

}
