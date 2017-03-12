# TrussFEM
Basic implementation of bar finite elements for truss structures

## Dependencies ##
* [Armadillo](http://arma.sourceforge.net/) 
* [LAPACK](http://icl.cs.utk.edu/lapack-for-windows/lapack/)

## Usage ##
TrussFEM.exe materialPath nodePath elementPath boundaryConditionsPath

materialPath, nodePath, elementPath, and boundaryConditionsPath are paths to text files describing the geometry, materials, and boundary conditions used to solve the truss problem. Their formats are described below.

## Input File Formats ##
### Material Definition File ###
Material file is the following format:

[id_1  A_1   E_1;
 id_2  A_2   E_2;
 ...
 id_MM A_MM  E_MM]

where id_i is material i's unique integer id, A_i is the cross-sectional area of the material and E_i is the material's young's modulus.

### Node Definition File ###
Node file is the following format:
[id_1  x_1   y_1;
 id_2  x_2   y_2;
 ...
 id_N  x_N   y_N]
where id_i is node i's unique integer id, x_i, y_i are node i's coordinates

### Element Defintion File ###
Element file is the following format:
[id_1  nid_11    nid_12    mid_1;
 id_2  nid_21    nid_22    mid_2;
 ...
 id_M  nid_M1    nid_M2    mid_N]
where id_i is element i's unique integer id, nid_i1, nid_12 are the two nodes comprising the elemnt, and mid_i is the integer id of the element
