# CFD Toolkit Theory Guide

CFD Toolkit is a modular finite-volume framework with a clean separation of dimensions for solving transport equations. This guide explains the mathematical ideas, conventions, and discretization choices behind the library, connecting the governing equations to their numerical representation and implementation.

The guide begins with the general transport equation and develops its terms and discretization step by step. Each topic is introduced as it becomes relevant to the library, so the theory remains connected to the code and its current capabilities.

# The Divergence Theorem

The divergence theorem, also known as Gauss's theorem, relates the behavior of a vector field inside a volume to its flux through the volume's boundary. For a vector field $\mathbf{F}$ defined on a volume $V$ with closed boundary $\partial V$, it states that

$$
\int_V \nabla \cdot \mathbf{F}\,\mathrm{d}V = \oint_{\partial V} \mathbf{F} \cdot \mathbf{n}\,\mathrm{d}A  \qquad (1)
$$

In Eq. (1), $\mathbf{n}$ is the **outward unit normal to the boundary**, and $\mathbf{F} \cdot \mathbf{n}$ is the outward flux per unit area. The theorem applies, for example, when $V$ has a piecewise smooth boundary and $\mathbf{F}$ has continuous first derivatives on a region containing $V$. Flux leaving the volume is positive; flux entering it is negative. The surface integral therefore measures the **net outward flux**, accounting for both inflow and outflow.

In Cartesian coordinates, the divergence is

$$
\nabla \cdot \mathbf{F} = \frac{\partial F_x}{\partial x} + \frac{\partial F_y}{\partial y} + \frac{\partial F_z}{\partial z}  \qquad (2)
$$

The divergence in Eq. (2) measures the local net outward flux per unit volume. A positive divergence indicates a local excess of outward flux, while a negative divergence indicates a local excess of inward flux. Eq. (1) says that integrating this local imbalance over the volume gives the net flux across its boundary. For example, if $\mathbf{F}=\rho\mathbf{u}$ is mass flux, the surface integral is the net mass flow rate out of $V$, and $\nabla\cdot(\rho\mathbf{u})$ has units of mass per unit volume per unit time.

In one dimension, the theorem reduces to the fundamental theorem of calculus. For an interval $[x_w,x_e]$ and a scalar flux component $F_x$,

$$
\int_{x_w}^{x_e} \frac{\mathrm{d}F_x}{\mathrm{d}x}\,\mathrm{d}x = F_x(x_e)-F_x(x_w)  \qquad (3)
$$

The signs in Eq. (3) follow from the outward normals: $n_w=-1$ at the west boundary and $n_e=+1$ at the east boundary. With a constant cross-sectional area $A$, the corresponding volume integral is $A[F_x(x_e)-F_x(x_w)]$.

# Derivation of the Conservative Scalar Transport Equation

A scalar transport equation expresses the conservation of an extensive quantity carried by a fluid. Let $\phi$ be the amount of that quantity per unit mass of fluid, $\rho$ the fluid density, and $\mathbf{u}$ the fluid velocity. Thus $\rho\phi$ is the amount per unit volume. The derivation below uses a **fixed control volume** $V$ with boundary $\partial V$ and outward unit normal $\mathbf{n}$. The volume need not coincide with a fluid parcel: fluid and the quantity it carries may cross its boundary.

## 1. Balance over a control volume

The conservation statement is

$$
\text{rate of accumulation in }V
= \text{net rate entering through }\partial V
+ \text{net rate generated within }V.
$$

Let $\mathbf{j}_\phi$ denote the non-advective flux of the transported quantity, measured relative to the mass-averaged fluid motion, and let $S_\phi$ be its net volumetric production rate. Both fluxes below are expressed as outward fluxes:

- **Accumulation:** $\displaystyle \frac{\mathrm{d}}{\mathrm{d}t}\int_V \rho\phi\,\mathrm{d}V$.
- **Advection:** $\displaystyle \oint_{\partial V}\rho\phi\,\mathbf{u}\cdot\mathbf{n}\,\mathrm{d}A$, the quantity carried by the bulk mass flow.
- **Non-advective transport:** $\displaystyle \oint_{\partial V}\mathbf{j}_\phi\cdot\mathbf{n}\,\mathrm{d}A$, which can represent molecular diffusion or conduction, depending on the quantity and model.
- **Volumetric production:** $\displaystyle \int_V S_\phi\,\mathrm{d}V$, positive for net generation and negative for net consumption.

Since outward flux removes quantity from $V$, the balance is

$$
\frac{\mathrm{d}}{\mathrm{d}t}\int_V \rho\phi\,\mathrm{d}V
+ \oint_{\partial V}(\rho\phi\mathbf{u}+\mathbf{j}_\phi)\cdot\mathbf{n}\,\mathrm{d}A
= \int_V S_\phi\,\mathrm{d}V. \qquad (4)
$$

The source must include all relevant volumetric contributions to the transported quantity, including any carried by mass injected into or removed from the control volume. Boundary inflows and outflows instead appear in the surface flux.

## 2. Differential conservation law

For a fixed control volume, with sufficient regularity to interchange differentiation and integration, the time derivative in Eq. (4) can be taken inside the integral. Applying the divergence theorem to the surface flux gives

$$
\int_V \left[
\frac{\partial(\rho\phi)}{\partial t}
+ \nabla\cdot(\rho\phi\mathbf{u})
+ \nabla\cdot\mathbf{j}_\phi
- S_\phi
\right]\mathrm{d}V = 0.
$$

Because this balance holds for every fixed control volume, the integrand vanishes locally wherever the fields are sufficiently smooth:

$$
\boxed{
\frac\partial{\partial t}(\rho\phi)
+ \nabla\cdot(\rho\mathbf{u}\phi)
= -\nabla\cdot\mathbf{j}_\phi + S_\phi.
} \qquad (5)
$$

This is the conservative scalar transport law before specifying a constitutive model for the non-advective flux. Its defining feature is that storage and transport are written in terms of the conserved density $\rho\phi$ and its fluxes. Equation (4) remains the appropriate balance across discontinuities, where the pointwise differential form requires additional care.

## 3. Gradient-diffusion model

For an isotropic medium, a common constitutive approximation is

$$
\mathbf{j}_\phi = -\Gamma_\phi\nabla\phi, \qquad \Gamma_\phi \geq 0, \qquad (6)
$$

where $\Gamma_\phi$ is the effective diffusion coefficient for the chosen scalar. The minus sign makes the diffusive flux point down the scalar gradient. Substitution into Eq. (5) gives the conservative scalar transport equation

$$
\boxed{
\frac{\partial(\rho\phi)}{\partial t}
+ \nabla\cdot(\rho\mathbf{u}\phi)
= \nabla\cdot(\Gamma_\phi\nabla\phi) + S_\phi.
} \qquad (7)
$$

Its terms are, from left to right: transient accumulation, net outward advective transport, diffusion, and volumetric production. The diffusion term is a **net diffusive influx per unit volume** because the outward diffusive flux is $-\Gamma_\phi\nabla\phi$. The coefficient may vary in space or time; it must remain inside the divergence in general. Equation (6) is a model, not a consequence of conservation, and more general transport can require a tensor coefficient or additional flux terms.

If $Q$ denotes the unit of the transported extensive quantity, then $\rho\phi$ has units $Q/\mathrm{m}^3$, each flux has units $Q/(\mathrm{m}^2\,\mathrm{s})$, and every term in Eq. (7), including $S_\phi$, has units $Q/(\mathrm{m}^3\,\mathrm{s})$. The units of $\Gamma_\phi$ depend on the definition of $\phi$. For a species mass fraction, for example, $Q$ is species mass and a common model is $\Gamma_\phi=\rho D$, where $D$ is a mass diffusivity.

## 4. Relationship to the material-derivative form

The product rule relates the conservative left-hand side to the material derivative $D\phi/Dt=\partial\phi/\partial t+\mathbf{u}\cdot\nabla\phi$:

$$
\frac{\partial(\rho\phi)}{\partial t}
+ \nabla\cdot(\rho\mathbf{u}\phi)
= \rho\frac{D\phi}{Dt}
+ \phi\left[\frac{\partial\rho}{\partial t}+\nabla\cdot(\rho\mathbf{u})\right]. \qquad (8)
$$

If mass satisfies $\partial\rho/\partial t+\nabla\cdot(\rho\mathbf{u})=S_m$, then Eq. (7) becomes

$$
\rho\frac{D\phi}{Dt}
= \nabla\cdot(\Gamma_\phi\nabla\phi)+S_\phi-\phi S_m. \qquad (9)
$$

Only when $S_m=0$ does the last term vanish. In particular, for constant density and source-free mass conservation, $\nabla\cdot\mathbf{u}=0$ and the accumulation and advection terms reduce to $\rho(\partial\phi/\partial t+\mathbf{u}\cdot\nabla\phi)$. Constant density alone does not justify discarding a mass source.

## 5. One-dimensional finite-volume balance

For a cell bounded by $x_w$ and $x_e$, with constant cross-sectional area $A$, define the total flux in the positive $x$ direction as

$$
F_x = \rho u\phi - \Gamma_\phi\frac{\partial\phi}{\partial x}.
$$

Integrating Eq. (7) over the cell gives

$$
\frac{\mathrm{d}}{\mathrm{d}t}\int_{x_w}^{x_e}\rho\phi\,A\,\mathrm{d}x
+ A\bigl[F_x(x_e)-F_x(x_w)\bigr]
= \int_{x_w}^{x_e}S_\phi\,A\,\mathrm{d}x. \qquad (10)
$$

The west-face contribution has a minus sign because its outward normal points in the negative $x$ direction. In a finite-volume discretization, a shared internal-face flux enters the two adjacent cell balances with opposite signs, so it cancels when the balances are summed. Boundary fluxes and sources remain in the global balance. Numerical interpolation, time integration, and boundary conditions are needed to close the discrete equations; their choices do not change the conservation statement.

For the toolkit's present one-dimensional steady diffusion workflow, setting the transient and advective terms to zero gives $-\mathrm{d}(\Gamma_\phi\,\mathrm{d}\phi/\mathrm{d}x)/\mathrm{d}x=S_\phi$. The implemented Laplacian assembles internal-face contributions for this negative diffusion operator; boundary contributions are applied separately. The general transient and advective terms in Eq. (7) describe the theory and do not imply that those operators are currently implemented.
