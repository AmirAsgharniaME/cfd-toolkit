# Formula Writing Rules

These rules describe how mathematical formulas are written in `Theory_Guide.md`. When asked to apply them to another Markdown file, follow every rule below and change only the formula formatting and the equation references. Keep the mathematics, wording, and meaning of the file unchanged.

## 1. Inline formulas (inside a sentence)

1. Wrap every mathematical symbol or expression that appears in running text in single dollar signs: `$...$`. This includes a lone symbol such as `$V$`, `$A$`, or `$\partial V$`, and also short expressions such as `$n_w=-1$` or `$[x_w,x_e]$`.
2. Write no space just inside the delimiters. Write `$V$`, not `$ V $`.
3. Leave sentence punctuation outside the closing `$`: `... on a volume $V$,` and `... $A[F_x(x_e)-F_x(x_w)]$.`
4. Keep an inline formula on one line. Never split it across lines.
5. Use inline math for definitions, sign conventions, intervals, and short relations (`$\mathbf{F}=\rho\mathbf{u}$`, `$n_e=+1$`). Use a displayed equation (section 2) for a main result or a long expression.
6. Do not write math symbols as plain text, such as `F_x`, `div F`, or `del . F`, in prose. Convert them to LaTeX inside `$...$`.
7. Write words and units in plain text outside the math (`mass per unit volume per unit time`). Put only symbols inside `$...$`.

## 2. Displayed formulas (on their own lines)

1. Put a displayed equation in a block with a blank line before it and a blank line after it.
2. Put `$$` alone on the opening line and `$$` alone on the closing line. Put the LaTeX on the line between them. Never write `$$ ... $$` on a single line.

   ```markdown
   $$
   \nabla \cdot \mathbf{F} = \frac{\partial F_x}{\partial x} + \frac{\partial F_y}{\partial y} + \frac{\partial F_z}{\partial z}  \qquad (2)
   $$
   ```

3. Introduce the equation with a lead-in clause that ends without a period, so the sentence continues into the display, for example `..., it states that`, `In Cartesian coordinates, the divergence is`, or `For an interval $[x_w,x_e]$ and a scalar flux component $F_x$,`.
4. Do not end the displayed LaTeX with a period or comma.
5. Use a display for each numbered equation. Do not number inline formulas.

## 3. Equation numbering

1. Give each displayed equation a number placed at the right end of the same LaTeX line: two spaces, then `\qquad (n)`.

   ```
   <expression>  \qquad (n)
   ```

2. Use plain integers in parentheses, in order of appearance: `(1)`, `(2)`, `(3)`, and so on. Do not skip or repeat numbers. Do not use section-based numbers such as `(2.1)` and do not use `\tag{}` or `\label{}`.
3. Keep the number inside the `$$` block. Never put it in prose next to the display.
4. If an equation is added, removed, or reordered, renumber all following displays and update every reference in the text.

## 4. Referring to equations in text

1. Write references as `Eq. (n)` in plain text (capital E, period after `Eq`, a space, then the number in parentheses). Do not wrap the reference in `$...$`.
2. Use the reference as part of a sentence: `In Eq. (1), ...`, `The divergence in Eq. (2) measures ...`, `Eq. (1) says that ...`, `The signs in Eq. (3) follow from ...`.
3. Every `Eq. (n)` must match a displayed equation numbered `(n)`.
4. Explain the new symbols and sign conventions of an equation in the sentences right after it, and refer back to it with `Eq. (n)`.

## 5. LaTeX notation conventions

| Element | Convention | Example |
| --- | --- | --- |
| Vector quantity | `\mathbf{...}` (bold upright) | `\mathbf{F}`, `\mathbf{n}`, `\mathbf{u}` |
| Scalar or region symbol | Plain italic letter, no wrapper | `V`, `A`, `x` |
| Component or location | Subscript, with braces when longer than one character | `F_x`, `x_w`, `x_e`, `n_w`, `\frac{\partial F_x}{\partial x}` |
| Differential | Upright `\mathrm{d}` preceded by a thin space `\,` | `\,\mathrm{d}V`, `\,\mathrm{d}A`, `\,\mathrm{d}x` |
| Derivative | `\frac{\mathrm{d}F_x}{\mathrm{d}x}` for ordinary, `\frac{\partial F_x}{\partial x}` for partial | |
| Divergence | `\nabla \cdot \mathbf{F}` | |
| Dot product | `\cdot` | `\mathbf{F} \cdot \mathbf{n}` |
| Volume integral | `\int_V` | |
| Closed surface integral | `\oint_{\partial V}` | |
| Definite integral limits | `\int_{x_w}^{x_e}` | |
| Boundary of a region | `\partial V` | |
| Fractions | `\frac{numerator}{denominator}` | |
| Evaluation at a point | Function-style parentheses | `F_x(x_e)` |
| Grouping | Square brackets around a parenthesized difference | `A[F_x(x_e)-F_x(x_w)]` |
| Greek letters | LaTeX commands | `\rho` |
| Interval | Square brackets | `[x_w,x_e]` |
| Signs in values | Write the sign explicitly | `n_w=-1`, `n_e=+1` |

Additional notes:

- Put the thin space `\,` before every differential and between an integrand and its differential, as in `\mathbf{F}\,\mathrm{d}A`.
- Use `\cdot` for dot products and divergence. Do not use `.` or `*`.
- Spaces around `=`, `+`, and `-` in the source are optional. Displayed equations usually have spaces around operators, while compact inline formulas may omit them (`$n_w=-1$`). Use the same style inside one formula.
- Keep a symbol the same everywhere it appears: the same letter, the same bold or plain style, and the same subscript.
- Use `\mathbf` for a vector wherever it appears, inline or displayed (`$\mathbf{F}$`).

## 6. Text around formulas

- State what a symbol means the first time it appears, in the same or the next sentence (`$\mathbf{n}$ is the **outward unit normal to the boundary**`).
- Use bold (`**...**`) for key physical terms in prose, never inside math, and only to emphasize a defining term.
- State sign conventions and assumptions in prose directly after the equation they belong to.
- Explain units in words outside the math.
- Keep the prose a normal paragraph. Do not put prose inside `$$` blocks.

## 7. Procedure for applying the rules to another file

1. Find every mathematical expression in the file: symbols, equations, derivatives, integrals, and plain-text pseudo-math.
2. Classify each one. Short symbols and relations in a sentence are inline (section 1). Main results, long expressions, and anything that needs an equation number are displayed (section 2).
3. Rewrite inline items as `$...$` and displayed items as a blank-line-separated `$$` block with each delimiter alone on its line.
4. Number the displayed equations in order with `  \qquad (n)` inside the block, starting at `(1)`.
5. Change every textual reference to `Eq. (n)` with the matching number, and add `Eq. (n)` references where the text points to an equation.
6. Apply the notation conventions (section 5): `\mathbf` for vectors, `\mathrm{d}` with `\,` for differentials, `\partial`, `\nabla`, `\cdot`, and consistent subscripts.
7. Check the result against the checklist below.
8. Keep the original mathematics, assumptions, units, and sign conventions. If a formula looks inconsistent or wrong, leave it and report it rather than silently correcting it.

## 8. Final checklist

- [ ] Every inline formula is in `$...$` with no inner padding and with sentence punctuation outside it.
- [ ] Every displayed formula has a blank line before and after, and `$$` alone on both delimiter lines.
- [ ] No displayed formula ends with a period or comma.
- [ ] Each displayed equation ends with `  \qquad (n)` inside the block, with consecutive integers from `(1)`.
- [ ] Every textual reference is `Eq. (n)`, outside math, and points to the right equation.
- [ ] Vectors use `\mathbf`, differentials use `\,\mathrm{d}`, and symbols are consistent.
- [ ] All `$` and `$$` delimiters are balanced and the LaTeX is valid.
- [ ] No mathematical content was changed.
