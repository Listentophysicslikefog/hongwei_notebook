/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "transport/shm/state.h"

namespace apollo
{
    namespace cyber
    {
        namespace transport
        {

            State::State(const uint64_t &ceiling_msg_size)
                : ceiling_msg_size_(ceiling_msg_size) {}

            State::~State() {}

        } // namespace transport
    } // namespace cyber
} // namespace apollo
