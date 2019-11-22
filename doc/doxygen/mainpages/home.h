/**
@mainpage Documentation

<div style="width: 85%; margin: 0 auto;">
<img src="graphgen.png" height=400px style="float: right; padding-right:100px" />

<div style="width: 70%;">
<h2 class="num">Introduction</h2>
GRAPHGEN is the all encompassing GRAPHs GENerator! It is a general open-source framework for optimizing the performance of many binary image processing algorithms. Starting from just a set of rules, it automatically generates decision trees with minimum path-length considering image pattern frequencies, then applies state prediction and code compression producing Directed Rooted Acyclic Graphs (DRAG) that combine these different optimization techniques. You can find a theoretical description of GRAPHGEN capabilities <a href="2020_CVPR_One_DAG_to_Rule_Them_All.pdf">here</a>.
<br/><br/>
GRAPHGEN is able to:
- generate the Optimal Decision Tree (ODTree) associated to a given problem <a href="#HYPERCUBE">[1]</a>;
- compress the ODT into a Directed Rooted Acyclic Graph (DRAG) and generate it, in order to better fit instruction cache <a href="#DRAG">[2]</a>
- apply pixel (or state) prediction <a href="#EFM">[3]</a>,<a href="#CTB">[4]</a> thus generating a Forest of Decision Trees (FDTrees) from the original ODTree. Prediction allows to recycle information obtained in the previous step in the current one, thus saving memory accesses and reducing the total execution time <a href="#PRED">[5]</a>;
- remove condition checks by generating special Decision Trees (DTrees) for the start/end of the line and special FDTrees for the first/last line <a href="#Spaghetti">[6]</a>;
- compress the FDTrees into a multi-rooted acyclic graph in order to better fit instruction cache;
- introduce frequencies in the generation of ODTrees to better fit and improve the performance of an algorithm over a particular use-case scenario;



<h2 class="num">How to Run GRAPHGEN</h2>
<h3 class="nocount">2.1 Requirements (Windows)</h3>
- For compiling: Visual Studio 2017 (later versions are not tested).
- <a href="https://cmake.org/" target="_blank">CMake</a> 3.12 or later.
- OpenCV 3.x (optional, only needed for frequency calculation).
- graphviz (included in the repository as executable).
- yaml-cpp (included in the repository as submodule).

<h3 class="nocount">2.2 Requirements (Linux)</h3>
- For compiling: GCC 9.x or later (for full std::filesystem support).
- <a href="https://cmake.org/" target="_blank">CMake</a> 3.12 or later.
- <a href="https://www.graphviz.org/download/" target="_blank">graphviz</a> for producing SVG representations of the generated graphs, using the `dot` command. Can be installed e.g. through apt: `sudo apt install graphviz`.
- OpenCV 3.x (optional, only needed for frequency calculation).
- yaml-cpp (included in the repository as submodule).

<h3 class="nocount">2.3 Setup</h3>
- Install the requirements as described above.
- Open CMake and point it to the root directory of this repository. The build folder can be e.g. a subfolder called "bin" or "build".
- Press "Configure".
- These are important variables to set:
    - `GRAPHGEN_FREQUENCIES_ENABLED`: Enables frequency calculation and corresponding build targets (e.g. `Spaghetti_FREQ`). If enabled:
        - `OpenCV_DIR`: Point to the build folder of an OpenCV 3.x install with identical architecture and compiler.
        - `GRAPHGEN_FREQUENCIES_DATASET_DOWNLOAD`: Enable if you wish to download the datasets used in frequency calculation (archive size: ca. 2-3 GB). Obligatory for frequency calculation if you have not downloaded them before.
    - On Linux: if you wish to change the architecture to 64-bit (default is 32-bit), change occurences of `-m32` to `-m64` in `CMAKE_CXX_FLAGS` and `CMAKE_C_FLAGS`.
    - On Linux: You can adjust the build type in `CMAKE_BUILD_TYPE` (`Release` preferred for faster decision tree and forest calculation).
- Press "Generate" to generate the project files.
- Build and execute the project:
    - On Windows: open the generated project solution, select the desired start-up target and execute either in Debug or Release.
    - On Linux: `cd` into the build folder, `make` and then execute one of the built targets (e.g. `./SAUF`).
- The outputs (generated code and graphs) of the executables will be stored inside the build folder in a subfolder called `outputs` (e.g. `GRAPHGEN/build/outputs/SAUF/SAUF_tree.svg`).

<h3 class="nocount">2.4 Configuring GRAPHGEN</h3>
Some application behavior can be configured by changing the `config.yaml` in the build folder.

- `force_odt_generation`: Forces the optimal decision tree to be generated in every execution. This may be necessary when implementing own algorithms and debugging them. *Default: false*
- `datasets`: Datasets used for frequency calculation. Available datasets from the [YACCLAB dataset](https://github.com/prittt/YACCLAB#the-yacclab-dataset) (downloaded with `GRAPHGEN_FREQUENCIES_DATASET_DOWNLOAD`): `"3dpes"`, `"check"`, `"fingerprints"`, `"hamlet"`, `"medical"`, `"mirflickr"`, `"tobacco800"`, `"xdocs"`, `"classical"`, `"granularity"`
- `paths`:
    - `input`: path to the folder containing the datasets used for frequency calculation.
    - `output`: path where all outputs (code, graphs, frequenices) will be stored
- `dot`:
    - `out_format`: Output format of the generated graphs. Currently supported: `"pdf"`, `"png"`, and `"svg"`. *Default: svg*
    - `background`: Color of the background of the generated graphs. can be one of the color supported by dot, such as `"white"`, `"red"`, `"turquoise"`, `"sienna"`, `"transparent"`, etc.


<h2 class="num">Code Structure</h2>
The source code contains the library itself and a dozens of example targets specific for different algorithms, already available in literature or newly generated with GRAPHGEN (the latter are identified with a *). A complete description follows.

<h3 class="nocount">3.1 Connected Component Labeling (Labeling)</h3>
- `SAUF` reproduce the Scan Array-based Union Find algorithm generating the optimal decision tree for the Rosenfeld scanning mask <a href="#SAUF">[7]</a>.
- `PRED` reproduce the algorithm proposed in <a href="#PRED">[5]</a>, generating the optimal decision tree for the Rosenfeld scanning mask and thus applying prediction.
- `PRED++*` applies the code compression strategy on the forest of PRED algorithms
- `SAUF3D*` generate the optimal decision tree for the 3D Rosenfeld scanning mask.
- `SAUF++3D*` generate the optimal decision tree for the 3D Rosenfeld scanning mask and applies compression converting the tree into a Directed Rooted Acyclic Graphs (DRAG).
- `BBDT` reproduce the Block-Based approach with Decision Trees generating the optimal decision tree for the Grana scanning mask <a href="#BBDT">[8]</a>.
- `BBDT_FREQ` is the optimal decision tree generated from the Grana scanning mask considering patterns frequency <a href="#BBDT_FREQ">[10]</a>.
- `DRAG` reproduce the connected component labeling on DRAG ([ˈdrʌg]) algorithm that is the optimal decision tree of BBDT converted into a Directed Rooted Acyclic Graphs (DRAG) and compressed <a href="DRAG">[2]</a>.
- `DRAG_FREQ*` the same as DRAG but generated considering patterns frequency.
- `Spaghetti` reproduce the Spaghetti algorithm proposed in <a href="Spaghetti">[6]</a> generating the optimal decision tree associated to the Grana scanning mask and applying prediction and compression.
- `Spaghetti_FREQ` the same as Spaghetti but considers patterns frequencies.
- `Tagliatelle*` generates the optimal decision tree for the Grana scanning mask and applies prediction
- `PRED3D*` generates the optimal decision tree for the 3D Rosenfeld scanning mask and applies prediction
- `PRED++3D*` generates the optimal decision tree for the 3D Rosenfeld scanning mask and applies prediction and compression

<h3 class="nocount">3.2 Thinning</h3>
- `ZS_TREE` reproduce the algorithm presented in <a href="BBDT_FREQ">[10]</a> generating the optimal decision tree for the Zhang and Suen <a href="ZS">[11]</a> approach.
- `ZS_Spaghetti*` generates the optimal decision tree for the Zhang and Suen <a href="ZS">[11]</a> approach applying both prediction and compression.
- `ZS_Spaghetti_FREQ*` the same as `ZS_Spaghetti` but considering patterns frequency
- `GH_TREE*` generates the optimal decision tree for the Guo and Hall <a href="GH">[12]</a> approach.
- `GH_Spaghetti*` generates the optimal decision tree for the Guo and Hall <a href="GH">[12]</a> approach, applying also prediction and compression.
- `GH_Spaghetti_FREQ*` the same as `GH_Spaghetti` but considering patterns frequency.
- `CH_TREE*` generates the optimal decision tree for the Chen and Hsu <a href="CH">[13]</a> approach.
- `CH_Spaghetti*` generates the optimal decision tree for the Chen and Hsu <a href="CH">[13]</a>, applying also prediction and compression.
- `CH_Spaghetti_FREQ*` the same as `CH_Spaghetti` but considering patterns frequency.

<h3 class="nocount">3.3 Chain Code</h3>
- `Cederberg_TREE*` generates the optimal decision tree for the Cederberg <a href="Cederberg">[14]</a> algorithm
- `Cederberg_Spaghetti*` generates the optimal decision tree for the Cederberg <a href="Cederberg">[14]</a> algorithm, applying also prediction and compression
- `Cederberg_Spaghetti_FREQ*` the same as `Cederberg_Spaghetti` but considering patterns frequency.


<h2 class="num">References</h2>
<table style="border:0;">
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="HYPERCUBE">[1]</a>
    </td>
    <td>
        H. Schumacher and K. C. Sevcik, "The syntheticapproach  to  decision  table  conversion," Commun. ACM, 19(6):343–351, June 1976
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="DRAG">[2]</a>
    </td>
    <td>
        Bolelli, Federico; Baraldi, Lorenzo; Cancilla, Michele; Grana, Costantino "Connected Components Labeling on DRAGs" Proceedings of the 23rd International Conference on Pattern Recognition, Beijing, China, 20-24 Aug 2018.
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="EFM">[3]</a>
    </td>
    <td>
        L. He, Y. Chao, and K. Suzuki, “An efficient first-scan method for label-equivalence-based  labeling  algorithms,” Pattern Recognition Letters,vol. 31, no. 1, pp. 28–35, 2010.
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="CTB">[4]</a>
    </td>
    <td>
         L.  He,  X.  Zhao,  Y.  Chao,  and  K.  Suzuki, Configuration-Transition-Based  Connected-Component  Labeling, IEEE  Transactions  on  Image Processing, vol. 23, no. 2, pp. 943–951, 2014.
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="PRED">[5]</a>
    </td>
    <td>
        C. Grana, L. Baraldi, and F. Bolelli, Optimized Connected Components Labeling  with  Pixel  Prediction, in Advanced  Concepts  for  Intelligent Vision Systems, 2016.
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="Spaghetti">[6]</a>
    </td>
    <td>
        Bolelli, Federico; Allegretti Stefano; Baraldi, Lorenzo; Grana, Costantino "Spaghetti Labeling: Directed Acyclic Graphs for Block-Based Connected Components Labeling" IEEE Transactions on Image Processing, 2019.
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="SAUF">[7]</a>
    </td>
    <td>
        K. Wu, E. Otoo, and K. Suzuki, Optimizing two-pass connected-component labeling algorithms,” Pattern Analysis and Applications, vol. 12, no. 2, pp. 117–135, 2009.
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="BBDT">[8]</a>
    </td>
    <td>
        C.  Grana,  D.  Borghesani,  and  R.  Cucchiara,  “Optimized  Block-based Connected Components Labeling with Decision Trees,” IEEE Transac-tions on Image Processing, vol. 19, no. 6, pp. 1596–1609, 2010.
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="CCIT">[9]</a>
    </td>
    <td>
        W.-Y.  Chang,  C.-C.  Chiu,  and  J.-H.  Yang,  “Block-based  connected-component  labeling  algorithm  using  binary  decision  trees,” Sensors, vol. 15, no. 9, pp. 23 763–23 787, 2015.
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="BBDT_FREQ">[10]</a>
    </td>
    <td>
        C. Grana, M. Montangero, and D. Borghesani, “Optimal decision trees for local image processing algorithms,” Pattern Recognition Letters, 33(16):2302–2310, 2012.
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="ZS">[11]</a>
    </td>
    <td>
        T.Y. Zhang, and C. Y. Suen, “A Fast Parallel Algorithm for Thinning Digital Patterns,” Communications of the ACM, 27(3):236–239, 1984.
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="GH">[12]</a>
    </td>
    <td>
        Z. Guo, and R. W. Hall, “Parallel Thinning with Two-Subiteration Algorithms,” Communications of the ACM, 32(3):359–373, 1989.
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="CH">[13]</a>
    </td>
    <td>
        Y.-S. Chen and W.-H. Hsu, “A modified fast parallel algorithm for thinning digital patterns,” Pattern Recognition Letters, 7(2):99–106, 1988
    </td>
</tr>
<tr>
    <td style="vertical-align: top !important;" align="right">
        <a name="Cederberg">[14]</a>
    </td>
    <td>
        R. L., Cederberg “Chain-link coding and segmentation for raster scan devices,” Computer Graphics and Image Processing, 10(3), 224-234, 1979.
    </td>
</tr>
</table>

</div>
</div>
*/