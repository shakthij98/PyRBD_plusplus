#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <rbd.h>

namespace py = pybind11;
using namespace rbd;

PYBIND11_MODULE(rbd_bindings, m)
{

      m.def("evaluateAvailability", [](std::vector<std::vector<int>> &mincutset, const std::map<int, double> &probabilities, int src, int dst)
            {
              ProbabilityArray probArr(probabilities);
              return rbd::evaluateAvailability(src, dst, probArr, mincutset);}, 
              "Directly evaluate availability using minimal cutsets and probabilities", 
              py::arg("src"), py::arg("dst"), py::arg("probabilities"), py::arg("mincutset"), py::call_guard<py::gil_scoped_release>());

      m.def("evaluateAvailabilityTopology", [](std::vector<std::vector<std::vector<int>>> &mincutsets, const std::map<int, double> &probabilities, const std::vector<std::pair<int, int>> &nodePairs)
            {
              ProbabilityArray prob_array(probabilities);
              return rbd::evaluateAvailabilityTopology(nodePairs, prob_array, mincutsets); }, 
              "Directly evaluate availability using minimal cutsets and probabilities", 
              py::arg("nodePairs"), py::arg("probabilities"), py::arg("mincutset"), py::call_guard<py::gil_scoped_release>());

      m.def("evaluateAvailabilityTopologyMultiThreading", [](std::vector<std::vector<std::vector<int>>> &mincutsets, const std::map<int, double> &probabilities, const std::vector<std::pair<int, int>> &nodePairs)
            {
              ProbabilityArray prob_array(probabilities);
              return rbd::evaluateAvailabilityTopologyMultiThreading(nodePairs, prob_array, mincutsets); }, 
              "Directly evaluate availability using minimal cutsets and probabilities", 
              py::arg("probabilities"), py::arg("nodePairs"), py::arg("mincutset"), py::call_guard<py::gil_scoped_release>());
      
      m.def("evaluateAvailabilityTopologyMultiProcessing", [](std::vector<std::vector<std::vector<int>>> &mincutsets, const std::map<int, double> &probabilities, const std::vector<std::pair<int, int>> &nodePairs)
            {
              ProbabilityArray prob_array(probabilities);
              return rbd::evaluateAvailabilityTopologyMultiProcessing(nodePairs, prob_array, mincutsets); }, 
              "Directly evaluate availability using minimal cutsets and probabilities", 
              py::arg("probabilities"), py::arg("nodePairs"), py::arg("mincutset"), py::call_guard<py::gil_scoped_release>());

      m.def("boolExprCount", &rbd::boolExprCount,
            "Compute the length of the probability set",
            py::arg("src"), py::arg("dst"), py::arg("mincutset"));

      m.def("minCutSetToProbaset", &rbd::minCutSetToProbaset,
            "Convert mincutset to probability set",
            py::arg("src"), py::arg("dst"), py::arg("mincutset"));
}
