#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include <ncurses.h>

#define BIT_1 0x01
#define BIT_2 0x02
#define BIT_3 0x04
#define BIT_4 0x08
#define BIT_5 0x10
#define BIT_6 0x20
#define BIT_7 0x40
#define BIT_8 0x80

typedef uint8_t* Field;

/**
 * Converts cartesian coordinates for the uint8_t cell group
 * to the linearn one dimensional index for that uint8_t cell
 * group.
 */
uint32_t cto1(uint8_t y, uint8_t x, uint8_t width)
{
    return y*width+x;
}

/**
 * Renders the field to the screen.
 */
void renderField(Field f, uint8_t height, uint8_t width)
{
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width / 8; j++)
        {
            for (int k = 0; k < 8; k++)
                mvaddch(i, j*8+k, ' ');

            uint32_t index = cto1(i, j, width/8);
            if (f[index] & BIT_1)
                mvaddch(i, j*8 + 0, 'x');
            if (f[index] & BIT_2)
                mvaddch(i, j*8 + 1, 'x');
            if (f[index] & BIT_3)
                mvaddch(i, j*8 + 2, 'x');
            if (f[index] & BIT_4)
                mvaddch(i, j*8 + 3, 'x');
            if (f[index] & BIT_5)
                mvaddch(i, j*8 + 4, 'x');
            if (f[index] & BIT_6)
                mvaddch(i, j*8 + 5, 'x');
            if (f[index] & BIT_7)
                mvaddch(i, j*8 + 6, 'x');
            if (f[index] & BIT_8)
                mvaddch(i, j*8 + 7, 'x');
        }
    }
}

/**
 * Returns true if the bit at (y,x) is alive; false otherwise.
 *
 * Note: the y and x value is the coordinates of the
 * bit itself, not a uint8_t cell group.
 */
uint8_t bitIsAlive(Field f, uint8_t y, uint8_t x, uint8_t width)
{
    uint32_t index = y*width/8 + x/8;
    uint8_t cell_offset = x % 8;
    uint8_t mask;

    // Masking the appropriate bit
    switch(cell_offset)
    {
        case 0:
            mask = BIT_1;
            break;
        case 1:
            mask = BIT_2;
            break;
        case 2:
            mask = BIT_3;
            break;
        case 3:
            mask = BIT_4;
            break;
        case 4:
            mask = BIT_5;
            break;
        case 5:
            mask = BIT_6;
            break;
        case 6:
            mask = BIT_7;
            break;
        case 7:
            mask = BIT_8;
            break;

    }

    return f[index] & mask;
}

/**
 * Takes a field state and returns the field
 * corresponding to the next "tick" of Conway's
 * Game of Life.
 */
Field update(Field field, uint8_t height, uint8_t width)
{
    Field new_field = calloc(height * width/8, sizeof(*new_field));
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width / 8; j++)
        {
            uint32_t index = cto1(i, j, width / 8);
            new_field[index] = field[index];
        }
    }

    // Loop through all bits
    for (size_t i = 1; i < height - 1; i++)
    {
        for (size_t j = 1; j < width - 1; j++)
        {
            uint8_t num_live_neighbors = 0;
            for (size_t live_y = i - 1; live_y < i + 2; live_y++)
            {
                for (size_t live_x = j - 1; live_x < j + 2; live_x++)
                {
                    // Skipping itself
                    if (live_y == i && live_x == j) continue;

                    if (bitIsAlive(field, live_y, live_x, width))
                        num_live_neighbors++;
                }
            }

            uint32_t cell_index = i*width/8 + j/8;
            uint8_t cell_offset = j % 8;
            uint8_t mask;
            // Masking the appropriate bit
            switch(cell_offset)
            {
                case 0:
                    mask = BIT_1;
                    break;
                case 1:
                    mask = BIT_2;
                    break;
                case 2:
                    mask = BIT_3;
                    break;
                case 3:
                    mask = BIT_4;
                    break;
                case 4:
                    mask = BIT_5;
                    break;
                case 5:
                    mask = BIT_6;
                    break;
                case 6:
                    mask = BIT_7;
                    break;
                case 7:
                    mask = BIT_8;
                    break;

            }

            if (bitIsAlive(field, i, j, width))
            {
                // Rule 1: Any live cell with fewer than two live neighbours dies, as if by underpopulation.
                // Rule 2: Any live cell with two or three live neighbours lives on to the next generation.
                // Rule 3: Any live cell with more than three live neighbours dies, as if by overpopulation.
                if (num_live_neighbors < 2 || num_live_neighbors > 3)
                    new_field[cell_index] ^= mask;
            }
            else
            {
                // Rule 4: Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
                if (num_live_neighbors == 3)
                    new_field[cell_index] |= mask;
            }
        }
    }

    return new_field;
}

/**
 * Main function
 */
int main(int argc, char *argv[])
{
    uint8_t cols = 160;
    uint8_t rows = 44;
    Field field;

    srand(time(NULL));

    // Initializing the field
    field = calloc(rows * (cols/8), sizeof(*field));
    for (size_t i = 0; i < rows; i++)
    {
        for (size_t j = 0; j < cols / 8; j++)
        {
            uint32_t index = cto1(i, j, cols / 8);
            field[index] = rand() % 0x100;
        }
    }

    // nCurses init
    initscr();
    curs_set(0);
    noecho();
    refresh();

    // Running loop
    while (1)
    {
        renderField(field, rows, cols);
        refresh();
        Field tmp = field;
        field = update(field, rows, cols);
        free(tmp);
        usleep(200 * 1000);
    }

    // Cleanup
    free(field);
    endwin();
    return EXIT_SUCCESS;
}
