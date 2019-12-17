// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "Testing/TestUtility.hpp"

#include <Pothos/Proxy.hpp>

#include <string>

//
// Test function
//

template <typename T>
void testBlockExecutionCommon(
    const Pothos::Proxy& testBlock,
    bool longTimeout = false);

void testBlockExecutionCommon(
    const Pothos::Proxy& testBlock,
    bool longTimeout = false);

//
// Calls into manual tests
//

template <typename T>
void testManualBlockExecution();
