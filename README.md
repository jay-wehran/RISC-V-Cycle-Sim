The repository contains the source code for my project on the 32-bit RISC-V Cycle Accurate Simulator. The simulator supports both Single-Stage and Five-Stage pipelined architectures with hazard detection and forwarding to resolve RAW and control hazards.

My thoughts on this project: While this was an absolute behemoth to code up (potentially due to what may be slightly inefficient code), I learned a tremendous amount about ISA. Doing this project really helped me to connect a lot of dots when it came to computing, and why certain aspects of a computer process the way it does. I have a lot more respect for sign-extension after doing this project (as before I was simply doing arithmetic with it, not handling it), as well as a newfound fancy for the bitset library. A true life saver.

In this repository you will find only the source code. Do note that the input to this system were two binary files (imem.txt, which held a set of instructions which were byte-addressable, as well as dmem.txt, which held a current state of memory.) 

To track performance metrics, at the end of runtime, I would have files printed which tracked the current state of memory as well as the overall performance of the system (CPI, # instructions, Instructions per Cycle, etc.)

