// Quartus Prime Verilog Template
// Four Bits wide, 4-bit long shift register for crosstalk noise filtering on the LogicalStep Board

module pb_n_filters
(
	input clk, 
	input [3:0]pb_n,
	output [3:0]pb_n_fltrd
);

	// Declare a shift register for filtering each input
	reg [3:0] sr_a;
	reg [3:0] sr_b;
	reg [3:0] sr_c;
	reg [3:0] sr_d;

	// Shift everything over, load the incoming parallel bits
	always @ (posedge clk)
	begin
			sr_a[2:1] <= sr_a[1:0];
			sr_a[0] <= pb_n[0];
			sr_a[3] <= sr_a[2] | sr_a[1] | sr_a[0];

			sr_b[2:1] <= sr_b[1:0];
			sr_b[0] <= pb_n[1];
			sr_b[3] <= sr_b[2] | sr_b[1] | sr_b[0];
					
			sr_c[2:1] <= sr_c[1:0];
			sr_c[0] <= pb_n[2];
			sr_c[3] <= sr_c[2] | sr_c[1] | sr_c[0];
			
			sr_d[2:1] <= sr_d[1:0];
			sr_d[0] <= pb_n[3];
			sr_d[3] <= sr_d[2] | sr_d[1] | sr_d[0];
			
	end

	// Catch the outgoing bit
	assign pb_n_fltrd[0] = sr_a[3] ; // all three previous stages have to be active for the output to be active (ACTIVE LOW)
	assign pb_n_fltrd[1] = sr_b[3] ; // all three previous  stages have to be active for the output to be active (ACTIVE LOW)
	assign pb_n_fltrd[2] = sr_c[3] ; // all three previous  stages have to be active for the output to be active (ACTIVE LOW)
	assign pb_n_fltrd[3] = sr_d[3] ; // all three previous stages have to be active for the output to be active (ACTIVE LOW)

endmodule
