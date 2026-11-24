#!/usr/bin/env bash
# Runs the parser (experiment 2) on the lab samples and compares stdout.
set -u
parser=${1:?usage: run-parser-samples.sh path/to/zcc-parse}
fail=0

check() {
  local name=$1
  local block input expected actual
  block=$(cat)
  input=${block%%$'\n===\n'*}
  expected=${block#*$'\n===\n'}
  actual=$(printf '%s\n' "$input" | "$parser")
  if [[ "$actual" == "$expected" ]]; then
    printf 'ok   %s\n' "$name"
  else
    printf 'FAIL %s\n' "$name"
    diff -u <(printf '%s\n' "$expected") <(printf '%s\n' "$actual") | sed 's/^/     /'
    fail=1
  fi
}

check sample1 <<'EOF'
int a,b,c;
double d,e,f;
int j;
{
    d=2.4;
    e=-3.6*5;
    f=!(d*e)/((d+e)<=d);
    a=4;
    b=+3;
    c=111;
    while a>=0 do
        {
            a=a-1;
            d=d*e;
            while (b!=0 && !c>0)|| !a>0 do
                {
                    b=b-1 <c || a==0;
                    printf(b,d);
                };
            if a==1 then
                printf(c,a,b);
        };
}
===
7
a 0 null 0
b 0 null 4
c 0 null 8
d 1 null 12
e 1 null 20
f 1 null 28
j 0 null 36
28
55
0: (=,2.400000,-,T0_d)
1: (=,T0_d,-,TB3)
2: (=,3.600000,-,T1_d)
3: (-,0,T1_d,T2_d)
4: (=,5,-,T3_i)
5: (*,T2_d,T3_i,T4_i)
6: (=,T4_i,-,TB4)
7: (*,TB3,TB4,T5_d)
8: (+,TB3,TB4,T6_d)
9: (<=,T6_d,TB3,T7_i)
10: (/,T5_d,T7_i,T8_i)
11: (!,T8_i,-,T9_i)
12: (=,T9_i,-,TB5)
13: (=,4,-,T10_i)
14: (=,T10_i,-,TB0)
15: (=,3,-,T11_i)
16: (+,0,T11_i,T12_i)
17: (=,T12_i,-,TB1)
18: (=,111,-,T13_i)
19: (=,T13_i,-,TB2)
20: (=,0,-,T14_i)
21: (j>=,TB0,T14_i,23)
22: (j,-,-,54)
23: (=,1,-,T15_i)
24: (-,TB0,T15_i,T16_i)
25: (=,T16_i,-,TB0)
26: (*,TB3,TB4,T17_d)
27: (=,T17_d,-,TB3)
28: (=,0,-,T18_i)
29: (j!=,TB1,T18_i,31)
30: (j,-,-,34)
31: (=,0,-,T19_i)
32: (j>,TB2,T19_i,34)
33: (j,-,-,37)
34: (=,0,-,T20_i)
35: (j>,TB0,T20_i,47)
36: (j,-,-,37)
37: (=,1,-,T21_i)
38: (-,TB1,T21_i,T22_i)
39: (<,T22_i,TB2,T23_i)
40: (=,0,-,T24_i)
41: (==,TB0,T24_i,T25_i)
42: (||,T23_i,T25_i,T26_i)
43: (=,T26_i,-,TB1)
44: (W,-,-,TB1)
45: (W,-,-,TB3)
46: (j,-,-,28)
47: (=,1,-,T27_i)
48: (j==,TB0,T27_i,50)
49: (j,-,-,20)
50: (W,-,-,TB2)
51: (W,-,-,TB0)
52: (W,-,-,TB1)
53: (j,-,-,20)
54: (End,-,-,-)
EOF

check sample2_undefined <<'EOF'
int a,b;
{
a=0;
b=1;
c=2;
}
===
Syntax Error
EOF

check sample3 <<'EOF'
int a,b,c;

{
    scanf(a);
    b=0;
    while a>=0 do
        {
            a=a-1;
            c=a-(a/2)*2;
            if c!=0 then
                b=b+a;
        };
    printf(b);
}
===
3
a 0 null 0
b 0 null 4
c 0 null 8
11
23
0: (R,-,-,TB0)
1: (=,0,-,T0_i)
2: (=,T0_i,-,TB1)
3: (=,0,-,T1_i)
4: (j>=,TB0,T1_i,6)
5: (j,-,-,21)
6: (=,1,-,T2_i)
7: (-,TB0,T2_i,T3_i)
8: (=,T3_i,-,TB0)
9: (=,2,-,T4_i)
10: (/,TB0,T4_i,T5_i)
11: (=,2,-,T6_i)
12: (*,T5_i,T6_i,T7_i)
13: (-,TB0,T7_i,T8_i)
14: (=,T8_i,-,TB2)
15: (=,0,-,T9_i)
16: (j!=,TB2,T9_i,18)
17: (j,-,-,3)
18: (+,TB1,TB0,T10_i)
19: (=,T10_i,-,TB1)
20: (j,-,-,3)
21: (W,-,-,TB1)
22: (End,-,-,-)
EOF

check sample4 <<'EOF'
int a,b,c,d,e,f;
{
    a=0;
    b=1;
    c=2;
    d=0;
    e=0;
    f=0;
    if !(!(a && d || (e<5)))  then
        d=b+c;
    if ! (a<=b) then
        e=b*c;

    printf(d,e,f);
}
===
6
a 0 null 0
b 0 null 4
c 0 null 8
d 0 null 12
e 0 null 16
f 0 null 20
9
29
0: (=,0,-,T0_i)
1: (=,T0_i,-,TB0)
2: (=,1,-,T1_i)
3: (=,T1_i,-,TB1)
4: (=,2,-,T2_i)
5: (=,T2_i,-,TB2)
6: (=,0,-,T3_i)
7: (=,T3_i,-,TB3)
8: (=,0,-,T4_i)
9: (=,T4_i,-,TB4)
10: (=,0,-,T5_i)
11: (=,T5_i,-,TB5)
12: (jnz,TB0,-,14)
13: (j,-,-,16)
14: (jnz,TB3,-,19)
15: (j,-,-,16)
16: (=,5,-,T6_i)
17: (j<,TB4,T6_i,19)
18: (j,-,-,21)
19: (+,TB1,TB2,T7_i)
20: (=,T7_i,-,TB3)
21: (j<=,TB0,TB1,25)
22: (j,-,-,23)
23: (*,TB1,TB2,T8_i)
24: (=,T8_i,-,TB4)
25: (W,-,-,TB3)
26: (W,-,-,TB4)
27: (W,-,-,TB5)
28: (End,-,-,-)
EOF

check sample5_decls_only <<'EOF'
int a,b,c;
double x,y;
int d,e;
double z;

===
8
a 0 null 0
b 0 null 4
c 0 null 8
x 1 null 12
y 1 null 20
d 0 null 28
e 0 null 32
z 1 null 36
0
1
0: (End,-,-,-)
EOF

exit "$fail"
