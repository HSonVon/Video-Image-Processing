
module Manhattan (Clk, RstN, X, T, WTA);
	input Clk;
	input RstN;
	input [7:0] X;
	input [7:0] T;
	output [11:0] WTA;

	reg signed [7:0] x,t;
	reg [11:0] WTA, tmp;
	reg [4:0] cnt = 1'b0;
	reg [8:0] sum, sum_xor;
	reg Ca;
always @(posedge Clk, negedge RstN) begin
	if(~RstN) begin
		WTA <= 12'b0;
		sum <= 1'b0;
		tmp <= 1'b0;
	end else begin
		x = ~X + 1'b1 ;
		t =  T;
		sum =  {1'b0,t} + {1'b0,x};
		Ca = sum[8] ;			
		if (cnt == 16) begin
			WTA <= tmp;
			cnt <= 1'b0;
			tmp <= 1'b0;
		end else begin
			cnt <= cnt + 1'b1;
			if(Ca == 0) begin
				sum_xor = sum[7:0] ^ {8{~Ca}} ;
				tmp = tmp + sum_xor[7:0] + 1'b1;
			end else 
				tmp <= tmp + sum[7:0] ; 
		end
   end
end
endmodule 