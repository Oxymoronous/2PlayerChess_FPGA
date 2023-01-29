//Based on Color_Mapper.sv
//determines the logic of drawing the mouse cursor based on mouse sprite
//

module mouse_cursor (
	input logic 	[9:0]	mouse_x, mouse_y, draw_x, draw_y, 
	output logic 	pixel_type,		//if pixel type is single bit 1, then we dont care about inside_chess
											//this overrides all other possible RGB outputs
	output logic inside_chess				//if pixel type is NOT single bit 1, then inside_chess or outside_chess determines accessing VRAM
);
	logic inside_mouse;
	logic [3:0] mouse_rom_addr;
	logic [15:0] mouse_rom_out;
	mouse_rom mymouserom(.addr(mouse_rom_addr), .data(mouse_rom_out));
	
	//check whether dx, dy are inside a mouse sprite
		//if dx, dy are inside a mouse sprite:
			//if dx, dy lands on a mouse pixel
				//determine whether that is a mouse 1 pixel, or a mouse 0 pixel
		//else
			//return a special pixel type to indicate that we did not land on a mouse
		
	always_comb begin
		if ((draw_x < mouse_x + 16) && (draw_x >= mouse_x) && (draw_y >= mouse_y) && (draw_y < mouse_y + 16)) begin
			inside_mouse = 1'b1;
		end
		else
			inside_mouse = 1'b0;
		
		if ((draw_x >= 140) && (draw_x <= 499) && (draw_y >=60) && (draw_y <= 419))begin
			inside_chess = 1'b1;
		end
		else
			inside_chess = 1'b0;
		
		if (inside_mouse) begin
			mouse_rom_addr = draw_y - mouse_y;		//this should be from a range of 0 to 15, used as an index to get us a row slice of mouse sprite
			pixel_type = mouse_rom_out[(draw_x - mouse_x) ^ 15];	//if draw_x - mouse_x == 15, we want to get the lowest bit, 0
																		//if draw_x - mouse_x == 0, we want to get the highest bit, 15
		end
		
		else begin
			mouse_rom_addr = 4'b0000;	//not sure if this is needed, just adding for system verilog if-else purposes
			pixel_type = 0;	//if it is a mouse pixel, it can only be 0 or 1, 2'b11 == a non-mouse pixel outside ;
		end
	end

endmodule
