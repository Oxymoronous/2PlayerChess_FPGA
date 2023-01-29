/*
 * ECE385-HelperTools/PNG-To-Txt
 * Author: Rishi Thakkar
 * 
 */

 
//modified to infer on-chip memory ROM for 32x32 chess sprites
module  frameRAM
(
		input [14:0] read_address,	//2^14 = 16384, 2^15 = 32768, corresponds to how many rows we have in total
		input Clk,

		output logic [11:0] data_Out
);

//Memory structure:
//	[(number of bits per pixel - 1) : 0] mem [0 : (number of addresses-1)]
//	we have 13 sprites (6 black, 6 white, 1 empty tile)
//	1 sprite = 2025 rows (1 row = 24 pixels = 8 red, 8 green, 8 blue)
// so memory structure will have 2025 * 13 = 26325 rows, with each row having 24 bits
//	this in total makes chess sprite takes up 13312 * 24 = 319488 bits

logic [11:0] mem [0:26324];

initial
begin
	 $readmemh("sprite_bytes/brook.txt", mem, 0, 2024);	//black rook
	 $readmemh("sprite_bytes/bknight.txt", mem, 2025, 4049);	//black knight
	 $readmemh("sprite_bytes/bbishop.txt", mem, 4050, 6074);	//black bishop
	 $readmemh("sprite_bytes/bqueen.txt", mem, 6075, 8099);	//black queen
	 $readmemh("sprite_bytes/bking.txt", mem, 8100, 10124);	//black king
	 $readmemh("sprite_bytes/bpawn.txt", mem, 10125, 12149);	//black pawn
	 
	 $readmemh("sprite_bytes/Chess_pld45.txt", mem, 12150, 14174);	//white pawn
	 $readmemh("sprite_bytes/Chess_rld45.txt", mem, 14175, 16199);	//white rook
	 $readmemh("sprite_bytes/Chess_nld45.txt", mem, 16200, 18224);	//white knight
	 $readmemh("sprite_bytes/Chess_bld45.txt", mem, 18225, 20249);	//white bishop
	 $readmemh("sprite_bytes/Chess_qld45.txt", mem, 20250, 22274);	//white queen
	 $readmemh("sprite_bytes/Chess_kld45.txt", mem, 22275, 24299);	//white king
	 
	 $readmemh("sprite_bytes/Emptytile_45.txt", mem, 24300, 26324);	//empty tile
end


always_ff @ (posedge Clk) begin
	data_Out<= mem[read_address];
end

endmodule
