# PyRBD++
An open-source Python tool for RBD evaluation based on the [PyRBD](https://github.com/shakthij98/PyRBD), which is suitable for complex systems and networks with bidirectional links between components and cyclic loops. PyRBD++ addresses the efficiency limitations of PyRBD by combining boolean techniques with Minimal Cut Set (MCS) methods. The boolean algorithms are implemented in C++ and integrated with Python, significantly improving computational performance.

If you use this tool, please cite us as follows.
S. Janardhanan, Y. Chen, C. Mas-Machuca, "PyRBD++: An Open-Source Fast Reliability Block Diagram Evaluation Tool", submitted to International Workshop on Resilient Networks Design and Modeling RNDM 2025.

## Installation
Clone the source code

```bash
git clone
```

Install the python libraries:

```bash
pip install -r requirements.txt
```

Build the programm

```bash
mkdir build # if not exist
cd build
cmake ..
make
cd ..
```

## Usage
We provide a example using single-core, multithreading, multiprocessing implementation to evaluate the availability of the topology Germany_17. See the [example.py](example.py)
