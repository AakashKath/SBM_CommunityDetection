# SBM_CommunityDetection
Community Detection in the Stochastic Block Model

## Dependencies
1. The program uses C++17 but should be compatible with older versions as well(tested on C++17 only though).
2. Graphviz is needed for graph visualization, so you need to install `libgraphviz-dev`. We will make this optional in future versions.

`Note: All dependencies can automatically be downloaded using the build script.`

## Steps to Execute
1. To build the program, execute:
    ```shell
    ./build.sh
    ```
2. After successful build, the executable can be found at:
    ```shell
    ./build/bin/SBM_CommunityDetection
    ```
3. The program also creates a JPEG image of the graph, which was initially used for testing purposes but can be extended for a live graph update demo.
