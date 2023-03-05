.extern ext1, ext2
.global a
.equ a, 2

.section s1
.word 2, ext1, a, s1, s2, l1
.skip 10

.section s2
jmp %l1
ldr r2, l1
l1:
add r1, r2
push r1
pop r1
ldr r1, [r1+2]
ldr r2, [r2+a]
call *a
call *ext1
.end