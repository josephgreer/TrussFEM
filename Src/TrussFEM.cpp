#include "TrussFEM.h"

namespace TFM
{
  using namespace arma;

  // Node file is the following format:
  // [id_1  x_1   y_1;
  //  id_2  x_2   y_2;
  //  ...
  //  id_N  x_N   y_N]
  // where id_i is node i's unique integer id, x_i, y_i are node i's coordinates

  // Element file is the following format:
  // [id_1  nid_11    nid_12    mid_1;
  //  id_2  nid_21    nid_22    mid_2;
  //  ...
  //  id_M  nid_M1    nid_M2    mid_N]
  // where id_i is element i's unique integer id, nid_i1, nid_12 are the two 
  // nodes comprising the elemnt, and mid_i is the integer id of the element

  // Material file is the following format:
  // [id_1  A_1   E_1;
  //  id_2  A_2   E_2;
  //  ...
  //  id_MM A_MM  E_MM]
  // where id_i is material i's unique integer id, A_i is the cross-sectional area of the material
  // and E_i is the material's young's modulus.

  // Boundary condition file is the following format:
  // [nid_1 t_1 v_1x v_1y;
  //  nid_2 t_2 v_2x v_2y;
  //  ...
  //  nid_NN t_NN v_NNx v_NNy]
  // where nid_i is the node id to which boundary condition i applies.  
  // t_i is the type of boundary condition i is. t_i = 1 is a position constraint on node i. t_i = 2 is a force input on node i
  // v_ix and v_iy are the values of the boundary condition and their interpetation depends on t_i.
  // If t_i is 1 (i.e. a DOF constraint), then v_ix = 1 implies that node i is constrained in the x direction, similarly for v_iy. Otherwise these DOFs are unconstrained.
  // If t_i is 2 (i.e. a force input), then v_ix is the x component of the force acting on node i and v_iy is the y component of the force acting on node i.

  void TrussFEMProblem::LoadData(const char * nodesPath, const char * elementsPath, const char * materialsPath,
    const char *boundaryConditionsPath)
  {
    mat nodes; nodes.load(nodesPath);
    mat elements; elements.load(elementsPath);
    mat materials; materials.load(materialsPath);
    mat bcs;  bcs.load(boundaryConditionsPath);

    if (nodes.n_cols != 3) {
      throw std::runtime_error("Invalid node defintion file format. See readme.md for an explanation of how to format the node definition file.");
    }
    s32 incId = 0;
    for (s32 i = 0; i < nodes.n_rows; i++)
    {
      s32 nid = (s32)(nodes(i, 0));
      vec2 pos; pos << nodes(i, 1) << nodes(i, 2);
      Node n; n.pos = pos; n.incId = incId++;
      m_nodes[nid] = n;
    }

    if (materials.n_cols != 3) {
      throw std::runtime_error("Invalid material defintion file format. See readme.md for an explanation of how to format the material definition file.");
    }
    for (s32 i = 0; i < materials.n_rows; i++)
    {
      s32 mid = (s32)(materials(i, 0));
      Material m; m.A = materials(i, 1); m.E = materials(i, 2);
      m_materials[mid] = m;
    }

    if (elements.n_cols != 4) {
      throw std::runtime_error("Invalid element defintion file format. See readme.md for an explanation of how to format the element definition file.");
    }
    for (s32 i = 0; i < elements.n_rows; i++)
    {
      s32 eid = (s32)(elements(i, 0));
      s32 nid1 = (s32)(elements(i, 1));
      s32 nid2 = (s32)(elements(i, 2));
      s32 mid = (s32)(elements(i, 3));
    }

    if (bcs.n_cols != 4) {
      throw std::runtime_error("Invalid boundary condition defintion file format. See readme.md for an explanation of how to format the boundary condition definition file.");
    }
    for (s32 i = 0; i < bcs.n_rows; i++)
    {
      s32 nid = (s32)(elements(i, 0));
      s32 type = (s32)(elements(i, 1));
      f64 v1 = (s32)(elements(i, 2));
      f64 v2 = (s32)(elements(i, 3));

      if (type == 1) {  // Constraint is a DOF constraint
        s32 sv1 = (s32)v1;
        s32 sv2 = (s32)v2;
        if ((sv1 != 0 && sv1 != 1) || (sv2 != 0 && sv2 != 1))
          throw std::runtime_error("Incorrect value for DOF constraint. Must be 0 or 1.");
        m_nodes[nid].bcs.dof(0) = sv1;
        m_nodes[nid].bcs.dof(1) = sv2;
      }
      else if (type == 2) { // Constraint is a force
        m_nodes[nid].bcs.forces(0) = v1;
        m_nodes[nid].bcs.forces(1) = v2;
      }
      else {
        throw std::runtime_error("Invalid boundary condition type");
      }
    }


  }

  TrussFEMProblem::TrussFEMProblem(const char * nodesPath, const char * elementsPath, const char * materialsPath,
    const char *boundaryConditionsPath, const char *outFileName)
  {
    m_outFileName = outFileName;

    LoadData(nodesPath, elementsPath, materialsPath, boundaryConditionsPath);
    
    PopulateStiffnessMatrix();
    PopulateForceVector();
    ApplyPenaltyMethod();

    Solve();
  }

  // Global Stiffness Matrix
  void TrussFEMProblem::PopulateStiffnessMatrix()
  {
    LinkElement e; f64 A, E, l, k, theta;
    mat22 localStiffness;
    mat44 globalStiffness = zeros(4,2);
    vec2 delta;
    Node ns[2];
    s32 oneToFourToNodeIds[4] = { 0,0,1,1 };
    s32 oneToFourPlusOnes[4] = { 0,1,0,1 };

    m_K = zeros(2*m_nodes.size(), 2*m_nodes.size());

    for (std::map<s32, LinkElement>::iterator i = m_elements.begin(); i != m_elements.end(); i++) {
      e = i->second;
      if (m_materials.find(e.mId) == m_materials.end())
        throw std::runtime_error(std::string("Error: invalid material id ") + std::to_string(e.mId) + " for element " + std::to_string(i->first));
      A = m_materials[e.mId].A;
      E = m_materials[e.mId].E;

      ns[0] = m_nodes[e.nodeIds(0)]; ns[1] = m_nodes[e.nodeIds(1)];

      delta = ns[0].pos - ns[1].pos;
      l = norm(delta);
      k = A*E / l;
      theta = atan2(delta(1), delta(0));

      localStiffness(0, 0) = k; localStiffness(0, 1) = -k;
      localStiffness(1, 0) = -k; localStiffness(1, 1) = k;

      globalStiffness = zeros(4, 2);
      globalStiffness(0, 0) = cos(theta); globalStiffness(1, 0) = sin(theta);
      globalStiffness(2, 1) = globalStiffness(0, 0); globalStiffness(3, 1) = globalStiffness(1, 0);
      globalStiffness = globalStiffness*localStiffness*globalStiffness.t();


      s32 nnr, nnc;
      for (s32 rr = 0; rr < 4; rr++) {
        for (s32 cc = 0; cc < 4; cc++) {
          nnr= 2 * ns[2 * oneToFourToNodeIds[rr]].incId + oneToFourPlusOnes[rr];
          nnc = 2 * ns[2 * oneToFourToNodeIds[cc]].incId + oneToFourPlusOnes[cc];
          m_K(nnr, nnc) += globalStiffness(rr, cc);
        }
      }
    }
  }

  void TrussFEMProblem::PopulateForceVector()
  {
    m_F = zeros(m_nodes.size(), 1);
    // For each node..
    Node n;
    for (std::map<s32, Node>::iterator i = m_nodes.begin(); i != m_nodes.end(); i++) {
      n = i->second;
      m_F(span(2 * n.incId, 2 * n.incId + 1)) = n.bcs.forces;
    }
  }

  void TrussFEMProblem::ApplyPenaltyMethod()
  {
    // For each node, apply the penalty method if that node's x or y degree of freedom is constrained.
    Node n;
    for (std::map<s32, Node>::iterator i = m_nodes.begin(); i != m_nodes.end(); i++) {
      n = i->second;
      if (n.bcs.dof(0) == 1)
        m_K(2 * n.incId, 2 * n.incId) += KSTAR;
      if (n.bcs.dof(1) == 1)
        m_K(2 * n.incId + 1, 2 * n.incId + 1) += KSTAR;
    }
  }

  void TrussFEMProblem::Solve()
  {
    // Check that the stiffness matrix is of the appropriate size
    if (m_K.n_cols != m_nodes.size() || m_K.n_rows != m_nodes.size())
      throw std::runtime_error("Stiffness matrix dimensions do not match the number of nodes.");

    if (m_F.n_rows != m_nodes.size())
      throw std::runtime_error("Force vector dimensions do not match the number of nodes.");

    // Check that the condition number is not too high
    if (cond(m_K) >= MAX_COND)
      throw std::runtime_error("Stiffness matrix is ill-conditioned for solving (condition number = " + std::to_string(cond(m_K)) + ". Verify there are enough constraints.");

    //Solve Ku = F for u
    m_u = inv(m_K)*m_F;
  }

  void TrussFEMProblem::PostProcessStressesAndStrains()
  {
    LinkElement e; vec2 n1xy, n2xy, delta; vec2 us;
    f64 l;
    // For each element. Find the strains based on the displacement and then translate into stress
    for (std::map < s32, LinkElement >::iterator i = m_elements.begin(); i != m_elements.end(); i++)
    {
      e = i->second;
      delta = m_nodes[e.nodeIds(1)].pos - m_nodes[e.nodeIds(0)].pos;
      l = norm(delta);
      delta = delta / l;

      n1xy = m_u.rows(span(2 * m_nodes[e.nodeIds(0)].incId, 2 * m_nodes[e.nodeIds(0)].incId + 1));
      n2xy = m_u.rows(span(2 * m_nodes[e.nodeIds(1)].incId, 2 * m_nodes[e.nodeIds(1)].incId + 1));

      // dot into the element's coordinate system...
      us(0) = dot(delta, n1xy);
      us(1) = dot(delta, n2xy);
      i->second.results.epsilon_xx = (us(1) - us(0)) / l;
      i->second.results.sigma_xx = m_materials[e.mId].E*i->second.results.epsilon_xx;
      i->second.results.valid = true;

      for (s32 jj = 0; jj < 2; jj++) {
        m_nodes[i->second.nodeIds(jj)].results.epsilon_xx += i->second.results.epsilon_xx;
        m_nodes[i->second.nodeIds(jj)].results.sigma_xx += i->second.results.sigma_xx;
        m_nodes[i->second.nodeIds(jj)].results.nel++;
      }
    }

    // Now calculate averages for each node
    for (std::map<s32, Node>::iterator i = m_nodes.begin(); i != m_nodes.end(); i++) {
      if (i->second.results.nel == 0)
        continue;
      i->second.results.epsilon_xx = i->second.results.epsilon_xx / i->second.results.nel;
      i->second.results.sigma_xx = i->second.results.sigma_xx / i->second.results.nel;
      i->second.results.valid = true;
    }
  }

  void TrussFEMProblem::PrintAndSaveResults()
  {
    arma::mat element_results, node_results;

    element_results = zeros(m_elements.size(), 3);
    node_results = zeros(m_nodes.size(), 3);

    printf("######################################################\n");
    printf("Element results\n");
    printf("######################################################\n");
    s32 ii = 0;
    for (std::map < s32, LinkElement >::iterator i = m_elements.begin(); i != m_elements.end(); i++)
    {
      if (!i->second.results.valid)
        continue;
      element_results(ii, 0) = i->first;
      element_results(ii, 1) = i->second.results.epsilon_xx;
      element_results(ii, 2) = i->second.results.sigma_xx;
      printf("Element Id = %d, Strain = %f Stress = %f\n", element_results(ii, 0), element_results(ii, 1), element_results(ii, 2));
      ii++;
    }
    element_results.save(m_outFileName + "_element_results.txt", raw_ascii);
    printf("######################################################\n");
    printf("End Element results\n");
    printf("######################################################\n");

    printf("######################################################\n");
    printf("Node results\n");
    printf("######################################################\n");
    ii = 0;
    for (std::map < s32, Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); i++)
    {
      if (!i->second.results.valid)
        continue;
      node_results(ii, 0) = i->first;
      node_results(ii, 1) = i->second.results.epsilon_xx;
      node_results(ii, 2) = i->second.results.sigma_xx;
      printf("Node Id = %d, Strain = %f, Stress = %f, ux = %f, uy = %f\n", node_results(ii, 0), node_results(ii, 1), 
        node_results(ii, 2), m_u(2*i->second.incId), m_u(2*i->second.incId+1));
      ii++;
    }
    node_results.save(m_outFileName + "_node_results.txt", raw_ascii);
    printf("######################################################\n");
    printf("End Node results\n");
    printf("######################################################\n");
  }
}