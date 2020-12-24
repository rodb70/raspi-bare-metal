#include <stdarg.h>
#include "bcm2835.h"
#include "ili9340.h"

#define ILI_CMD_PIN 20
#define ILI_LED_PIN 18
#define ILI_RST_PIN 19

#define MEMORY_ACCESS_CONTROL_MY  0x80
#define MEMORY_ACCESS_CONTROL_MX  0x40
#define MEMORY_ACCESS_CONTROL_MV  0x20
#define MEMORY_ACCESS_CONTROL_ML  0x10
#define MEMORY_ACCESS_CONTROL_RGB 0x00
#define MEMORY_ACCESS_CONTROL_BGR 0x08
#define MEMORY_ACCESS_CONTROL_MH  0x04

enum
{
    SLEEP_OUT = 0x11,
    GAMMA_SET = 0x26,
    DISPLAY_ON = 0x29,
    COLUMN_ADDRESS_SET = 0x2a,
    PAGE_ADDRESS_SET = 0x2b,
    MEMORY_WRITE = 0x2c,
    MEMORY_ACCESS_CONTROL = 0x36,
    COLMOD_PIXEL_FORMAT_SET = 0x3a,
    FRAME_RATE_CONTROL = 0xb1,
    DISPLAY_FUNCTION_CONTROL = 0xb6,
    POWER_CONTROL_1 = 0xc0,
    POWER_CONTROL_2 = 0xc1,
    VCOM_CONTROL_1 = 0xc5,
    VCOM_CONTROL_2 = 0xc7,
    POWER_CONTROL_A = 0xcb,
    POWER_CONTROL_B = 0xcf,
    POSITIVE_GAMMA_CORRECTION = 0xe0,
    NEGATIVE_GAMMA_CORRECTION = 0xe1,
    DRIVER_TIMING_CONTROL_A = 0xe8,
    DRIVER_TIMING_CONTROL_B = 0xea,
    POWER_ON_SEQUENCE_CONTROL =  0xed,
    ENABLE_3G = 0xf2,
    PUMP_RATIO_CONTROL = 0xf7
};

uint16_t width;
uint16_t height;
uint8_t rotation;

uint8_t framebuffer[2 * ILI9340_TFTWIDTH * ILI9340_TFTHEIGHT];
uint16_t dirty_x0;
uint16_t dirty_y0;
uint16_t dirty_x1;
uint16_t dirty_y1;

void ili9340_write_command(uint8_t command, int param_len, ...)
{
    char buffer[ 50 ];
    va_list args;

    bcm2835_gpio_write( ILI_CMD_PIN, LOW );
    bcm2835_spi_transfer( command );
    bcm2835_gpio_write( ILI_CMD_PIN, HIGH );

    if( param_len )
    {
        va_start( args, param_len );
        for( int i = 0; i < param_len; i++ )
        {
            buffer[ i ] = (uint8_t) va_arg( args, int );
        }
        va_end( args );
        bcm2835_spi_writenb( buffer, param_len );
    }
}

void ili9340_draw_line(void)
{
}

void ili9340_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    if( x0 >= width )
    {
        x0 = width - 1;
    }
    if( y0 >= height )
    {
        y0 = height - 1;
    }
    if( x1 >= width )
    {
        x1 = width - 1;
    }
    if( y1 >= height )
    {
        y1 = height - 1;
    }
    if( x0 > x1 || y0 > y1 )
    {
        return;
    }

    ili9340_write_command( COLUMN_ADDRESS_SET, 4, x0 >> 8, x0 & 0xff, x1 >> 8, x1 & 0xff );
    ili9340_write_command( PAGE_ADDRESS_SET, 4, y0 >> 8, y0 & 0xff, y1 >> 8, y1 & 0xff );

    ili9340_write_command( MEMORY_WRITE, 0 );
}

void ili9340_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if( x >= width )
    {
        x = width - 1;
    }
    if( y >= height )
    {
        y = height - 1;
    }

    ili9340_mkdirty( x, y, x, y );

    uint32_t offset = ( y * width + x ) << 1;
    framebuffer[ offset++ ] = ( color >> 8 ) & 0xff;
    framebuffer[ offset ] = color & 0xff;
}

void ili9340_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if( x >= width )
    {
        x = width - 1;
    }
    if( y >= height )
    {
        y = height - 1;
    }
    if( x + w > width )
    {
        w = width - x;
    }
    if( y + h > height )
    {
        h = height - y;
    }

    ili9340_mkdirty( x, y, x + w - 1, y + h - 1 );

    uint8_t hi = (color >> 8) & 0xff, lo = color & 0xff;
    uint32_t offset = (y * width + x) << 1;

    while( h-- )
    {
        for( uint16_t i = w; i; i-- )
        {
            framebuffer[ offset++ ] = hi;
            framebuffer[ offset++ ] = lo;
        }

        offset += (width - w) << 1;
    }
}

void ili9340_draw_line_v(uint16_t x, uint16_t y, uint16_t h, uint16_t color)
{
    ili9340_fill_rect( x, y, 1, h, color );
}

void ili9340_draw_line_h(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{
    ili9340_fill_rect( x, y, w, 1, color );
}

void ili9340_mkdirty(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    if( x0 < dirty_x0 )
    {
        dirty_x0 = x0;
    }
    if( y0 < dirty_y0 )
    {
        dirty_y0 = y0;
    }
    if( x1 > dirty_x1 )
    {
        dirty_x1 = x1;
    }
    if( y1 > dirty_y1 )
    {
        dirty_y1 = y1;
    }
}

void ili9340_update_display(void)
{
    if( dirty_x0 >= width || dirty_x1 >= width || dirty_y0 >= height || dirty_y1 >= height )
    {
        dirty_x0 = 0;
        dirty_x1 = width - 1;
        dirty_y0 = 0;
        dirty_y1 = height - 1;
    }

    if( dirty_x0 > dirty_x1 )
    {
        dirty_x0 = 0;
        dirty_x1 = width - 1;
    }

    if( dirty_y0 > dirty_y1 )
    {
        dirty_y0 = 0;
        dirty_y1 = height - 1;
    }

    ili9340_set_addr_window( dirty_x0, dirty_y0, dirty_x1, dirty_y1 );

    uint32_t offset = 2 * (dirty_y0 * width + dirty_x0);
    uint32_t len = 2 * (dirty_x1 - dirty_x0 + 1);

    while( dirty_y0 <= dirty_y1 )
    {
        bcm2835_spi_writenb( (void*) (framebuffer + offset), len );
        offset += width << 1;
        dirty_y0++;
    }

    dirty_x0 = width;
    dirty_x1 = 0;
    dirty_y0 = height;
    dirty_y1 = 0;
}

uint16_t ili9340_get_width(void)
{
    return width;
}

uint16_t ili9340_get_height(void)
{
    return height;
}

void ili9340_set_rotation(uint8_t m)
{
    rotation = m % 4; // can't be higher than 3
    switch( rotation )
    {
    case 0:
        ili9340_write_command( MEMORY_ACCESS_CONTROL, 1, MEMORY_ACCESS_CONTROL_MX | MEMORY_ACCESS_CONTROL_BGR );

        width = ILI9340_TFTWIDTH;
        height = ILI9340_TFTHEIGHT;
        break;

    case 1:
        ili9340_write_command( MEMORY_ACCESS_CONTROL, 1, MEMORY_ACCESS_CONTROL_MV | MEMORY_ACCESS_CONTROL_BGR );

        width = ILI9340_TFTHEIGHT;
        height = ILI9340_TFTWIDTH;
        break;

    case 2:
        ili9340_write_command( MEMORY_ACCESS_CONTROL, 1, MEMORY_ACCESS_CONTROL_MY | MEMORY_ACCESS_CONTROL_BGR );

        width = ILI9340_TFTWIDTH;
        height = ILI9340_TFTHEIGHT;
        break;

    case 3:
        ili9340_write_command( MEMORY_ACCESS_CONTROL, 1,
                MEMORY_ACCESS_CONTROL_MV | MEMORY_ACCESS_CONTROL_MY | MEMORY_ACCESS_CONTROL_MX | MEMORY_ACCESS_CONTROL_BGR );

        width = ILI9340_TFTHEIGHT;
        height = ILI9340_TFTWIDTH;
        break;
    }

    dirty_x0 = width;
    dirty_x1 = 0;
    dirty_y0 = height;
    dirty_y1 = 0;
}

void ili9340_init(void)
{
    bcm2835_spi_begin();
    bcm2835_spi_setDataMode( BCM2835_SPI_MODE0 );
    bcm2835_spi_setClockDivider( BCM2835_SPI_CLOCK_DIVIDER_64 );
    bcm2835_spi_chipSelect( BCM2835_SPI_CS0 );
    bcm2835_gpio_fsel( ILI_CMD_PIN, BCM2835_GPIO_FSEL_OUTP );
    bcm2835_gpio_fsel( ILI_LED_PIN, BCM2835_GPIO_FSEL_OUTP );
    bcm2835_gpio_fsel( ILI_RST_PIN, BCM2835_GPIO_FSEL_OUTP );

    /* Switch off back light */
    bcm2835_gpio_write( ILI_LED_PIN, LOW );

    /* Toggle LCD reset */
    bcm2835_gpio_write( ILI_RST_PIN, LOW );
    bcm2835_delay( 1 );
    bcm2835_gpio_write( ILI_RST_PIN, HIGH );
    bcm2835_delay( 120 );
    /* Switch on back light */
    bcm2835_gpio_write( ILI_LED_PIN, HIGH );

    /* Initialise LCD */
    /* Unknown command */
    ili9340_write_command( 0xEF, 3, 0x03, 0x80, 0x02 );
    /* Power control B */
    ili9340_write_command( POWER_CONTROL_B, 3, 0x00, 0xC1, 0x30 );
    /*  Power on sequence control */
    ili9340_write_command( POWER_ON_SEQUENCE_CONTROL, 4, 0x64, 0x03, 0x12, 0x81 );
    /* Driver timing control A */
    ili9340_write_command( DRIVER_TIMING_CONTROL_A, 3, 0x85, 0x00, 0x78 );
    /* Power control A */
    ili9340_write_command( POWER_CONTROL_A, 5, 0x39, 0x2C, 0x00, 0x34, 0x02 );
    /* Pump ratio control */
    ili9340_write_command( PUMP_RATIO_CONTROL, 1, 0x20 );
    /* Driver timing control B */
    ili9340_write_command( DRIVER_TIMING_CONTROL_B, 2, 0x00, 0x00 );

    /* Power Control 1 */
    ili9340_write_command( POWER_CONTROL_1, 1, 0x23 );

    /* Power Control 2 */
    ili9340_write_command( POWER_CONTROL_2, 1, 0x10 );

    /* VCOM Control 1 */
    ili9340_write_command( VCOM_CONTROL_1, 2, 0x3e, 0x28 );

    /* VCOM Control 2 */
    ili9340_write_command( VCOM_CONTROL_2, 1, 0x86 );

    /* COLMOD: Pixel Format Set */
    /* 16 bits/pixel */
    ili9340_write_command( COLMOD_PIXEL_FORMAT_SET, 1, 0x55 );

    /* Frame Rate Control */
    /* Division ratio = fosc, Frame Rate = 79Hz */
    ili9340_write_command( FRAME_RATE_CONTROL, 2, 0x00, 0x18 );

    /* Display Function Control */
    ili9340_write_command( DISPLAY_FUNCTION_CONTROL, 3, 0x08, 0x82, 0x27 );

    /* Gamma Function Disable */
    ili9340_write_command( ENABLE_3G, 1, 0x00 );

    /* Gamma curve selected */
    ili9340_write_command( GAMMA_SET, 1, 0x01 );

    /* Positive Gamma Correction */
    ili9340_write_command( POSITIVE_GAMMA_CORRECTION, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03,
                           0x0E, 0x09, 0x00 );

    /* Negative Gamma Correction */
    ili9340_write_command( NEGATIVE_GAMMA_CORRECTION, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C,
                           0x31, 0x36, 0x0F );

    /* Sleep OUT */
    ili9340_write_command( SLEEP_OUT, 0 );

    bcm2835_delay( 120 );

    /* Display ON */
    ili9340_write_command( DISPLAY_ON, 0 );

    ili9340_set_rotation( 0 );

    dirty_x0 = width;
    dirty_x1 = 0;
    dirty_y0 = height;
    dirty_y1 = 0;
}

void ili9340_close(void)
{
    bcm2835_spi_end();
}

