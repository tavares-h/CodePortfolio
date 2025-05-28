# PERSONAL NOTES
This project was a part of my ECEGR 2220 course, Microprocessor design. The projects purpose was to create a simple verilog processor with 3 major instructions. The instructions were to add & subtract, to load a register with a immediate input value, and to move one register value to a different register.

The project and the class were very important to my understanding of computer architecture. Throughout the course we covered the foundation of computer architecture, and this project gave us insight on the implementation of CPU architecture. Learning about what instruction sets & cycles are, how a control unit works, etc.

On top of the conceptual ideas, the class also exposed me to hardware description langauges like Verilog and VHDL.

Embedded systems, System-on-Chips, FPGAs are all things i've grown an interest in and im grateful this class gave me these related experiences.

# PROJECT NOTES
The foundation of this implementation is through a finite state machine / state table. The FSM determines the state based on the entered instruction opcode. Each instruction has predefined behavior due to their varying length in steps/cycles. For example, the addition instruction requires 3 steps/cycles to complete the instruction, where as moving an immediate value into register is one.

Walkthrough:
The FSM sees what instruction it is and goes through the defined steps. At each step a case statement, is used to correlate the instruction to the current step. For example, step zero of the FSM is to enable reading in the instruction to the IR registor. Then it moves to the next state (step 1 of every instruction), from here the case statement determines the instruction again. If the instruction is move immediate, then the first step is to open up the bus and enable the selected register to read in the value. There are multiple cycles for certain instructions to account for the prerequisite steps such as moving data around and storing the results before moving them back to a register, so on.


These modules define the response to the steps in the main modules FSM:
1. Multiplexor module
	This module decide the register selection, determining what register gets loaded on to the bus wire.
2. ALU module
	This module performed the subtraction and addition on specified registers, and stores the result in its own temporary register.
3. Decode module
	This module determines what registers are specified by the input. The registers here get used throughout the system, like in the FSM.	
4. Regi module
	Defines the behavior of registers in this system. for instance, each has an enable bit, deciding whether to read in from the bus or not, etc.
	The registers are 8 bits in length.
5. IRregi module
	This module defines the behavior of the register that holds the instructions. It is seperate declaration because I decided the ISA should 9 bits.
	There are 3 bits for each register and 3 bits for the instruction opcode. 
	This allows 2 selections from the 8 registers and 8 possible instructions.

The most important component is the clock, facilitating the progression through the program. Most of the modules / components are connected and updated on the positive edge of the clock. Everything is connected to the clock at all times, so its important that enable bits are implemented to specify what really gets updated on this clock signal. The clock signal combined with enable bits is how this simple CPU controls the order of operations. This implementation uses a button to simulate the clock input.
