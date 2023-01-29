//outputs new coordinates of (mouse_x, mouse_y) after getting displacement vector from USB mouse movements
//our mouse is a 16 x 16 sprite
`define VGA_X_MAX	640
`define VGA_Y_MAX 480

module mouse(
input logic Reset, frame_clk, 
input logic [15:0] x_displacement, y_displacement,		//signals passed in will always be positive position that is within screen bounds
output logic [9:0] mouse_x, mouse_y
);
logic [15:0] mouse_x_pos, mouse_y_pos;
//the center of the chess board is (320, 240)
parameter [9:0] mouse_x_center = 320;
parameter [9:0] mouse_y_center = 240;
parameter [9:0] mouse_x_min = 0; 		//	7 
parameter [9:0] mouse_x_max = 640 - 16;			// 632
parameter [9:0] mouse_y_min = 0;		// 7
parameter [9:0] mouse_y_max = 480 - 16; 			// 472 

always_ff @ (posedge Reset or posedge frame_clk) 
begin
		if (Reset) begin
			mouse_y_pos <= mouse_y_center;
			mouse_x_pos <= mouse_x_center;
		end
end

assign mouse_x = x_displacement[9:0];
assign mouse_y = y_displacement[9:0];

endmodule
				