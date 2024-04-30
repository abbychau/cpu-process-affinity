#include <cstdio>
#include <thread>
#include <vector>
#include <windows.h>
#include <chrono>
#include <cmath>
#include <random>
#include <string>

const int NUM_ITERATIONS = 1000000;

void benchmarkTask(std::vector<DWORD_PTR> pCoreMasks, std::vector<DWORD_PTR> eCoreMasks, bool pCoreOnly, bool eCoreOnly) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> pCoreDistribution(0, pCoreMasks.size() - 1);
    std::uniform_int_distribution<> eCoreDistribution(0, eCoreMasks.size() - 1);

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        if (pCoreOnly && !pCoreMasks.empty()) {
            // Set the thread affinity to a random P-Core
            DWORD_PTR mask = pCoreMasks[pCoreDistribution(gen)];
            SetThreadAffinityMask(GetCurrentThread(), mask);
        } else if (eCoreOnly && !eCoreMasks.empty()) {
            // Set the thread affinity to a random E-Core
            DWORD_PTR mask = eCoreMasks[eCoreDistribution(gen)];
            SetThreadAffinityMask(GetCurrentThread(), mask);
        } else {
            // Randomly select a P-Core or E-Core
            bool usePCore = (i % 2 == 0);

            if (usePCore && !pCoreMasks.empty()) {
                // Set the thread affinity to a random P-Core
                DWORD_PTR mask = pCoreMasks[pCoreDistribution(gen)];
                SetThreadAffinityMask(GetCurrentThread(), mask);
            } else if (!usePCore && !eCoreMasks.empty()) {
                // Set the thread affinity to a random E-Core
                DWORD_PTR mask = eCoreMasks[eCoreDistribution(gen)];
                SetThreadAffinityMask(GetCurrentThread(), mask);
            }
        }

        // Perform some work
        double result = 0.0;
        for (int j = 0; j < 1000; ++j) {
            result += std::sin(j) * std::cos(j);
        }
    }
}

int main(int argc, char* argv[]) {
    bool pCoreOnly = false;
    bool eCoreOnly = false;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--p-core-only") {
            pCoreOnly = true;
        } else if (arg == "--e-core-only") {
            eCoreOnly = true;
        }
    }

    DWORD returnedLength = 0;
    BOOL ret = GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &returnedLength);
    if (ret == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        std::vector<char> buffer(returnedLength);
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* processorInfo = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(buffer.data());

        ret = GetLogicalProcessorInformationEx(RelationProcessorCore, processorInfo, &returnedLength);
        if (ret) {
            std::vector<DWORD_PTR> pCoreMasks;
            std::vector<DWORD_PTR> eCoreMasks;

            char* ptr = buffer.data();
            while (ptr < buffer.data() + returnedLength) {
                SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(ptr);
                if (info->Relationship == RelationProcessorCore) {
                    if (info->Processor.EfficiencyClass == 0 || info->Processor.EfficiencyClass == 2) {
                        pCoreMasks.push_back(info->Processor.GroupMask[0].Mask);
                    } else if (info->Processor.EfficiencyClass == 1) {
                        eCoreMasks.push_back(info->Processor.GroupMask[0].Mask);
                    }
                }
                ptr += info->Size;
            }

            printf("Number of P-Cores: %zu\n", pCoreMasks.size());
            printf("Number of E-Cores: %zu\n", eCoreMasks.size());

            // Create multiple benchmark threads
            const int numThreads = 4;
            std::vector<std::thread> threads;
            auto start = std::chrono::high_resolution_clock::now();

            for (int i = 0; i < numThreads; ++i) {
                threads.emplace_back(benchmarkTask, pCoreMasks, eCoreMasks, pCoreOnly, eCoreOnly);
            }

            // Wait for all threads to finish
            for (auto& thread : threads) {
                thread.join();
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            printf("Benchmark completed in %lld milliseconds.\n", duration);
        }
    }

    return 0;
}
