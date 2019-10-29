![Header Image](./doc/logo/graphgen_inline.png)

---

<!--
Can be displayed only if the repo is public.
[![Documentation](https://codedocs.xyz/prittt/graphgen.svg)]()
-->

**GRAPHGEN** is a framework for generating decision trees in various formats for different algorithmic problems of binary image processing (Connected Components Labeling, Thinning and ChainCode (Contour Tracing)). 

*Supported platforms*: Windows (VS2017), Linux (GCC 9.x or later).

## Requirements
### Windows
* For compiling: Visual Studio 2017 (later versions are not tested).
* [CMake](https://cmake.org/) 3.12 or later.
* *OpenCV 3.x (optional, only needed for frequency calculation).*
* *graphviz (included in the repository as executable).*
* *yaml-cpp (included in the repository as submodule).*


### Linux
* For compiling: GCC 9.x or later (for full std::filesystem support).
* [CMake](https://cmake.org/) 3.12 or later.
* [graphviz](https://www.graphviz.org/download/) for producing SVG representations of the generated graphs, using the `dot` command. Can be installed e.g. through apt: `sudo apt install graphviz`.
* *OpenCV 3.x (optional, only needed for frequency calculation).*
* *yaml-cpp (included in the repository as submodule).*

## Setup
1) Install the requirements as described above.
2) Open CMake and point it to the root directory of this repository. The build folder can be e.g. a subfolder called "bin" or "build".
3) Press "Configure".
4) These are important variables to set:
    * `GRAPHGEN_FREQUENCIES_ENABLED`: Enables frequency calculation and corresponding build targets (e.g. `Spaghetti_FREQ`). If enabled:
        * `OpenCV_DIR`: Point to the build folder of an OpenCV 3.x install with identical architecture and compiler.
        * `GRAPHGEN_FREQUENCIES_DATASET_DOWNLOAD`: Enable if you wish to download the datasets used in frequency calculation (archive size: ca. 2-3 GB). Obligatory for frequency calculation if you have not downloaded them before.
    * On Linux: if you wish to change the architecture to 64-bit (default is 32-bit), change occurences of `-m32` to `-m64` in `CMAKE_CXX_FLAGS` and `CMAKE_C_FLAGS`.
    * On Linux: You can adjust the build type in `CMAKE_BUILD_TYPE` (`Release` preferred for faster decision tree and forest calculation).
5) Press "Generate" to generate the project files.
6) Build and execute the project: 
    * On Windows: open the generated project solution, select the desired start-up target and execute either in Debug or Release.
    * On Linux: `cd` into the build folder, `make` and then execute one of the built targets (e.g. `./SAUF`).
7) The outputs (generated code and graphs) of the executables will be stored inside the build folder in a subfolder called `outputs` (e.g. `GRAPHGEN/build/outputs/SAUF/SAUF_tree.svg`).

## config.yaml
Some application behavior can be configured by changing the `config.yaml` in the build folder.

- `force_odt_generation`: Forces the optimal decision tree to be generated in every execution. This may be necessary when implementing own algorithms and debugging them. *Default: false*
- `datasets`: Datasets used for frequency calculation. Available datasets from the [YACCLAB dataset](https://github.com/prittt/YACCLAB#the-yacclab-dataset) (downloaded with `GRAPHGEN_FREQUENCIES_DATASET_DOWNLOAD`): `"3dpes"`, `"check"`, `"fingerprints"`, `"hamlet"`, `"medical"`, `"mirflickr"`, `"tobacco800"`, `"xdocs"`, `"classical"`, `"granularity"`
- `paths`: 
    * `input`: path to the folder containing the datasets used for frequency calculation. 
    * `output`: path where all outputs (code, graphs, frequenices) will be stored
- `dot`: 
    * `out_format`: Output format of the generated graphs. Currently supported: `"pdf"`, `"png"`, and `"svg"`. *Default: svg*
    * `background`: Color of the background of the generated graphs. can be one of the color supported by dot, such as `"white"`, `"red"`, `"turquoise"`, `"sienna"`, `"transparent"`, etc. 
