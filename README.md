# SBM_CommunityDetection

## Overview

This repository, **SBM_CommunityDetection**, is dedicated to the study of community detection within the framework of the **Stochastic Block Model (SBM)**. The project explores and implements various algorithms designed to identify communities or clusters in networks that are generated according to the SBM.

## What is the Stochastic Block Model (SBM)?

The Stochastic Block Model is a popular generative model for random graphs, which is widely used in the field of network science to model community structures. In an SBM, nodes are divided into distinct groups or communities, and edges between pairs of nodes are created based on probabilities that depend on the groups to which the nodes belong. The SBM is particularly useful for studying community detection because it provides a controlled environment to analyze how well different algorithms can uncover the underlying community structure.

## What is Community Detection?

Community detection is a crucial task in network analysis, aiming to identify groups of nodes (communities) that are more densely connected internally than with the rest of the network. This process is important in many applications, such as social network analysis, biology, and recommendation systems. Effective community detection can reveal hidden structures in data, aiding in better understanding of the underlying systems.

## Algorithms Used

This project implements two specific community detection algorithms:

1. **dynamic_community_detection**: The details of this algorithm are based on the methodology presented in *[Dynamic community detection in evolving networks using locality modularity optimization](https://link.springer.com/article/10.1007/s13278-016-0325-1)*. This algorithm is specifically designed focusing on optimizing modularity to accurately detect community structure.

2. **belief_propagation**: This algorithm is inspired by the work presented in *[Streaming Belief Propagation for Community Detection](https://proceedings.neurips.cc/paper/2021/hash/e2a2dcc36a08a345332c751b2f2e476c-Abstract.html)*. It utilizes belief propagation techniques.

For more detailed descriptions of these algorithms, including their theoretical background and implementation details, please refer to the original papers cited within the repository.

## Dependencies
1. The program uses C\++17 but should be compatible with older versions as well(tested on C++17 only though).
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
