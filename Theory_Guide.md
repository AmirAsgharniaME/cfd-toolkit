# CFD Toolkit Theory Guide

CFD Toolkit is a modular finite-volume framework that separates dimensions when solving transport equations. This guide explains the mathematical ideas, conventions, and discretization choices behind the library, connecting the governing equations to their numerical representation and implementation.

The guide begins with the general transport equation and develops its terms and discretization step by step. Each topic is introduced as it becomes relevant to the library, so the theory remains connected to the code and its current capabilities.

# Eulerian and Lagrangian Views in Fluid Mechanics

Fluid motion can be described by following individual material particles or by observing what happens at locations in space. These are the **Lagrangian** and **Eulerian** viewpoints, respectively. They describe the same flow, but use different ways to identify where and when a property is observed.

## Lagrangian description

In solid mechanics, it is common to identify a material point in an undeformed reference configuration and follow its displacement as the body deforms. The Lagrangian description of a fluid starts with the same idea: label a fluid particle and follow it as it moves. A fluid particle here means a small material parcel, not necessarily a molecule or a suspended solid particle.

Let $\mathbf{X}$ label a particle by its position at a reference time $`t_0`$. Its position at a later time is given by the motion or trajectory

$$
\mathbf{x}=\boldsymbol{\chi}(\mathbf{X},t)  \qquad (1)
$$

In Eq. (1), $`\boldsymbol{\chi}(\mathbf{X},t_0)=\mathbf{X}`$. Keeping $\mathbf{X}$ fixed follows the same particle. Its velocity is the time derivative of its position,

$$
\mathbf{v}(\mathbf{X},t)=\frac{\partial\boldsymbol{\chi}}{\partial t}(\mathbf{X},t)  \qquad (2)
$$

In Eq. (2), $\mathbf{v}$ is the particle velocity. A property such as temperature can similarly be recorded as a particle history $`\phi_L(\mathbf{X},t)`$. Its time derivative at fixed $\mathbf{X}$ describes the change experienced by that particle. A material volume contains the same fluid particles as it moves and deforms, so no advective mass crosses its boundary. Heat or other quantities can still cross the boundary by non-advective processes, and sources can act within the volume.

This viewpoint is useful when particle histories matter, for example in particle tracking, droplet motion, and fluid-particle interaction. Four related flow-visualization curves answer different questions:

- **Streamline:** What direction does the velocity field point in at this instant? A streamline is a curve everywhere tangent to the instantaneous velocity field. It is a snapshot of the field, so in an unsteady flow it is not generally the trajectory of any one particle.
- **Pathline:** Where has this particular particle traveled? A pathline is the actual trajectory traced over time by one identified fluid particle, found by following that particle as it moves.
- **Streakline:** Where are the particles now that passed through this fixed point? At a specified instant, a streakline is the locus of all particles that previously passed through the same fixed point in space. Continuous dye or smoke injection from a fixed source produces a streakline and records particles released at different times.
- **Timeline:** Where are the particles now that were marked along this line earlier? A timeline is formed by marking a set of particles that lie on the same line at one instant. The marked particles then move and deform that line; this is distinct from tracking one particle or continuously injecting a tracer at one fixed point.

In steady flow, streamlines, pathlines, and streaklines coincide under the usual assumptions because the velocity field does not change with time. In unsteady flow, they generally differ. A timeline does not generally coincide with any of these three; its shape depends on how the marked particles move after the marking.

## From following particles to observing locations

Fluids can undergo large, continuous motion and deformation. Instead of keeping track of every particle, we can place a probe at a fixed position and record the properties of whichever particles pass it. This change of bookkeeping leads to the Eulerian description: label the observation by its current position $\mathbf{x}$ and time $t$, rather than by the particle's reference position $\mathbf{X}$.

For example, the Eulerian velocity field assigns a velocity to each position and time,

$$
\mathbf{u}=\mathbf{u}(\mathbf{x},t)  \qquad (3)
$$

In Eq. (3), $\mathbf{u}$ is the Eulerian velocity. Density, pressure, temperature, and a transported scalar are likewise fields such as $`\rho(\mathbf{x},t)`$, $`p(\mathbf{x},t)`$, $`T(\mathbf{x},t)`$, and $`\phi(\mathbf{x},t)`$. The value $`\phi(\mathbf{x}_0,t_0)`$ is observed at the fixed location $`\mathbf{x}_0`$ at the instant $`t_0`$; it is the property of whichever particle is there at that instant.

The two descriptions meet at the particle's current position. Evaluating the Eulerian velocity and scalar fields along its trajectory gives the same velocity and property recorded in its Lagrangian history:

$$
\mathbf{v}(\mathbf{X},t)=\mathbf{u}(\boldsymbol{\chi}(\mathbf{X},t),t)  \qquad (4)
$$

Similarly,

$$
\phi_L(\mathbf{X},t)=\phi(\boldsymbol{\chi}(\mathbf{X},t),t)  \qquad (5)
$$

Eq. (4) relates the velocity descriptions, and Eq. (5) relates the scalar descriptions. A fixed probe generally observes different particles over time. Thus $\partial\phi/\partial t$, the **local** or **Eulerian time derivative**, measures the change at a fixed position; the change experienced by one moving particle also depends on how the field varies along its path. The material derivative below makes that connection precise.

Fixed control volumes and stationary meshes also use the Eulerian viewpoint. Fluid crosses the faces of a finite-volume cell while the cell remains associated with the same region of space. Its conservation balance therefore accounts for accumulation inside the cell and fluxes through its boundary. The conservative scalar transport equation developed later expresses this balance in differential form; its derivation and assumptions are discussed there.

## Connecting the two descriptions: the material derivative

The Eulerian and Lagrangian descriptions are connected by the chain rule. Let $\phi(\mathbf{x},t)$ be any differentiable scalar field and let a particle follow $\mathbf{x}(t)$. Along that particle,

$$
\frac{\mathrm{d}}{\mathrm{d}t}\phi(\mathbf{x}(t),t) =\frac{\partial\phi}{\partial t} +\frac{\mathrm{d}\mathbf{x}}{\mathrm{d}t}\cdot\nabla\phi =\frac{\partial\phi}{\partial t}+\mathbf{u}\cdot\nabla\phi  \qquad (6)
$$

In Eq. (6), the derivative follows the particle. The operator

$$
\boxed{\frac{D}{Dt} =\frac{\partial}{\partial t}+\mathbf{u}\cdot\nabla}  \qquad (7)
$$

in Eq. (7) is the **material derivative**, also called the **substantial derivative**. It gives the rate of change experienced by a fluid particle while the fields are represented in Eulerian coordinates.

Here, $\mathbf{u}\cdot\nabla$ is a single directional (or convective) derivative operator, not an ordinary product whose factors can be freely rearranged. It acts on the quantity written after it. For a two-dimensional velocity $\mathbf{u}=(u,v)$, $\mathbf{u}\cdot\nabla = u\frac{\partial}{\partial x}+v\frac{\partial}{\partial y}$. Thus, for a scalar $\phi$, $\mathbf{u}\cdot\nabla\phi = u\frac{\partial\phi}{\partial x}+v\frac{\partial\phi}{\partial y}$, which is a scalar. For a vector $`\mathbf{a}=(a_1,a_2)`$, the same operator acts componentwise: $`\mathbf{u}\cdot\nabla\mathbf{a}=(\mathbf{u}\cdot\nabla a_1,\mathbf{u}\cdot\nabla a_2)`$, which is a vector. The operand's type determines the result's type; in particular, $\mathbf{u}\cdot\nabla\mathbf{u}$ means that the operator acts on each component of the velocity vector. This is distinct from $\nabla\cdot\mathbf{u}$, which is the divergence of the velocity field. When learning the notation, it is helpful to read $\mathbf{u}\cdot\nabla\phi$ as “the operator $\mathbf{u}\cdot\nabla$ acting on $\phi$.”

The two terms have distinct meanings:

- $\partial\phi/\partial t$ is the local change observed at a fixed position.
- $\mathbf{u}\cdot\nabla\phi$ is the change caused by the particle moving through spatial variations of $\phi$; it is the advective or convective contribution.

For a one-dimensional flow with velocity $u(x,t)$,

$$
\frac{D\phi}{Dt}=\frac{\partial\phi}{\partial t}+u\frac{\partial\phi}{\partial x}  \qquad (8)
$$

Eq. (8) gives this derivative in one dimension. For a vector field, the same idea applies component by component. In particular, the material acceleration is

$$
\frac{D\mathbf{u}}{Dt} =\frac{\partial\mathbf{u}}{\partial t}+\left(\mathbf{u}\cdot\nabla\right)\mathbf{u}  \qquad (9)
$$

In Eq. (9), the first term is the local acceleration at a fixed point. The second is convective acceleration: a particle can accelerate even in a steady Eulerian flow if it moves into a region with a different velocity. For example, fluid moving through a converging channel can have $\partial\mathbf{u}/\partial t=0$ while $D\mathbf{u}/Dt\ne0$.

## Comparison of the viewpoints

| Feature | Eulerian view | Lagrangian view |
|---|---|---|
| Object being followed | A fixed location, control volume, or mesh cell | A fluid particle or material volume |
| Independent description | Fields $f(\mathbf{x},t)$ over space and time | Particle histories $f(t;\mathbf{X})$ along trajectories |
| Motion across the boundary | Fluid generally crosses a fixed boundary | No advective mass crosses a material boundary |
| Natural derivative | Local derivative $\partial/\partial t$ | Ordinary derivative along a particle path |
| Combined rate of change | Material derivative $`D/Dt=\partial_t+\mathbf{u}\cdot\nabla`$ | Direct time derivative of the particle property |
| Typical applications | CFD on fixed or moving meshes, probes, control-volume balances | Particle tracking, sprays, droplets, trajectories, fluid histories |

The choice concerns modeling and computation rather than the underlying physics. A flow solver may use Eulerian fields for pressure and velocity while also integrating Lagrangian particles through those fields. Conversely, particle measurements can be converted into Eulerian estimates by averaging observations over spatial cells. In this toolkit, the finite-volume mesh and its cell-centered fields use the Eulerian viewpoint: each cell stores field values for a spatial region, and transport between cells is represented by face fluxes. The material derivative remains essential for interpreting the equations and for relating the balance over a fixed cell to the change experienced by the moving fluid.

# The Divergence Theorem

The divergence theorem, also known as Gauss's theorem, relates the behavior of a vector field inside a volume to its flux through the volume's boundary. For a vector field $\mathbf{F}$ defined on a volume $V$ with closed boundary $\partial V$, it states that

$$
\int_V \nabla \cdot \mathbf{F}\mkern3mu\mathrm{d}V = \oint_{\partial V} \mathbf{F} \cdot \mathbf{n}\mkern3mu\mathrm{d}A  \qquad (10)
$$

In Eq. (10), $\mathbf{n}$ is the **outward unit normal to the boundary**, and $\mathbf{F} \cdot \mathbf{n}$ is the outward flux per unit area. The theorem applies, for example, when $V$ has a piecewise smooth boundary and $\mathbf{F}$ has continuous first derivatives on a region containing $V$. Flux leaving the volume is positive; flux entering it is negative. The surface integral therefore measures the **net outward flux**, accounting for both inflow and outflow.

In Cartesian coordinates, the divergence is

$$
\nabla \cdot \mathbf{F} = \frac{\partial F_x}{\partial x} + \frac{\partial F_y}{\partial y} + \frac{\partial F_z}{\partial z}  \qquad (11)
$$

The divergence in Eq. (11) measures the local net outward flux per unit volume. The terms on the right use the Cartesian components of the vector field and their corresponding coordinates. A positive divergence indicates a local excess of outward flux, while a negative divergence indicates a local excess of inward flux. Eq. (10) says that integrating this local imbalance over the volume gives the net flux across its boundary. For example, if $\mathbf{F}=\rho\mathbf{u}$ is mass flux, where $\rho$ is the fluid density and $\mathbf{u}$ is the fluid velocity, the surface integral is the net mass flow rate out of $V$, and $\nabla\cdot(\rho\mathbf{u})$ has units of mass per unit volume per unit time.

In one dimension, the theorem reduces to the fundamental theorem of calculus. For an interval $`[x_w,x_e]`$ with west and east endpoints, respectively, and a scalar flux component $`F_x`$,

$$
\int_{x_w}^{x_e} \frac{\mathrm{d}F_x}{\mathrm{d}x}\mkern3mu\mathrm{d}x = F_x(x_e)-F_x(x_w)  \qquad (12)
$$

The signs in Eq. (12) follow from the outward normals: $`n_w=-1`$ at the west boundary and $`n_e=+1`$ at the east boundary. With a constant cross-sectional area $A$, the corresponding volume integral is $`A[F_x(x_e)-F_x(x_w)]`$.


# Derivation of the Conservative Scalar Transport Equation

A scalar transport equation describes the balance of an extensive property carried by a fluid. Denote the total extensive property of a fluid volume by uppercase $\Phi$ and its fluid mass by $m$. The corresponding specific property is lowercase $\phi=\frac{\Phi}{m}$: it is the intensive property per unit mass. For a nonuniform fluid volume, this ratio is the mass-averaged specific property; the field $\phi$ gives its local value. Let $\rho$ be the fluid density and $\mathbf{u}$ the fluid velocity. The product $\rho\phi$ is the transported amount per unit volume. In terms of units, if $M$ and $L$ denote dimensions of mass and length, and $Q$ denotes the unit dimension of the transported quantity itself (for example, energy in joules or amount of substance in moles), then $[\rho]=M/L^3$, $[\phi]=Q/M$, and $[\rho\phi]=Q/L^3$. The unit symbol $Q$ is distinct from the extensive property symbol $\Phi$. This volumetric density is used because, in a **fixed control volume** $V$, the total extensive property $\Phi$ is obtained by integrating $\rho\phi$ over volume, while transport through its boundary $\partial V$ is expressed using the same amount per unit volume. The boundary has outward unit normal $\mathbf{n}$. The volume need not coincide with a fluid parcel: fluid and the quantity it carries may cross its boundary.

## 1. Balance over a control volume

The scalar transport equation expresses a physical balance law for the property $\phi$ per unit mass. For the fixed control volume, the rate of accumulation in $V$ equals the net rate entering through $\partial V$ plus the net rate generated within $V$.

Let $`\mathbf{j}_\phi`$ denote the non-advective flux of the transported quantity, measured relative to the mass-averaged fluid motion, and let $`S_\phi`$ denote its net volumetric production rate. The following terms use an outward-flux convention:

- **Accumulation:**

- **Advection:** the quantity carried by the bulk mass flow

- **Non-advective transport:** transport that can represent molecular diffusion or conduction, depending on the quantity and model

- **Volumetric production:** positive for net generation and negative for net consumption

$$
\frac{\mathrm{d}}{\mathrm{d}t}\int_V \rho\phi\mkern3mu\mathrm{d}V  \qquad (13)
$$

$$
\oint_{\partial V}\rho\phi\mkern3mu\mathbf{u}\cdot\mathbf{n}\mkern3mu\mathrm{d}A  \qquad (14)
$$

$$
\oint_{\partial V}\mathbf{j}_\phi\cdot\mathbf{n}\mkern3mu\mathrm{d}A  \qquad (15)
$$

$$
\int_V S_\phi\mkern3mu\mathrm{d}V  \qquad (16)
$$

Eq. (13), Eq. (14), Eq. (15), and Eq. (16) give accumulation, advection relative to the stationary boundary, non-advective transport, and volumetric production, respectively. Since outward flux removes the quantity from $V$, the balance is

$$
\frac{\mathrm{d}}{\mathrm{d}t}\int_V \rho\phi\mkern3mu\mathrm{d}V + \oint_{\partial V}(\rho\phi\mathbf{u}+\mathbf{j}_\phi)\cdot\mathbf{n}\mkern3mu\mathrm{d}A = \int_V S_\phi\mkern3mu\mathrm{d}V  \qquad (17)
$$

The source must include all relevant volumetric contributions to the transported quantity, including any carried by mass injected into or removed from the control volume. Boundary inflows and outflows instead appear in the surface flux.

## 2. Differential conservation law

For a fixed control volume, with sufficient regularity to interchange differentiation and integration, the time derivative in Eq. (17) can be taken inside the integral. Applying the **divergence theorem** to the surface flux gives

$$
\int_V \left[\frac{\partial(\rho\phi)}{\partial t} + \nabla\cdot(\rho\phi\mathbf{u}) + \nabla\cdot\mathbf{j}_\phi - S_\phi\right]\mkern3mu\mathrm{d}V = 0  \qquad (18)
$$

Because the balance in Eq. (18) holds for every fixed control volume, the integrand vanishes locally wherever the fields are sufficiently smooth:

$$
\boxed{\frac{\partial}{\partial t}(\rho\phi) + \nabla\cdot(\rho\mathbf{u}\phi) = -\nabla\cdot\mathbf{j}_\phi + S_\phi}  \qquad (19)
$$

Eq. (19) is the conservative scalar transport law before specifying a constitutive model for the non-advective flux. Its defining feature is that storage and transport are written in terms of the conserved density $\rho\phi$ and its fluxes. The fixed-volume balance and its derivation here do not require the Reynolds transport theorem. Eq. (17) remains the appropriate balance across discontinuities, where the pointwise differential form requires additional care.

## 3. Material and fixed-volume viewpoints

The same physical law can be applied to a material volume $V(t)$, which follows the same fluid particles as it moves and deforms. Its boundary moves with the local fluid velocity, so the advective flux relative to that boundary is zero, although non-advective transport can still cross it. With the same outward-normal and source conventions as Eq. (17), its balance is

$$
\frac{\mathrm{d}}{\mathrm{d}t}\int_{V(t)}\rho\phi\mkern3mu\mathrm{d}V = -\oint_{\partial V(t)}\mathbf{j}_\phi\cdot\mathbf{n}\mkern3mu\mathrm{d}A + \int_{V(t)}S_\phi\mkern3mu\mathrm{d}V  \qquad (20)
$$

The derivative on the left of Eq. (20) already includes the motion and deformation of $V(t)$. To rewrite it using spatial fields, the **Reynolds transport theorem** gives

$$
\frac{\mathrm{d}}{\mathrm{d}t}\int_{V(t)}\rho\phi\mkern3mu\mathrm{d}V = \int_{V(t)}\left[\frac{\partial(\rho\phi)}{\partial t} + \nabla\cdot(\rho\phi\mathbf{u})\right]\mkern3mu\mathrm{d}V  \qquad (21)
$$

Eq. (21) is a mathematical identity for differentiating an integral over a moving domain, not another transport or conservation law. Applied to Eq. (20), it gives the same time derivative and advective divergence expression found in Eq. (19). The divergence term accounts for the moving boundary when expressing the rate in spatial coordinates; it does not mean fluid crosses the material boundary. Fluid motion and its advective effect have not disappeared.

For sufficiently smooth fields, the product rule gives

$$
\frac{\partial(\rho\phi)}{\partial t} + \nabla\cdot(\rho\mathbf{u}\phi) = \rho\frac{D\phi}{Dt} + \phi\left[\frac{\partial\rho}{\partial t}+\nabla\cdot(\rho\mathbf{u})\right]  \qquad (22)
$$

In Eq. (22), $D\phi/Dt=\partial\phi/\partial t+\mathbf{u}\cdot\nabla\phi$ follows a fluid particle; the second term shows the advective effect of moving through spatial variations. The bracket is the continuity-equation residual, which vanishes under source-free mass conservation. Applying the divergence theorem to the non-advective flux in Eq. (20) and localizing the balance then gives the material-derivative form, provided the fields are sufficiently smooth

$$
\boxed{\rho\frac{D\phi}{Dt} = -\nabla\cdot\mathbf{j}_\phi + S_\phi}  \qquad (23)
$$

The roles are distinct: the divergence theorem converts surface fluxes to volume divergences; the Reynolds transport theorem rewrites the rate of an integral over a moving volume; the product rule gives Eq. (22); and the continuity equation removes its residual when mass has no volumetric source. Under that assumption and sufficient smoothness, Eq. (23) is an equivalent rewriting of the Eulerian conservative law, Eq. (19). The name **non-conservative form** describes its algebraic form, not a physical failure of conservation.

The fixed-volume integral balance, Eq. (17), remains the preferred starting point for the finite-volume method: fluxes through a shared face enter neighboring cells with opposite signs and cancel in their combined balance. The integral conservation law also applies in situations with discontinuities, where the pointwise smooth-field manipulations above may not be valid.

# Gradient-diffusion model

For an isotropic medium, a common constitutive approximation is

$$
\mathbf{j}_\phi = -\Gamma_\phi\nabla\phi, \qquad \Gamma_\phi \geq 0  \qquad (24)
$$

In Eq. (24), $`\Gamma_\phi`$ is the effective diffusion coefficient for the chosen scalar. The minus sign makes the diffusive flux point down the scalar gradient. Substitution into Eq. (19) gives the conservative scalar transport equation

$$
\boxed{\frac{\partial(\rho\phi)}{\partial t} + \nabla\cdot(\rho\mathbf{u}\phi) = \nabla\cdot(\Gamma_\phi\nabla\phi) + S_\phi}  \qquad (25)
$$

The terms in Eq. (25) are transient accumulation, net outward advective transport, diffusion, and volumetric production, from left to right. The diffusion term is a **net diffusive influx per unit volume** because the outward diffusive flux is $`-\Gamma_\phi\nabla\phi`$. The coefficient may vary in space or time; it must remain inside the divergence in general. Eq. (24) is a model, not a consequence of conservation, and more general transport can require a tensor coefficient or additional flux terms.

Using $Q$ for the unit dimension of the transported quantity, $\rho\phi$ has units $Q/\mathrm{m}^3$, each flux has units $Q/(\mathrm{m}^2\mkern3mu\mathrm{s})$, and every term in Eq. (25), including $`S_\phi`$, has units $Q/(\mathrm{m}^3\mkern3mu\mathrm{s})$. The units of $`\Gamma_\phi`$ depend on the definition of $\phi$. For a species mass fraction, for example, $Q$ denotes the dimension of species mass and a common model is $`\Gamma_\phi=\rho D`$, where $D$ is a mass diffusivity.

If mass has a volumetric source $S_m$, so that $`\partial\rho/\partial t+\nabla\cdot(\rho\mathbf{u})=S_m`$, Eq. (22) instead converts Eq. (25) to

$$
\rho\frac{D\phi}{Dt} = \nabla\cdot(\Gamma_\phi\nabla\phi)+S_\phi-\phi S_m  \qquad (26)
$$

The last term vanishes when $S_m=0$. For constant density and source-free mass conservation, $\nabla\cdot\mathbf{u}=0$ and accumulation and advection reduce to $\rho(\partial\phi/\partial t+\mathbf{u}\cdot\nabla\phi)$. Constant density alone does not justify discarding a mass source.

# Integral Form of the Scalar Transport Equation

Rearranging the general scalar transport equation in Eq. (19) places the non-advective flux divergence on the left:

$$
\frac{\partial(\rho\phi)}{\partial t} + \nabla\cdot(\rho\mathbf{u}\phi) + \nabla\cdot\mathbf{j}_\phi = S_\phi  \qquad (27)
$$

Integrating Eq. (27) over an arbitrary fixed three-dimensional control volume $`V_P`$ gives

$$
\int_{V_P}\frac{\partial(\rho\phi)}{\partial t}\mkern3mu\mathrm{d}V + \int_{V_P}\nabla\cdot(\rho\mathbf{u}\phi)\mkern3mu\mathrm{d}V + \int_{V_P}\nabla\cdot\mathbf{j}_\phi\mkern3mu\mathrm{d}V = \int_{V_P}S_\phi\mkern3mu\mathrm{d}V  \qquad (28)
$$

Let $`\partial V_P`$ be the closed boundary of $`V_P`$ and $\mathbf{n}$ its outward unit normal. Applying the Gauss divergence theorem to both divergence terms in Eq. (28), and interchanging the time derivative and volume integral for sufficiently smooth fields, gives

$$
\boxed{\frac{\mathrm{d}}{\mathrm{d}t}\int_{V_P}\rho\phi\mkern3mu\mathrm{d}V + \oint_{\partial V_P}\rho\phi\mathbf{u}\cdot\mathbf{n}\mkern3mu\mathrm{d}A + \oint_{\partial V_P}\mathbf{j}_\phi\cdot\mathbf{n}\mkern3mu\mathrm{d}A = \int_{V_P}S_\phi\mkern3mu\mathrm{d}V}  \qquad (29)
$$

Eq. (29) is the conservation law in its **integral form** and the starting point for the finite-volume method (FVM). Its left side contains accumulation and net outward advective and non-advective fluxes; its right side is the integrated source. The quantity $`\mathbf{j}_\phi\cdot\mathbf{n}`$ is the outward non-advective flux, so fluxes through a shared face enter neighboring cells with opposite signs.

The gradient-diffusion model in Eq. (24) defines $`\mathbf{j}_\phi=-\Gamma_\phi\nabla\phi`$, where $`\Gamma_\phi`$ is the effective diffusion coefficient for the scalar $\phi$. Substituting Eq. (24) into the non-advective surface integral in Eq. (29) gives $`+\oint_{\partial V_P}\mathbf{j}_\phi\cdot\mathbf{n}\mkern3mu\mathrm{d}A=-\oint_{\partial V_P}\Gamma_\phi\nabla\phi\cdot\mathbf{n}\mkern3mu\mathrm{d}A`$. The minus sign comes from the constitutive law for the diffusive flux; it is not a separate sign convention for outward flux.

# Discretization of Surface Integrals

The **face-based approach** connects the continuous surface integrals in Eq. (29) to face contributions in ApexFVM. For a vector flux $\mathbf{F}$, the closed surface integral over the boundary of cell $P$ is approximated by

$$
\oint_{\partial V_P}\mathbf{F}\cdot\mathbf{n}\mkern3mu\mathrm{d}A \approx \sum_{f\in\mathrm{faces}(P)}\mathbf{F}_f\cdot\mathbf{S}_{P,f}  \qquad (36)
$$

In Eq. (36), $`\mathbf{F}_f`$ is the representative flux at face $f$, and $`\mathbf{S}_{P,f}`$ is the face area vector directed outward from cell $P$. The **face area vector** is defined by

$$
\mathbf{S}_f = A_f\mathbf{n}_f  \qquad (37)
$$

In Eq. (37), $`A_f`$ is the face area and $`\mathbf{n}_f`$ is the unit normal. The library convention directs $`\mathbf{S}_f`$ from the Owner to the Neighbor for an internal face. Thus $`\mathbf{S}_{P,f}=\mathbf{S}_f`$ for the owner cell and $`\mathbf{S}_{P,f}=-\mathbf{S}_f`$ for the neighbor cell. A boundary face vector points outward from its owner. Shared face contributions therefore have opposite signs in the two cell balances.

Using outward face vectors for cell $P$, the advective flux is discretized as

$$
\oint_{\partial V_P}\rho\phi\mathbf{u}\cdot\mathbf{n}\mkern3mu\mathrm{d}A \approx \sum_{f\in\mathrm{faces}(P)}(\rho\phi\mathbf{u})_f\cdot\mathbf{S}_{P,f} \equiv \sum_{f\in\mathrm{faces}(P)}F_{P,f}\phi_f  \qquad (38)
$$

In Eq. (38), the equivalence symbol defines the face-based numerical scheme factorization, with face scalar value $`\phi_f`$ and mass flux from cell $P$'s perspective defined by

$$
F_{P,f} = (\rho\mathbf{u})_f\cdot\mathbf{S}_{P,f}  \qquad (39)
$$

The mass flux in Eq. (39) is positive for flow out of cell $P$. Relative to the global face orientation $`F_f=(\rho\mathbf{u})_f\cdot\mathbf{S}_f`$, it equals $`F_f`$ for the owner and reverses sign for the neighbor. The face values are supplied by the chosen numerical scheme.

The non-advective flux is likewise approximated using outward face vectors:

$$
\oint_{\partial V_P}\mathbf{j}_\phi\cdot\mathbf{n}\mkern3mu\mathrm{d}A \approx \sum_{f\in\mathrm{faces}(P)}(\mathbf{j}_\phi)_f\cdot\mathbf{S}_{P,f}  \qquad (40)
$$

Substituting the gradient-diffusion model $`\mathbf{j}_\phi=-\Gamma_\phi\nabla\phi`$ from Eq. (24) into Eq. (40) gives

$$
-\oint_{\partial V_P}\Gamma_\phi\nabla\phi\cdot\mathbf{n}\mkern3mu\mathrm{d}A \approx -\sum_{f\in\mathrm{faces}(P)}(\Gamma_\phi\nabla\phi)_f\cdot\mathbf{S}_{P,f}  \qquad (41)
$$

The minus sign in Eq. (41) comes from the constitutive diffusion law, not a separate outward-normal sign convention. These discrete face contributions, combined with accumulation and source terms, form the algebraic system for cell $P$ and connect the continuous theory to the multi-dimensional framework.
