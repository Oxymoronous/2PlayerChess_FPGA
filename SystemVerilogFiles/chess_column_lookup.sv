//Look up table to determine chess tile column based on dx position
`define TILE45	45

module chess_determine_rowcol(
	input logic [9:0] x_coordinate, y_coordinate,	//passing in dx, dy
	output int unsigned tile_row, tile_col
);

int unsigned x_temp, y_temp;
assign x_temp = x_coordinate;		//convert coordinate 
assign y_temp = y_coordinate;

logic c0, c1, c2, c3, c4, c5, c6, c7, one;
logic r0, r1, r2, r3, r4, r5, r6, r7;

int col_answer, row_answer;

assign one = 1'b1;
assign c0 = (x_temp >= 140) && (x_temp <=184);
assign c1 = (x_temp >= 185) && (x_temp <=229);
assign c2 = (x_temp >= 230) && (x_temp <=274);
assign c3 = (x_temp >= 275) && (x_temp <=319);
assign c4 = (x_temp >= 320) && (x_temp <=364);
assign c5 = (x_temp >= 365) && (x_temp <=409);
assign c6 = (x_temp >= 410) && (x_temp <=454);
assign c7 = (x_temp >= 455) && (x_temp <=499);

assign r0 = (y_temp >= 60) && (y_temp <=104);
assign r1 = (y_temp >= 105) && (y_temp <=149);
assign r2 = (y_temp >= 150) && (y_temp <=194);
assign r3 = (y_temp >= 195) && (y_temp <=239);
assign r4 = (y_temp >= 240) && (y_temp <=284);
assign r5 = (y_temp >= 285) && (y_temp <=329);
assign r6 = (y_temp >= 330) && (y_temp <=374);
assign r7 = (y_temp >= 375) && (y_temp <=419);


always_comb begin		
	unique case (one)
		c0	:	col_answer = 0;
		c1	:	col_answer = 1;
		c2	:	col_answer = 2;
		c3	:	col_answer = 3;	
		c4	:	col_answer = 4;
		c5	:	col_answer = 5;
		c6	:	col_answer = 6;
		c7	:	col_answer = 7;
	endcase

	unique case (one)
		r0	:	row_answer = 0;
		r1	:	row_answer = 1;
		r2	:	row_answer = 2;
		r3	:	row_answer = 3;	
		r4	:	row_answer = 4;
		r5	:	row_answer = 5;
		r6	:	row_answer = 6;
		r7	:	row_answer = 7;
	endcase
end

assign tile_row = row_answer;
assign tile_col = col_answer;

endmodule

//	c0 = x_temp inside {[140:184]};
//	c1 = x_temp inside {[185:229]};
//	c2 = x_temp inside {[230:274]};
//	c3 = x_temp inside {[275:319]};
//	c4 = x_temp inside {[320:364]};
//	c5 = x_temp inside {[365:409]};
//	c6 = x_temp inside {[410:454]};
//	c7 = x_temp inside {[455:499]};

	
//	r0 = y_temp inside {[60:104]};
//	r1 = y_temp inside {[105:149]};
//	r2 = y_temp inside {[150:194]};
//	r3 = y_temp inside {[195:239]};
//	r4 = y_temp inside {[240:284]};
//	r5 = y_temp inside {[285:329]};
//	r6 = y_temp inside {[330:374]};
//	r7 = y_temp inside {[375:419]};