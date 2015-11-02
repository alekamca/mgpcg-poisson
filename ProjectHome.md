This is the source code for the paper:
A. McAdams, E. Sifakis and J. Teran, "A parallel multigrid Poisson solver for fluids simulation on large grids" in Eurographics/ACM SIGGRAPH Symposium on Computer Animation (M. Otaduy and Z. Popovic, eds), 2010

Abstract:
We present a highly efficient numerical solver for the Poisson equation on irregular voxelized domains supporting an arbitrary mix of Neumann and Dirichlet boundary conditions. Our approach employs a multigrid cycle as a preconditioner for the conjugate gradient method, which enables the use of a lightweight, purely geometric multigrid scheme while drastically improving convergence and robustness on irregular domains. Our method is designed for parallel execution on shared-memory platforms and poses modest requirements in terms of bandwidth and memory footprint.Our solver will accommodate as many as 768×768×1152 voxels with a memory footprint less than 16GB, while a full smoke simulation at this resolution fits in 32GB of RAM. Our preconditioned conjugate gradient solver typically reduces the residual by one order of magnitude every 2 iterations, while each PCG iteration requires approximately 6.1sec on a 16-core SMP at 768^3 resolution. We demonstrate the efficacy of our method on animations of smoke flow past solid objects and free surface water animations using Poisson pressure projection at unprecedented resolutions.

For more information and a copy of the paper, see [here](http://pages.cs.wisc.edu/~sifakis/project_pages/mgpcg.html)

This project makes use of the [PhysBAM](http://physbam.stanford.edu/) library.

Builds have been tested on

  * Ubuntu 9.10 with gcc 4.4 and icc 11.1
  * Mac OSX 10.5 and 10.6 with gcc 4.4