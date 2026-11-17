# CFD Toolkit Theory Guide

CFD Toolkit is a modular finite-volume framework with a clean separation of dimensions for solving transport equations. This guide explains the mathematical ideas, conventions, and discretization choices behind the library, connecting the governing equations to their numerical representation and implementation.

The guide begins with the general transport equation and develops its terms and discretization step by step. Each topic is introduced as it becomes relevant to the library, so the theory remains connected to the code and its current capabilities.

# 1.Derivation of the Conservative Scalar Transport Equation

## 1.1 Motivation and Scope

A finite-volume method (FVM) discretizes *conservation statements over control volumes*, not differential equations directly. Every sign in the assembled matrix therefore has to trace back to a physical balance and to a clear convention for the direction of the surface normal. The purpose of this document is to make that chain explicit: from a physical balance over a material system, through the Reynolds transport theorem (RTT) and the divergence theorem, to the discrete two-point flux, and finally to the exact entries of the matrix $A \phi = b$.
