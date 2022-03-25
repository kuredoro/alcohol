#pragma once

#include <boost/mp11/integral.hpp>
#include <boost/mp11/detail/mp_plus.hpp>
#include <utility>
#include <memory>

namespace util
{

using namespace boost::mp11;

template <class T, class... Args>
auto make_unique_from_ref(Args&&... args)
{
    return std::make_unique<std::remove_reference_t<T>>(std::forward<Args>(args)...);
}

namespace detail
{

    template <class Index = mp_size_t<0>>
    struct for_each_argument
    {
        template <class F>
        static void run(F&& f) {}

        template <class F, class Arg, class... Args>
        static void run(F&& f, Arg&& arg, Args&&... args)
        {
            std::forward<F>(f)(Index::value, std::forward<Arg>(arg));
            
            for_each_argument<mp_plus<Index, mp_size_t<1>>>::run(std::forward<F>(f), std::forward<Args>(args)...);
        }
    };

}

template <class F, class... Args>
void for_each_argument(F&& f, Args&&... args)
{
    detail::for_each_argument<mp_size_t<0>>::run(std::forward<F>(f), std::forward<Args>(args)...);
}

}
