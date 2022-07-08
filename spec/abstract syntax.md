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
- **esign** *OP* *E*
- **eincdec** *OP* *E* *BOOL*
- **enot** *E*
- **ederef** *E*
- **eaddr** *E*
- **ecast** *TY* *E*
- **ecall** *BOOL* *E₁* ?(*E₂*...*Eₙ*)
- **efeild** *E* *STR*
- **emethod** *E₁* *BOOL* *STR* ?(*E₃*...*Eₙ*)
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

eboollit
--------
> Γ ⊢ **eboollit** :: **tbool**

eintlit
-------
> Γ ⊢ **eintlit** :: **tint**

efloatlit
---------
> Γ ⊢ **efloatlit** :: **tfloat**

estrlit
-------
> Γ ⊢ **estrlit** :: **tstr**

estructlit
----------
> Γ ⊢ *TY* :: **tstruct** <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **estructlit** *TY* ?((*STR*, *TY*)₁...(*STR*, *TY*)ₙ) :: **tstruct**

eident
------
> Γ ⊢ T :: **eident** ∈ Γ <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **eident** :: T

euninit
-------
**euninit** TODO

ebin
----
> *OP* ∈ {**opplus**, **opminus**} <br>
> T ∈ {**tyint**, **tyfloat**} <br>
> Γ ⊢ *E₁* :: T   Γ ⊢ *E₁* :: T <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **ebin** *OP* *E₁* *E₂* :: T

esscript
--------
> T₁ = **tyarr** *TY*   T₂ = **tyint** <br>
> Γ ⊢ *E₁* :: T₁   Γ ⊢ *E₂* :: T₂ <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **esscript** *E₁* *E₂* :: *TY*

esign
-----
> *OP* ∈ {**opplus**, **opminus**} <br>
> Γ ⊢ *E* :: T  T ∈ {**tyint**, **tyfloat**} <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **esign** *OP* *E* :: T

eincdec
-------
> *OP* ∈ {**opinc**, **opdec**} <br>
> Γ ⊢ *E* :: T  T ∈ {**tyint**, **tyfloat**} <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **eincdec** *OP* *E* *BOOL* :: T

enot
----
> Γ ⊢ *E* :: T  T = **tybool** <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **enot** *E* :: T

ederef
------
> Γ ⊢ *E* :: T  T = **typtr** *TY* <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **ederef** *E* :: *TY*

eaddr
-----
> Γ ⊢ *E* :: T <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **eaddr** *E* :: **typtr** T

ecast
-----
**ecast** TODO

ecall
-----
> Γ ⊢ *E₁* :: (*E₂*...*Eₙ*) -> T <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **ecall** FALSE *E₁* ?(*E₂*...*Eₙ*) :: T

> Γ ⊢ (*E₂*...*Eₙ*) = (T₁...) <br>
> Γ ⊢ *E₁* :: (T₁...Tₙ) -> T₂ <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **ecall** TRUE *E₁* ?(*E₂*...*Eₙ*) :: (...Tₙ) -> T₂

efeild
------
> Γ ⊢ *E* :: **tystruct**  T :: *STR* ∈ *E* <br>
> \----------------------------------------------------------- <br>
> Γ ⊢ **efeild** *E* *STR* :: T