# Formula Writing Rules

Use these rules when writing or revising mathematical formulas in a Markdown document. They capture the conventions demonstrated in `Theory_Guide.md`.

## Inline formulas

- Put a formula that appears within a sentence between single dollar signs: `$...$`.
- Keep short mathematical expressions inline when they fit naturally into the prose. For example: `If $\mathbf{F}=\rho\mathbf{u}$ is mass flux, ...`.
- Keep sentence punctuation outside the closing `$` unless the punctuation is part of the mathematical expression itself.
- Use LaTeX commands for mathematical notation: for example, `\mathbf{F}` for a bold vector, `\nabla \cdot \mathbf{F}` for divergence, and `\mathrm{d}V` for a differential element.

## Displayed formulas

- Put an important or multi-part equation on its own lines, with a blank line before and after it.
- Use a pair of `$$` delimiters, each on a line by itself. Put the LaTeX expression between them, for example:

  ```markdown
  $$
  \int_V \nabla \cdot \mathbf{F}\,\mathrm{d}V = \oint_{\partial V} \mathbf{F} \cdot \mathbf{n}\,\mathrm{d}A  \qquad (1)
  $$
  ```

- Append the equation number inside the display, at the right end of the formula, using `\qquad (n)`. The guide numbers equations with plain parenthesized integers: `(1)`, `(2)`, `(3)`.
- Number displayed equations consecutively within the document. Refer to a numbered formula in prose as `Eq. (n)`, preserving the capitalization and period.
- Do not add a second equation number in the surrounding prose or put the number outside the `$$` delimiters.

## Notation and prose around formulas

- Use conventional LaTeX math commands and keep notation consistent across related formulas. The guide uses `\int`, `\oint`, `\nabla`, `\cdot`, `\partial`, `\frac`, `\mathrm{d}`, and `\mathbf`.
- Use subscripts for coordinate components and locations where needed, such as `$F_x$`, `$x_w$`, and `$x_e$`.
- Introduce a displayed equation with a short sentence or clause when the surrounding explanation calls for it (for example, “it states that”). Explain important symbols and sign conventions in the prose immediately around the equation.
- Write equation references as part of a sentence, such as “In Eq. (2), ...” or “Eq. (1) says that ...”. Ensure the referenced number matches the display.
- Preserve the mathematical meaning, assumptions, definitions, units, and sign conventions of the source when editing formulas. Do not change a formula merely to make its notation match a preference; clarify or flag an inconsistency instead.

## Applying these rules to another file

1. Identify which expressions belong inline and which should be displayed separately.
2. Format inline expressions with `$...$`; format standalone equations with a blank line around a `$$` block.
3. Number displayed equations in order using `\qquad (n)` inside each display, then update every prose reference to `Eq. (n)`.
4. Check that the LaTeX delimiters are balanced, symbols and subscripts are consistent, and punctuation reads correctly around each formula.
5. Verify that formatting edits preserve the source's mathematics and that each explanatory statement and equation reference agrees with the formula.
