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

#ifndef CYBER_MAINBOARD_MODULE_ARGUMENT_H_
#define CYBER_MAINBOARD_MODULE_ARGUMENT_H_

#include <list>
#include <string>

#include "common/global_data.h"
#include "common/log.h"
#include "common/types.h"

namespace apollo
{
  namespace cyber
  {
    namespace mainboard
    {

      static const char DEFAULT_process_group_[] = "mainboard_default";
      static const char DEFAULT_sched_name_[] = "CYBER_DEFAULT";

      class ModuleArgument
      {
      public:
        ModuleArgument() = default;
        virtual ~ModuleArgument() = default;
        void DisplayUsage();
        void ParseArgument(int argc, char *const argv[]);
        void GetOptions(const int argc, char *const argv[]);
        const std::string &GetBinaryName() const;
        const std::string &GetProcessGroup() const;
        const std::string &GetSchedName() const;
        const std::list<std::string> &GetDAGConfList() const;

      private:
        std::list<std::string> dag_conf_list_;
        std::string binary_name_;
        std::string process_group_;
        std::string sched_name_;
      };

      inline const std::string &ModuleArgument::GetBinaryName() const
      {
        return binary_name_;
      }

      inline const std::string &ModuleArgument::GetProcessGroup() const
      {
        return process_group_;
      }

      inline const std::string &ModuleArgument::GetSchedName() const
      {
        return sched_name_;
      }

      inline const std::list<std::string> &ModuleArgument::GetDAGConfList() const
      {
        return dag_conf_list_;
      }

    } // namespace mainboard
  } // namespace cyber
} // namespace apollo

#endif // CYBER_MAINBOARD_MODULE_ARGUMENT_H_
