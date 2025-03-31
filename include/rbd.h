#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <set>
#include <utility>
#include <unordered_set>
#include <regex>
#include <chrono>
#include <future>
#include <utility>
#include <unistd.h>     
#include <sys/types.h>  
#include <sys/wait.h>  
#include <cstring>      


namespace rbd
{

    class ProbabilityArray
    {
    
    private:
        std::vector<double> pos_array;
        std::vector<double> neg_array;

    public:
        ProbabilityArray(const std::map<int,double> avail_arr): pos_array(avail_arr.size() + 1), neg_array(avail_arr.size() + 1)
        {   
            for (const auto &pair : avail_arr)
            {
                pos_array[pair.first] = pair.second;
                neg_array[pair.first] = 1 - pair.second;
            }
        }

        double operator[](int i) const
        {
            if (std::abs(i) > pos_array.size())
            {
                throw std::out_of_range("Index out of range in ProbabilityArray");
            }
            if (i == 0)
            {
                std::cerr << "Error on handling node 0!" << std::endl;
            }
            if (i > 0)
            {
                return pos_array[i];
            }
            else
            {
                return neg_array[-i];
            }
        }

        void print() const
        {
            std::cout << "Positive Array: ";
            for (double elem : pos_array)
            {

                std::cout << elem << " ";
            }
            std::cout << std::endl;

            std::cout << "Negative Array: ";
            for (double elem : neg_array)
            {
                std::cout << elem << " ";
            }
            std::cout << std::endl;
        }
    };
    
    /**
     * @brief Create a couple of disjoint sets from set2
     * Algorithm:
     * 1. Find the set: set1 \ set2, e.g. set1 = {1, 2, 3, 4, 5}, set2 = {2, 4}, then RC = set1 \ set2 = {1, 3, 5}
     * 2. add the elements in RC to set2 to create several new sets. The rule is every time add one more element from RC to set2 and the last element to add is negative.
     *      1) first step: {2, 4} -> {2, 4, -1}
     *      2) second step: {2, 4, 1} -> {2, 4, 1, -3}
     *      3) third step: {2, 4, 1, -3} -> {2, 4, 1, -3, -5}
     * 3. return the new disjoint sets {{2, 4, 1}, {2, 4, 1, -3}, {2, 4, 1, 3, -5}}
     * @param set1 
     * @param set2 
     * @return disjointed sets
     * 
     */
    std::vector<std::vector<int>> makeDisjointSet(std::vector<int> set1, std::vector<int> set2);

    /**
     * @brief Convert the minimal cut sets to the probability sets
     * Algorithm:
     * 1. Remove the set: {src} and {dst} from the min-cutsets
     * 2. Inverse the min-cutsets, e.g. min-cutsets = {{1, 2, 3}, {1, 2, -3}} -> {{-1, -2, -3}, {-1, -2, 3}}
     * 3. Select the left most set as the selected_set and the remaining min-cutsets as the right remaining sets e.g min-cutsets = {set1, set2, set3}
     * then selected_set = set1, remaining sets = {set2, set3}
     * 4. Create the disjoint sets from the set1 and each set in the remaining sets e.g. makeDisjointSet(set1, set2), makeDisjointSet(set1, set3)
     * 5. The new disjoint set should follow the rules:
     * Absorption: x + xy = x
     * 6. Add the new disjoint sets to the probability sets
     * 7. repeat the steps 2-5 for the second left most set and the remaining sets are the right remaining sets e.g. selected_set = set2, remaining sets = {set3}
     * @param src_dst 
     * @param min_cutsets 
     * @return std::vector<std::vector<int>> 
     */
    std::vector<std::vector<int>> minCutSetToProbaset (const int &src, const int &dst, std::vector<std::vector<int>>& mincutset);

    /**
     * @brief Compute the probability of the given probability set and the probability array
     * e.g. prob_set = {{1, 2, 3}, {1, 2, -3}}, prob_array = {0.1, 0.2, 0.3}
     * then the probability is 0.1 * 0.2 * 0.3 + 0.1 * 0.2 * 0.7 = 0.02
     * @param src_dst pair
     * @param prob_set 
     * @param prob_array 
     * @return the probability in double
     */
    double probasetToAvailability(const int &src, const int &dst, const ProbabilityArray &probArr, std::vector<std::vector<int>> &prob_set);

    /**
     * @brief Evaluate the availability for a specific src-dst pair and topology file
     * Better version of evaluateAvailability with direct min_cutsets and prob_array as input from python
     * @param min_cutsets 
     * @param prob_array 
     * @param src 
     * @param dst 
     * @return the availability in double
     */
    double evaluateAvailability(const int &src, const int &dst, const ProbabilityArray& probArr, std::vector<std::vector<int>>& mincutset);

    /**
     * @brief Evaluate the availability for a specific src-dst pair and topology file
     * @param nodePairs
     * @param src
     * @param dst
     * @return the list of (src, dst, availability) tuples
     */

    std::vector<std::tuple<int, int, double>> evaluateAvailabilityTopology(const std::vector<std::pair<int, int>>& nodePairs, const ProbabilityArray& probArr, std::vector<std::vector<std::vector<int>>>& mincutsets);

    /**
     * @brief Evaluate the availability for a specific src-dst pair and topology file with multi-threading
     * @param nodePairs
     * @param probArr
     * @param mincutsets
     * @return 
     */
    std::vector<std::tuple<int, int, double>> evaluateAvailabilityTopologyMultiThreading(const std::vector<std::pair<int, int>>& nodePairs, const ProbabilityArray& probArr, std::vector<std::vector<std::vector<int>>>& mincutsets);

    /**
     * @brief Evaluate the availability for a specific src-dst pair and topology file with multi-processing
     * @param nodePairs
     * @param probArr
     * @param mincutsets
     * @return 
     */
    std::vector<std::tuple<int, int, double>> evaluateAvailabilityTopologyMultiProcessing(const std::vector<std::pair<int, int>>& nodePairs, const ProbabilityArray& probArr, std::vector<std::vector<std::vector<int>>>& mincutsets);

    /**
     * @brief Compute the length of the probability set
     * @param min_cutsets 
     * @param src 
     * @param dst 
     * @return the length of the probability set plus 2 for the src and dst
     */
    int boolExprCount(const int &src, const int &dst, std::vector<std::vector<int>>& mincutsets);

}