/* Copyright 2023 Jack A Bernard Jr.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SUMTY_DETAIL_FWD_HPP
#define SUMTY_DETAIL_FWD_HPP

#include <system_error> // for std::error_code

namespace sumty {

template <typename... T>
class variant;

template <typename T>
class option;

template <typename T, typename E = std::error_code>
class result;

template <typename T>
class ok_t;

template <typename E>
class error_t;

}

#endif
