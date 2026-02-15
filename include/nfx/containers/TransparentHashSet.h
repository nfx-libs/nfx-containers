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

/**
 * @file TransparentHashSet.h
 * @brief Generic unordered set with transparent lookup and configurable hashing
 * @details Provides a thin wrapper around std::unordered_set with enhanced type support
 *          including automatic transparent lookup via hashing::Hasher
 */

#pragma once

#include <nfx/Hashing.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_set>

namespace nfx::containers
{
    //=====================================================================
    // TransparentHashSet class
    //=====================================================================

    /**
     * @brief Generic unordered set with transparent lookup support
     * @tparam TKey Key type (any hashable type)
     * @tparam Hash Hash functor (default: hashing::Hasher<uint32_t> for high-performance nfx hashing)
     * @tparam KeyEqual Equality comparator (default: std::equal_to<> for transparent lookup)
     * @see hashing::Hasher for universal high-performance hashing
     */
    template <typename TKey,
        typename Hash = hashing::Hasher<uint32_t>,
        typename KeyEqual = std::equal_to<>>
    class TransparentHashSet final : public std::unordered_set<TKey, Hash, KeyEqual>
    {
        using Base = std::unordered_set<TKey, Hash, KeyEqual>;

    public:
        //----------------------------------------------
        // Type aliases
        //----------------------------------------------

        /** @brief Type alias for key type */
        using key_type = TKey;

        /** @brief Type alias for value type (same as key_type for sets) */
        using value_type = TKey;

        /** @brief Type alias for hasher type */
        using hasher = Hash;

        /** @brief Type alias for key equality comparator */
        using key_equal = KeyEqual;

        /** @brief Type alias for size type */
        using size_type = size_t;

        /** @brief Type alias for difference type */
        using difference_type = std::ptrdiff_t;

        //----------------------------------------------
        // Inherited Constructors
        //----------------------------------------------

        using Base::Base;
    };
} // namespace nfx::containers
