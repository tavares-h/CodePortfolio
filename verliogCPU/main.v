module main (DIN, Rin, Resetn, Clk, Run, Done, loadA, loadG, Gout, loadIr, AddSub, DINout, Rout, BusWires);
    input [8:0] DIN; // switches, ideal should be 16bits but only have 8 switches
    input Resetn, Clk, Run;
    output reg Done;
    output [8:0] BusWires;
    
    output reg loadA; // register A before ALU, temporary so we can load two different data
    output reg loadG; // register G after ALu, temporary to decide what to do with it, ie store it and where
    output reg Gout; // puts g through the multiplexor to a reg or back to add sub
    output reg loadIr; // registor Ir, holds the opcode instructions, we want to disable this when we want an immediate input from DIN
    output reg AddSub; // selects mode for ALU
    output reg DINout; // lets the immediate in DIN through the multiplexor
    output reg [7:0] Rout; // enables what registors can output
	 output reg [7:0] Rin; // enables what registers can take input
    
    reg [1:0] Tstep_Q;
    reg [1:0] Tstep_D;
    
    parameter T0 = 2'b00, T1 = 2'b01, T2 = 2'b10, T3 = 2'b11;
    
    wire [7:0] Xreg; // holds register x address
    wire [7:0] Yreg; // holds register y address
    
    wire [7:0] R0;
    wire [7:0] R1;
    wire [7:0] R2;
    wire [7:0] R3;
    wire [7:0] R4;
    wire [7:0] R5;
    wire [7:0] R6;
    wire [7:0] R7;
    wire [8:0] Ir; // instruction w
    wire [7:0] RegG; // temporary storage for the ALU resumt
    wire [7:0] RegA; // temporary storage for one of the operands
    wire [7:0] ALUout; // for piping alu result
    
    wire [2:0] I = Ir[8:6]; // opcode for instructions, ie move, movei, add, sub
    
    decode decX (Ir[5:3], 1'b1, Xreg);
    decode decY (Ir[2:0], 1'b1, Yreg);
	 
	 initial begin
		  loadA = 0;
        loadG = 0;
        Gout = 0;
        loadIr = 1'b1;
        AddSub = 0;
        DINout = 0;
        Done = 0;
        Rout = 0;
		  Rin = 0;
		  Tstep_Q = T0;
		  Tstep_D = T1;
	 end
		  
    always @ (posedge Clk or negedge Resetn) begin
		  if (!Resetn) begin
				Tstep_Q <= T0;
		  end else
				Tstep_Q <= Tstep_D;
	 end
    
    always @ (Tstep_Q) begin
        case (Tstep_Q) // state table, the logic for moving between states
            T0: Tstep_D = Run ? T1 : T0;
            T1: Tstep_D = Done ? T0 : T2;
            T2: Tstep_D = Done ? T0 : T3;
            T3: Tstep_D = T0;
        endcase
    end
    
    always @ (Tstep_Q) begin
        // initialize values
        case (Tstep_Q)
            T0: begin
                 loadIr = 1'b1;
					  loadA = 0;
					  loadG = 0;
					  Gout = 0;
					  AddSub = 0;
					  DINout = 0;
					  Done = 0;
            end
            T1: case (I)
                3'b000: begin // move state 1
                    Rout = Yreg; // what register goes through multiplexor onto bus
						  Rin = Xreg; // determines what registor takes data on the buss
                    Done = 1; // stage is finished, ie the instructions are fully finished
						  loadA = 0; // loads A registor for AddSub
						  loadG = 0; // loads G registor for addsub result
						  Gout = 0; // lets g through multiplexor, to potential enter registors or back into Addsub
						  loadIr = 0; // enables loading of instructions
						  AddSub = 0; // determines whether operations are add or sub
						  DINout = 0; // allows input through multiplexor for addsub or putting into registor
                end
                3'b001: begin // move immediate state 1
                    Rout = 8'b00000000; // stops registors from going through mux
						  Rin = 8'b00000000; // opens this registor to recive data from bus
                    DINout = 0; // allos DIN onto the bus
                    Done = 0; // process is finished
						  loadA = 0;
						  loadG = 0;
						  Gout = 0;
						  loadIr = 0;
						  AddSub = 0;
                end
                3'b010: begin // add state 1
                    Rout = Xreg;
						  Rin = 8'b00000000; // stops registors from getting data
                    loadA = 1; // loads registor A with data on bus
						  loadG = 0;
						  Gout = 0;
						  loadIr = 0;
						  AddSub = 0;
						  DINout = 0;
						  Done = 0;
                end
                3'b100: begin // subtract state 1
                    Rout = Xreg;
						  Rin = 8'b00000000;
                    loadA = 1;
						  loadG = 0;
						  Gout = 0;
						  loadIr = 0;
						  AddSub = 0;
						  DINout = 0;
						  Done = 0;
                end
            endcase
            T2: case (I)
					 3'b001: begin
						  Rout = 8'b00000000; // stops registors from going through mux
						  Rin = Xreg; // opens this registor to recive data from bus
                    DINout = 1; // allos DIN onto the bus
                    Done = 1; // process is finished
						  loadA = 0;
						  loadG = 0;
						  Gout = 0;
						  loadIr = 0;
						  AddSub = 0;
					 end
                3'b010: begin // add state 2
                    Rout = Yreg;
						  Rin = 8'b00000000;
                    loadG = 1;
						  loadA = 0;
						  Gout = 0;
						  loadIr = 0;
						  AddSub = 0; // zero represents addition
						  DINout = 0;
						  Done = 0;
                end
                3'b100: begin // subtract state 2
                    Rout = Yreg;
						  Rin = 8'b00000000;
                    loadG = 1;
                    AddSub = 1;
						  loadA = 0;
						  Gout = 0;
						  loadIr = 0;
						  DINout = 0;
						  Done = 0;
                end
            endcase
            T3: case (I)
                3'b010: begin // add state 3
                    Rout = 8'b00000000;
						  Rin = Xreg;
                    Gout = 1;
                    Done = 1;
						  loadA = 0;
						  loadG = 0;
						  loadIr = 0;
						  AddSub = 0;
						  DINout = 0;
                end
                3'b100: begin // subtract state 3
                    Rout = 8'b00000000;
						  Rin = Xreg;
                    Gout = 1;
                    Done = 1;
						  loadA = 0;
						  loadG = 0;
						  loadIr = 0;
						  AddSub = 0;
						  DINout = 0;
                end
            endcase
        endcase
    end
    
	 mux Multi (DIN, DINout, Gout, R0, R1, R2, R3, R4, R5, R6, R7, RegG, Rout, BusWires);
	 
    regi reg_0(BusWires, Rin[0], Clk, R0);
    regi reg_1(BusWires, Rin[1], Clk, R1);
    regi reg_2(BusWires, Rin[2], Clk, R2);
    regi reg_3(BusWires, Rin[3], Clk, R3);
    regi reg_4(BusWires, Rin[4], Clk, R4);
    regi reg_5(BusWires, Rin[5], Clk, R5);
    regi reg_6(BusWires, Rin[6], Clk, R6);
    regi reg_7(BusWires, Rin[7], Clk, R7);
    regi reg_A(BusWires, loadA, Clk, RegA);
    regi reg_G(ALUout, loadG, Clk, RegG);
    IRregi reg_IR(DIN, loadIr, Clk, Ir);
    
    alu ALU_unit (BusWires, RegA, AddSub, ALUout);
    
endmodule

module decode (R, En, Y);
// decodes the 3 bits and outputs the register selection
    input [2:0] R;
    input En;
    output reg [7:0] Y;
    
    always @ (R or En) begin
        if (En == 1)
            case (R)
                3'b000: Y = 8'b00000001;
                3'b001: Y = 8'b00000010;
                3'b010: Y = 8'b00000100;
                3'b011: Y = 8'b00001000;
                3'b100: Y = 8'b00010000;
                3'b101: Y = 8'b00100000;
                3'b110: Y = 8'b01000000;
                3'b111: Y = 8'b10000000;
            endcase
        else
            Y = 8'b00000000;
    end
endmodule

module regi (R, En, Clk, Q);
// used throughout to store and load data
    input [7:0] R;
    input En, Clk;
    output reg [7:0] Q;
    always @ (posedge Clk)
        if (En)
            Q <= R;
endmodule

module IRregi (R, En, Clk, Q);
// used throughout to store and load data
    input [8:0] R;
    input En, Clk;
    output reg [8:0] Q;
    always @ (posedge Clk)
        if (En)
            Q <= R;
endmodule

module mux (DIN, DINout, Gout, R0, R1, R2, R3, R4, R5, R6, R7, RegG, Rout, BusWires);
// control sends an Rout signal signifying which register gets loaded
    input DINout, Gout;
    input [7:0] Rout, R0, R1, R2, R3, R4, R5, R6, R7, RegG;
    input [8:0] DIN;
    output reg [8:0] BusWires;
    
    always @ (Rout or Gout or DINout) begin
		  if (Gout == 1) begin
            BusWires <= RegG;
        end else if (DINout == 1) begin
            BusWires <= DIN;
        end else
            case (Rout)
                8'b00000001: BusWires <= R0;
                8'b00000010: BusWires <= R1;
                8'b00000100: BusWires <= R2;
                8'b00001000: BusWires <= R3;
                8'b00010000: BusWires <= R4;
                8'b00100000: BusWires <= R5;
                8'b01000000: BusWires <= R6;
                8'b10000000: BusWires <= R7;
            endcase
    end
endmodule

module alu (BusWires, RegAin, AddSub, RegGout);
// determines the arithmetic operation depending on FSM control code
    input [8:0] BusWires;
    input [7:0] RegAin;
    input AddSub;
    output reg [7:0] RegGout;
    
    always @ (BusWires or RegAin or AddSub) begin
        if (AddSub)
            RegGout = RegAin - BusWires;
        else
            RegGout = RegAin + BusWires;
    end
endmodule

