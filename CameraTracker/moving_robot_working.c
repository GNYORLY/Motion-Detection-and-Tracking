#include <stdio.h>
#include <xio.h>
#include <stdbool.h>

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

bool get_switch()
{
    return false;
}

int main()
{
    // Robot variables.
    short robot_size = 25;

    short robot_min_x_area = 0;
    short robot_max_x_area = 640;

    short robot_min_y_area = 0;
    short robot_max_y_area = 240;

    short robot_min_x_position = robot_min_x_area + robot_size;
    short robot_max_x_position = robot_max_x_area - robot_size;

    short robot_min_y_position = robot_min_y_area + robot_size;
    short robot_max_y_position = robot_max_y_area - robot_size;

    short robot_x_velocity = 0;
    short robot_y_velocity = 0;

    short robot_x_position = robot_max_x_area / 2;
    short robot_y_position = robot_max_y_area / 2;

    // Camera variables.
    bool show_robot_vision = true;

    // Code
    u32 lDvmaBaseAddress = XPAR_DVMA_0_BASEADDR;
    short posX = 0;
    short posY = 0;

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


    CamIicCfg(XPAR_CAM_IIC_0_BASEADDR, CAM_CFG_640x480P);

    for (posX = 0; posX < 2560; posX++)
    {
        for (posY = 0; posY < 720; posY++)
        {
            XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (posY * 2560 + posX), 0X0F0);
        }
    }

    CamCtrlInit(XPAR_CAM_CTRL_0_BASEADDR, CAM_CFG_640x480P, 640 * 2);

    // Draw the background color
    for (posX = 0; posX < 2560; posX++)
    {
        for (posY = 0; posY < 720; posY++)
        {
            XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (posY * 2560 + posX), 0xFFF);
        }
    }

    short color_square[10][10];

    short i = 0;
    short j = 0;
    for (i = 0; i < 10; i++)
    {
        for (j = 0; j < 10; j++)
        {
            color_square[i][j] = 0;
        }

    }

    short last_red_row = 0;
    short last_red_column = 0;

    while (true)
    {
        bool new_switch_value = get_switch();

        if (show_robot_vision != new_switch_value)
        {
            show_robot_vision = new_switch_value;

            // Clear the top-left of the screen.
            for (posX = 0; posX < 640; posX++)
            {
                for (posY = 0; posY < 480; posY++)
                {
                    XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (posY * 2560 + posX), 0xFFF);
                }
            }
        }

        // Scanning through pixels
        short row = 0;
        short col = 0;

        short pos_x = 0;
        short pos_y = 0;

        short most_red_row = 0;
        short most_red_column = 0;

        for (row = 0; row < 10; row++)
        {
            for (col = 0; col < 10; col++)
            {
                short total_red = 0;

                for (pos_x = row * 64; pos_x < (row + 1) * 64; ++pos_x)
                {
                    for (pos_y = col * 48; pos_y < (col + 1) * 48; ++pos_y)
                    {
                        u16 color = XIo_In16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x + 640));

                        if (show_robot_vision)
                        {
                            if (((color >> 8) & 0b1111) > 10)
                            {
                                XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x), color);
                            }
                            else
                            {
                                XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x), 0);
                            }
                        }


                        total_red += ((color >> 8) & 0b1111);

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
        } // End finding red


        /*** Drawing ***/
        // Stay at the point
        if (!(most_red_row == last_red_row && most_red_column == last_red_column))
        {
            if (!show_robot_vision)
            {
                // Draw our starting point in green
                for (pos_x = last_red_row * 64; pos_x < (last_red_row + 1) * 64; ++pos_x)
                {
                    for (pos_y = last_red_column * 48; pos_y < (last_red_column + 1) * 48; ++pos_y)
                    {
                        XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x), 0x00F0);
                    }
                }

                // Draw our target point in red
                for (pos_x = most_red_row * 64; pos_x < (most_red_row + 1) * 64; ++pos_x)
                {
                    for (pos_y = most_red_column * 48; pos_y < (most_red_column + 1) * 48; ++pos_y)
                    {
                        XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x), 0x0F00);
                    }
                }
            }

            /*** End of Drawing ***/

            /*** Drawing the irobot path ***/
            for (row = 0; row < 10; row++)
            {
                for (col = 0; col < 10; col++)
                {
                    // Draw the current block in red.
                    if (row == most_red_row && col == most_red_column)
                    {
                        for (pos_x = row * 64; pos_x < row * 64 + 5; ++pos_x)
                        {
                            for (pos_y = 480 + col * 24; pos_y < 480 + col * 24 + 5; ++pos_y)
                            {
                                XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x + 640), 0x0F00);
                            }
                        }
                    }
                        // Draw the last block in green.
                    else if (row == last_red_row && col == last_red_column)
                    {
                        for (pos_x = row * 64; pos_x < row * 64 + 5; ++pos_x)
                        {
                            for (pos_y = 480 + col * 24; pos_y < 480 + col * 24 + 5; ++pos_y)
                            {
                                XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x + 640), 0x00F0);
                            }
                        }
                    }
                        // Draw everything else in black.
                    else
                    {
                        for (pos_x = row * 64; pos_x < row * 64 + 5; ++pos_x)
                        {
                            for (pos_y = 480 + col * 24; pos_y < 480 + col * 24 + 5; ++pos_y)
                            {
                                XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x + 640), 0x0);
                            }
                        }
                    }
                }
            }

            robot_x_velocity = most_red_row - last_red_row;
            robot_y_velocity = most_red_column - last_red_column;

            /*** End of Drawing the irobot path ***/
            last_red_row = most_red_row;
            last_red_column = most_red_column;
        }

        // Draw the robot.
        if (robot_x_position + robot_x_velocity >= robot_min_x_position &&
            robot_x_position + robot_x_velocity <= robot_max_x_position)
        {
            robot_x_position += robot_x_velocity;
        }

        if (robot_y_position + robot_y_velocity >= robot_min_y_position &&
            robot_y_position + robot_y_velocity <= robot_max_y_position)
        {
            robot_y_position += robot_y_velocity;
        }


        // The x-coordinates of the robot playing area go from 0 to 640.
        // The y-coordinates go from 480 to 720.

        for (posX = robot_min_x_area; posX < robot_max_x_area; posX++)
        {
            // Shift everything down by 480 and to the right by 0
            // so that the robot playing area is in the bottom-left corner.
            for (posY = robot_min_x_area + 480; posY < robot_max_x_area + 480; posY++)
            {
                if (posX >= 0 + robot_x_position - robot_size &&
                    posX <= 0 + robot_x_position + robot_size &&
                    posY >= 480 + robot_y_position - robot_size &&
                    posY <= 480 + robot_y_position + robot_size)
                {
                    XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (posY * 2560 + posX), 0xF0F);
                }
                else
                {
                    XIo_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (posY * 2560 + posX), 0xFFF);
                }
            }
        }
    }
}
