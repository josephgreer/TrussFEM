#pragma once
#include <armadillo>
#include "Core.h"
#include <map>
#include <vector>

namespace TFM
{
  // Defines a node
  struct Node
  {
    arma::vec2 pos;             // 2D position of the node
    arma::vec2 forces;          // forces applied to the node in the x and y coordinates
    struct NodeResults
    {
      bool valid;               // Are the results valid? (i.e. has the problem been solved?)
      arma::vec2 uv;            // Displacement in global coordinate system
      f64 stress;               // Average stress for every element containing this node
      NodeResults() : valid(false) {}
    } results;
    struct BoundaryConditions
    {
      arma::uvec2 dof;    // dof(0) = 1 implies x DOF is constrained to 0, dof(1) = 1 implies y DOF is constrained to 0
      arma::vec2 forces;  // forces acting on the node

      BoundaryConditions()
      {
        dof(0) = dof(1) = 0;
        forces(0) = forces(1) = 0;
      }
    } bcs;
  };

  struct Material
  {
    f64 A;                      // cross-sectional area of the elements
    f64 E;                      // Young's modulus of the element
  };

  // Defines a link element
  struct LinkElement
  {
    arma::uvec2 nodeIds;        // stores the ids of the two nodes the link element comprises
    s32 mId;                    // stores the id of the material of the element
    struct ElementResults
    {
      bool valid;               // Are the results valid? (i.e. has the problem been solved?)
      f64 theta;                // Angle of the element (defined by the position of its constituent nodes)
      f64 stress;               // Contains the element stress
      ElementResults() : valid(false) {}
    } results;
  };

  // Container for a truss FEM problem
  class TrussFEMProblem
  {
  protected:
    std::map < s32, Material > m_materials;          // Stores the material defintions (key is the material's unique ID)
    std::map < s32, Node > m_nodes;                  // Stores the nodes (key is the node's unique ID)
    std::map < s32, LinkElement > m_elements;        // Stores the elements (key is the element's unique ID)

    // Populate the various lists of data for the FEM problem from the appropriate definition file paths.
    void LoadData(const char * nodesPath, const char * elementsPath, const char * materialsPath,
      const char *boundaryConditionsPath);

  public:
    TrussFEMProblem(const char * nodesPath, const char * elementsPath, const char * materialsPath,
      const char *boundaryConditionsPath);
  };
}