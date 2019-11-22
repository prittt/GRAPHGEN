![Header Image](./doc/logo/graphgen_inline.png)

---


**GRAPHGEN** is a framework for optimizing algorithms that can be modeled with decision tables such as Connected Component Labeling, Thinning, Chain Code (Contour Tracing), and Morphological operators. Generally, all those algorithms in which the output value for each image pixel is obtained from the value of the pixel itself and of some of its neighbors can be defined in such a way. The framework allows to automatically apply many different optimization strategies to a given problem, taking its definition in terms of conditions to check and actions to be performed as input and directly producing the C/C++ code including those optimizations as output. In short, GRAPHGEN is able to: 
- generate the Optimal Decision Tree (ODT) associated to a given problem <a href="#HYPERCUBE">[1]</a>;
- compress the ODT into a Directed Rooted Acyclic Graph (DRAG) and generate it, in order to better fit instruction cache <a href="#DRAG">[2]</a>;
- apply pixel (or state) prediction <a href="#EFM">[3]</a>,<a href="#CTB">[4]</a> thus generating a Forest of Decision Trees (FDT) from the original ODT. Prediction allows to recycle information obtained in the previous step in the current one, thus saving memory accesses and reducing the total execution time <a href="#PRED">[5]</a>;
- remove condition checks by generating special decision trees (DT) for the start/end of the line and special FDT for the first/last line <a href="#Spaghetti">[6]</a>;
- compress the FDT into a multi-rooted acyclic graph in order to better fit instruction cache;
- introduce frequencies in the generation of ODTs to better fit data and improve the performance of an algorithm over a particular use-case scenario;

As mentioned, the generation process is only related to the definition of the problem, meaning that the same problem can be solved using different definitions such as exploiting different scanning masks <a href="#CTB">[4]</a>, <a href="#SAUF">[7]</a>, <a href="#BBDT">[8]</a>, <a href="#CCIT">[9]</a>.

For all the aforementioned optimization strategies GRAPHGEN is able to generate both the visual representation of the Decision Tree/Forest/DRAG and the C/C++ code implementing it.

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
    * `GRAPHGEN_FREQUENCIES_ENABLED`: enables frequency calculation and corresponding build targets (e.g. `Spaghetti_FREQ`). If enabled:
        * `OpenCV_DIR`: point to the build folder of an OpenCV 3.x install with identical architecture and compiler.
        * `GRAPHGEN_FREQUENCIES_DATASET_DOWNLOAD`: enable if you wish to download the datasets used in frequency calculation (archive size: ca. 2-3 GB). Obligatory for frequency calculation if you have not downloaded them before.
    * On Linux: if you wish to change the architecture to 64-bit (default is 32-bit), change occurences of `-m32` to `-m64` in `CMAKE_CXX_FLAGS` and `CMAKE_C_FLAGS`.
    * On Linux: you can adjust the build type in `CMAKE_BUILD_TYPE` (`Release` preferred for faster decision tree and forest calculation).
5) Press "Generate" to generate the project files.
6) Build and execute the project: 
    * On Windows: open the generated project solution, select the desired start-up target and execute either in Debug or Release.
    * On Linux: `cd` into the build folder, `make` and then execute one of the built targets (e.g. `./SAUF`).
7) The outputs (generated code and graphs) of the executables will be stored inside the build folder in a subfolder called `outputs` (e.g. `GRAPHGEN/build/outputs/SAUF/SAUF_tree.svg`).

## config.yaml
Some application behavior can be configured by changing the `config.yaml` in the build folder.

- `force_odt_generation`: forces the optimal decision tree to be generated in every execution. This may be necessary when implementing own algorithms and debugging them. *Default: false*
- `datasets`: datasets used for frequency calculation. Available datasets from the [YACCLAB dataset](https://github.com/prittt/YACCLAB#the-yacclab-dataset) (downloaded with `GRAPHGEN_FREQUENCIES_DATASET_DOWNLOAD`): `"3dpes"`, `"check"`, `"fingerprints"`, `"hamlet"`, `"medical"`, `"mirflickr"`, `"tobacco800"`, `"xdocs"`, `"classical"`, `"granularity"`
- `paths`: 
    * `input`: path to the folder containing the datasets used for frequency calculation. 
    * `output`: path where all outputs (code, graphs, frequenices) will be stored
- `dot`: 
    * `out_format`: output format of the generated graphs. Currently supported: `"pdf"`, `"png"`, and `"svg"`. *Default: svg*
    * `background`:background color of the generated graphs. It can be one of the color supported by dot, such as `"white"`, `"red"`, `"turquoise"`, `"sienna"`, `"transparent"`, etc.
    
 ## References
  
<p align="justify"><em><a name="HYPERCUBE">[1]</a>H. Schumacher and K. C. Sevcik, "The syntheticapproach  to  decision  table  conversion," Commun. ACM, 19(6):343–351, June 1976</em></p>

<p align="justify"><em><a name="DRAG">[2]</a> Bolelli, Federico; Baraldi, Lorenzo; Cancilla, Michele; Grana, Costantino "Connected Components Labeling on DRAGs" Proceedings of the 23rd International Conference on Pattern Recognition, Beijing, China, 20-24 Aug 2018.</em></p>

<p align="justify"><em><a name="EFM">[3]</a>L. He, Y. Chao, and K. Suzuki, “An efficient first-scan method for label-equivalence-based  labeling  algorithms,” Pattern Recognition Letters,vol. 31, no. 1, pp. 28–35, 2010.</em></p>

<p align="justify"><em><a name="CTB">[4]</a> L.  He,  X.  Zhao,  Y.  Chao,  and  K.  Suzuki, Configuration-Transition-
Based  Connected-Component  Labeling, IEEE  Transactions  on  Image Processing, vol. 23, no. 2, pp. 943–951, 2014.</em></p>

<p align="justify"><em><a name="PRED">[5]</a> C. Grana, L. Baraldi, and F. Bolelli, Optimized Connected Components Labeling  with  Pixel  Prediction, in Advanced  Concepts  for  Intelligent Vision Systems, 2016.</em></p>

<p align="justify"><em><a name="Spaghetti">[6]</a> Bolelli, Federico; Allegretti Stefano; Baraldi, Lorenzo; Grana, Costantino "Spaghetti Labeling: Directed Acyclic Graphs for Block-Based Connected Components Labeling" IEEE Transactions on Image Processing, 2019.</em></p>

<p align="justify"><em><a name="SAUF">[7]</a> K. Wu, E. Otoo, and K. Suzuki, Optimizing two-pass connected-component labeling algorithms,” Pattern Analysis and Applications, vol. 12, no. 2, pp. 117–135, 2009.</em></p>

<p align="justify"><em><a name="BBDT">[8]</a> C.  Grana,  D.  Borghesani,  and  R.  Cucchiara,  “Optimized  Block-based Connected Components Labeling with Decision Trees,” IEEE Transac-tions on Image Processing, vol. 19, no. 6, pp. 1596–1609, 2010.</em></p>

<p align="justify"><em><a name="CCIT">[9]</a> W.-Y.  Chang,  C.-C.  Chiu,  and  J.-H.  Yang,  “Block-based  connected-component  labeling  algorithm  using  binary  decision  trees,” Sensors, vol. 15, no. 9, pp. 23 763–23 787, 2015.</em></p>
