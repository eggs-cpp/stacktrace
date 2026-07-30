// Eggs.Stacktrace
//
// Copyright (c) 2026 Agustin Berge
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

// dladdr() is a GNU extension; expose it before any system headers.
#ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#endif

#include <eggs/stacktrace.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "detail/backend.hpp"

#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>

namespace eggs {
namespace detail {

void capture(
    std::vector<stacktrace_entry>& frames, std::size_t skip,
    std::size_t max_depth
) noexcept
{
    if (max_depth == 0) return;

    constexpr int kMax = 128;
    void* buf[kMax];
    int const got = ::backtrace(buf, kMax);

    // +1 to skip this capture() frame itself in the backtrace.
    std::size_t const got_sz = static_cast<std::size_t>(got);
    std::size_t const first = skip + 1;
    if (got_sz <= first) return;

    // Avoid overflow: compute available frames then cap, not first + max_depth.
    std::size_t const take = std::min(got_sz - first, max_depth);
    try {
        for (std::size_t i = first; i < first + take; ++i)
            frames.push_back(capture_helper::make(buf[i]));
    } catch (...) {
    }
}

std::string symbolize_description(void* address)
{
    if (!address) return {};

    ::Dl_info info{};
    if (::dladdr(address, &info) == 0 || !info.dli_sname) return {};

    int status = -1;
    char* demangled =
        abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
    std::string result =
        (status == 0 && demangled) ? demangled : info.dli_sname;
    std::free(demangled);
    return result;
}

std::string symbolize_source_file(void*)
{
    return {};
}

std::uint_least32_t symbolize_source_line(void*)
{
    return 0;
}

} // namespace detail
} // namespace eggs
