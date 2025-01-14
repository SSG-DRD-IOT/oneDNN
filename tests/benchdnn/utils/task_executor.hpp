/*******************************************************************************
* Copyright 2023 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef UTILS_TASK_EXECUTOR_HPP
#define UTILS_TASK_EXECUTOR_HPP

#include "utils/parallel.hpp"
#include "utils/task.hpp"

template <typename prb_t, typename perf_report_t, typename create_func_t,
        typename check_cache_func_t, typename do_func_t>
struct task_executor_t {
    virtual ~task_executor_t() { assert(tasks_.empty()); }

    void submit(const prb_t &prb, const std::string &perf_template,
            const create_func_t &create_func,
            const check_cache_func_t &check_cache_func,
            const do_func_t &do_func) {
        tasks_.emplace_back(
                prb, perf_template, create_func, check_cache_func, do_func);
        if (has_bench_mode_modifier(mode_modifier_t::par_create)
                && static_cast<int>(tasks_.size()) < dnnl_get_max_threads())
            return;
        flush();
    }

    void flush() {
        benchdnn_parallel_nd(tasks_.size(), [&](int i) { tasks_[i].create(); });

        for (auto &t : tasks_) {
            t.check_cache();
            t.exec();
        }

        tasks_.clear();
    }

    std::vector<task_t<prb_t, perf_report_t, create_func_t, check_cache_func_t,
            do_func_t>>
            tasks_;
};

#endif
