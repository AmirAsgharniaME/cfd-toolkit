# CFD Toolkit Theory Guide

CFD Toolkit is a modular finite-volume framework that separates dimensions when solving transport equations. This guide explains the mathematical ideas, conventions, and discretization choices behind the library, connecting the governing equations to their numerical representation and implementation.

The guide begins with the general transport equation and develops its terms and discretization step by step. Each topic is introduced as it becomes relevant to the library, so the theory remains connected to the code and its current capabilities.

# Eulerian and Lagrangian Views in Fluid Mechanics

Fluid motion can be described from two complementary viewpoints. The **Eulerian** viewpoint describes what happens at fixed locations in space. The **Lagrangian** viewpoint follows individual fluid particles as they move. Both describe the same physical flow; they organize the observations and the resulting equations differently.

## Eulerian description

In an Eulerian description, the flow variables are fields defined over space and time. A velocity field, for example, assigns a velocity to every position and time,

$$
\mathbf{u}=\mathbf{u}(\mathbf{x},t)  \qquad (1)
$$

where $\mathbf{x}$ is a spatial position. Likewise, density, pressure, temperature, and a transported scalar are represented by fields such as $\rho(\mathbf{x},t)$, $p(\mathbf{x},t)$, $T(\mathbf{x},t)$, and $\phi(\mathbf{x},t)$. The value $`\phi(\mathbf{x}_0,t_0)`$ means the value measured at the fixed location $`\mathbf{x}_0`$ at the instant $`t_0`$; it does not identify the particular fluid particle that happens to be there.

This viewpoint is natural for experiments and numerical methods that use fixed probes, control volumes, or stationary meshes. A finite-volume cell is an Eulerian region: fluid crosses its faces while the cell remains associated with the same region of space. Conservation laws are therefore written as rates of accumulation inside a fixed region plus fluxes through its boundary. The conservative scalar transport equation developed later in this guide has precisely this form,

$$
\frac{\partial(\rho\phi)}{\partial t} +\nabla\cdot(\rho\mathbf{u}\phi) =\nabla\cdot(\Gamma_\phi\nabla\phi)+S_\phi  \qquad (2)
$$

The time derivative at a fixed position, $\partial\phi/\partial t$, is called the **local** or **Eulerian time derivative**. It measures how the field changes at that location. It does not, by itself, measure how the property of a particular moving fluid particle changes, because the particle generally passes through locations where the field has different values.

## Lagrangian description

In a Lagrangian description, the observer follows a material particle or a material volume. Let $\mathbf{X}$ label a particle by its position at a reference time $`t_0`$. Its current position is a trajectory,

$$
\mathbf{x}=\boldsymbol{\chi}(\mathbf{X},t)  \qquad (3)
$$

and the particle velocity is the rate of change of that trajectory,

$$
\frac{\mathrm{d}\boldsymbol{\chi}}{\mathrm{d}t}(\mathbf{X},t) =\mathbf{u}(\boldsymbol{\chi}(\mathbf{X},t),t)  \qquad (4)
$$

The particle's density, temperature, or scalar value is obtained by evaluating the corresponding Eulerian field along its path. For example,

$$
\phi_{\mathrm{particle}}(t) =\phi(\boldsymbol{\chi}(\mathbf{X},t),t)  \qquad (5)
$$

The Lagrangian derivative of this quantity is an ordinary derivative with respect to time along the trajectory. This description is useful when the history of a particle matters, such as in particle tracking, droplet and sediment motion, sprays, atmospheric transport, and fluid-particle interaction. A material volume contains the same fluid particles at every instant, so no advective mass crosses its boundary. Diffusion, heat transfer, and chemical reactions can still transfer or produce the relevant quantity within that material volume.

The terms **pathline**, **streakline**, and **timeline** refer to different ways of visualizing a flow. A pathline is the trajectory of one particle. A streakline is the set of particles that have previously passed through a specified fixed point, such as the visible line produced by continuously injecting dye. A timeline is the line formed by particles that were marked simultaneously. In a steady flow, these three curves coincide; in an unsteady flow, they generally differ.

## Connecting the two descriptions: the material derivative

The Eulerian and Lagrangian descriptions are connected by the chain rule. Let $\phi(\mathbf{x},t)$ be any differentiable scalar field and let a particle follow $\mathbf{x}(t)$. Along that particle,

$$
\frac{\mathrm{d}}{\mathrm{d}t}\phi(\mathbf{x}(t),t) =\frac{\partial\phi}{\partial t} +\frac{\mathrm{d}\mathbf{x}}{\mathrm{d}t}\cdot\nabla\phi =\frac{\partial\phi}{\partial t}+\mathbf{u}\cdot\nabla\phi  \qquad (6)
$$

The operator

$$
\boxed{\frac{D}{Dt} =\frac{\partial}{\partial t}+\mathbf{u}\cdot\nabla}  \qquad (7)
$$

is the **material derivative**, also called the **substantial derivative**. It gives the rate of change experienced by a fluid particle while the fields are represented in Eulerian coordinates. The two terms have distinct meanings:

- $\partial\phi/\partial t$ is the local change observed at a fixed position.
- $\mathbf{u}\cdot\nabla\phi$ is the change caused by the particle moving through spatial variations of $\phi$; it is the advective or convective contribution.

For a one-dimensional flow with velocity $u(x,t)$,

$$
\frac{D\phi}{Dt}=\frac{\partial\phi}{\partial t}+u\frac{\partial\phi}{\partial x}  \qquad (8)
$$

For a vector field, the same idea applies component by component. In particular, the material acceleration is

$$
\frac{D\mathbf{u}}{Dt} =\frac{\partial\mathbf{u}}{\partial t}+\left(\mathbf{u}\cdot\nabla\right)\mathbf{u}  \qquad (9)
$$

The first term is the local acceleration at a fixed point. The second is convective acceleration: a particle can accelerate even in a steady Eulerian flow if it moves into a region with a different velocity. For example, fluid moving through a converging channel can have $\partial\mathbf{u}/\partial t=0$ while $D\mathbf{u}/Dt\ne0$.

## Material and spatial balances

The distinction between the viewpoints also explains two common forms of conservation laws. A balance written for a fixed control volume is Eulerian and contains fluxes through the control surface. A balance written for a material volume follows the same fluid particles, so its boundary has no advective mass flux. For a conserved scalar with specific value $\phi$, non-advective flux $`\mathbf{j}_\phi`$, and volumetric source $`S_\phi`$, the material-volume balance can be written as

$$
\frac{\mathrm{d}}{\mathrm{d}t}\int_{V(t)}\rho\phi\mkern3mu\mathrm{d}V =-\oint_{\partial V(t)}\mathbf{j}_\phi\cdot\mathbf{n}\mkern3mu\mathrm{d}A +\int_{V(t)}S_\phi\mkern3mu\mathrm{d}V  \qquad (10)
$$

The advective flux does not appear because the boundary moves with the fluid. Applying the transport theorem and using mass conservation converts this material-volume statement into the Eulerian local form,

$$
\rho\frac{D\phi}{Dt} =-\nabla\cdot\mathbf{j}_\phi+S_\phi  \qquad (11)
$$

when there is no volumetric mass source. With the diffusion model $`\mathbf{j}_\phi=-\Gamma_\phi\nabla\phi`$, this becomes

$$
\rho\frac{D\phi}{Dt} =\nabla\cdot(\Gamma_\phi\nabla\phi)+S_\phi  \qquad (12)
$$

Expanding $D\phi/Dt$ recovers the familiar local-plus-advective form. The conservative Eulerian form is obtained by combining this equation with the continuity equation. The forms are mathematically equivalent when the required smoothness and mass-balance assumptions hold, but the conservative form remains the appropriate integral statement across discontinuities and is the form most directly used by finite-volume discretizations.

## Comparison of the viewpoints

| Feature | Eulerian view | Lagrangian view |
|---|---|---|
| Object being followed | A fixed location, control volume, or mesh cell | A fluid particle or material volume |
| Independent description | Fields $f(\mathbf{x},t)$ over space and time | Particle histories $f(t;\mathbf{X})$ along trajectories |
| Motion across the boundary | Fluid generally crosses a fixed boundary | No advective mass crosses a material boundary |
| Natural derivative | Local derivative $\partial/\partial t$ | Ordinary derivative along a particle path |
| Combined rate of change | Material derivative $`D/Dt=\partial_t+\mathbf{u}\cdot\nabla`$ | Direct time derivative of the particle property |
| Typical applications | CFD on fixed or moving meshes, probes, control-volume balances | Particle tracking, sprays, droplets, trajectories, fluid histories |

The choice is a modeling and computational choice rather than a difference in the underlying physics. A flow solver may use Eulerian fields for pressure and velocity while also integrating Lagrangian particles through those fields. Conversely, particle measurements can be converted into Eulerian estimates by averaging observations over spatial cells. In this toolkit, the finite-volume mesh and its cell-centered fields use the Eulerian viewpoint: each cell stores field values for a spatial region, and transport between cells is represented by face fluxes. The material derivative remains essential for interpreting the equations and for relating the fixed-cell balance to the change experienced by the moving fluid.

# The Divergence Theorem

The divergence theorem, also known as Gauss's theorem, relates the behavior of a vector field inside a volume to its flux through the volume's boundary. For a vector field $\mathbf{F}$ defined on a volume $V$ with closed boundary $\partial V$, it states that

$$
\int_V \nabla \cdot \mathbf{F}\mkern3mu\mathrm{d}V = \oint_{\partial V} \mathbf{F} \cdot \mathbf{n}\mkern3mu\mathrm{d}A  \qquad (13)
$$

In Eq. (13), $\mathbf{n}$ is the **outward unit normal to the boundary**, and $\mathbf{F} \cdot \mathbf{n}$ is the outward flux per unit area. The theorem applies, for example, when $V$ has a piecewise smooth boundary and $\mathbf{F}$ has continuous first derivatives on a region containing $V$. Flux leaving the volume is positive; flux entering it is negative. The surface integral therefore measures the **net outward flux**, accounting for both inflow and outflow.

In Cartesian coordinates, the divergence is

$$
\nabla \cdot \mathbf{F} = \frac{\partial F_x}{\partial x} + \frac{\partial F_y}{\partial y} + \frac{\partial F_z}{\partial z}  \qquad (14)
$$

The divergence in Eq. (14) measures the local net outward flux per unit volume. The terms on the right use the Cartesian components of the vector field and their corresponding coordinates. A positive divergence indicates a local excess of outward flux, while a negative divergence indicates a local excess of inward flux. Eq. (13) says that integrating this local imbalance over the volume gives the net flux across its boundary. For example, if $\mathbf{F}=\rho\mathbf{u}$ is mass flux, where $\rho$ is the fluid density and $\mathbf{u}$ is the fluid velocity, the surface integral is the net mass flow rate out of $V$, and $\nabla\cdot(\rho\mathbf{u})$ has units of mass per unit volume per unit time.

In one dimension, the theorem reduces to the fundamental theorem of calculus. For an interval $`[x_w,x_e]`$ with west and east endpoints, respectively, and a scalar flux component $`F_x`$,

$$
\int_{x_w}^{x_e} \frac{\mathrm{d}F_x}{\mathrm{d}x}\mkern3mu\mathrm{d}x = F_x(x_e)-F_x(x_w)  \qquad (15)
$$

The signs in Eq. (15) follow from the outward normals: $`n_w=-1`$ at the west boundary and $`n_e=+1`$ at the east boundary. With a constant cross-sectional area $A$, the corresponding volume integral is $`A[F_x(x_e)-F_x(x_w)]`$.


# Derivation of the Conservative Scalar Transport Equation

A scalar transport equation conserves the total amount of an extensive quantity carried by a fluid; denote that total amount by $\Phi$ and the corresponding fluid mass by $m$. The field variable $\phi$ is not this total amount: it is the amount per unit mass, $\phi=\frac{\Phi}{m}$, so it is a specific quantity and is intensive with respect to the amount of fluid. Let $\rho$ be the fluid density and $\mathbf{u}$ the fluid velocity. Their product $\rho\phi$ is the transported amount per unit volume. In terms of units, if $M$, $L$, and $Q$ denote units of mass, length, and transported quantity, then $[\rho]=M/L^3$, $[\phi]=Q/M$, and $[\rho\phi]=Q/L^3$. This volumetric density is used because, in a **fixed control volume** $V$, the total amount is obtained by integrating over volume, while transport through its boundary $\partial V$ is expressed using the same amount per unit volume. The boundary has outward unit normal $\mathbf{n}$. The volume need not coincide with a fluid parcel: fluid and the quantity it carries may cross its boundary.

## 1. Balance over a control volume

The conservation statement is that the rate of accumulation in $V$ equals the net rate entering through $\partial V$ plus the net rate generated within $V$.

Let $`\mathbf{j}_\phi`$ denote the non-advective flux of the transported quantity, measured relative to the mass-averaged fluid motion, and let $`S_\phi`$ be its net volumetric production rate. The following terms use an outward-flux convention:

- **Accumulation:**

$$
\frac{\mathrm{d}}{\mathrm{d}t}\int_V \rho\phi\mkern3mu\mathrm{d}V  \qquad (16)
$$

- **Advection:** the quantity carried by the bulk mass flow

$$
\oint_{\partial V}\rho\phi\mkern3mu\mathbf{u}\cdot\mathbf{n}\mkern3mu\mathrm{d}A  \qquad (17)
$$

- **Non-advective transport:** transport that can represent molecular diffusion or conduction, depending on the quantity and model

$$
\oint_{\partial V}\mathbf{j}_\phi\cdot\mathbf{n}\mkern3mu\mathrm{d}A  \qquad (18)
$$

- **Volumetric production:** positive for net generation and negative for net consumption

$$
\int_V S_\phi\mkern3mu\mathrm{d}V  \qquad (19)
$$

Since outward flux removes the quantity from $V$, the balance is

$$
\frac{\mathrm{d}}{\mathrm{d}t}\int_V \rho\phi\mkern3mu\mathrm{d}V + \oint_{\partial V}(\rho\phi\mathbf{u}+\mathbf{j}_\phi)\cdot\mathbf{n}\mkern3mu\mathrm{d}A = \int_V S_\phi\mkern3mu\mathrm{d}V  \qquad (20)
$$

The source must include all relevant volumetric contributions to the transported quantity, including any carried by mass injected into or removed from the control volume. Boundary inflows and outflows instead appear in the surface flux.

## 2. Differential conservation law

For a fixed control volume, with sufficient regularity to interchange differentiation and integration, the time derivative in Eq. (20) can be taken inside the integral. Applying the divergence theorem to the surface flux gives

$$
\int_V \left[\frac{\partial(\rho\phi)}{\partial t} + \nabla\cdot(\rho\phi\mathbf{u}) + \nabla\cdot\mathbf{j}_\phi - S_\phi\right]\mkern3mu\mathrm{d}V = 0  \qquad (21)
$$

Because the balance in Eq. (21) holds for every fixed control volume, the integrand vanishes locally wherever the fields are sufficiently smooth:

$$
\boxed{\frac{\partial}{\partial t}(\rho\phi) + \nabla\cdot(\rho\mathbf{u}\phi) = -\nabla\cdot\mathbf{j}_\phi + S_\phi}  \qquad (22)
$$

Eq. (22) is the conservative scalar transport law before specifying a constitutive model for the non-advective flux. Its defining feature is that storage and transport are written in terms of the conserved density $\rho\phi$ and its fluxes. Eq. (20) remains the appropriate balance across discontinuities, where the pointwise differential form requires additional care.

## 3. Gradient-diffusion model

For an isotropic medium, a common constitutive approximation is

$$
\mathbf{j}_\phi = -\Gamma_\phi\nabla\phi, \qquad \Gamma_\phi \geq 0  \qquad (23)
$$

In Eq. (23), $`\Gamma_\phi`$ is the effective diffusion coefficient for the chosen scalar. The minus sign makes the diffusive flux point down the scalar gradient. Substitution into Eq. (22) gives the conservative scalar transport equation

$$
\boxed{\frac{\partial(\rho\phi)}{\partial t} + \nabla\cdot(\rho\mathbf{u}\phi) = \nabla\cdot(\Gamma_\phi\nabla\phi) + S_\phi}  \qquad (24)
$$

The terms in Eq. (24) are transient accumulation, net outward advective transport, diffusion, and volumetric production, from left to right. The diffusion term is a **net diffusive influx per unit volume** because the outward diffusive flux is $`-\Gamma_\phi\nabla\phi`$. The coefficient may vary in space or time; it must remain inside the divergence in general. Eq. (23) is a model, not a consequence of conservation, and more general transport can require a tensor coefficient or additional flux terms.

If $Q$ denotes the unit of the transported extensive quantity, then $\rho\phi$ has units $Q/\mathrm{m}^3$, each flux has units $Q/(\mathrm{m}^2\mkern3mu\mathrm{s})$, and every term in Eq. (24), including $`S_\phi`$, has units $Q/(\mathrm{m}^3\mkern3mu\mathrm{s})$. The units of $`\Gamma_\phi`$ depend on the definition of $\phi$. For a species mass fraction, for example, $Q$ is species mass and a common model is $`\Gamma_\phi=\rho D`$, where $D$ is a mass diffusivity.

## 4. Relationship to the material-derivative form

The product rule relates the conservative left-hand side to the material derivative $D\phi/Dt=\partial\phi/\partial t+\mathbf{u}\cdot\nabla\phi$:

$$
\frac{\partial(\rho\phi)}{\partial t} + \nabla\cdot(\rho\mathbf{u}\phi) = \rho\frac{D\phi}{Dt} + \phi\left[\frac{\partial\rho}{\partial t}+\nabla\cdot(\rho\mathbf{u})\right]  \qquad (25)
$$

In Eq. (25), $D\phi/Dt$ is the material derivative. If mass satisfies $`\partial\rho/\partial t+\nabla\cdot(\rho\mathbf{u})=S_m`$, where the right-hand side is the mass source, then Eq. (24) becomes

$$
\rho\frac{D\phi}{Dt} = \nabla\cdot(\Gamma_\phi\nabla\phi)+S_\phi-\phi S_m  \qquad (26)
$$

Only when $`S_m=0`$ does the last term in Eq. (26) vanish. In particular, for constant density and source-free mass conservation, $\nabla\cdot\mathbf{u}=0$ and the accumulation and advection terms reduce to $\rho(\partial\phi/\partial t+\mathbf{u}\cdot\nabla\phi)$. Constant density alone does not justify discarding a mass source.

## 5. One-dimensional finite-volume balance

For a cell bounded by $`x_w`$ and $`x_e`$, with constant cross-sectional area $A$, define the total flux in the positive $x$ direction as

$$
F_x = \rho u\phi - \Gamma_\phi\frac{\partial\phi}{\partial x}  \qquad (27)
$$

In Eq. (27), $u$ is the velocity component in the positive $x$ direction. Integrating Eq. (24) over the cell gives

$$
\frac{\mathrm{d}}{\mathrm{d}t}\int_{x_w}^{x_e}\rho\phi\mkern3muA\mkern3mu\mathrm{d}x + A\bigl[F_x(x_e)-F_x(x_w)\bigr] = \int_{x_w}^{x_e}S_\phi\mkern3muA\mkern3mu\mathrm{d}x  \qquad (28)
$$

The west-face contribution in Eq. (28) has a minus sign because its outward normal points in the negative $x$ direction. In a finite-volume discretization, a shared internal-face flux enters the two adjacent cell balances with opposite signs, so it cancels when the balances are summed. Boundary fluxes and sources remain in the global balance. Numerical interpolation, time integration, and boundary conditions are needed to close the discrete equations; their choices do not change the conservation statement.

For the toolkit's present one-dimensional steady diffusion workflow, setting the transient and advective terms to zero gives $`-\mathrm{d}(\Gamma_\phi\mkern3mu\mathrm{d}\phi/\mathrm{d}x)/\mathrm{d}x=S_\phi`$. The implemented Laplacian assembles internal-face contributions for this negative diffusion operator; boundary contributions are applied separately. The general transient and advective terms in Eq. (24) describe the theory and do not imply that those operators are currently implemented.
