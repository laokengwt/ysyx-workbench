module ImmGen #(
    parameter DATAWIDTH = 32  
)(
    input  logic [31:0] inst,
    output logic [DATAWIDTH-1:0] imm
);
    // I-type立即数提取（ADDI指令）
    always_comb begin
        // 提取12位立即数并进行符号扩展
        imm = {{20{inst[31]}}, inst[31:20]};
    end

endmodule
