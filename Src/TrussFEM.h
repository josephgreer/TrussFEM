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
    s32 incId;                  // incremental id that is filled in automatically at time of construction
    struct NodeResults
    {
      bool valid;               // Are the results valid? (i.e. has the problem been solved?)
      f64 sigma_xx;             // Average stress for every element containing this node
      f64 epsilon_xx;           // Average strain for every element containing this node
      s32 nel;                  // Number of elements that contain this node
      NodeResults() : valid(false), nel(0) {}
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
    Node() : incId(-1) {}
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
      f64 sigma_xx;             // Contains the element stress (solved for)
      f64 epsilon_xx;           // Contains the element strain (solved for)
      ElementResults() : valid(false) {}
    } results;
  };

#define KSTAR 1e8
#define MAX_COND 1e6

  // Container for a truss FEM problem
  class TrussFEMProblem
  {
  protected:
    std::map < s32, Material > m_materials;          // Stores the material defintions (key is the material's unique ID)
    std::map < s32, Node > m_nodes;                  // Stores the nodes (key is the node's unique ID)
    std::map < s32, LinkElement > m_elements;        // Stores the elements (key is the element's unique ID)
    arma::mat m_K;                                   // Stiffness matrix. Populated during class construction
    arma::vec m_F;                                   // Force Vector
    arma::vec m_u;                                   // Displacements. Needs to be solved for
    arma::vec m_exx;                                 // Strains in link direction
    arma::vec m_sigmaxx;                             // Stresses in link direction
    std::string m_outFileName;                       // Output file name

    // Populate the various lists of data for the FEM problem from the appropriate definition file paths.
    void LoadData(const char * nodesPath, const char * elementsPath, const char * materialsPath,
      const char *boundaryConditionsPath);

  public:
    TrussFEMProblem(const char * nodesPath, const char * elementsPath, const char * materialsPath,
      const char *boundaryConditionsPath, const char *outFileName);
    void PopulateStiffnessMatrix();
    void PopulateForceVector();
    void ApplyPenaltyMethod();
    void Solve();
    void PostProcessStressesAndStrains();
    void PrintAndSaveResults();
  };
}