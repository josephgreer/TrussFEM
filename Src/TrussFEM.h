#pragma once
#include <armadillo>
#include "Core.h"
#include <map>
#include <vector>

using namespace arma;

namespace TFM
{
  // Defines a node
  struct Node
  {
    s32 id;                     // unique identifier integer
    vec2 pos;                   // 2D position of the node
    struct NodeResults
    {
      bool valid;               // Are the results valid? (i.e. has the problem been solved?)
      vec2 uv;                  // Displacement in global coordinate system
      f64 stress;               // Average stress for every element containing this node
      NodeResults() : valid(false) {}
    } results;
  };

  // Defines a link element
  struct LinkElement
  {
    s32 id;                     // unique identifier for the element
    uvec2 nodeIds;              // stores the ids of the two nodes the link element comprises
    f64 A;                      // cross-sectional area of the elements
    f64 E;                      // Young's modulus of the element
    struct ElementResults
    {
      bool valid;               // Are the results valid? (i.e. has the problem been solved?)
      f64 stress;               // Contains the element stress
      ElementResults() : valid(false) {}
    } results;
  };

  // Container for a truss FEM problem
  class TrussFEMProblem
  {
  protected:
    std::map < s32, Node > m_nodes;                  // Stores the nodes
    std::map < s32, LinkElement > m_elements;        // Stores the elements
    bool m_isSolved;                                 // Is the FEM problem solved?

  public:
    TrussFEMProblem(const char * nodesPath, const char * elementsPath, const char * materialsPath);
    void Solve();
  };
}