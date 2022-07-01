Syntax
======

Expressions
-----------

#### Expr, E ::=
- **eboollit**
- **eintlit**
- **efloatlit**
- **estrlit**
- **estructlit** *TY* ?((*STR*, *TY*)₁...(*STR*, *TY*)ₙ)
- **eident**
- **euninit**
- **ebin** *OP* *E₁* *E₂*
- **esscript** *E₁* *E₂*
- **esign** (*opplus*|*opminus*) *E*
- **eincdec** (*opinc*|*opdec*) *E* *BOOL*
- **enot** *opnot* *E*
- **ederef** *E*
- **eaddr** *E*
- **ecast** *TY* *E*
- **ecall** *BOOL* *E₁* ?(*E₂*...*Eₙ*)
- **efeild** *E₁* *E₂*
- **emethod** *E₁* *BOOL* *E₂* ?(*E₃*...*Eₙ*)
- **epath** *E₁* *E₂*
- **eif** *E₁* *E₂* ?*E₃*
- **estruct** *TY*

Statements
----------

#### Stmt, S ::=
- **sret** *E*
- **sdefer** *S*
- **sdisc** *E*
- **sblk** ?(*S₁*...*S₂*)
- **svardecl** *E₁* ?*TY* *E₂* *BOOL*
- **sfor** *S₁* *E₁* *E₂* *S₂* *BOOL*
- **sbrk**
- **scnt**
- **impl** *TY* (*S₁*...*Sₙ*)

Types
-----

#### Type, TY ::=
- **tyvoid**
- **tybool**
- **tyint** *BOOL* *NUM*
- **tyfloat** *NUM*
- **tystr**
- **typtr** *TY*
- **tyarr** *TY*
- **tyfunc** *TY* ?((*STR*, *TY*)₁...(*STR*, *TY*)ₙ)
- **tystruct** ((*STR*, *TY*)₁...(*STR*, *TY*)ₙ)

Operators
---------

#### Opr, OP ::=
- **opplus**
- **opminus**
- **opinc**
- **opdec**
- **opnot**

Typing
======
<pre>

eboollit
---
> Γ ⊢ **eboollit** :: **tbool**

eintlit
---
> Γ ⊢ **eintlit** :: **tint**

efloatlit
---
> Γ ⊢ **efloatlit** :: **tfloat**

estrlit
---
> Γ ⊢ **estrlit** :: **tstr**

estructlit
---
> Γ ⊢ *TY* :: **tstruct**
> \-----------------------------------------------------------
> Γ ⊢ **estructlit** *TY* ?((*STR*, *TY*)₁...(*STR*, *TY*)ₙ) :: **tstruct**

eident
---
> Γ ⊢ T :: **eident** ∈ Γ
> \-----------------------------------------------------------
> Γ ⊢ **eident** :: T

euninit
---
**euninit** TODO

ebin
---
> T ∈ {**tyint**, **tyfloat**}
> Γ ⊢ *E₁* :: T   Γ ⊢ *E₁* :: T
> \-----------------------------------------------------------
> Γ ⊢ **ebin** *OP* *E₁* *E₂* :: T

esscript
---
> T₁ = **tyarr** *TY*   T₂ = **tyint**
> Γ ⊢ *E₁* :: T₁   Γ ⊢ *E₂* :: T₂
> \-----------------------------------------------------------
> Γ ⊢ **esscript** *E₁* *E₂* :: *TY*

esign
---
> Γ ⊢ *E* :: T  T ∈ {**tyint**, **tyfloat**}
> \-----------------------------------------------------------
> Γ ⊢ **esign** (*opplus*|*opminus*) *E* :: T

eincdec
---
> Γ ⊢ *E* :: T  T ∈ {**tyint**, **tyfloat**}
> \-----------------------------------------------------------
> Γ ⊢ **eincdec** (*opinc*|*opdec*) *E* *BOOL* :: T

enot
---
> Γ ⊢ *E* :: T  T = **tybool**
> \-----------------------------------------------------------
> Γ ⊢ **enot** *opnot* *E* :: T

ederef
---
> Γ ⊢ *E* :: T  T = **typtr** *TY*
> \-----------------------------------------------------------
> Γ ⊢ **ederef** *E* :: *TY*

eaddr
---
> Γ ⊢ *E* :: T
> \-----------------------------------------------------------
> Γ ⊢ **eaddr** *E* :: **typtr** T

ecast
---
**ecast** TODO
</pre>