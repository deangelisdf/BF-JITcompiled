# BF-JITcompiled
Compilation of BrainFuck in Just-In-Time without use external library for JIT Compilation.<br>
<br>
BrainFuck is an esoteric programming language simply to study and hard to use, because you have few operators available.<br>
For the same reason, it is simple to implement an interpeter.<br>
<br>
In this project i implement, in firts time an interpeter, in second time a JIT compilation to reduce overhead to interpreter
<br>
This is a initial beta version of my personal JIT compiler for BrainFuck<br>
Tested only on Ubuntu 17.10, CPU intel x86_64. Compiled with GCC<br>
In next time, i want add Windows compatibility.<br>

Documentation in code brainfuck.h

Reference:
OpCode x86_64 http://ref.x86asm.net/
BrainFuck https://en.wikipedia.org/wiki/Brainfuck
mmap https://www.gnu.org/software/libc/manual/html_node/Memory_002dmapped-I_002fO.html
