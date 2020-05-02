// Copyright (c) 2019-2020 Nicholas Corgan
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "Testing/TestUtility.hpp"

#include <Pothos/Proxy.hpp>

#include <string>

namespace NPTests
{

//
// Test function
//

void testBlockExecutionCommon(
    const Pothos::Proxy& testBlock,
    bool longTimeout = false);

//
// Calls into manual tests
//

template <typename T>
void testManualBlockExecution();

}
