#include <stdio.h>
#include <xio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "xparameters.h"
#include "cam_ctrl_header.h"
#include "vmodcam_header.h"

#define blDvmaCR        0x00000000 // Control Reg Offset
#define blDvmaFWR        0x00000004 // Frame Width Reg Offset
#define blDvmaFHR        0x00000008 // Frame Height Reg Offset
#define blDvmaFBAR    0x0000000c // Frame Base Addr Reg Offset
#define blDvmaFLSR    0x00000010 // Frame Line Stride Reg Offeset
#define blDvmaHSR        0x00000014 // H Sync Reg Offset
#define blDvmaHBPR    0x00000018 // H Back Porch Reg Offset
#define blDvmaHFPR    0x0000001c // H Front Porch Reg Offset
#define blDvmaHTR        0x00000020 // H Total Reg Offset
#define blDvmaVSR        0x00000024 // V Sync Reg Offset
#define blDvmaVBPR    0x00000028 // V Back Porch Reg Offset
#define blDvmaVFPR    0x0000002c // V Front Porch Reg Offset
#define blDvmaVTR        0x00000030 // V Total Reg Offset

void main()
{
    u32 lDvmaBaseAddress = XPAR_DVMA_0_BASEADDR;
    short posX = 0;
    short posY = 0

    xil_printf("test");
    for (posX = 0; posX < 2560; posX++)
    {
        for (posY = 0; posY < 720; posY++)
        {
            XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (posY * 2560 + posX), (posX / 40) << 3);
        }
    }

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


//    CamIicCfg(XPAR_CAM_IIC_0_BASEADDR, CAM_CFG_640x480P);
//
//    for (posX = 0; posX < 2560; posX++)
//    {
//        for (posY = 0; posY < 720; posY++)
//        {
//            XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (posY * 2560 + posX), 0X0F0);
//        }
//    }
//
//    xil_printf("Finished camera configuration.\r\n");
//
//    CamCtrlInit(XPAR_CAM_CTRL_0_BASEADDR, CAM_CFG_640x480P, 640 * 2);

    // Draw the background color
    for (posX = 0; posX < 2560; posX++)
    {
        for (posY = 0; posY < 720; posY++)
        {
            XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (posY * 2560 + posX), 0xFFF);
        }
    }

    short squares[10][10];
    short color_square[10][10];

    short i = 0;
    short j = 0;
    for (i = 0; i < 10; i++)
    {
        for (j = 0; j < 10; j++)
        {
            squares[i][j] = 0;
            color_square[i][j] = 0;
        }

    }

//    short count = 0;

    while (1)
    {
        // Scanning through pixels
        short row = 0;
        short col = 0;

        short pos_x = 0;
        short pos_ = 0;

        short most_red_row = 0;
        short most_red_column = 0;

        for (row = 0; row < 10; row++)
        {
            for (col = 0; col < 10; col++)
            {
                short total_red = 0;
                short total_green = 0;
                short total_blue = 0;

                for (pos_x = row * 64; pos_x < (row + 1) * 64; ++pos_x)
                {
                    for (pos_y = col * 48; pos_y < (col + 1) * 48; ++pos_y)
                    {
                        u16 color = XIo_In16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x + 640));

                        // 3333_2222_1111_0000
                        // 2s are red. 1s are green. 0s are blue.

                        float r = ((color >> 8) & 0b1111) / 15.0;
                        float g = ((color >> 4) & 0b1111) / 15.0;
                        float b = (color & 0b1111) / 15.0;

                        float h = 0;
                        float s = 0;
                        float v = 0;

                        float min = r < g ? r : g;
                        min = b < min ? b : min;

                        float max = r > g ? r : g;
                        max = b > max ? b : max;

                        v = max;

                        float delta = max - min;

                        if (max == 0)
                        {
                            s = 0;
                            h = -1;
                        }
                        else
                        {
                            s = delta / max;

                            if (r == max)
                            {
                                h = (g - b) / delta;
                            }
                            else if (g == max)
                            {
                                h = 2 + (b - r) / delta;
                            }
                            else
                            {
                                h = 4 + (r - g) / delta;
                            }

                            h *= 60;

                            if (h < 0)
                            {
                                h += 360;
                            }
                        }

                        total_red += (short) h;

                        if (h > -360 && h < 0 && s < 0.9 && v < 0.5)
                        {
                            XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x), color);
                        }
                        else
                        {
                            XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x), 0);
                        }

                    }
                }

                color_square[row][col] = total_red;
            }
        }
        // End Scanning

        // finding most red
        for (row = 0; row < 10; ++row)
        {
            for (col = 0; col < 10; ++col)
            {
                if (color_square[row][col] < color_square[most_red_row][most_red_column])
                {
                    most_red_row = row;
                    most_red_column = col;
                }
            }
        }
        squares[most_red_row][most_red_column] = 1; // End finding red

//        // Drawing
//        for (row = 0; row < 10; ++row)
//        {
//            for (col = 0; col < 10; ++col)
//            {
//                if (squares[row][col])    // if this is marked
//                {
//                    short pos_x = 0;
//                    short pos_y = 0;
//
//                    for (pos_x = row * 64; pos_x < (row + 1) * 64; ++pos_x)
//                    {
//                        for (pos_y = col * 48; pos_y < (col + 1) * 48; ++pos_y)
//                        {
//                            XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x), 0xF00);
//                        }
//                    }
//
//                }
//            }
//        }
//        // End of Draw
    }
}
