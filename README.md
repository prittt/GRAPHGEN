![Header Image](./doc/logo/graphgen_inline.png)

[![docs](https://readthedocs.org/projects/pip/badge/?version=latest&style=flat)](https://prittt.github.io/GRAPHGEN/)
[![release](https://img.shields.io/github/v/release/prittt/GRAPHGEN)](https://github.com/prittt/GRAPHGEN/releases/latest/)
[![license](https://img.shields.io/github/license/prittt/GRAPHGEN)](https://github.com/prittt/GRAPHGEN/blob/master/LICENSE)<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![contributors](https://img.shields.io/badge/all_contributors-3-orange.svg?style=flat)](#contributors)
<!-- ALL-CONTRIBUTORS-BADGE:END -->

<!--MAIN-DATA-BEGIN-->

<p align="justify">
GRAPHGEN is a general open-source framework for optimizing algorithms that can be modeled with decision tables such as Connected Component Labeling, Thinning, Chain Code (Contour Tracing), and Morphological operators. Generally, all those algorithms in which the output value for each image pixel is obtained from the value of the pixel itself and of some of its neighbors can be defined in such a way. The framework allows to automatically apply many different optimization strategies to a given problem, taking its definition in terms of conditions to check and actions to be performed as input and directly producing the C++ code including those optimizations as output.
</p>

In short, GRAPHGEN is able to:


* <p align="justify"> Generate the Optimal Decision Tree (ODTree) associated to a given problem <a href="#HYPERCUBE">[1]</a>;</p>
* <p align="justify"> Compress the ODTree into a Directed Rooted Acyclic Graph (DRAG) and generate it, in order to better fit instruction cache <a href="#DRAG">[2]</a>;</p>
* <p align="justify"> Apply pixel (or state) prediction <a href="#EFM">[3]</a>, <a href="#CTB">[4]</a> thus generating a Forest of Decision Trees (FDTrees) from the original ODTtree. Prediction allows to recycle information obtained in the previous step in the current one, thus saving memory accesses and reducing the total execution time <a href="#PRED">[5]</a>;</p>
* <p align="justify"> Remove condition checks by generating special Decision Trees (DTtrees) for the start/end of the line and special FDTrees for the first/last line <a href="#Spaghetti">[6]</a>;</p>
* <p align="justify"> Compress the FDT into a multi-rooted acyclic graph in order to better fit instruction cache;</p>
* <p align="justify"> Introduce frequencies in the generation of ODTs to better fit data and improve the performance of an algorithm over a particular use-case scenario.</p>

<p align="justify">
As mentioned, the generation process is only related to the definition of the problem, meaning that the same problem can be solved using different definitions such as exploiting different scanning masks <a href="#CTB">[4]</a>, <a href="#SAUF">[7]</a>, <a href="#BBDT">[8]</a>, <a href="#CCIT">[9]</a>.</p>

<p align="justify">
For all the aforementioned optimization strategies GRAPHGEN is able to generate both the visual representation of the Decision Tree/Forest/DRAG and the C++ code implementing it.
</p>

<b><i>Tested Platforms</i>: Windows (VS2017), Linux (GCC 9.x or later).</b>

## How to Run GRAPHGEN
### Requirements (Windows)
* For compiling: Visual Studio 2017 (later versions are not tested);
* [CMake](https://cmake.org/) 3.12 or later;
* OpenCV 3.x (optional, only needed for frequency calculation);
* graphviz (included in the repository as executable);
* yaml-cpp (included in the repository as submodule).


### Requirements (Linux)
* For compiling: GCC 9.x or later (for full std::filesystem support);
* [CMake](https://cmake.org/) 3.12 or later;
* [graphviz](https://www.graphviz.org/download/) for producing SVG representations of the generated graphs, using the `dot` command. Can be installed e.g. through apt: `sudo apt install graphviz`;
* OpenCV 3.x (optional, only needed for frequency calculation);
* yaml-cpp (included in the repository as submodule).

### Setup
* Install the requirements as described above;
* Open CMake and point it to the root directory of this repository. The build folder can be e.g. a subfolder called `bin` or `build`.
* Press "Configure";
* These are important variables to set:
    * `GRAPHGEN_FREQUENCIES_ENABLED`: enables frequency calculation and corresponding build targets (e.g. `Spaghetti_FREQ`). If enabled:
    `OpenCV_DIR` points to the build folder of an OpenCV 3.x install with identical architecture and compiler, and `GRAPHGEN_FREQUENCIES_DATASET_DOWNLOAD` must be enable if you wish to download the datasets used in frequency calculation (archive size: ca. 2-3 GB). This flag is mandatory for frequency calculation if you have not downloaded the dataset before.
    * **On Linux**: if you wish to change the architecture to 64-bit (default is 32-bit), change occurences of `-m32` to `-m64` in `CMAKE_CXX_FLAGS` and `CMAKE_C_FLAGS`;
    * **On Linux**: you can adjust the build type in `CMAKE_BUILD_TYPE` (`Release` preferred for faster decision tree and forest calculation).
* Press "Generate" to generate the project.
* Build and execute the project: 
    * **On Windows**: open the generated project solution, select the desired start-up target and execute either in Debug or Release.
    * **On Linux**: `cd` into the build folder, `make` and then execute one of the built targets (e.g. `./SAUF`).
* The outputs (generated code and graphs) of the executables will be stored inside the build folder in a subfolder called `outputs` (e.g. `GRAPHGEN/build/outputs/SAUF/SAUF_tree.svg`).

## Configuring GRAPHGEN
Some application behavior can be configured by changing the `config.yaml` in the build folder. 

- `force_odt_generation` - boolean value that forces the optimal decision tree to be generated in every execution. This may be necessary when implementing own algorithms and debugging them:

```yaml
force_odt_generation: false
```

- `datasets` - list of datasets to be used for frequency calculation. All the [YACCLAB](https://github.com/prittt/YACCLAB) datasets are available if the  `GRAPHGEN_FREQUENCIES_DATASET_DOWNLOAD` was `ON` during the configuration: 

```yaml
datasets: ["fingerprints", "hamlet", "3dpes", "xdocs", "tobacco800", "mirflickr", "medical", "classical"]
```

- `paths` - dictionary with both input (folder containing the datasets used for frequency calculation) and output (where output code, graphs, and frequenices will be stored) paths. It is automatically filled by Cmake during the creation of the project: 

```yaml
paths: {input: "<datasets_path>", output: "<output_results_path>"}
```

- `dot` - dictionary to configure dot output format. Three different configurations parameters are available: 
    * `out_format` - output format of the generated graphs. Currently supported formats are `"pdf"`, `"png"`, and `"svg"`;
    * `background` - background color of the generated graphs. It can be one of the color supported by dot, such as `"white"`, `"red"`, `"turquoise"`, `"sienna"`, `"transparent"`, etc;
    * `ranksep` - vertical separation for objects belonging to two consecutive lines of the output graph.

```yaml
dot: {out_format: "svg", background: "transparent", ranksep: "0.5"}
```

## Code Structure
<p align="justify">
The source code contains the library itself and dozens of example targets specific for different algorithms, already available in literature or newly generated with GRAPHGEN (the latter are identified with a *). A complete description follows.
</p>

### Connected Component Labeling (Labeling)
- `SAUF` reproduce the Scan Array-based Union Find algorithm generating the optimal decision tree for the Rosenfeld scanning mask <a href="#SAUF">[7]</a>;
- `PRED` reproduce the algorithm proposed in <a href="#PRED">[5]</a>, generating the optimal decision tree for the Rosenfeld scanning mask and thus applying prediction;
- `PRED++*` applies the code compression strategy on the forest of PRED algorithm;
- `SAUF3D*` generates the optimal decision tree for the 3D Rosenfeld scanning mask;
- `SAUF++3D*` generate the optimal decision tree for the 3D Rosenfeld scanning mask and applies compression converting the tree into a Directed Rooted Acyclic Graphs (DRAG);
- `BBDT` reproduces the Block-Based approach with Decision Trees generating the optimal decision tree for the Grana scanning mask <a href="#BBDT">[8]</a>;
- `BBDT_FREQ` is the optimal decision tree generated from the Grana scanning mask considering patterns frequency <a href="#BBDT_FREQ">[10]</a>.
- `DRAG` reproduces the connected component labeling on DRAG ([ˈdrʌg]) algorithm that is the optimal decision tree of BBDT converted into a Directed Rooted Acyclic Graphs (DRAG) and compressed <a href="#DRAG">[2]</a>;
- `DRAG_FREQ*` the same as DRAG but generated considering patterns frequency;
- `Spaghetti` reproduces the Spaghetti algorithm proposed in <a href="#Spaghetti">[6]</a> generating the optimal decision tree associated to the Grana scanning mask and applying prediction and compression;
- `Spaghetti_FREQ` the same as Spaghetti but considers patterns frequencies;
- `Tagliatelle*` generates the optimal decision tree for the Grana scanning mask and applies prediction;
- `PRED3D*` generates the optimal decision tree for the 3D Rosenfeld scanning mask and applies prediction;
- `PRED++3D*` generates the optimal decision tree for the 3D Rosenfeld scanning mask and applies prediction and compression.

### Image Skeletonization (Thinning)
- `ZS_TREE` reproduce the algorithm presented in <a href="#BBDT_FREQ">[10]</a> generating the optimal decision tree for the Zhang and Suen <a href="#ZS">[11]</a> approach;
- `ZS_Spaghetti*` generates the optimal decision tree for the Zhang and Suen <a href="#ZS">[11]</a> approach applying both prediction and compression;
- `ZS_Spaghetti_FREQ*` the same as `ZS_Spaghetti` but considering patterns frequency;
- `GH_TREE*` generates the optimal decision tree for the Guo and Hall <a href="#GH">[12]</a> approach;
- `GH_Spaghetti*` generates the optimal decision tree for the Guo and Hall <a href="#GH">[12]</a> approach, applying also prediction and compression.
- `GH_Spaghetti_FREQ*` the same as `GH_Spaghetti` but considering patterns frequency;
- `CH_TREE*` generates the optimal decision tree for the Chen and Hsu <a href="#CH">[13]</a> approach;
- `CH_Spaghetti*` generates the optimal decision tree for the Chen and Hsu <a href="#CH">[13]</a>, applying also prediction and compression;
- `CH_Spaghetti_FREQ*` the same as `CH_Spaghetti` but considering patterns frequency.

### Chain-Code
- `Cederberg_TREE*` generates the optimal decision tree for the Cederberg <a href="#Cederberg">[14]</a> algorithm;
- `Cederberg_Spaghetti*` generates the optimal decision tree for the Cederberg <a href="#Cederberg">[14]</a> algorithm, applying also prediction and compression;
- `Cederberg_Spaghetti_FREQ*` the same as `Cederberg_Spaghetti` but considering patterns frequency.

## Contributors

Thanks goes to these wonderful people ([emoji key](https://allcontributors.org/docs/en/emoji-key)):
<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tr>
    <td align="center"><a href="http://www.federicobolelli.it"><img src="https://avatars3.githubusercontent.com/u/6863130?v=4" width="100px;" alt=""/><br /><sub><b>Federico Bolelli</b></sub></a><br /><a href="https://github.com/prittt/GRAPHGEN/commits?author=prittt" title="Code">💻</a> <a href="#projectManagement-prittt" title="Project Management">📆</a> <a href="#maintenance-prittt" title="Maintenance">🚧</a> <a href="#infra-prittt" title="Infrastructure (Hosting, Build-Tools, etc)">🚇</a> <a href="#ideas-prittt" title="Ideas, Planning, & Feedback">🤔</a> <a href="https://github.com/prittt/GRAPHGEN/commits?author=prittt" title="Documentation">📖</a></td>
    <td align="center"><a href="https://github.com/stal12"><img src="https://avatars2.githubusercontent.com/u/34423515?v=1" width="100px;" alt=""/><br /><sub><b>Stefano Allegretti</b></sub></a><br /><a href="https://github.com/prittt/GRAPHGEN/commits?author=stal12" title="Code">💻</a> <a href="#maintenance-stal12" title="Maintenance">🚧</a> <a href="#infra-stal12" title="Infrastructure (Hosting, Build-Tools, etc)">🚇</a></td>
    <td align="center"><a href="https://github.com/CostantinoGrana"><img src="https://avatars2.githubusercontent.com/u/18437151?v=1" width="100px;" alt=""/><br /><sub><b>Costantino Grana</b></sub></a><br /><a href="https://github.com/prittt/GRAPHGEN/commits?author=CostantinoGrana" title="Code">💻</a> <a href="#projectManagement-CostantinoGrana" title="Project Management">📆</a> <a href="#ideas-CostantinoGrana" title="Ideas, Planning, & Feedback">🤔</a> <a href="#infra-CostantinoGrana" title="Infrastructure (Hosting, Build-Tools, etc)">🚇</a></td>
  </tr>
</table>

<!-- markdownlint-enable -->
<!-- prettier-ignore-end -->
<!-- ALL-CONTRIBUTORS-LIST:END -->

This project follows the [all-contributors](https://github.com/all-contributors/all-contributors) specification. Contributions of any kind welcome.

## References

<table style="border:0;">
<tr>
    <td style="vertical-align: top !important;" align="right">
      <a name="HYPERCUBE">[1]</a>
    </td>
    <td>
      <p align="justify">H. Schumacher and K. C. Sevcik, "The syntheticapproach to decision table conversion," Communications of the ACM, 19(6):343–351, June 1976.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
      <a name="DRAG">[2]</a>
    </td>
    <td>
      <p align="justify">F. Bolelli, L. Baraldi, M. Cancilla, C. Grana, "Connected Components Labeling on DRAGs," in International Conference on Pattern Recognition, 2018, pp. 121-126.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="EFM">[3]</a>
    </td>
    <td>
      <p align="justify">L. He, Y. Chao, and K. Suzuki, "An efficient first-scan method for label-equivalence-based  labeling  algorithms," Pattern Recognition Letters, vol. 31, no. 1, pp. 28–35, 2010.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
      <a name="CTB">[4]</a>
    </td>
    <td>
      <p align="justify">L. He, X. Zhao, Y. Chao, and  K. Suzuki, "Configuration-Transition-Based Connected-Component Labeling," IEEE  Transactions on Image Processing, vol. 23, no. 2, pp. 943–951, 2014.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
      <a name="PRED">[5]</a>
    </td>
    <td>
      <p align="justify"> C. Grana, L. Baraldi, and F. Bolelli, "Optimized Connected Components Labeling with Pixel Prediction", in Advanced  Concepts for Intelligent Vision Systems, 2016, pp. 431-440.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
      <a name="Spaghetti">[6]</a>
    </td>
    <td>
      <p align="justify">F. Bolelli, S. Allegretti, L. Baraldi, and C. Grana, "Spaghetti Labeling: Directed Acyclic Graphs for Block-Based Bonnected Components Labeling," IEEE Transactions on Image Processing, vol. 29, no. 1, pp. 1999-2012, 2019.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
      <a name="SAUF">[7]</a>
    </td>
    <td>
      <p align="justify">K. Wu, E. Otoo, and K. Suzuki, "Optimizing two-pass connected-component labeling algorithms," Pattern Analysis and Applications, vol. 12, no. 2, pp. 117–135, 2009.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
      <a name="BBDT">[8]</a>
    </td>
    <td>
      <p align="justify">C.  Grana,  D.  Borghesani,  and  R.  Cucchiara,  "Optimized  Block-based Connected Components Labeling with Decision Trees," IEEE Transactions on Image Processing, vol. 19, no. 6, pp. 1596–1609, 2010.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
      <a name="CCIT">[9]</a>
    </td>
    <td>
      <p align="justify">W.-Y. Chang, C.-C. Chiu, and  J.-H. Yang,  "Block-based connected-component labeling algorithm using binary decision trees," Sensors, vol. 15, no. 9, pp. 763–23, 2015.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="BBDT_FREQ">[10]</a>
    </td>
    <td>
        <p align="justify">C. Grana, M. Montangero, and D. Borghesani, "Optimal decision trees for local image processing algorithms,"Pattern Recognition Letters, vol. 33, no. 16, pp. 2302–2310, 2012.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="ZS">[11]</a>
    </td>
    <td>
        <p align="justify">T.Y. Zhang, and C. Y. Suen, "A Fast Parallel Algorithm for Thinning Digital Patterns," Communications of the ACM, vol. 27, no. 3, pp. 236–239, 1984.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="GH">[12]</a>
    </td>
    <td>
        <p align="justify">Z. Guo, and R. W. Hall, "Parallel Thinning with Two-Subiteration Algorithms," Communications of the ACM, vol. 32, no. 3, pp. 359–373, 1989.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="CH">[13]</a>
    </td>
    <td>
        <p align="justify">Y.-S. Chen and W.-H. Hsu, "A modified fast parallel algorithm for thinning digital patterns," Pattern Recognition Letters, vol. 7, no. 2, pp. 99–106, 1988.</p>
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="Cederberg">[14]</a>
    </td>
    <td>
        <p align="justify">R. L., Cederberg "Chain-link coding and segmentation for raster scan devices," Computer Graphics and Image Processing, vol. 10, no. 3, pp. 224-234, 1979.</p>
    </td>
</tr>
</table>