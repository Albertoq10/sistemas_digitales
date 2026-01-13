#include "ssd1306.h"


// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Screen object
static SSD1306_t SSD1306;


//
//  Send a byte to the command register
//
static uint8_t ssd1306_WriteCommand(I2C_HandleTypeDef *hi2c, uint8_t command)
{
    return HAL_I2C_Mem_Write(hi2c, SSD1306_I2C_ADDR, 0x00, 1, &command, 1, 10);
}


//
//  Initialize the oled screen
//
//se realizaron unos cambios para adaptar display a 128 x 32
uint8_t ssd1306_Init(I2C_HandleTypeDef *hi2c)
{
	// Wait for the screen to boot
    HAL_Delay(100);
    int status = 0;

    // Display OFF
    status += ssd1306_WriteCommand(hi2c, 0xAE); //Display off
    status += ssd1306_WriteCommand(hi2c, 0x20); //Set Memory Addressing Mode
    status += ssd1306_WriteCommand(hi2c, 0x00);
    status += ssd1306_WriteCommand(hi2c, 0x40);
    status += ssd1306_WriteCommand(hi2c, 0xA1);
    status += ssd1306_WriteCommand(hi2c, 0xC8);
    status += ssd1306_WriteCommand(hi2c, 0x81); // set contrast control register
    status += ssd1306_WriteCommand(hi2c, 0x7F);
    status += ssd1306_WriteCommand(hi2c, 0xA6); //Set normal display


    status += ssd1306_WriteCommand(hi2c, 0xA8);   // Multiplex ratio (32-1 = 0x1F)
    status += ssd1306_WriteCommand(hi2c, 0x1F);

    status += ssd1306_WriteCommand(hi2c, 0xD3);
    status += ssd1306_WriteCommand(hi2c, 0x00);
    status += ssd1306_WriteCommand(hi2c, 0xD5);
    status += ssd1306_WriteCommand(hi2c, 0x80);
    status += ssd1306_WriteCommand(hi2c, 0xD9);
    status += ssd1306_WriteCommand(hi2c, 0xF1);


    status += ssd1306_WriteCommand(hi2c, 0xDA);
    status += ssd1306_WriteCommand(hi2c, 0x02); // 128x32

    status += ssd1306_WriteCommand(hi2c, 0xDB);
    status += ssd1306_WriteCommand(hi2c, 0x40);
    status += ssd1306_WriteCommand(hi2c, 0x8D);
    status += ssd1306_WriteCommand(hi2c, 0x14);
    status += ssd1306_WriteCommand(hi2c, 0xA4);
    status += ssd1306_WriteCommand(hi2c, 0xAF);

    if (status != 0) return 1;

    // Clear screen
    ssd1306_Fill(Black);

    // Flush buffer
    ssd1306_UpdateScreen(hi2c);

    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;
    SSD1306.Inverted = 0;
    SSD1306.Initialized = 1;

    return 0;
}

//
//  Fill the whole screen with the given color
//
void ssd1306_Fill(SSD1306_COLOR color)
{
    // Fill screenbuffer with a constant value (color)
    uint32_t i;

    for(i = 0; i < sizeof(SSD1306_Buffer); i++)
    {
        SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

//
//  Write the screenbuffer with changed to the screen
//

void ssd1306_UpdateScreen(I2C_HandleTypeDef *hi2c)
{
    uint8_t page;

    for (page = 0; page < (SSD1306_HEIGHT / 8); page++)
    {
        ssd1306_WriteCommand(hi2c, 0xB0 + page); // set page address
        ssd1306_WriteCommand(hi2c, 0x00);        // lower column start address
        ssd1306_WriteCommand(hi2c, 0x10);        // higher column start address

        HAL_I2C_Mem_Write(
            hi2c,
            SSD1306_I2C_ADDR,
            0x40, 1,
            &SSD1306_Buffer[SSD1306_WIDTH * page],
            SSD1306_WIDTH,
            100
        );
    }
}

//
//  Draw one pixel in the screenbuffer
//  X => X Coordinate
//  Y => Y Coordinate
//  color => Pixel color
//
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    {
        // Don't write outside the buffer
        return;
    }

    // Check if pixel should be inverted
    if (SSD1306.Inverted)
    {
        color = (SSD1306_COLOR)!color;
    }

    // Draw in the correct color
    if (color == White)
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    }
    else
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}


//
//  Draw 1 char to the screen buffer
//  ch      => Character to write
//  Font    => Font to use
//  color   => Black or White
//
char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
{
    uint32_t i, b, j;

    // Check remaining space on current line
    if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font.FontWidth) ||
        SSD1306_HEIGHT <= (SSD1306.CurrentY + Font.FontHeight))
    {
        // Not enough space on current line
        return 0;
    }

    // Translate font to screenbuffer
    for (i = 0; i < Font.FontHeight; i++)
    {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++)
        {
            if ((b << j) & 0x8000)
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
            }
            else
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    SSD1306.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}

//
//  Write full string to screenbuffer
//
char ssd1306_WriteString(const char* str, FontDef Font, SSD1306_COLOR color)
{
    // Write until null-byte
    while (*str)
    {
        if (ssd1306_WriteChar(*str, Font, color) != *str)
        {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

//
//  Invert background/foreground colors
//
void ssd1306_InvertColors(void)
{
    SSD1306.Inverted = !SSD1306.Inverted;
}

//
//  Set cursor position
//
void ssd1306_SetCursor(uint8_t x, uint8_t y)
{
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}
