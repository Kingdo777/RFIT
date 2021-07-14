//
// Created by kingdo on 10/18/20.
//

#ifndef TPFAAS_RESOURCE_H
#define TPFAAS_RESOURCE_H

#include "RFIT/T/task.h"
#include <WAVM/Inline/HashMap.h>
#include <utils/locks.h>
#include <utils/queue.h>
#include <memory>
#include <utility>
#include <vector>

#define CPU_DEFAULT_SHARES 1024
#define CPU_DEFAULT_CFS_QUOTA_US 100000
#define CPU_DEFAULT_CFS_PERIOD_US 100000
#define DEFAULT_CPU_RATE 1.0

#define MEM_DEFAULT_SOFT_LIMIT 256
#define MEM_DEFAULT_HARD_LIMIT 256

namespace RFIT_NS {
    struct CpuResource {
    public:
        CpuResource() :
                impl(Impl()) {};

        CpuResource(uint64_t cpu_shares, int64_t cfs_quota_us, int64_t cfs_period_us) :
                impl(Impl(cpu_shares, cfs_quota_us, cfs_period_us)) {};

        [[nodiscard]] uint64_t getHash() const { return impl.hash; }

        friend bool operator==(const CpuResource &left, const CpuResource &right) {
            return left.impl.hash == right.impl.hash;
        }

        friend bool operator!=(const CpuResource &left, const CpuResource &right) {
            return left.impl.hash != right.impl.hash;
        }

        [[nodiscard]] string toString() const {
            return "CpuResource::cpu_shares-" + to_string(impl.cpuShares) + "::cfs_quota_us-" +
                   to_string(impl.cfs_quota_us) + "::cfs_period_us-" + to_string(impl.cfs_period_us);
        }

    private:
        struct Impl {
            uint64_t hash;
            uint64_t cpuShares;
            int64_t cfs_quota_us;
            int64_t cfs_period_us;

            explicit Impl(uint64_t cpu_shares = CPU_DEFAULT_SHARES, int64_t cfs_quota_us = CPU_DEFAULT_CFS_QUOTA_US,
                          int64_t cfs_period_us = CPU_DEFAULT_CFS_PERIOD_US);
        };

        const Impl impl;
    };

    struct MemResource {
    public:
        MemResource() :
                impl(Impl()) {};

        MemResource(int64_t mem_soft_limit, int64_t mem_hard_limit) :
                impl(Impl(mem_soft_limit, mem_hard_limit)) {};

        [[nodiscard]] uint64_t getHash() const { return impl.hash; }

        friend bool operator==(const MemResource &left, const MemResource &right) {
            return left.impl.hash == right.impl.hash;
        }

        friend bool operator!=(const MemResource &left, const MemResource &right) {
            return left.impl.hash != right.impl.hash;
        }

        [[nodiscard]] string toString() const {
            return "MemResource::mem_soft_limit-" + to_string(impl.mem_soft_limit) + "::mem_hard_limit-" +
                   to_string(impl.mem_hard_limit);
        }

    private:
        struct Impl {
            uint64_t hash;
            int64_t mem_soft_limit;
            int64_t mem_hard_limit;

            // 就一个构造函数,而且此构造函数，仅仅在getUniqueImpl中被调用，这样的话，所有的Impl就会保证是唯一的
            explicit Impl(int64_t mem_soft_limit = MEM_DEFAULT_SOFT_LIMIT,
                          int64_t mem_hard_limit = MEM_DEFAULT_HARD_LIMIT);
        };

        const Impl impl;
    };

    struct Resource {
        Resource() :
                impl(Impl()) {};

        Resource(CpuResource cpu,
                 MemResource mem) :
                impl(Impl(cpu, mem)) {};

        [[nodiscard]] uint64_t getHash() const { return impl.hash; }

        [[nodiscard]] string toString() const {
            return "Resource::{" + impl.cpu.toString() + "," + impl.mem.toString() + "}";
        }

    private:
        struct Impl {
            uint64_t hash;
            CpuResource cpu;
            MemResource mem;

            // 就一个构造函数,而且此构造函数，仅仅在getUniqueImpl中被调用，这样的话，所有的Impl就会保证是唯一的
            explicit Impl(CpuResource cpu = CpuResource(), MemResource mem = MemResource());
        };

        const Impl impl;
    };


    class R {
        friend class F;

        friend class RFIT;

    public:
        explicit R(Resource &&r) : resource(r) {};

        uint64_t getHash() {
            return resource.getHash();
        }

    private:
        const Resource resource;
    };
}
#endif //TPFAAS_RESOURCE_H
