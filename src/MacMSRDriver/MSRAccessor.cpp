// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2012, Intel Corporation
// written by Austen Ott
//
#include "MSRAccessor.h"
#include "PcmMsr/UserKernelShared.h"

#include <iostream>
#include <iomanip>

using namespace std;

MSRAccessor::MSRAccessor()
{
    service = IOServiceGetMatchingService(kIOMainPortDefault,
                                          IOServiceMatching(kPcmMsrDriverClassName));
    openConnection();
}

int32_t MSRAccessor::buildTopology(uint32_t num_cores, void* pTopos)
{
    topologyEntry *entries = (topologyEntry*)pTopos;
    size_t size = sizeof(topologyEntry)*num_cores;
    kern_return_t ret = IOConnectCallStructMethod(connect,
                                                  kBuildTopology,
                                                  NULL,
                                                  0,
                                                  entries,
                                                  &size);
    return (ret == KERN_SUCCESS) ? 0 : -1;
}

int32_t MSRAccessor::read(uint32_t core_num, uint64_t msr_num, uint64_t* value)
{
    pcm_msr_data_t idatas, odatas;
    size_t size = sizeof(pcm_msr_data_t);
    idatas.msr_num = (uint32_t)msr_num;
    idatas.cpu_num = core_num;

    kern_return_t ret = IOConnectCallStructMethod(connect,
                                                  kReadMSR,
                                                  &idatas,
                                                  size,
                                                  &odatas,
                                                  &size);
    if(ret == KERN_SUCCESS)
    {
        *value = odatas.value;
        return sizeof(*value);
    } else {
        return -1;
    }
}

int32_t MSRAccessor::write(uint32_t core_num, uint64_t msr_num, uint64_t value)
{
    pcm_msr_data_t idatas;
    size_t size = sizeof(pcm_msr_data_t);
    idatas.value = value;
    idatas.msr_num = (uint32_t)msr_num;
    idatas.cpu_num = core_num;
    kern_return_t ret = IOConnectCallStructMethod(connect,
                                                  kWriteMSR,
                                                  (void *)&idatas,
                                                  size,
                                                  NULL,
                                                  NULL);
    if(ret == KERN_SUCCESS)
    {
        return sizeof(value);
    } else {
        return -1;
    }
}

int32_t MSRAccessor::getNumInstances()
{
    kern_return_t   kernResult;
    size_t          num_outputs = 1;
    uint64_t        knum_insts;

    kernResult = IOConnectCallStructMethod(connect,
                                           kGetNumInstances,
                                           NULL, 0,
                                           &knum_insts,
                                           &num_outputs);

    if (kernResult != KERN_SUCCESS)
    {
      return -1;
    }

    return knum_insts;
}

int32_t MSRAccessor::incrementNumInstances()
{
    kern_return_t   kernResult;
    size_t          num_outputs = 1;
    uint64_t        knum_insts;

    kernResult = IOConnectCallStructMethod(connect,
                                           kIncrementNumInstances,
                                           NULL,
                                           0,
                                           &knum_insts,
                                           &num_outputs);
    if (kernResult != KERN_SUCCESS)
    {
        return -1;
    }

    return knum_insts;
}

int32_t MSRAccessor::decrementNumInstances()
{
    kern_return_t   kernResult;
    size_t          num_outputs = 1;
    uint64_t        knum_insts;

    kernResult = IOConnectCallStructMethod(connect,
                                           kDecrementNumInstances,
                                           NULL,
                                           0,
                                           &knum_insts,
                                           &num_outputs);
    if (kernResult != KERN_SUCCESS)
    {
        return -1;
    }
    return knum_insts;
}

MSRAccessor::~MSRAccessor()
{
    closeConnection();
}

kern_return_t MSRAccessor::openConnection()
{
    kern_return_t kernResult = IOServiceOpen(service, mach_task_self(), 0, &connect);

    if (kernResult != KERN_SUCCESS)
    {
        cerr << "IOServiceOpen returned 0x" << hex << uppercase << setw(8) << kernResult << endl;
    } else {
        kernResult = IOConnectCallScalarMethod(connect, kOpenDriver, NULL, 0, NULL, NULL);

        if (kernResult != KERN_SUCCESS) {
            cerr << "openClient returned 0x" << hex << uppercase << setw(8) << kernResult << endl;
        }
    }

    return kernResult;
}

void MSRAccessor::closeConnection()
{
    kern_return_t kernResult = IOConnectCallScalarMethod(connect, kCloseDriver, NULL, 0, NULL, NULL);
    if (kernResult != KERN_SUCCESS) {
        cerr << "closeClient returned 0x" << hex << uppercase << setw(8) << kernResult << endl;
    }

    kernResult = IOServiceClose(connect);
    if (kernResult != KERN_SUCCESS) {
        cerr << "IOServiceClose returned 0x" << hex << uppercase << setw(8) << kernResult << endl;
    }
}
