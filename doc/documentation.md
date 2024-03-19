# DOCUMENTATION

## Implementation

### Deployment

#### Tableau

Is a structure of data that is a representation of a Tableau.

#### TableauFilesManager

This class aims to read and write in files a specific ***Tableau***.

#### DualSolver

This class aims to bring the Tableau from an initial condition to the optimal condition, or to check if the solution of the Primal
is feasible, unfeasible or unbounded.
This class at first reads the data from an instance of a ***TableauFilesManager*** class.
Then it checks if the matrix $[d|A]$ is a full rank matrix, otherwise linearly dependent rows are deleted and it is asserted that
$m \le n$.
The Tableau can be processed with the Dual Simplex by calling the _startDualSimplex_ method.
Here the Tableau is brought to the canonical form at first.
Then an initial dual feasible solution (reduced costs $\ge 0$) is obtained with the artificial constraint technique.
Iterations of pivoting are done till the Dual is unbounded, Primal is infeasible or the solution is optimal.

## Theory

### Notations

```math
x = \begin{pmatrix} x_{1} \\ ... \\ x_{n} \\ \end{pmatrix}
```
column vector ($x \in \mathbb{R}^n$)

```math
c^{T} = \begin{pmatrix} c_{1} & ... & c_{n} \end{pmatrix}
```
row vector

```math
c^{T}x = \sum_{j=1..n}{c_{j}x_{j}}
```
dot product

### Linear Programming Problem

It is important to consider the number of decision variables, the number of equations $m$ and the number of total variables $n$
(that is the number of decision variables plus number of slack or surplus variables, inequalities).

The linear programming problem is always examined in its _standard form_:

```math
\left \{
\begin{array}{ll}
    \min{c^{T}x}\\
    Ax = d\\
    x \ge 0
\end{array}
\right.
```

Where:
- $x$ is the column vector of the decision variables (of size $n \times 1$);
- $c^{T}$ is the row vector (transposition of the column vector $c$) of the costs of each decision variable in the objective
  function;
- $z$ (or $c^{T}x$) is the value of the objective function;
- $A$ is the coefficients matrix of the equations (of size $m \times n$);
- $d$ is the column vector of the known terms of the equations (of size $m \times 1$).

It is assumed that $A$ is a full rank matrix and that $m \le n$.

The equations are read and saved in a tabular format called Tableau.

To understand the Tableau format it is used in the program, let's consider that $z$ and $d$ can be written in:

```math
\left \{
\begin{array}{ll}
    z = c_{B}^{T}x_{B} + c_{F}^{T}x_{F}\\
    d = Bx_{B} + Fx_{F}
\end{array}
\right.
```

Where $B$ is a base.

The base is a collection of linearly independent columns of the $A$ matrix.
So, considering the assumptions we have necessary that $B$ is a base of size exactly $m \times m$.

The Tableau can be initialized in:

|     0 | $c_{B}^{T}$ | $c_{F}^{T}$ |
| ----- | ----------- | ----------- |
|   $d$ |           B |           F |

It can be reached:

```math
\left \{
\begin{array}{ll}
    -c_{B}^{T}B^{-1}d = (c_{F}^{T}-c_{B}^{T}B^{-1}F)x_{F}\\
    B^{-1}d = B^{-1}Bx_{B} + B^{-1}Fx_{F}
\end{array}
\right.
```

Considering its _canonical form_ (the tableau format considered in the program):

| $-c_{b}^{T}B^{-1}d$ |   0 | $c_{f}^{T} - c_{b}^{T}B^{-1}d$ |
| ------------------- | --- | ------------------------------ |
|           $B^{-1}d$ | $I$ |                      $B^{-1}F$ |

Replacing this equation:

$x_{b} = B^{-1}d - B^{-1}Fx_{f}$ (from $Ax = d$ that can be written as $Bx_{b} + Fx_{f} = d$)

inside the following one:

$c^{T}x = c_{b}^{T}x_{b} + c_{f}^{T}x_{f}d$

and considering that $x_{f} = 0$ we have that:

$z = c_{b}^{T}B^{-1}d$, is the value of the objective function
$c_{f}^{T} - c_{b}^{T}B^{-1}d$, are the reduced costs

### Example

Tableau canonical form of test.dat input file:
| -z     | d  | x1 | x2 | x3    | x4    | x5    |
| --     | -- | -- | -- | --    | --    | --    |
| c      |  1 |  0 |  0 | -3.67 |  0.67 |  0.33 |
| x1     |  2 |  1 |  0 | -3.67 |  1.67 | -0.67 |
| x2     |  1 |  0 |  1 |  1.33 | -0.33 | -0.33 |

The upper left element is $-z$, so the negated value of the objective function for the minimization problem.
The first numeric row is the reduced costs row.
The first numeric column is the known terms column.
Everything but the first 2 rows and the first 2 columns is the coefficients matrix.
Every line of known term and a coefficient matrix row is an equation of the linear programming problem.
The $xN$ variables in first column are a reference of which variable the reduced cost refers to, while the variables in first
column references to the $N^{th}$ column in base in that moment.

The Dual Simplex starts from this Tableau (the same used in the Primal Simplex) and at the end matches the same optimality
conditions of the Primal Simplex.
