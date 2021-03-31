#include <stdio.h>
#include <xio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "xparameters.h"
#include "cam_ctrl_header.h"
#include "vmodcam_header.h"

#define blDvmaCR		0x00000000 // Control Reg Offset
#define blDvmaFWR		0x00000004 // Frame Width Reg Offset
#define blDvmaFHR		0x00000008 // Frame Height Reg Offset
#define blDvmaFBAR	0x0000000c // Frame Base Addr Reg Offset
#define blDvmaFLSR	0x00000010 // Frame Line Stride Reg Offeset
#define blDvmaHSR		0x00000014 // H Sync Reg Offset
#define blDvmaHBPR	0x00000018 // H Back Porch Reg Offset
#define blDvmaHFPR	0x0000001c // H Front Porch Reg Offset
#define blDvmaHTR		0x00000020 // H Total Reg Offset
#define blDvmaVSR		0x00000024 // V Sync Reg Offset
#define blDvmaVBPR	0x00000028 // V Back Porch Reg Offset
#define blDvmaVFPR	0x0000002c // V Front Porch Reg Offset
#define blDvmaVTR		0x00000030 // V Total Reg Offset
void main() {
	u32 lDvmaBaseAddress = XPAR_DVMA_0_BASEADDR;
	short posX, posY;

	xil_printf("test");
	for (posX = 0; posX < 2560; posX++)
		for (posY = 0; posY < 720; posY++)
			XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2*(posY*2560+posX),
					(posX / 40) << 3);

	XIo_Out32(lDvmaBaseAddress + blDvmaHSR, 40); // hsync
	XIo_Out32(lDvmaBaseAddress + blDvmaHBPR, 260); // hbpr
	XIo_Out32(lDvmaBaseAddress + blDvmaHFPR, 1540); // hfpr
	XIo_Out32(lDvmaBaseAddress + blDvmaHTR, 1650); // htr
	XIo_Out32(lDvmaBaseAddress + blDvmaVSR, 5); // vsync
	XIo_Out32(lDvmaBaseAddress + blDvmaVBPR, 25); // vbpr
	XIo_Out32(lDvmaBaseAddress + blDvmaVFPR, 745); // vfpr
	XIo_Out32(lDvmaBaseAddress + blDvmaVTR, 750); // vtr

	XIo_Out32(lDvmaBaseAddress + blDvmaFWR, 0x00000500); // frame width
	XIo_Out32(lDvmaBaseAddress + blDvmaFHR, 0x000002D0); // frame height
	XIo_Out32(lDvmaBaseAddress + blDvmaFBAR, XPAR_DDR2_SDRAM_MPMC_BASEADDR); // frame base addr
	XIo_Out32(lDvmaBaseAddress + blDvmaFLSR, 0x00000A00); // frame line stride
	XIo_Out32(lDvmaBaseAddress + blDvmaCR, 0x00000003); // dvma enable, dfl enable

	/*
	 CamIicCfg(XPAR_CAM_IIC_0_BASEADDR, CAM_CFG_640x480P);

	 for (posX = 0; posX < 2560; posX++)
	 for (posY = 0; posY < 720; posY++)
	 XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2*(posY*2560+posX),
	 0X0F0);

	 CamCtrlInit(XPAR_CAM_CTRL_0_BASEADDR, CAM_CFG_640x480P, 640 * 2);
	 */
	// Draw the background color
	for (posX = 0; posX < 2560; posX++)
		for (posY = 0; posY < 720; posY++)
			XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2*(posY*2560+posX),
					0xFFF);

	short color_square[10][10];

	short i, j;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			color_square[i][j] = 0;
		}

	}

	short last_red_row = 0, last_red_column = 0;

	while (1) {
		// Scanning through pixels
		short row = 0;
		short col = 0;

		short pos_x = 0;
		short pos_y = 0;

		short most_red_row = 0;
		short most_red_column = 0;

		for (row = 0; row < 10; row++) {
			for (col = 0; col < 10; col++) {
				short total_red = 0;
				short total_green = 0;
				short total_blue = 0;

				for (pos_x = row * 64; pos_x < (row + 1) * 64; ++pos_x) {
					for (pos_y = col * 48; pos_y < (col + 1) * 48; ++pos_y) {
						u16 color =
								XIo_In16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x + 640));

/*
						 if (((color >> 8) & 0b1111) > 10) {
						 XIo_Out16(
						 XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x),
						 color);
						 } else {
						 XIo_Out16(
						 XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x),
						 0);
						 }*/


						total_red += ((color >> 8) & 0b1111);

					}
				}

				color_square[row][col] = total_red;
			}
		}
		// End Scanning

		// finding most red
		for (row = 0; row < 10; ++row) {
			for (col = 0; col < 10; ++col) {
				if (color_square[row][col]
						< color_square[most_red_row][most_red_column]) {
					most_red_row = row;
					most_red_column = col;
				}
			}
		} // End finding red

		
		/*** Drawing ***/
		// Stay at the point
		if (most_red_row == last_red_row && most_red_column == last_red_column) {
			continue;
		}

		// Draw our starting point in green
		for (pos_x = last_red_row * 64; pos_x < (last_red_row + 1) * 64; ++pos_x) {
			for (pos_y = last_red_column * 48; pos_y < (last_red_column + 1) * 48;++pos_y) {
					XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x),0x00F0);
			}
		}

		// Draw our target point in red
		for (pos_x = most_red_row * 64; pos_x < (most_red_row + 1) * 64; ++pos_x) {
			for (pos_y = most_red_column * 48; pos_y < (most_red_column + 1) * 48;++pos_y) {
					XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x),0x0F00);
			}
		}

		/*** End of Drawing ***/

		/*** Drawing the irobot path ***/
		for (row = 0; row < 10; row++) {
			for (col = 0; col < 10; col++) {
				
				if (row == most_red_row && col == most_red_column) {
					for (pos_x = row * 64; pos_x < row * 64 + 5; ++pos_x) {
						for (pos_y = 480 + col * 24; pos_y < 480 + col * 24 + 5; ++pos_y) {
							u16 color =
								XIo_In16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x + 640), 0x0F00);
						}
					}
				} else if (row == last_red_row && col == last_red_column) {
					for (pos_x = row * 64; pos_x < row * 64 + 5; ++pos_x) {
						for (pos_y = 480 + col * 24; pos_y < 480 + col * 24 + 5; ++pos_y) {
							u16 color =
								XIo_In16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x + 640), 0x00F0);
						}
					}
				} else {
					for (pos_x = row * 64; pos_x < row * 64 + 5; ++pos_x) {
						for (pos_y = 480 + col * 24; pos_y < 480 + col * 24 + 5; ++pos_y) {
							u16 color =
								XIo_In16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x + 640), 0x0);
						}
					}
				}
			}
		}
		

		/*** End of Drawing the irobot path ***/

		last_red_row = most_red_row;
		last_red_column = most_red_column;
	}

}