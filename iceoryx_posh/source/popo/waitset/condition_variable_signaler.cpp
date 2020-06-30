// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License

#include "iceoryx_posh/internal/popo/waitset/condition_variable_signaler.hpp"

namespace iox
{
namespace popo
{
ConditionVariableSignaler::ConditionVariableSignaler(
    cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept
    : m_condVarDataPtr(condVarDataPtr)
{
}

void ConditionVariableSignaler::notifyOne() noexcept
{
    /// @todo post semaphore n-times?
    getMembers()->m_semaphore.post();
}

const ConditionVariableData* ConditionVariableSignaler::getMembers() const noexcept
{
    return m_condVarDataPtr;
}

ConditionVariableData* ConditionVariableSignaler::getMembers() noexcept
{
    return m_condVarDataPtr;
}

} // namespace popo
} // namespace iox