#include "rbd.h"

using json = nlohmann::json;

namespace rbd
{
    std::vector<std::vector<int>> makeDisjointSet(std::vector<int> set1, std::vector<int> set2)
    {
        // Check disjointness of set1 and set2: if x in set1 and x' in set2, set1 and set2 are disjoint
        // RC set: the elements in set1 but not in set2
        std::vector<int> RC;
        for (const int &elem : set1)
        {
            if (std::find(set2.begin(), set2.end(), -elem) != set2.end())
            {
                return {set2};
            }
            if (std::find(set2.begin(), set2.end(), elem) == set2.end())
            {
                RC.push_back(elem);
            }
        }

        // Absorption rule: x + xy = x, if the RC is empty return an empty vector
        if (RC.empty())
        {
            return std::vector<std::vector<int>>{};
        }

        // Save the result
        std::vector<std::vector<int>> result;

        for (size_t i = 0; i < RC.size(); i++)
        {
            // every time add one more element from RC to set2 and the element to add is negative.
            set2.push_back(-RC[i]);
            // save the new set
            result.push_back(set2);
            // inverse the last element to keep only the last element negative
            set2.back() = -set2.back();
        }

        return result;
    }

    std::vector<std::vector<int>> minCutSetToProbaset(const int &src, const int &dst, std::vector<std::vector<int>> &mincutset)
    {
        std::vector<std::vector<int>> probSet;
        // std::vector<std::vector<int>> tempSet{mincutset.begin(), mincutset.end()};

        // remove the sets: {src} and {dst}
        auto it_src = std::find(mincutset.begin(), mincutset.end(), std::vector<int>{src});
        if (it_src != mincutset.end())
        {
            mincutset.erase(it_src);
        }
        auto it_dst = std::find(mincutset.begin(), mincutset.end(), std::vector<int>{dst});
        if (it_dst != mincutset.end())
        {
            mincutset.erase(it_dst);
        }
        if (mincutset.empty())
        {
            return {};
        }

        // inverse the min_cutsets
        for (auto &set : mincutset)
        {
            for (auto &elem : set)
            {
                elem = -elem;
            }
        }

        // convert the min_cutsets to the probability sets
        while (mincutset.size() > 0)
        {

            if (mincutset.size() == 1)
            {
                probSet.push_back(mincutset[0]);
                break;
            }
            std::vector<int> selected_set = mincutset.front();
            probSet.push_back(selected_set);
            std::vector<std::vector<int>> remaining_sets(mincutset.begin() + 1, mincutset.end());
            mincutset.clear();

            for (const auto &set : remaining_sets)
            {
                std::vector<std::vector<int>> disjoint_sets = makeDisjointSet(selected_set, set);
                for (const auto &disjoint_set : disjoint_sets)
                {
                    mincutset.push_back(disjoint_set);
                }
            }
        }

        return probSet;
    }

    double probasetToAvailability(const int &src, const int &dst, const ProbabilityArray &probArr, std::vector<std::vector<int>> &probSet)
    {
        // Save the final result
        double unavil = 0.0;
        for (const auto &set : probSet)
        {
            // Save the temp result
            double temp = 1.0;
            for (const auto &num : set)
            {
                temp *= probArr[num];
            }
            unavil += temp;
        }

        double avail = 1.0 - unavil;
        double result = probArr[src] * probArr[dst] * avail;

        return result;
    }

    double evaluateAvailability(const int &src, const int &dst, const ProbabilityArray &probArr, std::vector<std::vector<int>> &mincutset)
    {
        // evaluate the unavailability for the given src-dst pair
        try
        {
            std::vector<std::vector<int>> probSet = minCutSetToProbaset(src, dst, mincutset);
            double availability = probasetToAvailability(src, dst, probArr, probSet);
            return availability;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
        return 0.0;
    }

    std::vector<std::tuple<int, int, double>> evaluateAvailabilityTopology(const std::vector<std::pair<int, int>> &nodePairs, const ProbabilityArray &probArr, std::vector<std::vector<std::vector<int>>> &mincutsets)
    {
        std::vector<std::tuple<int, int, double>> availabilities;

        try
        {
            for (size_t i = 0; i < nodePairs.size(); i++)
            {
                int src = nodePairs[i].first;
                int dst = nodePairs[i].second;
                double availability = evaluateAvailability(src, dst, probArr, mincutsets[i]);
                availabilities.emplace_back(src, dst, availability);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }

        return availabilities;
    }

    std::vector<std::tuple<int, int, double>> evaluateAvailabilityTopologyMultiThreading(const std::vector<std::pair<int, int>> &nodePairs, const ProbabilityArray &probArr, std::vector<std::vector<std::vector<int>>> &mincutsets)
    {
        std::vector<std::tuple<int, int, double>> availabilities;
        std::vector<std::future<std::tuple<int, int, double>>> futures;

        for (size_t i = 0; i < nodePairs.size(); ++i)
        {
            int src = nodePairs[i].first;
            int dst = nodePairs[i].second;
            auto &mincutset = mincutsets[i];

            futures.push_back(std::async(std::launch::async, [src, dst, &probArr, &mincutset]() 
            {
                double availability = evaluateAvailability(src, dst, probArr, mincutset);
                return std::make_tuple(src, dst, availability); 
            }));
        }

        for (auto &future : futures)
            {
                try
                {
                    availabilities.push_back(future.get());
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << std::endl;
                }
            }

        return availabilities;
    }

    std::vector<std::tuple<int, int, double>> evaluateAvailabilityTopologyMultiProcessing(const std::vector<std::pair<int, int>>& nodePairs, const ProbabilityArray& probArr, std::vector<std::vector<std::vector<int>>>& mincutsets)
    {
        std::vector<std::tuple<int, int, double>> availabilities;
        int num_processes = nodePairs.size();

        std::vector<pid_t> pids(num_processes);
        std::vector<int> pipes(num_processes * 2);

        // create a pipe for each process
        for (int i = 0; i < num_processes; ++i)
        {

            if (pipe(&pipes[i * 2]) == -1) {
                std::cerr << "Error creating pipes " << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        
        // create a child process for each src-dst pair
        for (size_t i = 0; i < num_processes; i++)
        {
            pids[i] = fork();

            if (pids[i] == -1)
            {
                std::cerr << "Error creating child process " << std::endl;
                exit(EXIT_FAILURE);
            }
            else if (pids[i] == 0) // child process
            {
                // close the read end of the pipe
                close(pipes[i * 2]);
                
                int src = nodePairs[i].first;
                int dst = nodePairs[i].second;
                auto &mincutset = mincutsets[i];
                double availability = evaluateAvailability(src, dst, probArr, mincutset);

                // write the availability to the pipe
                write(pipes[i * 2 + 1], &availability, sizeof(availability));
                close(pipes[i * 2 + 1]);
                exit(EXIT_SUCCESS);
            }
        }

        for (size_t i = 0; i < num_processes; i++)
        {
            // close the write end of the pipe
            close(pipes[i * 2 + 1]);

            double availability;
            read(pipes[i * 2], &availability, sizeof(availability));
            close(pipes[i * 2]);

            availabilities.push_back(std::make_tuple(nodePairs[i].first, nodePairs[i].second, availability));
        }

        for (size_t i = 0; i < num_processes; i++)
        {
            waitpid(pids[i], NULL, 0);
        }

        return availabilities;
        
    }


    int boolExprCount(const int &src, const int &dst, std::vector<std::vector<int>> &mincutset)
    {
        std::vector<std::vector<int>> probSet = minCutSetToProbaset(src, dst, mincutset);
        // return the length of the probability set plus 2 for the src and dst
        return probSet.size() + 2;
    }


}

int main() {
    
}