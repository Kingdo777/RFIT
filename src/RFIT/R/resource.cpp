//
// Created by kingdo on 2020/10/23.
//
#include "RFIT/R/resource.h"

namespace RFIT_NS {
    CpuResource::Impl::Impl(uint64_t cpu_shares, int64_t cfs_quota_us, int64_t cfs_period_us) :
            cpuShares(cpu_shares),
            cfs_quota_us(cfs_quota_us),
            cfs_period_us(cfs_period_us) {
        hash = WAVM::Hash<uint64_t>()(cpu_shares, cfs_quota_us);
        hash = WAVM::Hash<uint64_t>()(hash, cfs_period_us);
    }

    MemResource::Impl::Impl(int64_t mem_soft_limit, int64_t mem_hard_limit) :
            mem_soft_limit(mem_soft_limit),
            mem_hard_limit(mem_hard_limit) {
        hash = WAVM::Hash<uint64_t>()(mem_soft_limit, mem_hard_limit);
    }

    Resource::Impl::Impl(const CpuResource &cpu, const MemResource &mem) : cpu(cpu), mem(mem) {
        hash = WAVM::Hash<uint64_t>()(cpu.getHash(), mem.getHash());
    }
}