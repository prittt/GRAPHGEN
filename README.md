# GRAPHGEN

<!--
Can be displayed only if the repo is public.
[![Documentation](https://codedocs.xyz/prittt/graphgen.svg)]()
-->

## Requirements

### Windows
* Compiler: Visual Studio 2017 or later.
No further requirements. yaml-cpp and graphviz are distributed (as submodule and executable respectively) with Graphgen.

### Linux
* Compiler: GCC 9.x or later (for full std::filesystem support).
* [graphviz](https://www.graphviz.org/download/) for producing SVG representations of the generated graphs, using the `dot` command. Can be installed e.g. through apt: `sudo apt install graphviz`.