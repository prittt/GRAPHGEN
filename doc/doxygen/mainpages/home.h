/**
@mainpage Documentation

<p style="padding-left:400px">
<img src="graphgen.png" height=400px style="float: right; padding-right:100px" />

<h2 class="num">Introduction</h2>
GRAPHGEN is the all encompassing GRAPHs GENerator! It is a general open-source framework for optimizing the performance of many binary image processing algorithms. Starting from just a set of rules, it automatically generates decision trees with minimum path-length considering image pattern frequencies, then applies state prediction and code compression producing Directed Rooted Acyclic Graphs (DRAG) that combine these different optimization techniques. You can find a theoretical description of GRAPHGEN capabilities <a href="2020_CVPR_One_DAG_to_Rule_Them_All.pdf">here</a>.
<br/><br/>
GRAPHGEN is able to:
<ul>
    <li>generate the Optimal Decision Tree (ODT) associated to a given problem <a href="#HYPERCUBE">[1]</a>;</li>
    <li>compress the ODT into a Directed Rooted Acyclic Graph (DRAG) and generate it, in order to better fit instruction cache <a href="#DRAG">[2]</a></li>
    <li>apply pixel (or state) prediction <a href="#EFM">[3]</a>,<a href="#CTB">[4]</a> thus generating a Forest of Decision Trees (FDTrees) from the original ODTree. Prediction allows to recycle information obtained in the previous step in the current one, thus saving memory accesses and reducing the total execution time <a href="#PRED">[5]</a>;</li>
    <li>remove condition checks by generating special Decision Trees (DTrees) for the start/end of the line and special FDTrees for the first/last line <a href="#Spaghetti">[6]</a>;</li>
    <li>compress the FDTrees into a multi-rooted acyclic graph in order to better fit instruction cache;</li>
    <li>introduce frequencies in the generation of ODTrees to better fit and improve the performance of an algorithm over a particular use-case scenario;</li>
</ul>


<h2 class="num">How to Run GRAPHGEN</h2>
<h3 class="num">Requirements</h3>
<h4 class="num">Windows</h4>
- For compiling: Visual Studio 2017 (later versions are not tested).
- <a href="https://cmake.org/" target="_blank">CMake</a> 3.12 or later.
- OpenCV 3.x (optional, only needed for frequency calculation).
- graphviz (included in the repository as executable).
- yaml-cpp (included in the repository as submodule).

<h4 class="num">Linux</h4>
- For compiling: GCC 9.x or later (for full std::filesystem support).
- <a href="https://cmake.org/" target="_blank">CMake</a> 3.12 or later.
- <a href="https://www.graphviz.org/download/" target="_blank">graphviz</a> for producing SVG representations of the generated graphs, using the `dot` command. Can be installed e.g. through apt: `sudo apt install graphviz`.
- OpenCV 3.x (optional, only needed for frequency calculation).
- yaml-cpp (included in the repository as submodule).

<h3 class="num">Setup</h3>
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

<h3 class="num">Configuring GRAPHGEN</h3>
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
The source code contains the library itself and a dozens of example targets specific for different algorithms, already available in literature or newly generated with GRAPHGEN (the latter are identified with a *). A complete description follows: 

- SAUF reproduce the Scan Array-based Union Find algorithm generating the optimal decision tree for the Rosenfeld scanning mask [1].</li>
- PRED reproduce the algorithm proposed in [2], generating the optimal decision tree for the Rosenfeld scanning mask and thus applying prediction.</li>
- PRED++* applies the code compression strategy on the forest of PRED algorithms</li>
- SAUF3D* generate the optimal decision tree for the 3D Rosenfeld scanning mask.</li>
- SAUF++3D* generate the optimal decision tree for the 3D Rosenfeld scanning mask and applies compression converting the tree into a  Directed Rooted Acyclic Graphs (DRAG).</li>

<h2 class="num">References</h2>
<a name="HYPERCUBE">[1]</a><em>H. Schumacher and K. C. Sevcik, "The syntheticapproach  to  decision  table  conversion," Commun. ACM, 19(6):343–351, June 1976</em>

<em><a name="DRAG">[2]</a> Bolelli, Federico; Baraldi, Lorenzo; Cancilla, Michele; Grana, Costantino "Connected Components Labeling on DRAGs" Proceedings of the 23rd International Conference on Pattern Recognition, Beijing, China, 20-24 Aug 2018.</em></p>

<em><a name="EFM">[3]</a>L. He, Y. Chao, and K. Suzuki, “An efficient first-scan method for label-equivalence-based  labeling  algorithms,” Pattern Recognition Letters,vol. 31, no. 1, pp. 28–35, 2010.</em></p>

<em><a name="CTB">[4]</a> L.  He,  X.  Zhao,  Y.  Chao,  and  K.  Suzuki, Configuration-Transition-
Based  Connected-Component  Labeling, IEEE  Transactions  on  Image Processing, vol. 23, no. 2, pp. 943–951, 2014.</em></p>

<em><a name="PRED">[5]</a> C. Grana, L. Baraldi, and F. Bolelli, Optimized Connected Components Labeling  with  Pixel  Prediction, in Advanced  Concepts  for  Intelligent Vision Systems, 2016.</em></p>

<em><a name="Spaghetti">[6]</a> Bolelli, Federico; Allegretti Stefano; Baraldi, Lorenzo; Grana, Costantino "Spaghetti Labeling: Directed Acyclic Graphs for Block-Based Connected Components Labeling" IEEE Transactions on Image Processing, 2019.</em></p>

<em><a name="SAUF">[7]</a> K. Wu, E. Otoo, and K. Suzuki, Optimizing two-pass connected-component labeling algorithms,” Pattern Analysis and Applications, vol. 12, no. 2, pp. 117–135, 2009.</em></p>

<em><a name="BBDT">[8]</a> C.  Grana,  D.  Borghesani,  and  R.  Cucchiara,  “Optimized  Block-based Connected Components Labeling with Decision Trees,” IEEE Transac-tions on Image Processing, vol. 19, no. 6, pp. 1596–1609, 2010.</em></p>

<em><a name="CCIT">[9]</a> W.-Y.  Chang,  C.-C.  Chiu,  and  J.-H.  Yang,  “Block-based  connected-component  labeling  algorithm  using  binary  decision  trees,” Sensors, vol. 15, no. 9, pp. 23 763–23 787, 2015.</em></p>
</p>
*/