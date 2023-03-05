.extern e1, e2
.global gB, gC
.word 0x1, 2233

.section s1
gB: .word 2
.equ gC, 3
a: .word 1 

.section s2
jmp %a #-2+2 op code
jmp %gB
jmp %gC

.section s3
ldr r0, gB
ldr r0, gC

.section s4
ldr r1, $2
ldr r1, $a
ldr r1, 2
ldr r1, a
ldr r1, %a
ldr r1, r1
ldr r1, [r1]
ldr r1, [r1+1]
ldr r1, [r1+a]

.section s5
jmp 1
jmp a
jmp *1
jmp *a
jmp *r1
jmp *[r1]
jmp *[r1+1]
jmp *[r1+a]
.END