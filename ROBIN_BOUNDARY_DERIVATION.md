# Physical & Mathematical Derivation of Robin (Convection) Boundary Condition in FVM

## 1. Executive Summary & Core Principle

In Finite Volume Method (FVM), all surface flux balances are strictly evaluated using the **outward unit normal** ($\mathbf{n}_{\text{out}}$).

- **Newton's Law of Cooling is a directional, surface-local phenomenon**, not a global Cartesian field. Convection physically carries thermal energy away from the boundary into the adjacent fluid layer.
- The **outward scalar convective flux** ($q_{\text{conv, out}}$) leaving the computational domain is **universally** defined as:
  $$q_{\text{conv, out}} = h (\phi_b - \phi_\infty)$$
  regardless of whether the boundary is at the left ($x=0$), right ($x=L$), or on an arbitrarily oriented 3D surface.
- Equating this directly to the **outward diffusive flux** from Fick's / Fourier's law:
  $$q_{\text{diff, out}} = -\Gamma \nabla\phi \cdot \mathbf{n}_{\text{out}} = -\Gamma \frac{\partial \phi}{\partial n}$$
  yields the coordinate-free Robin boundary interface condition:
  $$-\Gamma \nabla\phi \cdot \mathbf{n}_{\text{out}} = h (\phi_b - \phi_\infty)$$

---

## 2. The Vector Paradox: Why Global Cartesian Flux Fails

A common confusion arises when attempting to assign a fixed Cartesian vector direction (e.g., $+\mathbf{i}$) to the convective heat flux on all boundaries:
$$\mathbf{q}_{\text{conv}} \stackrel{?}{=} h (\phi_b - \phi_\infty) \mathbf{i}$$

Projecting this onto outward face normals yields contradictory signs between left and right boundaries:
- Right boundary ($\mathbf{n}_{\text{out}} = +\mathbf{i}$):
  $$\mathbf{q}_{\text{conv}} \cdot \mathbf{n}_{\text{out}} = +h (\phi_b - \phi_\infty)$$
- Left boundary ($\mathbf{n}_{\text{out}} = -\mathbf{i}$):
  $$\mathbf{q}_{\text{conv}} \cdot \mathbf{n}_{\text{out}} = -h (\phi_b - \phi_\infty) \quad \text{(Physically incorrect for outward flux)}$$

### Physical Resolution via Boundary Layer Conduction
Convection is microscopic conduction across the adjacent fluid boundary layer of thickness $\delta_f$:

$$\mathbf{q} = -k_f \nabla T$$

1. **At the Right Boundary ($x = L$, fluid is at $x > L$):**
   - The temperature in the fluid drops from $T_b$ to $T_\infty$ along $+x$.
   - Spatial gradient in fluid: $\left.\frac{\partial T}{\partial x}\right|_{\text{fluid}} \approx \frac{T_\infty - T_b}{\delta_f} < 0$ (if $T_b > T_\infty$).
   - Local Cartesian flux vector:
     $$\mathbf{q}_{\text{conv, right}} = -k_f \left(\frac{T_\infty - T_b}{\delta_f}\right)\mathbf{i} = +h(T_b - T_\infty)\mathbf{i}$$

2. **At the Left Boundary ($x = 0$, fluid is at $x < 0$):**
   - The temperature drops from $T_b$ to $T_\infty$ along $-x$.
   - Spatial gradient in fluid: $\left.\frac{\partial T}{\partial x}\right|_{\text{fluid}} \approx \frac{T_b - T_\infty}{\delta_f} > 0$ (if $T_b > T_\infty$).
   - Local Cartesian flux vector:
     $$\mathbf{q}_{\text{conv, left}} = -k_f \left(\frac{T_b - T_\infty}{\delta_f}\right)\mathbf{i} = -h(T_b - T_\infty)\mathbf{i}$$

Projecting both true local vectors onto their respective outward normals ($\mathbf{n}_{\text{out}}$):
- **Left Face:** $\mathbf{q}_{\text{conv, left}} \cdot (-\mathbf{i}) = \left[-h(T_b - T_\infty)\mathbf{i}\right] \cdot (-\mathbf{i}) = +h(T_b - T_\infty)$
- **Right Face:** $\mathbf{q}_{\text{conv, right}} \cdot (+\mathbf{i}) = \left[+h(T_b - T_\infty)\mathbf{i}\right] \cdot (+\mathbf{i}) = +h(T_b - T_\infty)$

---

## 3. Finite Volume Formulation & Matrix Assembly

Starting from the steady diffusion PDE:
$$-\nabla \cdot (\Gamma \nabla \phi) = S$$

Integrating over control volume $V_P$ and applying the Divergence Theorem:
$$\sum_{f} \int_{A_f} \left(-\Gamma \nabla\phi \cdot \mathbf{n}_{\text{out}}\right) dA = \int_{V_P} S \, dV$$

Let $f = b$ be an exterior boundary face with area $A_b$. The outward integrated diffusive flux leaving face $b$ is:
$$Q_{\text{diff, out}} = q_{\text{diff, out}} A_b = \left(-\Gamma \nabla\phi \cdot \mathbf{n}_{\text{out}}\right) A_b$$

By flux continuity at the physical interface:
$$Q_{\text{diff, out}} = Q_{\text{conv, out}} = h A_b (\phi_b - \phi_\infty)$$

### First-Order Closure ($\phi_b \approx \phi_P$)
Approximating the boundary face scalar with the cell-center scalar:
$$Q_{\text{out}, b} \approx h A_b \phi_P - h A_b \phi_\infty$$

Substituting into the algebraic equation $a_P \phi_P - \sum_N a_N \phi_N = b_P$:
- The implicit term containing unknown $\phi_P$ stays on the LHS:
  $$a_P \mathrel{+}= h A_b$$
- The known ambient term moves to the RHS source vector:
  $$b_P \mathrel{+}= h A_b \phi_\infty$$

### Conclusion
- **$a_P$ contribution:** Strictly $+h A_b$ for both `Start` and `End`.
- **$b_P$ contribution:** Strictly $+h A_b \phi_\infty$ for both `Start` and `End`.
- **Off-diagonal coefficients:** Completely untouched ($a_N$ unaffected).

