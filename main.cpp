#include <cstdio>
#include <thread>
#include <vector>
#include <windows.h>
#include <chrono>
#include <cmath>

void pCoreTask(DWORD_PTR mask) {
    // Set the thread affinity to run on the specified P-Core
    SetThreadAffinityMask(GetCurrentThread(), mask);

    // Perform some work on the specified P-Core
    printf("Thread running on P-Core with mask %llx\n", mask);

    // Simulate some CPU-intensive work
    auto start = std::chrono::high_resolution_clock::now();
    double result = 0.0;
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 10) {
        for (int i = 0; i < 1000000; ++i) {
            result += std::sin(i) * std::cos(i);
        }
    }

    printf("Thread on P-Core with mask %llx finished. Result: %.2f\n", mask, result);
}

void eCoreTask(DWORD_PTR mask) {
    // Set the thread affinity to run on the specified E-Core
    SetThreadAffinityMask(GetCurrentThread(), mask);

    // Perform some work on the specified E-Core
    printf("Thread running on E-Core with mask %llx\n", mask);

    // Simulate some CPU-intensive work
    auto start = std::chrono::high_resolution_clock::now();
    double result = 0.0;
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 5) {
        for (int i = 0; i < 500000; ++i) {
            result += std::sin(i) * std::cos(i);
        }
    }

    printf("Thread on E-Core with mask %llx finished. Result: %.2f\n", mask, result);
}

int main() {
    DWORD returnedLength = 0;
    BOOL ret = GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &returnedLength);
    if (ret == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        std::vector<char> buffer(returnedLength);
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* processorInfo = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buffer.data());

        ret = GetLogicalProcessorInformationEx(RelationProcessorCore, processorInfo, &returnedLength);
        if (ret) {
            std::vector<std::thread> threads;

            char* ptr = buffer.data();
            while (ptr < buffer.data() + returnedLength) {
                SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(ptr);
                if (info->Relationship == RelationProcessorCore) {
                    if (info->Processor.EfficiencyClass == 0 || info->Processor.EfficiencyClass == 2) {
                        threads.emplace_back(pCoreTask, info->Processor.GroupMask[0].Mask);
                    } else if (info->Processor.EfficiencyClass == 1) {
                        threads.emplace_back(eCoreTask, info->Processor.GroupMask[0].Mask);
                    }
                }
                ptr += info->Size;
            }

            // Wait for all threads to finish
            for (auto& thread : threads) {
                thread.join();
            }
        }
    }

    return 0;
}
