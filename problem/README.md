# Get Started

```sh
git clone git@github.com:multivvac/parallel-programming-competition-uni.lu.git
cd parallel-programming-competition-uni.lu/problem
# code directory 
cd code
```

## Design Your Algorithm

Refer to the [problem statement and template code](./problem.md) to design your algorithm. Make sure to verify its correctness before moving on.

## Develop and Debug Locally

To build the project on your local machine, you will need
- CMake ≥ 3.25
- Boost
- CUDA.

If your local environment does not support these dependencies(or you don't want to install it manually), we recommend using [Mutagen](https://mutagen.io/) to synchronize your code with the HPC cluster. This way, you can write code locally and compile/execute it directly on the cluster.

Using [Mutagen](https://mutagen.io/) makes it simple to keep your local codebase in sync with the HPC cluster.
For example, if your local project is located at `~/parcomp2025` and the corresponding folder on the HPC cluster is `~/data/parcomp2025`, run the following command on your **local machine**:

```sh
mutagen sync create --name=parcomp25 ~/parcomp2025  <your-user-name>@access-iris.uni.lu:8022:~/data/parcomp2025
```
After this, any changes you make locally will automatically synchronize to the HPC cluster—typically within **one second**.

Build instructions (from the code directory `problem/code`):

```bash
cmake -S . -B build
cmake --build build -j
```

The compiled binary will be located at:

```
build/bin/ppc
```

Two sample datasets are available for testing and benchmarking:

```bash
# Default dataset: validate correctness
./build/bin/ppc --debug 1

# Larger dataset: evaluate performance under heavier workloads
./build/bin/ppc --debug 12
```

## Build your code on the HPC

Log in to the **Iris** HPC cluster and allocate the interactive node you got(you can check you reservation code in [team reservation section](#team-reservations)). Then load the required modules:

```bash
# allocate your node with reserveration code
salloc -p interactive --exclusive --reservation=<replace with your reservation code> --time=2:00:00
```

```bash
# load the required modules
ml compiler/GCC devel/Boost devel/CMake system/CUDA
```


```bash
# from the repo root
cmake -S . -B build
cmake --build build -j
# binary: build/bin/ppc
```

## Running

### Release mode (to submit you code)

```bash
./build/bin/ppc
```

You can also set environment variables to run with arguments, for example with environment variables:

```bash
export PPC_HOSTNAME=iris-069
export PPC_PORT=8013
./build/bin/ppc
```

Or with command line parameters explicitly:

```bash
./build/bin/ppc <host> <port>
# example:
./build/bin/ppc iris-069 8013
```

### Clean build

```bash
rm -rf build
cmake -S . -B build
cmake --build build -j
```

## Team Reservations
| Team               | Reservation |
|--------------------|-------------|
| Race Conditioners  |   iris-170  |
| Ancient-Forest     |   iris-171  |
| Vector Ninjas      |   iris-172  |
| Ctrl+C, Ctrl+Vibes |   iris-173  |
| Cache Queens       |   iris-174  |
| SIMD Legends       |   iris-175  |
