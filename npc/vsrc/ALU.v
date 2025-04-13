module ALU #(
    parameter DATAWIDTH = 32   
)(
    input  logic [DATAWIDTH-1:0]  A,
    input  logic [DATAWIDTH-1:0]  B,
    input  logic [1:0]            ALUControl,
    output logic [DATAWIDTH-1:0]  Result,
    output logic                 N,  // Negative
    output logic                 Z,  // Zero
    output logic                 V,  // Overflow
    output logic                 C   // Carry
);
    // 内部信号
    logic [DATAWIDTH:0] sum;  // 带进位的加法结果
    logic [DATAWIDTH:0] diff; // 带借位的减法结果

    // 计算加法和减法
    assign sum = {1'b0, A} + {1'b0, B};
    assign diff = {1'b0, A} - {1'b0, B};

    always_comb begin
        // 默认值
        Result = '0;
        N = '0;
        Z = '0;
        V = '0;
        C = '0;

        case (ALUControl)
            2'b00: begin // ADD
                Result = sum[DATAWIDTH-1:0];
                C = sum[DATAWIDTH];  // 进位标志
                // 溢出标志：两个正数相加得负数，或两个负数相加得正数
                V = (A[DATAWIDTH-1] == B[DATAWIDTH-1]) && 
                    (Result[DATAWIDTH-1] != A[DATAWIDTH-1]);
            end

            2'b01: begin // SUB
                Result = diff[DATAWIDTH-1:0];
                C = diff[DATAWIDTH]; // 借位标志（实际是!Borrow）
                // 溢出标志：正数减负数得负数，或负数减正数得正数
                V = (A[DATAWIDTH-1] != B[DATAWIDTH-1]) && 
                    (Result[DATAWIDTH-1] != A[DATAWIDTH-1]);
            end

            2'b10: begin // AND
                Result = A & B;
            end

            2'b11: begin // OR
                Result = A | B;
            end
        endcase

        // 设置标志位（对逻辑运算也有效）
        N = Result[DATAWIDTH-1];  // 负标志
        Z = (Result == '0);       // 零标志
    end

endmodule
