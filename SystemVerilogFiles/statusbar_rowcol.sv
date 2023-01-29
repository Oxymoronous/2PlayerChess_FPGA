module statusbar_rowcol
(	
input logic [9:0] dx, dy,
output int unsigned row, col
);

logic [9:0] tmp;
assign tmp = dx - 504;		//handling the offset

always_comb begin
	row = dy[9:4];//dy divide 16
	col = tmp[9:3];//dx divide 8
end

endmodule
