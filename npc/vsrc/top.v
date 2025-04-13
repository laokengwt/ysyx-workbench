module top (
  input wire clk,
  input wire rst,
  // 寄存器文件接口
  input reg [31:0] inst,
  output wire [31:0] pc
);

  wire [31:0] next_pc = pc + 4;  // 简单PC+4逻辑
  wire [31:0] reg_rdata1, reg_rdata2;
  wire [31:0] result;
  wire N, Z, V, C;
  wire [1:0] alu_op;
  wire reg_write;
  wire alu_src;
  wire [31:0] alu_op1, alu_op2;
  wire [31:0] imm;

  // 实例化PC寄存器
  PC pc_reg (
    .clk(clk),
    .rst(rst),
    .wen(1'b1),      // 持续更新PC
    .next_pc(next_pc),
    .pc(pc)
  );

  // 实例化寄存器文件（不写入数据）
  Registers reg_file (
    .clk(clk),
    .rst(rst),
    .waddr(inst[11:7]),        // 不写入
    .wdata(result),       // 数据为0
    .wen(reg_write),          // 写使能关闭
    .raddr1(inst[19:15]),   // 读取x1
    .raddr2(inst[24:20]),   // 读取x2
    .rdata1(reg_rdata1),
    .rdata2(reg_rdata2)
  );

  Control control(
    .inst(inst),
    .ALUop(alu_op),
    .RegWrite(reg_write),
    .ALUSrc(alu_src)
  );

  // 处理好ALU的两个操作数的源
  assign alu_op1 = reg_rdata1;
  // 对imm进行符号拓展
  ImmGen imm_gen(inst, imm);

  ALUSrc_mux21 alu_src_mux(reg_rdata2, imm, alu_src, alu_op2);

  ALU alu(
    .A(alu_op1),
    .B(alu_op2),
    .ALUControl(alu_op),
    .Result(result),
    .N(N),
    .Z(Z),
    .V(V),
    .C(C)
  );

  // 打印输出（在每个时钟上升沿后打印）
  always @(posedge clk) begin
    $display("PC=0x%08x | inst=0x%08x", pc, inst);
    $display("result=0x%08x", result);
    $display("N Z V C = %b %b %b %b", N, Z, V, C);
  end

endmodule
