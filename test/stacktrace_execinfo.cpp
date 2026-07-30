// Eggs.Stacktrace
//
// Copyright (c) 2026 Agustin Berge
//
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt

#include <eggs/stacktrace.hpp>

#include <string>

#include "detail/assert.hpp"

// C linkage + noinline + default visibility gives a predictable unmangled
// symbol that dladdr() can resolve via -rdynamic even under -fvisibility=hidden.
extern "C" [[gnu::noinline, gnu::visibility("default")]]
void eggs_stacktrace_execinfo_sentinel()
{
    auto st = eggs::stacktrace::current(0);
    EGGS_TEST_ASSERT(!st.empty());

    // Frame 0 must be this function.
    std::string const desc = st[0].description();
    EGGS_TEST_ASSERT(!desc.empty());
    EGGS_TEST_ASSERT(
        desc.find("eggs_stacktrace_execinfo_sentinel") != std::string::npos
    );

    // execinfo provides no DWARF source info.
    EGGS_TEST_ASSERT(st[0].source_file() == "");
    EGGS_TEST_ASSERT(st[0].source_line() == 0u);
}

int main()
{
    // -- current() returns non-empty --------------------------------------------

    auto const st = eggs::stacktrace::current(0);
    EGGS_TEST_ASSERT(!st.empty());

    // -- skip reduces frame count by exactly 1 ----------------------------------

    auto const st0 = eggs::stacktrace::current(0);
    auto const st1 = eggs::stacktrace::current(1);
    EGGS_TEST_ASSERT(st1.size() + 1 == st0.size());

    // -- max_depth caps the result -----------------------------------------------

    auto const st_capped = eggs::stacktrace::current(0, 1);
    EGGS_TEST_ASSERT(st_capped.size() == 1);

    // -- source_file and source_line are always empty / 0 ------------------------

    for (auto const& e : st) {
        EGGS_TEST_ASSERT(e.source_file() == "");
        EGGS_TEST_ASSERT(e.source_line() == 0u);
    }

    // -- description resolves via dladdr + ENABLE_EXPORTS ------------------------

    void (*volatile sentinel)() = &eggs_stacktrace_execinfo_sentinel;
    sentinel();

    return 0;
}
