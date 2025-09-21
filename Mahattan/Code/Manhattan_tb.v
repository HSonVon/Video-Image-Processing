module Manhattan_tb;
    reg clk, rst;
    reg [7:0] in_a, in_b;
    wire [11:0] distance;

    Manhattan uut (
        .Clk(clk),
        .RstN(rst),
        .X(in_a),
        .T(in_b),
        .WTA(result)
    );

    always #5 clk = ~clk;

    reg signed [7:0] test_in_a1 [0:15];
    reg signed [7:0] test_in_b1 [0:15];
    reg signed [7:0] test_in_a2 [0:15];
    reg signed [7:0] test_in_b2 [0:15];
    integer i;

    initial begin
        clk = 0; 
        rst = 0;

        test_in_a1[0]  = 8'd10;  test_in_b1[0]  = 8'd15;
        test_in_a1[1]  = 8'd8;   test_in_b1[1]  = 8'd6;
        test_in_a1[2]  = 8'd5;   test_in_b1[2]  = 8'd25;
        test_in_a1[3]  = 8'd12; test_in_b1[3]  = 8'd18;
        test_in_a1[4]  = 8'd40; test_in_b1[4]  = 8'd37;
        test_in_a1[5]  = 8'd25;  test_in_b1[5]  = 8'd22;
        test_in_a1[6]  = 8'd3;   test_in_b1[6]  = 8'd10;
        test_in_a1[7]  = 8'd8;   test_in_b1[7]  = 8'd16;
        test_in_a1[8]  = 8'd5;  test_in_b1[8]  = 8'd7;
        test_in_a1[9]  = 8'd8;  test_in_b1[9]  = 8'd4;
        test_in_a1[10] = 8'd3;   test_in_b1[10] = 8'd11;
        test_in_a1[11] = 8'd6;   test_in_b1[11] = 8'd15;
        test_in_a1[12] = 8'd10; test_in_b1[12] = 8'd7;
        test_in_a1[13] = 8'd5;   test_in_b1[13] = 8'd23;
        test_in_a1[14] = 8'd3;   test_in_b1[14] = 8'd10;
        test_in_a1[15] = 8'd8;   test_in_b1[15] = 8'd1;

        test_in_a2[0]  = 8'd12;  test_in_b2[0]  = 8'd13;
        test_in_a2[1]  = 8'd15;  test_in_b2[1]  = 8'd5;
        test_in_a2[2]  = 8'd6;   test_in_b2[2]  = 8'd2;
        test_in_a2[3]  = 8'd1; test_in_b2[3]  = 8'd17;
        test_in_a2[4]  = 8'd5; test_in_b2[4]  = 8'd30;
        test_in_a2[5]  = 8'd20;  test_in_b2[5]  = 8'd25;
        test_in_a2[6]  = 8'd5;   test_in_b2[6]  = 8'd6;
        test_in_a2[7]  = 8'd7;   test_in_b2[7]  = 8'd10;
        test_in_a2[8]  = 8'd6;  test_in_b2[8]  = 8'd9;
        test_in_a2[9]  = 8'd7;  test_in_b2[9]  = 8'd2;
        test_in_a2[10] = 8'd4;   test_in_b2[10] = 8'd12;
        test_in_a2[11] = 8'd8;   test_in_b2[11] = 8'd14;
        test_in_a2[12] = 8'd9;  test_in_b2[12] = 8'd5;
        test_in_a2[13] = 8'd7;   test_in_b2[13] = 8'd17;
        test_in_a2[14] = 8'd4;   test_in_b2[14] = 8'd8;
        test_in_a2[15] = 8'd9;   test_in_b2[15] = 8'd3;

        #40 rst = 1;

        for (i = 0; i < 16; i = i + 1) begin
            in_a = test_in_a1[i];
            in_b = test_in_b1[i];
            #10;
        end
	#10
        for (i = 0; i < 16; i = i + 1) begin
            in_a = test_in_a2[i];
            in_b = test_in_b2[i];
            #10;
        end

        #20 $stop;
    end
endmodule

