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
 * @file Containers.h
 * @brief Main umbrella header for nfx-containers library
 * @details Includes all hash container implementations: FastHashMap, FastHashSet,
 *          PerfectHashMap, TransparentHashMap, and TransparentHashSet.
 *          Include this single header to access all nfx-containers functionality.
 */

#pragma once

#include "containers/FastHashMap.h"
#include "containers/FastHashSet.h"
#include "containers/OrderedHashMap.h"
#include "containers/OrderedHashSet.h"
#include "containers/PerfectHashMap.h"
#include "containers/SmallVector.h"
#include "containers/TransparentHashMap.h"
#include "containers/TransparentHashSet.h"
