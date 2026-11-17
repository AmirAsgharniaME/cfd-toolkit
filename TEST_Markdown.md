# Physical & Mathematical Derivation of Robin (Convection) Boundary Condition in FVM

## 1. Executive Summary & Core Principle

In Finite Volume Method (FVM), all surface flux balances are strictly evaluated using the **outward unit normal** ($\mathbf{n}_{\mathrm{out}}$).

- **Newton's Law of Cooling is a directional, surface-local phenomenon**, not a global Cartesian field. Convection physically carries thermal energy away from the boundary into the adjacent fluid layer.
- The **outward scalar convective flux** ($q_{\mathrm{conv,out}}$) leaving the computational domain is **universally** defined as:

$$
q_{\mathrm{conv,out}} = h (\phi_b - \phi_\infty) \qquad (1)
$$

- In Eq. (1), $h$ is the transfer coefficient, $\phi_b$ is the boundary value, and $\phi_\infty$ is the ambient value. The relation holds regardless of whether the boundary is at the left ($x=0$), right ($x=L$), or on an arbitrarily oriented 3D surface.
- Equating this directly to the **outward diffusive flux** from Fick's / Fourier's law:

$$
q_{\mathrm{diff,out}} = -\Gamma \nabla\phi \cdot \mathbf{n}_{\mathrm{out}} = -\Gamma \frac{\partial \phi}{\partial n} \qquad (2)
$$

- This yields the coordinate-free Robin boundary interface condition:

$$
-\Gamma \nabla\phi \cdot \mathbf{n}_{\mathrm{out}} = h (\phi_b - \phi_\infty) \qquad (3)
$$

Here, $\Gamma$ is the diffusivity, and Eq. (3) equates the outward fluxes in Eq. (1) and Eq. (2).

## 2. The Vector Paradox: Why Global Cartesian Flux Fails

A common confusion arises when attempting to assign a fixed Cartesian vector direction (e.g., $+\mathbf{i}$) to the convective heat flux on all boundaries:

$$
\mathbf{q}_{\mathrm{conv}} \stackrel{?}{=} h (\phi_b - \phi_\infty) \mathbf{i} \qquad (4)
$$

Projecting this onto outward face normals yields contradictory signs between left and right boundaries:

- Right boundary ($\mathbf{n}_{\mathrm{out}} = +\mathbf{i}$):

  $$
  \mathbf{q}_{\mathrm{conv}} \cdot \mathbf{n}_{\mathrm{out}} = +h (\phi_b - \phi_\infty) \qquad (5)
  $$

- Left boundary ($\mathbf{n}_{\mathrm{out}} = -\mathbf{i}$):

  $$
  \mathbf{q}_{\mathrm{conv}} \cdot \mathbf{n}_{\mathrm{out}} = -h (\phi_b - \phi_\infty) \quad \text{(Physically incorrect for outward flux)} \qquad (6)
  $$

### Physical Resolution via Boundary Layer Conduction
Convection is microscopic conduction across the adjacent fluid boundary layer of thickness $\delta_f$ and conductivity $k_f$, with $h = k_f / \delta_f$ in this approximation:

$$
\mathbf{q} = -k_f \nabla T \qquad (7)
$$

1. **At the Right Boundary** ($x = L$, fluid is at $x > L$):
   - The temperature in the fluid drops from $T_b$ to $T_\infty$ along $+x$.
   - Spatial gradient in fluid:

     $$
     \left.\frac{\partial T}{\partial x}\right\vert_{\mathrm{fluid}} \approx \frac{T_\infty - T_b}{\delta_f} < 0 \qquad (8)
     $$

     (if $T_b > T_\infty$).
   - Local Cartesian flux vector:

     $$
     \mathbf{q}_{\mathrm{conv,right}} = -k_f \left(\frac{T_\infty - T_b}{\delta_f}\right)\mathbf{i} = +h(T_b - T_\infty)\mathbf{i} \qquad (9)
     $$

2. **At the Left Boundary** ($x = 0$, fluid is at $x < 0$):
   - The temperature drops from $T_b$ to $T_\infty$ along $-x$.
   - Spatial gradient in fluid:

     $$
     \left.\frac{\partial T}{\partial x}\right\vert_{\mathrm{fluid}} \approx \frac{T_b - T_\infty}{\delta_f} > 0 \qquad (10)
     $$

     (if $T_b > T_\infty$).
   - Local Cartesian flux vector:

     $$
     \mathbf{q}_{\mathrm{conv,left}} = -k_f \left(\frac{T_b - T_\infty}{\delta_f}\right)\mathbf{i} = -h(T_b - T_\infty)\mathbf{i} \qquad (11)
     $$

Projecting both true local vectors onto their respective outward normals ($\mathbf{n}_{\mathrm{out}}$):

- **Left Face:**

  $$
  \mathbf{q}_{\mathrm{conv,left}} \cdot (-\mathbf{i}) = \left[-h(T_b - T_\infty)\mathbf{i}\right] \cdot (-\mathbf{i}) = +h(T_b - T_\infty) \qquad (12)
  $$

- **Right Face:**

  $$
  \mathbf{q}_{\mathrm{conv,right}} \cdot (+\mathbf{i}) = \left[+h(T_b - T_\infty)\mathbf{i}\right] \cdot (+\mathbf{i}) = +h(T_b - T_\infty) \qquad (13)
  $$



## 3. Finite Volume Formulation & Matrix Assembly

Starting from the steady diffusion PDE:

$$
-\nabla \cdot (\Gamma \nabla \phi) = S \qquad (14)
$$

Integrating over control volume $V_P$ and applying the Divergence Theorem:

$$
\sum_f \int_{A_f} \left(-\Gamma \nabla\phi \cdot \mathbf{n}_{\mathrm{out}}\right)\,\mathrm{d}A = \int_{V_P} S\,\mathrm{d}V \qquad (15)
$$

Let $f = b$ be an exterior boundary face with area $A_b$. The outward integrated diffusive flux leaving face $b$ is:

$$
Q_{\mathrm{diff,out}} = q_{\mathrm{diff,out}} A_b = \left(-\Gamma \nabla\phi \cdot \mathbf{n}_{\mathrm{out}}\right) A_b \qquad (16)
$$

By flux continuity at the physical interface:

$$
Q_{\mathrm{diff,out}} = Q_{\mathrm{conv,out}} = h A_b (\phi_b - \phi_\infty) \qquad (17)
$$

### First-Order Closure ($\phi_b \approx \phi_P$)
Approximating the boundary face scalar with the cell-center scalar:

$$
Q_{\mathrm{out}, b} \approx h A_b \phi_P - h A_b \phi_\infty \qquad (18)
$$

Substituting Eq. (18) into the algebraic equation:

$$
a_P \phi_P - \sum_N a_N \phi_N = b_P \qquad (19)
$$

- The implicit term containing unknown $\phi_P$ stays on the LHS:

  $$
  a_P \mathrel{+}= h A_b \qquad (20)
  $$

- The known ambient term moves to the RHS source vector:

  $$
  b_P \mathrel{+}= h A_b \phi_\infty \qquad (21)
  $$

### Conclusion

- $a_P$ **contribution:** Strictly $+h A_b$ for both `Start` and `End`.
- $b_P$ **contribution:** Strictly $+h A_b \phi_\infty$ for both `Start` and `End`.
- **Off-diagonal coefficients:** Completely untouched ($a_N$ unaffected).
