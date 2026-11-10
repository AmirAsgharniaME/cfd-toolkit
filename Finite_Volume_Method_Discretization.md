# Dimension-Independent Finite Volume Method (FVM) Discretization for Diffusion

This document establishes the mathematical derivation and systematic, dimension-independent algorithmic framework for discretizing the steady-state diffusion equation using the Finite Volume Method (FVM).

## 1. Governing Equation & Control Volume Integration

We consider the steady-state diffusion equation with a volumetric source term:
$$-\nabla \cdot (\Gamma \nabla \phi) = S$$

where:
- $\phi$: scalar transported variable
- $\Gamma$: diffusion coefficient
- $S$: volumetric source term

Integrating over an arbitrary control volume $V_P$:
$$\int_{V_P} -\nabla \cdot (\Gamma \nabla \phi) \, dV = \int_{V_P} S \, dV$$

## 2. Gauss Divergence Theorem Application

Defining the diffusive flux vector as $\mathbf{F} = \Gamma \nabla \phi$, we apply Gauss's Divergence Theorem:
$$-\oint_{\partial V_P} (\Gamma \nabla \phi) \cdot \hat{\mathbf{n}} \, dA = \bar{S}_P V_P$$

where:
- $\partial V_P$: closed boundary enclosing control volume $V_P$
- $\hat{\mathbf{n}}$: outward-pointing unit normal vector
- $\bar{S}_P$: volume-averaged source term

## 3. Discrete Face Representation & Face Normal Convention

Approximating the surface integral as a summation over faces $f$:
$$-\sum_{f \in \mathcal{F}(P)} (\Gamma_f \nabla \phi|_f \cdot \hat{\mathbf{n}}_f) A_f = \bar{S}_P V_P$$

Using the face orientation factor $s_f$:
- $s_f = +1$ if outward normal points from $P$ to Neighbor $N(f)$
- $s_f = -1$ if outward normal points from Neighbor $N(f)$ to $P$

Gradient approximation:
$$\nabla \phi|_f \cdot \hat{\mathbf{n}}_f \approx s_f \left( \frac{\phi_{N(f)} - \phi_P}{d_{P,N(f)}} \right)$$

Substituting yields:
$$-\sum_{f \in \mathcal{F}(P)} \Gamma_f A_f s_f \left( \frac{\phi_{N(f)} - \phi_P}{d_{P,N(f)}} \right) = \bar{S}_P V_P$$

## 4. Dimension-Independent Matrix Assembly

Define conductance $D_f = \frac{\Gamma_f A_f}{d_{P,N(f)}}$.
The contribution per face is:
$$-D_f s_f (\phi_{N(f)} - \phi_P) = (D_f s_f) \phi_P - (D_f s_f) \phi_{N(f)}$$

**Assembly Rules for every face $f$:**
1. Diagonal entry ($A_{P,P}$): $A_{P,P} += D_f s_f$
2. Off-diagonal entry ($A_{P,N(f)}$): $A_{P,N(f)} -= D_f s_f$
3. Right-Hand Side ($b_P$): $b_P = \bar{S}_P V_P$

## 5. Verification with 1D East/West Formulation

Matching Patankar's standard formula:
$$a_P \phi_P = a_W \phi_W + a_E \phi_E + b$$
where $a_W = D_w$ and $a_E = D_e$. The signs align correctly with the $s_f$ convention.

## 6. C++ Implementation Blueprint
```cpp
#include <vector>
#include <cmath>

struct Face {
int neighbor_id;       // Index of neighbor cell N(f)
double area;           // Face surface area A_f
double gamma;          // Diffusion coefficient at face
double distance;       // Distance d_{P, N(f)}
double normal_sign;    // Orientation factor s_f (+1.0 or -1.0)
};

struct Cell {
int id;
double volume;
double source;
std::vector<Face> faces;
};

// Global Matrix Assembly Loop
void assemble_diffusion_matrix(
const std::vector<Cell>& cells,
std::vector<double>& diag,
std::vector<std::vector<std::pair<int, double>>>& off_diag,
std::vector<double>& b) 
{
const size_t n_cells = cells.size();
diag.assign(n_cells, 0.0);
off_diag.resize(n_cells);
b.assign(n_cells, 0.0);

for (size_t P = 0; P < n_cells; ++P) {
b[P] = cells[P].source * cells[P].volume;

for (const auto& face : cells[P].faces) {
int N = face.neighbor_id;
double D_f = (face.gamma * face.area) / face.distance;
double coeff = D_f * face.normal_sign;

// Diagonal entry update
diag[P] += coeff;

// Off-diagonal entry update
off_diag[P].emplace_back(N, -coeff);
}
}
}
