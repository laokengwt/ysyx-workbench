module PC #(
    parameter DATAWIDTH = 32
)(
    input  logic             clk,
    input  logic             rst,
    input  logic             wen,
    input  logic [DATAWIDTH-1:0] next_pc,
    output logic [DATAWIDTH-1:0] pc
);
    Reg #(
        .WIDTH(DATAWIDTH),
        .RESET_VAL(32'h8000_0000)
    )
    pc_reg (
        .clk(clk),
        .rst(rst),
        .din(next_pc),
        .dout(pc),
        .wen(wen)
    );

endmodule
