module finalproject(
	input MAX10_CLK1_50, 
	output				DRAM_CLK, 
	output				DRAM_CKE,
	output	[12:0]	DRAM_ADDR,
	output	[1:0]		DRAM_BA,
	inout		[15:0]	DRAM_DQ,
	output				DRAM_LDQM,
	output				DRAM_UDQM,
	output				DRAM_CS_N,
	output				DRAM_WE_N,
	output				DRAM_CAS_N,
	output				DRAM_RAS_N,
	
	output				VGA_HS,
	output				VGA_VS,
	output	[3:0]		VGA_R,
	output	[3:0]		VGA_G,
	output	[3:0]		VGA_B,
	
	inout		[15:0]	ARDUINO_IO,
	inout					ARDUINO_RESET_N
);

logic SPI0_CS_N, SPI0_SCLK, SPI0_MISO, SPI0_MOSI, USB_GPX, USB_IRQ, USB_RST;
logic [7:0] my_x_displacement, my_y_displacement, my_button_status;
//=======================================================
//  Structural coding
//=======================================================
	assign ARDUINO_IO[10] = SPI0_CS_N;
	assign ARDUINO_IO[13] = SPI0_SCLK;
	assign ARDUINO_IO[11] = SPI0_MOSI;
	assign ARDUINO_IO[12] = 1'bZ;
	assign SPI0_MISO = ARDUINO_IO[12];
	
	assign ARDUINO_IO[9] = 1'bZ;
	assign USB_IRQ = ARDUINO_IO[9];

	//Assignments specific to Circuits At Home UHS_20
	assign ARDUINO_RESET_N = USB_RST;
	assign ARDUINO_IO[7] = USB_RST;
	assign ARDUINO_IO[8] = 1'bZ;
	assign USB_GPX = 1'b0;
	
	//Assign uSD CS to '1' to prevent uSD card from interfering with USB Host (if uSD card is plugged in)
	assign ARDUINO_IO[6] = 1'b1;
	

finalproj_soc u0 (
		.button_status_export 					(my_button_status),						
		.clk_clk                           (MAX10_CLK1_50),    //clk.clk
		.reset_reset_n                     (1'b1),             //reset.reset_n
		.altpll_0_locked_conduit_export    (),    			   //altpll_0_locked_conduit.export
		.altpll_0_phasedone_conduit_export (), 				   //altpll_0_phasedone_conduit.export
		.altpll_0_areset_conduit_export    (),     			   //altpll_0_areset_conduit.export
    
		//.key_external_connection_export    (KEY),    		   //key_external_connection.export

		//SDRAM
		.sdram_clk_clk(DRAM_CLK),            				   //clk_sdram.clk
	   .sdram_wire_addr(DRAM_ADDR),               			   //sdram_wire.addr
		.sdram_wire_ba(DRAM_BA),                			   //.ba
		.sdram_wire_cas_n(DRAM_CAS_N),              		   //.cas_n
		.sdram_wire_cke(DRAM_CKE),                 			   //.cke
		.sdram_wire_cs_n(DRAM_CS_N),                		   //.cs_n
		.sdram_wire_dq(DRAM_DQ),                  			   //.dq
		.sdram_wire_dqm({DRAM_UDQM,DRAM_LDQM}),                //.dqm
		.sdram_wire_ras_n(DRAM_RAS_N),              		   //.ras_n
		.sdram_wire_we_n(DRAM_WE_N),                		   //.we_n
		
		//VGA
		.vga_port_red (VGA_R),
		.vga_port_green (VGA_G),
		.vga_port_blue (VGA_B),
		.vga_port_hs (VGA_HS),
		.vga_port_vs (VGA_VS),
		
		//USB SPI
		.spi0_SS_n(SPI0_CS_N),
		.spi0_MOSI(SPI0_MOSI),
		.spi0_MISO(SPI0_MISO),
		.spi0_SCLK(SPI0_SCLK),
		
		//USB GPIO
		.usb_rst_export(USB_RST),
		.usb_irq_export(USB_IRQ),
		.usb_gpx_export(USB_GPX),
		
		.x_displacement_export(my_x_displacement),
		.y_displacement_export(my_y_displacement)
);

endmodule
