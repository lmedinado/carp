#pragma once
#include <iosfwd>

namespace detail {
template <typename T, size_t ... I>
std::ostream &tuple_insertion_impl(std::ostream &os, T const &t, std::index_sequence<I...>) {
    ((os << std::get<I>(t) << ", "), ...);
    return os;
}
}

template <typename T>
auto operator<<(std::ostream &os, T const &t)
    -> std::enable_if_t<carp::detail::is_tuple<T>, std::ostream &> {

    os << "{ ";
    detail::tuple_insertion_impl(os, t, std::make_index_sequence<std::tuple_size_v<T>>{});
    return os << "}";
}