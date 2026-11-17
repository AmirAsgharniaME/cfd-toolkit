# CFD Toolkit Theory Guide

CFD Toolkit is a modular finite-volume framework with a clean separation of dimensions for solving transport equations. This guide explains the mathematical ideas, conventions, and discretization choices behind the library, connecting the governing equations to their numerical representation and implementation.

The guide begins with the general transport equation and develops its terms and discretization step by step. Each topic is introduced as it becomes relevant to the library, so the theory remains connected to the code and its current capabilities.


# 1. The Divergence Theorem

The divergence theorem, also known as Gauss's theorem, relates the behavior of a vector field inside a volume to its flux through the volume's boundary. For a vector field $\mathbf{F}$ defined on a volume $V$ with closed boundary $\partial V$, it states that

$$
\int_V \nabla \cdot \mathbf{F}\,\mathrm{d}V
= \oint_{\partial V} \mathbf{F} \cdot \mathbf{n}\,\mathrm{d}A.
\tag{1.1}
$$

In Equation (1.1), $\mathbf{n}$ is the **outward unit normal to the boundary**, and $\mathbf{F} \cdot \mathbf{n}$ is the outward flux per unit area. The theorem applies, for example, when $V$ has a piecewise smooth boundary and $\mathbf{F}$ has continuous first derivatives on a region containing $V$. Flux leaving the volume is positive; flux entering it is negative. The surface integral therefore measures the **net outward flux**, accounting for both inflow and outflow.

In Cartesian coordinates, the divergence is

$$
\nabla \cdot \mathbf{F}
= \frac{\partial F_x}{\partial x}
+ \frac{\partial F_y}{\partial y}
+ \frac{\partial F_z}{\partial z}.
\tag{1.2}
$$

The divergence in Equation (1.2) measures the local net outward flux per unit volume. A positive divergence indicates a local excess of outward flux, while a negative divergence indicates a local excess of inward flux. Equation (1.1) says that integrating this local imbalance over the volume gives the net flux across its boundary. For example, if $\mathbf{F}=\rho\mathbf{u}$ is mass flux, the surface integral is the net mass flow rate out of $V$, and $\nabla\cdot(\rho\mathbf{u})$ has units of mass per unit volume per unit time.

In one dimension, the theorem reduces to the fundamental theorem of calculus. For an interval $[x_w,x_e]$ and a scalar flux component $F_x$,

$$
\int_{x_w}^{x_e} \frac{\mathrm{d}F_x}{\mathrm{d}x}\,\mathrm{d}x
= F_x(x_e)-F_x(x_w).
\tag{1.3}
$$

The signs in Equation (1.3) follow from the outward normals: $n_w=-1$ at the west boundary and $n_e=+1$ at the east boundary. With a constant cross-sectional area $A$, the corresponding volume integral is $A[F_x(x_e)-F_x(x_w)]$.


