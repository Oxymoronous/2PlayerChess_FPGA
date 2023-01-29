/************************************************************************
Avalon-MM Interface VGA Text mode display

************************************************************************/
`define CHESSBOARD_DIM	256
`define BUFFER_TILE		64

module vga_text_avl_interface (
	// Avalon Clock Input, note this clock is also used for VGA, so this must be 50Mhz
	// We can put a clock divider here in the future to make this IP more generalizable
	input logic CLK,
	
	// Avalon Reset Input
	input logic RESET,
	
	// Avalon-MM Slave Signals
	input  logic AVL_READ,					// Avalon-MM Read
	input  logic AVL_WRITE,					// Avalon-MM Write
	input  logic AVL_CS,					// Avalon-MM Chip Select
	input  logic AVL_BYTE_EN,			// Avalon-MM Byte Enable
	input  logic [9:0] AVL_ADDR,			// Avalon-MM Address
	input  logic [7:0] AVL_WRITEDATA,		// Avalon-MM Write Data
	output logic [7:0] AVL_READDATA,		// Avalon-MM Read Data
	
	// Exported Conduit (mapped to VGA port - make sure you export in Platform Designer)
	output logic [3:0]  red, green, blue,	// VGA color channels (mapped to output pins in top-level)
	output logic hs, vs,						// VGA HS/VS
	
	//----------------MOUSE-------------------
	input logic [15:0] x_displacement, y_displacement, button_status
	
);


logic [9:0] dx, dy;
logic pixel_clk, blank, sync, mouse_pixel_type, mouse_inside_chess;
logic [7:0] VGA_READDATA;
logic [9:0] mousex_out, mousey_out;

   vga_controller myvga(.Clk(CLK),       // 50 MHz clock-------------------------------INPUT----------------
                          .Reset(RESET),     // reset signal
								  //----------------OUTPUT------------------
									.hs(hs),        // Horizontal sync pulse.  Active low
								   .vs(vs),        // Vertical sync pulse.  Active low
							      .pixel_clk(pixel_clk), // 25 MHz pixel clock output
									.blank(blank),     // Blanking interval indicator.  Active low.
									.sync(sync),      // Composite Sync signal.  Active low.  We don't use it in this lab,
												             //   but the video DAC on the DE2 board requires an input for it.
									//-----------------------OUTPUT-----------------------
									.DrawX(dx),     // horizontal coordinate
								    .DrawY(dy) );   // vertical coordinate
									 
	ocm onchip_memory(.avl_writein(AVL_WRITEDATA), .vga_writein(), .avl_addr(AVL_ADDR), .vga_addr(vram_index),
							.avl_we(AVL_WRITE), .vga_we(1'b0), .clk(CLK),
							.avl_readout(AVL_READDATA), .vga_readout(VGA_READDATA));
	
	mouse mymouse(.Reset(RESET), .frame_clk(vs), .x_displacement(x_displacement), .y_displacement(y_displacement), 
						.mouse_x(mousex_out), .mouse_y(mousey_out));
	
	mouse_cursor mymousecursor(.mouse_x(mousex_out), .mouse_y(mousey_out), .draw_x(dx), .draw_y(dy),
										.pixel_type(mouse_pixel_type), .inside_chess(mouse_inside_chess));
	
	
int unsigned chess_row, chess_column, vram_index, dx_uint, dy_uint, dx_foo, dy_foo, dy_foo2;
int unsigned sb_row, sb_col, px_pos;
logic [9:0] px_offset;

logic inside_chessboard, tile_parity, inside_statusbar;			//determine whether we are inside the 256*256 pixel dimensions
logic inside_settingsbar, inside_evalbar;
logic black_check, white_check, background_check, glow_check, border_check;

logic [3:0] chess_sprite_code; 
logic [14:0] chess_rom_index;
logic [11:0] chess_sprite_pixel;

//character for status bar
logic [6:0] character_code;
logic [10:0] font_addr;
logic [7:0] font_data;
logic font_pixel;

chess_determine_rowcol cdrc(.x_coordinate(dx), .y_coordinate(dy), .tile_row(chess_row), .tile_col(chess_column));
statusbar_rowcol 		  sbrc(.dx(dx), .dy(dy), .row(sb_row), .col(sb_col));

assign dx_uint = dx;
assign dy_uint = dy;
assign dx_foo = (dx_uint-140)%45;
assign dy_foo = ((dy_uint-60)%45) * 45;
assign dy_foo2 = (dy_uint-60)%45;

assign chess_sprite_code = VGA_READDATA[3:0];	//5'b? _ _ _ _
assign tile_parity = VGA_READDATA[4];				//5'b1 _ _ _ _
assign glow_check = VGA_READDATA[5];
assign chess_rom_index 	=	(chess_sprite_code * 2025) + dy_foo + dx_foo;

//first status bar pixel index is 504
//640 - 504 = 136 --> 136 / 8 = 17
assign character_code = VGA_READDATA[6:0];
assign character_invisible = VGA_READDATA[7];				//using invert bit as transparent bit
assign font_addr = {character_code, dy[3:0]};			//dy % 16 = 0..15 row inside character sprite
assign px_offset = (dx - 504);								//dx fixing offset
assign px_pos = (7-px_offset[2:0]);								//dx % 8 = get the exact pixel position
assign font_pixel = font_data[px_pos]; 
font_rom myfontrom(.addr(font_addr), .data(font_data));	

frameRAM mychessROM(.read_address(chess_rom_index), .Clk(CLK), .data_Out(chess_sprite_pixel));

always_comb begin
	if (chess_sprite_pixel == 12'hbbb) begin	//12'hbbb is the special color code for background
		background_check = 1;
	end
	
	else begin
		background_check = 0;
	end
	
	if (dx_foo == 0 || dy_foo == 0 || dx_foo == 44 || dy_foo2 == 44) begin
		border_check = 1;
	end
	else begin
		border_check = 0;
	end
	
end

always_comb begin
	//chessboard
	if ((dx >= 140) && (dx <=499) && (dy >= 60) && (dy <= 419)) begin
		inside_chessboard = 1'b1;
	end
	else begin
		inside_chessboard = 1'b0;
	end
	
	//status bar
	if ((dx >= 504) && (dx <= 639)) begin
		inside_statusbar = 1'b1;
	end
	else begin
		inside_statusbar = 1'b0;
	end
	
	//settings bar
	if ((dy>=352 && dy<=367) || (dy>=384 && dy<=399) || (dy>=416 && dy<=431))begin
		inside_settingsbar = 1'b1;
	end
	else begin
		inside_settingsbar = 1'b0;
	end
	
	//eval bar
	if(dy>=320 && dy <=335) begin
		inside_evalbar = 1'b1;
	end
	else begin
		inside_evalbar = 1'b0;
	end

	//getting the correct VRAM index
	if (inside_chessboard) begin
		vram_index = (chess_row * 8) + chess_column; //for chessboards, this should be in range 0 to 63 
																	//because we have 64 chess tiles
	end
	else if (inside_statusbar) begin
		vram_index = 64 + (sb_row * 17) + sb_col;
	end
	
	else begin
		vram_index = `BUFFER_TILE;
	end
end

always_ff @(posedge pixel_clk) begin 
	if (~blank) begin
		red <= 0;
		green <= 0;
		blue <= 0;
	end
	
	else if (mouse_pixel_type) begin
		red <= 4'b1001;
		green <= 4'b1001;
		blue <= 4'b1001;
	end
	
	//chess tile
	else if (inside_chessboard & ~background_check) begin
		red <= chess_sprite_pixel[11:8];
		green <= chess_sprite_pixel[7:4];
		blue <= chess_sprite_pixel[3:0];
	end
	
	//possible move tile
	//(maintain top pixel row, bottom pixel row, left pixel column, right pixel column as background)
	else if (inside_chessboard & background_check & glow_check & ~border_check) begin
		//light pink
		red <= 4'hf;
		green <= 4'hc;
		blue <= 4'hc;
	end
	
	//border line (don't want pink)
	else if (inside_chessboard & background_check) begin
		//darkbrown == 12bit => 1100_0110_0000 => 0xC60
		if (tile_parity) begin
			red	<=	4'hf;
			green <= 4'hc;
			blue	<= 4'h9;
		end
		
		//lightbrown 12bit => 1111_1100_1001 => 0xFC9
		else begin
			red	<=	4'hc;
			green <= 4'h6;
			blue	<= 4'h0;
		end
	end
	
	else begin
		if (inside_statusbar) begin
			//a regular character
			if (font_pixel & ~character_invisible & ~inside_settingsbar & ~inside_evalbar) begin
				//font grey 0x808080 = 12bit = 777
				red <= 4'b0111;	
				green <= 4'b0111;
				blue <= 4'b0111;
			end
			
			//background pixel inside a settings bar
			//if invisible bit is low, that means settings is clicked
			else if(~font_pixel & inside_settingsbar & ~character_invisible) begin
				//settings bar background is red
				red <= 4'b1111;
				green <= 4'b0000;
				blue <= 4'b0000;
			end
			
			//background pixel inside a settings bar 
			//and settings bar is unclicked 
			else if(~font_pixel & inside_settingsbar & character_invisible)begin
				//settings bar background = 090 = dark green
				red <= 4'b0000;
				green <= 4'b1001;
				blue <= 4'b0000;
			end
			
			//font pixel inside settings bar
			else if(font_pixel & inside_settingsbar)begin
				red <= 4'b1111;
				green <= 4'b1111;
				blue <= 4'b1111;
			end
			
			//a white advantage bar
			else if(font_pixel & inside_evalbar & ~character_invisible)begin
				red <= 4'b1111;
				green <= 4'b1111;
				blue <= 4'b1111;
			end
			
			//a black advantage bar
			else if(font_pixel & inside_evalbar & character_invisible)begin
				red <= 4'b0000;
				green <= 4'b0000;
				blue <= 4'b0000;
			end
			
			else begin
				//font bg light grey 0xe0e0e0 = 12bit = DDD
				red <= 4'b1101;	
				green <= 4'b1101;
				blue <= 4'b1101;
			end
		end
		else begin
			//dark blue = 0x306
			red <= 4'b0011;	
			green <= 4'b0000;
			blue <= 4'b0110;
		end
	end
end
		
endmodule
