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
 * @file TransparentHashMap.h
 * @brief Generic unordered map with transparent lookup and configurable hashing
 * @details Provides a thin wrapper around std::unordered_map with enhanced type support
 *          including automatic transparent lookup and flexible hash configuration
 */

#pragma once

#include <nfx/Hashing.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace nfx::containers
{
    //=====================================================================
    // TransparentHashMap class
    //=====================================================================

    /**
     * @brief Generic unordered map with transparent lookup
     * @tparam TKey Key type
     * @tparam TValue Mapped value type
     * @tparam Hash Hash functor type (default: hashing::Hasher<uint32_t>)
     * @tparam KeyEqual Key equality comparator (default: std::equal_to<> for transparent comparison)
     */
    template <typename TKey,
        typename TValue,
        typename Hash = hashing::Hasher<uint32_t>,
        typename KeyEqual = std::equal_to<>>
    class TransparentHashMap final : public std::unordered_map<TKey, TValue, Hash, KeyEqual>
    {
        using Base = std::unordered_map<TKey, TValue, Hash, KeyEqual>;

    public:
        //----------------------------------------------
        // Type aliases
        //----------------------------------------------

        /** @brief Type alias for key type */
        using key_type = TKey;

        /** @brief Type alias for mapped value type */
        using mapped_type = TValue;

        /** @brief Type alias for value type (pair<const Key, Value>) */
        using value_type = std::pair<const TKey, TValue>;

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
