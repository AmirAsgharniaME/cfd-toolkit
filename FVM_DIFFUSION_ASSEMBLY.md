# Finite-Volume Diffusion: From Conservation Law to Face-Based Assembly

> A working reference for the cell-centered diffusion operator in `cfd-toolkit`. The goal is to understand *what each matrix entry means*, not to memorize an east/west stencil. This document assumes scalar, isotropic diffusivity and an orthogonal mesh for its simple two-point flux approximation; the conservation and assembly logic itself is more general.

## 1. Start from a transport equation

For a scalar field $\phi$, use this conservative sign convention:

$$
\underbrace{\frac{\partial(\rho\phi)}{\partial t}}_{\text{transient}}
+\underbrace{\nabla\cdot(\rho\mathbf{u}\phi)}_{\text{convection}}
=\underbrace{\nabla\cdot(\Gamma\nabla\phi)}_{\text{diffusion}}
+\underbrace{S}_{\text{volumetric source}}.
$$

Here $S>0$ generates the transported quantity. Move diffusion to the left:

$$
\frac{\partial(\rho\phi)}{\partial t}
+\nabla\cdot(\rho\mathbf{u}\phi)
-\nabla\cdot(\Gamma\nabla\phi)=S.
$$

Define the physical diffusive flux vector $\mathbf{J}_d=-\Gamma\nabla\phi$. Then the spatial part can be written $\nabla\cdot(\rho\mathbf{u}\phi+\mathbf{J}_d)$. **The minus sign belongs to the physical diffusion law**: diffusion points down the gradient. It is not an owner/neighbour sign convention.

For now, drop convection and assume steady state, so the transient term also vanishes:

$$
-\nabla\cdot(\Gamma\nabla\phi)=S
\qquad\Longleftrightarrow\qquad
\nabla\cdot\mathbf{J}_d=S.
$$

If there is no source, set $S=0$; do not silently assume this for the general derivation.

## 2. Integrate over one control volume

For a cell $P$ with control volume $V_P$ and outward unit normal $\mathbf{n}_{P,\mathrm{out}}$, integrate and apply the divergence theorem:

$$
\int_{V_P}\nabla\cdot\mathbf{J}_d\,dV
=\oint_{\partial V_P}\mathbf{J}_d\cdot\mathbf{n}_{P,\mathrm{out}}\,dA
=\int_{V_P}S\,dV.
$$

With the outward area vector

$$
\mathbf{S}_{P,f} = \mathbf{n}_{P,f,\mathrm{out}} A_f,
$$

the finite-volume balance becomes


$$
\sum_{f\in\partial P}F_{P,f}=b_P,
\qquad F_{P,f}\approx\mathbf{J}_{d,f}\cdot\mathbf{S}_{P,f},
\qquad b_P=\int_{V_P}S\,dV.
$$

**Interpretation:** each $F_{P,f}$ is a *signed outward* flux. Positive means leaving $P$; negative means entering $P$. The sum is **net** outward flux, not a sum of positive magnitudes. An inward flux already carries its minus sign through the dot product. With a positive source, net outward flux equals production.

Strictly, divergence is *net outward flux per unit volume* in the infinitesimal limit. The finite-volume balance above instead integrates that divergence over an actual finite cell; there is no need to take a pointwise limit when assembling its equation.

## 3. One internal face, one physical flux, two cell perspectives

Consider an internal face $f$ shared by owner $O$ and neighbour $N$. Define the **stored** face area vector once:

$$
\mathbf{S}_f=A_f\mathbf{n}_f,\qquad\mathbf{n}_f:\ O\longrightarrow N.
$$

This direction is a mesh-storage convention, not a statement that the physical flux actually travels from $O$ to $N$. The face is outward-facing for $O$, but inward-facing for $N$:

$$
\mathbf{S}_{O,f}=+\mathbf{S}_f,\qquad
\mathbf{S}_{N,f}=-\mathbf{S}_f.
$$

Evaluate the face flux once with the stored orientation:

$$
F_f=\mathbf{J}_{d,f}\cdot\mathbf{S}_f.
$$

The two *outward* contributions are therefore

$$
F_{O,f}=+F_f,\qquad
F_{N,f}=-F_f.
$$

If $F_f<0$, the physical flux goes from $N$ to $O$. Nothing in the orientation convention must change. **The sign reversal for the neighbour comes from its opposite outward normal; the physical flux vector is the same on both sides of the interface.** This is the core of local conservation.

## 4. Approximate the diffusive face flux

On an orthogonal mesh, let $d_{ON}>0$ be the cell-center distance along the face normal, and let $\mathbf{e}_{ON}$ point from $O$ to $N$. With scalar $\Gamma_f>0$:

$$
\nabla\phi\cdot\mathbf{n}_f
\approx\frac{\phi_N-\phi_O}{d_{ON}},
\qquad
F_f\approx-\Gamma_f A_f\frac{\phi_N-\phi_O}{d_{ON}}.
$$

Define the positive face conductance

$$
D_f=\frac{\Gamma_f A_f}{d_{ON}}>0,
\qquad F_f=D_f(\phi_O-\phi_N).
$$

The vector picture is approximated as:

$$
\nabla\phi \approx \frac{\phi_N - \phi_O}{d_{ON}} \mathbf{e}_{ON}
$$

followed by the face flux $\mathbf{J}_d \cdot \mathbf{S}_f$. On an orthogonal face, $\mathbf{e}_{ON} \parallel \mathbf{S}_f$, so $\mathbf{e}_{ON} \cdot \mathbf{S}_f = A_f$. If $\phi_O > \phi_N$, then $F_f > 0$: the owner loses the transported quantity through that face.





In a 1D mesh, $d_{ON}=d_{Of}+d_{fN}$ when the face lies between the two cell centers. For piecewise-constant diffusivities on the two half-distances, resolve the two resistances in series rather than blindly interpolating $\Gamma$:

$$
D_f=\frac{A_f}{d_{Of}/\Gamma_O+d_{fN}/\Gamma_N}.
$$

This reduces to $\Gamma A_f/d_{ON}$ for uniform $\Gamma$. On non-orthogonal or highly skewed meshes, the two-point normal-gradient formula needs an appropriate geometric treatment and possibly a non-orthogonal correction; **do not mistake this simple flux formula for a universal unstructured-mesh discretization**. The owner/neighbour *flux-sign and scatter* rule still applies.

## 5. Cell-based assembly: reason from one cell row

Take a cell $P$. For each internal face touching it, let $Q$ be the cell across that face. Because we always want $P$'s outward flux, the two-point contribution is

$$
F_{P,f}=D_f(\phi_P-\phi_Q).
$$

This is true whether $P$ is the stored owner or the stored neighbour: when $P=N$, it follows from $-F_f=D_f(\phi_N-\phi_O)$. Summing the internal-face contributions gives

$$
\sum_{f\in\partial P\,\mathrm{internal}}D_f(\phi_P-\phi_{Q(f)})
+\sum_{f\in\partial P\,\mathrm{boundary}}F_{P,f}^{\mathrm{boundary}}=b_P.
$$

For a cell with **only internal faces** and prescribed source $b_P$, its matrix row is

$$
\left(\sum_fD_f\right)\phi_P
+\sum_f(-D_f)\phi_{Q(f)}=b_P.
$$

For a 1D interior cell with west/east neighbours $W,E$, this is

$$
(D_w+D_e)\phi_P-D_w\phi_W-D_e\phi_E=b_P.
$$

A literal cell-based implementation loops over every cell, then over its faces, and updates **that cell's row**. At an internal face it will encounter the face once from $O$ and again from $N$; both rows are necessary. If an implementation loops only over interior cells and skips the first/last cell, those boundary-cell rows miss their *internal-face* contributions. Boundary conditions cannot reconstruct that missing interior coupling.

## 6. Face-based assembly: compute once, scatter to both rows

Instead, loop over each internal face exactly once. The already-derived flux $F_f=D_f(\phi_O-\phi_N)$ contributes to the two balance equations as follows:

| Row | Outward contribution | Diagonal update | Off-diagonal update |
| --- | --- | --- | --- |
| $O$ | $+D_f\phi_O-D_f\phi_N$ | $A_{OO}\mathrel{+}=D_f$ | $A_{ON}\mathrel{+}=-D_f$ |
| $N$ | $-D_f\phi_O+D_f\phi_N$ | $A_{NN}\mathrel{+}=D_f$ | $A_{NO}\mathrel{+}=-D_f$ |

Equivalently, insert the local block

$$
A^{(f)}=D_f
\begin{bmatrix}1&-1\\-1&1\end{bmatrix}
\quad\text{on rows/columns }(O,N).
$$

The neighbour's entire face flux is the negative of the owner's. Nevertheless, **both diagonal entries are positive and both off-diagonal entries are negative**: negating $D_f(\phi_O-\phi_N)$ gives $D_f(\phi_N-\phi_O)$. Do not negate only one coefficient or confuse a signed flux with the positive conductance $D_f$.

For the diffusion term on a suitable orthogonal mesh, dimension-independent assembly can be expressed as:

```cpp
for (FaceId f : mesh.internal_faces()) {
    const CellId owner = mesh.owner(f);
    const CellId neighbour = mesh.neighbour(f);
    const double D = diffusion_conductance(mesh, gamma, f);

    matrix.Add_a_P(owner, D);
    matrix.Add_OffDiagonal(owner, neighbour, -D);
    matrix.Add_a_P(neighbour, D);
    matrix.Add_OffDiagonal(neighbour, owner, -D);
}

for (CellId p : mesh.cells()) {
    rhs[p] += integrated_source(p);
}

// Add boundary-face contributions to existing cell rows afterward.
assemble_boundary_conditions(mesh, matrix, rhs);
```

`diffusion_conductance` encapsulates the selected geometry and interpolation scheme. Keep 1D-specific mesh construction and face indexing in 1D mesh code, not in the generic face loop. An internal face needs an owner and a neighbour; a boundary face has only an owner and is handled separately. If the matrix has one unknown per cell, its dimensions are exactly $N_{\mathrm{cells}}\times N_{\mathrm{cells}}$. No extra boundary unknown or boundary row is introduced by this treatment.

### A three-cell example

With internal faces $(0,1)$ and $(1,2)$ and conductances $D_{01},D_{12}$, before sources and boundary conditions:

$$
A_{\mathrm{internal}}=
\begin{bmatrix}
D_{01}&-D_{01}&0\\
-D_{01}&D_{01}+D_{12}&-D_{12}\\
0&-D_{12}&D_{12}
\end{bmatrix}.
$$

Notice that **both boundary cells already have an internal-face diagonal and off-diagonal**. Boundary conditions add to those rows; they do not replace them. For internal-face assembly, each row sums to zero because a spatially constant $\phi$ has zero internal diffusive flux.

## 7. Boundary fluxes: add to the existing owner row

Boundary faces have an outward area vector defined relative to their **only** adjacent cell $P$. Apply the boundary condition using that cell's outward-flux convention. With a prescribed value $\phi_b$ at a boundary face a distance $d_{Pb}$ from the cell center, the simple orthogonal approximation gives

$$
F_{P,b}=D_b(\phi_P-\phi_b),\qquad
D_b=\frac{\Gamma_b A_b}{d_{Pb}}.
$$

Since $F_{P,b}$ belongs on the **left-hand side** of $\sum F=b_P$, its contribution is

$$
A_{PP}\mathrel{+}=D_b,\qquad b_P\mathrel{+}=D_b\phi_b.
$$

For a prescribed **outward integrated diffusive flux** defined as:

$$
q_b = \int_{b} \mathbf{J}_d \cdot \mathbf{n}_{\mathrm{out}} \, dA
$$

write $\sum F_{\mathrm{internal}} + q_b = b_P$, giving $b_P \mathrel{+}= -q_b$. A positive outward flux therefore reduces the RHS in this convention; an insulated face has $q_b = 0$. If a boundary API instead prescribes a *flux density* or a *gradient*, convert it to the integrated outward flux first and check the sign. Source linearization and other boundary types can add further diagonal/RHS terms; they do not change the internal-face rule.


A pure zero-flux/Neumann diffusion problem with no anchoring value leaves a constant nullspace: the absolute level of $\phi$ is undetermined, and the prescribed sources and boundary fluxes must satisfy global compatibility. A Dirichlet condition (or another valid reference constraint) can remove that ambiguity.

## 8. Cell-based versus face-based: what really changes?

**Nothing about the physical equation changes.** Both approaches assemble the same cell balances when they use the same face-flux approximation and boundary treatment. Only the order in which contributions reach the matrix changes:

- **Cell-based:** visit a cell; visit its faces; add each outward contribution to that cell's row. An internal face is considered from both cells' perspectives.
- **Face-based:** visit an internal face; evaluate its geometry/conductance once; add $+F_f$ to the owner row and $-F_f$ to the neighbour row immediately.

Using *one shared discrete internal-face flux* with opposite signs makes internal contributions cancel exactly in the **discrete conservation balance** when all cell equations are summed. It does not, by itself, guarantee that the computed solution is accurate on an arbitrary mesh: accuracy depends on the flux approximation and mesh quality. In floating-point arithmetic, do not promise bitwise-zero global sums after independently accumulated operations.

**One-sentence mental model:** The divergence theorem asks for the *net outward flux of each cell*; a face-based loop computes one oriented physical face flux and immediately deposits that flux with opposite signs into the two cells that share the face.

## 9. Implementation checklist

- Keep $\mathbf{J}_d=-\Gamma\nabla\phi$ separate from your face-orientation convention.
- Define one stable owner-to-neighbour orientation per internal face; derive cell-outward signs from it.
- Compute an internal face's conductance/flux model once; add both rows without overwriting previous contributions.
- Loop over **all** internal faces, including faces adjacent to boundary cells; do not assume face IDs start or end at special values in generic code.
- Add source and boundary contributions to existing rows with the chosen $\sum F=b$ convention.
- Test a constant field (zero internal flux), a two-cell interface (equal-and-opposite outward fluxes), and a three-cell Dirichlet problem (correct end rows).
- When moving beyond orthogonal meshes, upgrade the face-normal gradient and diffusivity treatment while retaining the same conservation/scatter architecture.

## Further reading

- OpenFOAM Documentation, *Schemes* and *Laplacian schemes* (general scalar transport and Gauss-based Laplacian discretization).
- CFD Direct, *Notes on Computational Fluid Dynamics: General Principles*, section 3.7, *Laplacian discretisation* (face interpolation and owner/neighbour perspective).
- RWTH Aachen University, *MSSD Lecture 06: Finite volumes* (integral conservation and equal-and-opposite numerical face fluxes).
