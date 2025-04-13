module Registers #(
  parameter ADDR_WIDTH = 5,
  parameter DATA_WIDTH = 32
) (
  input wire clk,
  input wire rst,
  input wire [ADDR_WIDTH-1:0] waddr,
  input wire [DATA_WIDTH-1:0] wdata,
  input wire wen,
  input wire [ADDR_WIDTH-1:0] raddr1,
  input wire [ADDR_WIDTH-1:0] raddr2,
  output wire [DATA_WIDTH-1:0] rdata1,
  output wire [DATA_WIDTH-1:0] rdata2
);

  reg [DATA_WIDTH-1:0] rf [0:2**ADDR_WIDTH-1];

  integer i;

  // init the registers
  initial begin
    for (i = 0; i < 2**ADDR_WIDTH; i = i + 2) begin
      rf[i] = {DATA_WIDTH{1'b0}};
    end
  end

  // synchronous reset
  always @(posedge clk) begin
    if (rst) begin
      for (i = 0; i < 2**ADDR_WIDTH; i = i + 1) begin
        rf[i] <= {DATA_WIDTH{1'b0}};
      end
    end
  end

  // write data
  always @(posedge clk) begin
    if (wen && waddr != 0) rf[waddr] <= wdata;
  end

  // distinguish the $0 reg and the others
  assign rdata1 = (raddr1 == 0) ? 0 : rf[raddr1];
  assign rdata2 = (raddr2 == 0) ? 0 : rf[raddr2];

endmodule
