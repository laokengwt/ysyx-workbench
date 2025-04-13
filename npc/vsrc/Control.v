module Control (
  input  wire [31:0] inst,       // 指令输入
  output wire [1:0]  ALUop,      // ALU操作码
  output wire        RegWrite,    // 寄存器写使能
  output wire        ALUSrc      // ALU第二个操作数选择
);
  wire is_addi  = (inst[6:0] == 7'b0010011);  // ADDI
  wire is_add   = (inst[6:0] == 7'b0110011 && inst[14:12] == 3'b000); // ADD

  assign ALUop   = (is_addi || is_add) ? 2'b00 : 2'b00;  // 00=ADD
  assign RegWrite = is_addi || is_add;                   // 需要写寄存器
  assign ALUSrc   = is_addi;                             // 1=立即数, 0=寄存器wire is_addi = (inst[6:0] == 7'b0010011);
endmodule
