#
# Copyright (c) 2020 Damian Jarek (damian.jarek93@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/djarek/propalloc
#

function(propalloc_add_test test_name)
    add_executable(${test_name} "${test_name}.cpp")
    target_link_libraries(${test_name} PRIVATE propalloc::propalloc)
    add_test("${test_name}_test" ${test_name})
endfunction(propalloc_add_test)

propalloc_add_test(vector)
