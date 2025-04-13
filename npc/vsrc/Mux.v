module ALUSrc_mux21(
  input [31:0] a,
  input [31:0] b,
  input s,
  output [31:0] y 
);

  MuxKey #(2, 1, 32) i0 (y, s, {
    1'b0, a,
    1'b1, b
  });

endmodule
