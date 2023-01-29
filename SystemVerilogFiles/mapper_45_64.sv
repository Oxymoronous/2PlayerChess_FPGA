module mapper_45_64(
input logic [9:0] dx, dy,
output logic [5:0] dxout, dyout
);

int dxfoo, dyfoo;

//handling offsets
assign dxfoo = dx - 140;
assign dyfoo = dy - 60;

always_comb begin
	dxout = dxfoo % 45;
	dyout = dyfoo % 45;
end

endmodule
