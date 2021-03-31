#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct grid_square Grid_Square;

static const int MIN_X = 0;
static const int MAX_X = 2560;

static const int MIN_Y = 0;
static const int MAX_Y = 720;

static const int ROWS = 10;
static const int COLUMNS = 10;

static const u32 XPAR_DDR2_SDRAM_MPMC_BASEADDR = 2;

struct grid_square
{
    int pos_x_begin;
    int pos_x_end;

    int pos_y_begin;
    int pos_y_end;

    u32 total_red;
    bool was_most_red;
};

void draw_square(const Grid_Square *square);

void scan_pixels(Grid_Square *square);

void find_most_red(Grid_Square squares[10][10]);

void initialize_squares(Grid_Square squares[10][10]);

void find_red_intensity(Grid_Square squares[10][10]);

u16 Xil_In16(u32 Addr)
{
    return rand();
}

void Xil_Out16(u32 Addr, u16 Value)
{
}

void initialize_squares(Grid_Square squares[10][10])
{
    for (int row = 0; row < ROWS; ++row)
    {
        for (int column = 0; column < COLUMNS; ++column)
        {
            squares[row][column].pos_x_begin = MIN_X + MAX_X / ROWS * row;
            squares[row][column].pos_x_end = MIN_X + MAX_X / ROWS * (row + 1);

            squares[row][column].pos_y_begin = MIN_Y + MAX_Y / COLUMNS * column;
            squares[row][column].pos_y_end = MIN_Y + MAX_Y / COLUMNS * (column + 1);

            printf("Minimum x: %d. Maximum x: %d. Minimum y: %d. Maximum y; %d\n",
                   squares[row][column].pos_x_begin,
                   squares[row][column].pos_x_end,
                   squares[row][column].pos_y_begin,
                   squares[row][column].pos_y_end);
        }
    }

}

void find_red_intensity(Grid_Square squares[10][10])
{
    for (int row = 0; row < ROWS; ++row)
    {
        for (int column = 0; column < COLUMNS; ++column)
        {
            scan_pixels(&squares[row][column]);
        }
    }

    find_most_red(squares);
}

void scan_pixels(Grid_Square *square)
{
    u32 total_red = 0;

    for (int pos_x = square->pos_x_begin; pos_x < square->pos_x_end; ++pos_x)
    {
        for (int pos_y = square->pos_y_begin; pos_y < square->pos_y_end; ++pos_y)
        {
            u16 color = Xil_In16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x));

            // 3333_2222_1111_0000
            // 2s are red. 1s are green. 0s are blue.
            total_red += ((color >> 8) & 0b1111);
        }
    }

    square->total_red = total_red;
}

void find_most_red(Grid_Square squares[10][10])
{
    Grid_Square *most_red_square = &squares[0][0];

    for (int row = 0; row < ROWS; ++row)
    {
        for (int column = 0; column < COLUMNS; ++column)
        {
            if (most_red_square->total_red < squares[row][column].total_red)
            {
                most_red_square = &squares[row][column];
            }
        }
    }

    most_red_square->was_most_red = true;
}

int draw_most_red(Grid_Square squares[10][10])
{
    int count = 0;

    for (int row = 0; row < ROWS; ++row)
    {
        for (int column = 0; column < COLUMNS; ++column)
        {
            if (squares[row][column].was_most_red)
            {
                draw_square(&squares[row][column]);
                count++;
            }
        }
    }

    printf("Number of black squares: %d.\n", count);
    return count;
}

void draw_square(const Grid_Square *square)
{
    for (int pos_x = square->pos_x_begin; pos_x < square->pos_x_end; ++pos_x)
    {
        for (int pos_y = square->pos_y_begin; pos_y < square->pos_y_end; ++pos_y)
        {
            // Paint it black.
            Xil_Out16(XPAR_DDR2_SDRAM_MPMC_BASEADDR + 2 * (pos_y * 2560 + pos_x), 0);
        }
    }
}

void clear_screen(Grid_Square squares[10][10])
{
    for (int row = 0; row < ROWS; ++row)
    {
        for (int column = 0; column < COLUMNS; ++column)
        {
            squares[row][column].was_most_red = false;
        }
    }
}

int main()
{
    srand(time(NULL));

    Grid_Square squares[10][10] = {0};

    initialize_squares(squares);

    while (true)
    {
        find_red_intensity(squares);
        int count = draw_most_red(squares);

        if (count > 75)
        {
            clear_screen(squares);
        }
    }

    return 0;
}
