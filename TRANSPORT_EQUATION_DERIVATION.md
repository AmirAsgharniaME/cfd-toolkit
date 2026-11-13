# Derivation of the Conservative Scalar Transport Equation

A self-contained, sign-explicit derivation for the finite-volume solver in this project.

---

## 1. Motivation and Scope

A finite-volume method (FVM) discretizes *conservation statements over control volumes*, not differential
equations directly. Every sign in the assembled matrix therefore has to trace back to a physical balance and
to a clear convention for the direction of the surface normal. The purpose of this document is to make that
chain explicit: from a physical balance over a material system, through the Reynolds transport theorem (RTT)
and the divergence theorem, to the discrete two-point flux, and finally to the exact entries of the matrix
`A phi = b`.

The derivation is a sequence of *three logically distinct ingredients*:

| # | Ingredient | Status | Content |
|---|------------|--------|---------|
| 1 | **Balance law (conservation)** | **Postulated** (physical principle) | The conserved amount inside a material system changes only because of what crosses its boundary and what is produced inside it. |
| 2 | **Kinematics / RTT** | **Derived** (mathematics) | Relates the rate of change of an integral over a moving material volume to integrals over a fixed volume plus boundary flux. |
| 3 | **Constitutive law** | **Postulated** (empirical model) | A relation between the diffusive flux of the transported quantity and its gradient, e.g. Fick's or Fourier's law. |

Only step 2 is mathematics that can be *proved*. Step 1 is a physical axiom; step 3 is a modeling assumption
supported by experiment and by deeper theory, but it is **not** a consequence of conservation. In particular,
**conservation alone does not imply Fick's law**, and Fick's law does not follow from the divergence theorem.
What conservation guarantees is a specific *structure* of the equation (a divergence-form balance); the
constitutive law fills in the quantitative closure, and the sign in the constitutive law is a separate
modeling choice that must not be confused with the sign that arises from moving an outward flux to the left
hand side of the balance (see §3 and §6).

**Scope.** One conserved scalar, single phase, isotropic medium, fixed (Eulerian) control volume `V` with
boundary `∂V`. Compressible flow is allowed (density stays inside the time derivative and inside the
advective term). Non-isotropic/anisotropic media, non-orthogonal meshes, and multiphase interfaces are
discussed only where they affect the sign bookkeeping (§7).

---

## 2. Notation and Units

Let `[X]` denote the SI unit of a quantity `X`.

| Symbol | Meaning | SI unit |
|--------|---------|---------|
| `phi` | conserved *specific* amount, i.e. amount per unit mass | `[phi]` (e.g. `J/kg`, `kg/kg`, `mol/kg`) |
| `rho` | density of the carrier material | `kg/m^3` |
| `u` | flow velocity field of the carrier material | `m/s` |
| `rho phi` | conserved amount per unit *volume* | `[phi] kg/m^3` |
| `V` | fixed, arbitrary control volume (stationary in the chosen frame, `∂V` fixed) | `m^3` |
| `n` | **outward** unit normal of `∂V` (points out of the CV) | dimensionless |
| `J_diff` | diffusive flux of the amount, measured **relative to the material motion** | `[phi] kg/(m^2 s)` |
| `S_phi` | volumetric production (source) of the conserved amount | `[phi] kg/(m^3 s)` |
| `Gamma` | material diffusion property for the transported amount (diffusivity-like, positive) | so that `Gamma grad phi` has units of `J_diff`, see §5 |
| `A_f` | area of face `f` | `m^2` |
| `d_PN` | distance between centers `P` and `N` measured along the line joining them (positive) | `m` |
| `D_f` | face conductance/coefficient, `D_f = Gamma_f A_f / d_PN > 0` | such that `D_f * phi` has units of "amount per time" |

Consistency check for units (used repeatedly below):

- `int_V rho phi dV`: `[phi] kg/m^3 * m^3 = [phi] kg` — an *amount*.
- `∫_∂V rho phi u · n dA`: `([phi] kg/m^3)(m/s)(m^2) = [phi] kg/s` — rate of amount.
- `∫_∂V J_diff · n dA`: `[phi] kg/(m^2 s) * m^2 = [phi] kg/s` — rate of amount.
- `∫_V S_phi dV`: `[phi] kg/(m^3 s) * m^3 = [phi] kg/s` — rate of amount.

All four terms of the eventual balance have the same unit; any candidate equation that fails this check is
wrong regardless of its signs.

---

## 3. Material-System Balance and the Fixed-CV Statement

### 3.1 The physical postulate on a material system

A **material (Lagrangian) system** `V_m(t)` is a collection of the same material particles for all time; its
boundary moves with the material velocity `u`. The conservation principle states:

> The rate of change of the conserved amount inside `V_m(t)` equals the amount that diffuses in through its
> boundary (relative to the material) plus the amount produced inside it.

In symbols, with `n` the outward normal of `∂V_m(t)`:

$$
\frac{d}{dt}\int_{V_m(t)} \rho \phi \, dV
  \;=\; - \int_{\partial V_m(t)} J_{\text{diff}} \cdot n \, dA
  \;+\; \int_{V_m(t)} S_\phi \, dV. \tag{3.1}
$$

**Why the minus sign here.** `J_diff · n` is the *outward* component of the diffusive flux, so
`∫ J_diff · n dA` is the net amount of the quantity **leaving** the system per unit time by diffusion. Leaving
reduces the inventory, hence it enters the rate-of-change statement with a minus sign. This minus sign has
nothing to do with the material property `Gamma`: it is the sign of "loss on the right-hand side of a
conservation statement". It appears for every quantity that can leave a volume, whatever the physics.

**What is postulated vs. derived in (3.1).** The *form* of (3.1) is the physical postulate. Its individual
terms are modeling objects: `J_diff` is whatever microscopic/collisional transport is present (measured
relative to the moving material), and `S_phi` collects all volumetric production/consumption. Neither is
derived from conservation. In particular, (3.1) is compatible with *any* constitutive relation for `J_diff`.

### 3.2 Reynolds transport theorem

For a smooth scalar field `f(x,t)` and a material volume `V_m(t)` whose boundary moves with velocity `u`,

$$
\frac{d}{dt}\int_{V_m(t)} f \, dV
\;=\; \int_{V_m(t)} \frac{\partial f}{\partial t} \, dV
  \;+\; \int_{\partial V_m(t)} f \, u \cdot n \, dA. \tag{3.2}
$$

Equation (3.2) is pure kinematics: it states that the change of the integral over the moving volume is the
change of the integrand (holding the volume fixed) plus the amount swept across the moving boundary because the
boundary itself is moving with velocity `u`. It holds for the same fixed normal convention `n` (outward).

Applying (3.2) with `f = rho phi` to the left-hand side of (3.1), and noting that `∂V_m(t)` and `V_m(t)` can be
replaced by any fixed volume `V` that *coincides* with the material system at the instant considered (the
integrals depend on position, not on whether the surface is instantaneously moving), we obtain for that
instant:

$$
\int_{V} \frac{\partial (\rho\phi)}{\partial t}\, dV
  \;+\; \int_{\partial V} \rho\phi\, u\cdot n \, dA
  \;=\; - \int_{\partial V} J_{\text{diff}}\cdot n \, dA
  \;+\; \int_{V} S_\phi \, dV. \tag{3.3}
$$

### 3.3 The fixed (Eulerian) CV statement

Moving the two surface integrals to the left-hand side gives the **integral form on a fixed control volume**
(for arbitrary fixed `V`):

$$
\boxed{
\frac{d}{dt}\int_{V} \rho \phi \, dV
  \;+\; \int_{\partial V} \rho \phi\, u \cdot n \, dA
  \;+\; \int_{\partial V} J_{\text{diff}} \cdot n \, dA
  \;=\; \int_{V} S_\phi \, dV
} \tag{3.4}
$$

**Same transport theorem, same reasoning.** Equation (3.4) does not require a new physical assumption beyond
(3.1): it is (3.1) rewritten for a fixed volume using (3.2). The *content* is the same inventory balance;
what changed is the bookkeeping. In (3.4) both surface terms are written with the **outward** normal `n` and
both carry a **plus** sign on the left-hand side: they are net *outgoing* rates. On the right-hand side of
(3.1) the diffusive term carried a minus sign because it was phrased as "loss"; on the left-hand side of (3.4)
it carries a plus sign because it is phrased as "outflow". Moving a term across the equals sign flips its sign;
this is arithmetic, not physics.

So there are now **two** surface terms that both quantify outflow:

- `∫_∂V ρφ u · n dA` — **advective** outflow, the quantity carried out by the bulk motion of the material
  (it arose from the RTT term in (3.2));
- `∫_∂V J_diff · n dA` — **diffusive** outflow, the quantity carried out by transport relative to the
  material.

**Important clarification.** In (3.4) both surface terms are written with an *explicit plus sign in front of an
outward-normal integral*. This plus sign is the sign of "outflow on the left-hand side". It must not be
confused with the minus sign inside the constitutive law `J_diff = -Gamma grad phi` (§5), which is a
*modeling* statement about the direction of diffusion (down the gradient). The two minus signs are unrelated:
one comes from algebra (moving a loss to the other side), the other from the material response. Section 6
contains a full ledger.

---

## 4. Divergence Theorem and Localization

### 4.1 Divergence theorem

For a sufficiently smooth vector field `F` and a volume `V` with piecewise smooth boundary and outward normal
`n`:

$$
\int_{\partial V} F \cdot n \, dA \;=\; \int_{V} \nabla \cdot F \, dV. \tag{4.1}
$$

Applied to (3.4) with `F = rho phi u` and `F = J_diff` (the theorem is linear, so it can be applied to each
surface term independently):

$$
\int_{V} \frac{\partial (\rho\phi)}{\partial t}\, dV
+ \int_{V} \nabla \cdot (\rho \phi u) \, dV
+ \int_{V} \nabla \cdot J_{\text{diff}} \, dV
= \int_{V} S_\phi \, dV. \tag{4.2}
$$

**No sign is absorbed by the divergence theorem.** (4.1) maps an outward-normal surface integral to the volume
integral of the divergence with the *same* sign. The theorem does not change a plus sign into a minus sign or the reverse. A sign flip in
moving from a surface statement to a volume statement, if one appears, must have come from the *reasoning*
beforehand (as in §3.3) or from the constitutive law (§5), never from (4.1). This is an explicit correction to
any earlier statement suggesting that "Gauss's theorem swallows the minus sign": it does not, and it cannot,
because (4.1) is an identity with no free sign parameter.

### 4.2 Localization

Because `V` is arbitrary (4.2) must hold for every admissible sub-volume. If the integrand combination is
continuous, the standard localization argument (shrink `V` to a point; the mean value theorem makes the volume
average tend to the point value) yields the **differential (strong) form**:

$$
\boxed{
\frac{\partial (\rho \phi)}{\partial t}
+ \nabla \cdot (\rho \phi u)
+ \nabla \cdot J_{\text{diff}}
= S_\phi
} \tag{4.3}
$$

This is still purely a consequence of the balance postulate (plus mathematics). It is the conservation form:
all flux terms are inside divergences. The FVM discretization in §7–§9 will not use (4.3) directly; it will use
the *surface* form (3.4), which is the primary object of the method and remains valid even when the fields are
not smooth (shocks, interfaces, discontinuities between material properties), where (4.3) is not defined in the
classical sense.

---

## 5. Constitutive Law and the Final Equation

### 5.1 Fick/Fourier-type closure

Diffusion is a *material response*: the transported amount flows from regions of higher `phi` to regions of
lower `phi`, at a rate proportional to the gradient. In an isotropic medium this is expressed as

$$
\boxed{\,J_{\text{diff}} \;=\; -\,\Gamma\, \nabla \phi\,}, \qquad \Gamma > 0, \tag{5.1}
$$

where `Gamma` is the appropriate material diffusion property (a scalar, possibly field-dependent, e.g.
`Gamma = Gamma(x, phi)`). For mass diffusion `Gamma` is the (mass-weighted) diffusivity; for heat transfer
written in the form below, `phi` must be chosen so that `rho phi` is the enthalpy density; see §5.4.

Substituting (5.1) into (4.3):

$$
\frac{\partial (\rho \phi)}{\partial t}
+ \nabla \cdot (\rho \phi u)
- \nabla \cdot (\Gamma \nabla \phi)
= S_\phi. \tag{5.2}
$$

Moving the diffusion term to the right-hand side gives the **standard alternative form**

$$
\frac{\partial (\rho \phi)}{\partial t}
+ \nabla \cdot (\rho \phi u)
= \nabla \cdot (\Gamma \nabla \phi) + S_\phi. \tag{5.3}
$$

**Algebraic equivalence.** (5.2) and (5.3) differ only by the sign in front of `div(Gamma grad phi)`; adding
`+div(Gamma grad phi)` to both sides of (5.2) yields (5.3). This is arithmetic. Physically, on the right-hand
side of (5.3) the term `div(Gamma grad phi)` plays the role of a *diffusive source*: it is positive where the
medium is a net source of the amount, which happens around local minima of `phi`. Which of the two forms one
calls "the equation" is a convention; both are the same conservation statement. In this project, the FVM
assembly is derived from the **face-based** form (§7), which does not depend on this choice.

### 5.2 Dimensional consistency

Requirement: `[Gamma grad phi] = [J_diff]`, i.e. `[Gamma][phi]/m = [phi] kg/(m^2 s)`, hence

$$
[\Gamma] = \mathrm{kg}/(\mathrm{m\,s}) = \mathrm{Pa\,s}\quad(\text{for the generic amount }[phi]),
$$

which is a dynamic viscosity/diffusivity-type unit and is consistent with `Gamma = k/(rho c_p)` in the heat
case (§5.4) or with `Gamma = rho D` for a mass fraction (with `phi` dimensionless). The final equation
(5.2) is unit-consistent: every term has units `[phi] kg/(m^3 s)`.

### 5.3 Source sign convention

The right-hand side `S_phi` uses the **production-positive** convention:

- `S_phi > 0`: the amount is produced inside the CV; the inventory of the CV tends to grow.
- `S_phi < 0`: the amount is consumed (a sink).

This convention is fixed by (3.1). Any solver-level convention that says "`b` positive means `phi` grows" is
consistent with it, whereas a sink implemented as a positive number is not. Note also the popular practice of
linearizing a sink as `S_phi = S_u - S_p phi` with `S_p >= 0`; when `S_p` is positive it must be added to the
matrix diagonal (`+S_p V` in the `A phi = b` system) and `S_u V` to the RHS — that sink diagonal is a
*source linearization* term and is entirely separate from the diffusion diagonal `+D_f` of §7.

### 5.4 Particular case: heat transfer

For heat, the natural conserved density is the energy per unit volume; with `T` the temperature,

$$
\frac{\partial (\rho c_p T)}{\partial t}
+ \nabla \cdot (\rho c_p T u)
= \nabla \cdot (k \nabla T) + S_h,
$$

which is (5.3) with `phi = c_p T` and `Gamma = k`, provided `rho phi = rho c_p T` and `c_p` is the
specific heat consistent with the energy form being used. Equivalently, with `phi = T`, `Gamma = k/c_p` and
`rho -> rho c_p` — but then the time derivative and the advective term must carry the `c_p`, so
**`phi` and the heat capacity must be defined consistently**; mixing `phi = T` in the diffusion term with an
`rhs` expressed in enthalpy units produces unit-inconsistent results even when all signs are correct.

---

## 6. The Stationary, Non-Advective Case and a Correct Sign Ledger

### 6.1 The steady diffusion equation

For a steady state (`∂/∂t = 0`) without advection (`u = 0`), (5.2) reduces to

$$
\boxed{\,-\,\nabla \cdot (\Gamma \nabla \phi) \;=\; S_\phi\,} \tag{6.1}
$$

The **explicit minus** before the divergence in (6.1) comes *exclusively* from extracting the negative of the
constitutive law (5.1), `J_diff = -Gamma grad phi`, into the equation. The conservation balance itself (3.4)
produced the divergence of the diffusive flux with a **plus** sign on the left-hand side; substituting the
constitutive law then inserted the minus. Replace the constitutive law by its general form and the plus returns:

$$
\nabla \cdot J_{\text{diff}} = S_\phi \quad\Longleftrightarrow\quad J_{\text{diff}} = -\Gamma\nabla\phi \;\Rightarrow\; -\nabla\cdot(\Gamma\nabla\phi) = S_\phi . \tag{6.2}
$$

### 6.2 Sign ledger

All signs are tracked in one place. "Section" identifies where the sign first appears.

| # | Sign | Equation | Where it comes from | Nature | What it means |
|---|------|----------|--------------------|--------|---------------|
| **N1** | `J_diff · n` enters with a **minus** on the RHS | `d/dt∫ρφ = -∫J_diff·n dA + ∫S` (3.1) | Physical inventory balance | **Physical/algebraic** | Outward diffusive flux is a *loss*, so it is subtracted from the inventory rate |
| **N2** | `∫J_diff·n dA` moves to the **left** with a **plus** | (3.4) | Moving a term across `=` | Pure algebra (not new physics) | The same loss, now written as "outflow on the LHS" |
| **N3** | `+∇·J_diff` on the LHS | (4.3) | Divergence theorem (4.1) applied to the `+∫J_diff·n` term | Pure mathematics; **no sign change** | Flux divergence on the LHS has the same sign as the surface term |
| **N4** | `J_diff = -Γ∇φ` | (5.1) | Constitutive (Fick/Fourier) law | **Modeling postulate** | Diffusion is down-gradient |
| **N5** | `-∇·(Γ∇φ)` on LHS in (5.2)/(6.1) | (5.2), (6.1) | N3 **⊕** N4 combined | Consequence of N3+N4 | The minus is N4's; it is **not** N2, and **not** the divergence theorem |
| **N6** | `+Γ∇φ` appears explicitly in the flux after substituting N4 | (6.1) with linear `φ` | N4 applied once | Expressing the physical flux | In 1D with `∇φ = c > 0`, `J_diff = -Γc < 0` (down-gradient flow) |
| **N7** | Outward flux `D_f(φ_P - φ_N)` | (7.4) | N6 + geometry | Computed, not modeled | Flux written in terms of center values |

**Explicit correction of an earlier misleading statement.** It is sometimes said that "Gauss's theorem turns
the `-∫J·n` into `-∫∇·J`" or that the theorem "absorbs" the sign. That is wrong. In (3.1) the term had a
minus *because it stood on the right-hand side as a loss*; the theorem (4.1) then maps the surface integral to
the volume integral **preserving the sign**, giving `-∫∇·J` if the surface term was `-∫J·n`, or `+∫∇·J` if
it was `+∫J·n` (as in (3.4)). In both cases the ultimate minus in front of `div(Γ∇φ)` in (5.2)/(6.1) originates
solely from the constitutive law (5.1). N1/N2 are bookkeeping on the balance; N4 is physics; N3 is an
identity. Exactly **two independent negative signs** are at work, and neither can be removed by an
algebraic identity.
