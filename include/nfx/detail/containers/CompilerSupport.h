/*
 * MIT License
 *
 * Copyright (c) 2025 nfx
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

/** @brief No unique address attribute for zero-cost empty member optimization */
#if defined( __clang__ )
// Clang or Clang-CL: Use feature detection regardless of _MSC_VER
#    if __has_cpp_attribute( no_unique_address )
#        define NFX_CONTAINERS_NO_UNIQUE_ADDRESS [[no_unique_address]]
#    else
#        define NFX_CONTAINERS_NO_UNIQUE_ADDRESS
#    endif
#elif defined( _MSC_VER ) && _MSC_VER >= 1928
// MSVC
#    define NFX_CONTAINERS_NO_UNIQUE_ADDRESS [[no_unique_address]]
#elif defined( __has_cpp_attribute ) && __has_cpp_attribute( no_unique_address ) >= 201803L
#    if defined( __cpp_lib_no_unique_address ) && __cpp_lib_no_unique_address >= 201803L
#        define NFX_CONTAINERS_NO_UNIQUE_ADDRESS [[no_unique_address]]
#    else
#        define NFX_CONTAINERS_NO_UNIQUE_ADDRESS
#    endif
#else
#    define NFX_CONTAINERS_NO_UNIQUE_ADDRESS
#endif
