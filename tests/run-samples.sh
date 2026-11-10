#!/usr/bin/env bash
set -u
lexer=${1:?usage: run-samples.sh path/to/zcc-lex}
fail=0

check() {
  local name=$1
  local block input expected actual
  block=$(cat)
  input=${block%%$'\n===\n'*}
  expected=${block#*$'\n===\n'}
  actual=$(printf '%s\n' "$input" | "$lexer")
  if [[ "$actual" == "$expected" ]]; then
    printf 'ok   %s\n' "$name"
  else
    printf 'FAIL %s\n' "$name"
    diff -u <(printf '%s\n' "$expected") <(printf '%s\n' "$actual") | sed 's/^/     /'
    fail=1
  fi
}

check sample1 <<'EOF'
int a, b;

double c=1.2; // This is a comment

scanf(a);

scanf(b);

printf(c);
===
int INTSYM
a IDENT
, COMMA
b IDENT
; SEMICOLON
double DOUBLESYM
c IDENT
= AO
1.2 DOUBLE
; SEMICOLON
scanf SCANFSYM
( BRACE
a IDENT
) BRACE
; SEMICOLON
scanf SCANFSYM
( BRACE
b IDENT
) BRACE
; SEMICOLON
printf PRINTFSYM
( BRACE
c IDENT
) BRACE
; SEMICOLON
EOF

check sample2 <<'EOF'
int a = 6037210

int b = 06356417
===
Malformed number: Leading zeros in an integer.
EOF

check sample3 <<'EOF'
12910
1223.219
27912.120921
2181123.
2810.
12123
===
Malformed number: Decimal point at the beginning or end of a floating point number.
EOF

check sample4 <<'EOF'
int a,b;
c=2;
d=123.21;
?
*
~
===
Unrecognizable characters.
EOF

check sample5 <<'EOF'
12738
0.2919
.0199
1210
1.111
1.201
10291.1290
===
Malformed number: Decimal point at the beginning or end of a floating point number.
EOF

check sample6 <<'EOF'
1234
2222222
1928301
1.87273
0.9218
3.12919
1.0291
1.1.112182
21211
===
Malformed number: More than one decimal point in a floating point number.
EOF

check edge_lone_dot <<'EOF'
.
===
Malformed number: Decimal point at the beginning or end of a floating point number.
EOF

check edge_two_dots <<'EOF'
..
===
Malformed number: More than one decimal point in a floating point number.
EOF

check edge_zero_point <<'EOF'
0.29
===
0.29 DOUBLE
EOF

check edge_leading_zero_and_end_dot <<'EOF'
01.
===
Malformed number: Decimal point at the beginning or end of a floating point number.
EOF

check edge_unclosed_comment <<'EOF'
int a; /* int b; 0635 ?
===
int INTSYM
a IDENT
; SEMICOLON
EOF

check edge_keyword_prefix <<'EOF'
intx ifx thenx dox
===
intx IDENT
ifx IDENT
thenx IDENT
dox IDENT
EOF

check edge_all_operators <<'EOF'
= == > >= < <= || && ! != + - * / , ( ) { } ;
===
= AO
== RO
> RO
>= RO
< RO
<= RO
|| LO
&& LO
! LO
!= RO
+ PLUS
- MINUS
* TIMES
/ DIVISION
, COMMA
( BRACE
) BRACE
{ BRACE
} BRACE
; SEMICOLON
EOF

# A lexical error must still exit 0: some judges read a non-zero status as a crash.
printf '0635' | "$lexer" >/dev/null
if [[ $? -ne 0 ]]; then
  echo "FAIL exit status on error was non-zero"
  fail=1
else
  echo "ok   exit-code-on-error"
fi

exit "$fail"
