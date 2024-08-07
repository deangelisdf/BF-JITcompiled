# BF-JITcompiled
Compilation of BrainFuck in Just-In-Time without use external library for JIT Compilation using copy-and-patch strategy.<br>
<br>
BrainFuck is an esoteric programming language simply to study and hard to use, because you have few operators available.<br>
For the same reason, it is simple to implement an interpeter.<br>
<br>
In this project i implement, in firts time an interpeter, in second time a JIT compilation to reduce overhead to interpreter
<br>
This is a initial beta version of my personal JIT compiler for BrainFuck<br>
Tested only on Ubuntu 17.10, CPU intel x86_64. Compiled with GCC<br>
In next time, i want add Windows compatibility.<br>

# Features
<ul>
<li>Interpreter</li>
<li>JiT Compilation</li>
<li>Optimization</li>
</ul>

# Test used
<b>"hello world"</b> <br>
<code>+[-[<<[+[--->]-[<<<]]]>>>-]>-.---.>..>.<<<<-.<+.>>>>>.>.<<.<-.</code> <br>
<br>
<b>"Hello World!\n"</b> by https://esolangs.org/wiki/Brainfuck#Memory_and_wrapping <br>
<code>++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.</code> <br>
<br>
<b>"Mandelbrot"</b> written by Erik Bosman <br>
<code>http://esoteric.sange.fi/brainfuck/utils/mandelbrot/mandelbrot.b</code>
<p align="center">
  <img src="https://github.com/deangelisdf/BF-JITcompiled/blob/master/example.png" width="350" title="the best test">
</p>

Documentation in code brainfuck.h

# Reference <br>
x86_64 Architecture http://6.035.scripts.mit.edu/sp17/x86-64-architecture-guide.html <br>
OpCode x86_64 http://ref.x86asm.net/ <br>
BrainFuck https://en.wikipedia.org/wiki/Brainfuck <br>
mmap https://www.gnu.org/software/libc/manual/html_node/Memory_002dmapped-I_002fO.html
