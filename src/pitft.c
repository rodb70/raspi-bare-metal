#include "bcm2835.h"
#include "ili9340.h"
#include "raycasting.h"
#include "mmu.h"

#define UP 7
#define DOWN 16
#define LEFT 26
#define RIGHT 17

void draw_vert_line(int x, int y, int h, int color_mode)
{
    if( color_mode )
    {
        ili9340_draw_line_v( x, y, h, ILI9340_RED );
    }
    else
    {
        ili9340_draw_line_v( x, y, h, ILI9340_BLACK );
    }
}

int run(void)
{
    /* Setup translation tables with Core 0 */
    MmuSetupPagetable();

    /* setup mmu on core 0 */
    MmuEnable();

    bcm2835_init();
    bcm2835_aux_muart_init();

    ili9340_init();
    ili9340_set_rotation( 3 );

    uint16_t screen_height = ili9340_get_height();
    uint16_t screen_width = ili9340_get_width();

    init_world( screen_width, screen_height );
    cast_rays( draw_vert_line, ili9340_update_display );

    bcm2835_gpio_fsel( UP, BCM2835_GPIO_FSEL_INPT );
    bcm2835_gpio_fsel( DOWN, BCM2835_GPIO_FSEL_INPT );
    bcm2835_gpio_fsel( LEFT, BCM2835_GPIO_FSEL_INPT );
    bcm2835_gpio_fsel( RIGHT, BCM2835_GPIO_FSEL_INPT );
    bcm2835_gpio_set_pud( UP, BCM2835_GPIO_PUD_UP );
    bcm2835_gpio_set_pud( DOWN, BCM2835_GPIO_PUD_UP );
    bcm2835_gpio_set_pud( LEFT, BCM2835_GPIO_PUD_UP );
    bcm2835_gpio_set_pud( RIGHT, BCM2835_GPIO_PUD_UP );

    int pinUP_state = 1;
    int pinDOWN_state = 1;
    int pinLEFT_state = 1;
    int pinRIGHT_state = 1;

    while( 1 )
    {
        bcm2835_aux_muart_transfernb( "in the loop" );
        if( !bcm2835_gpio_lev( DOWN ) && pinDOWN_state )
        {
            pinDOWN_state = 0;
            player.rot_dir = 1.;
            bcm2835_aux_muart_transfernb( "DOWN - left" );
        }
        else if( bcm2835_gpio_lev( DOWN ) && !pinDOWN_state )
        {
            pinDOWN_state = 1;
        }

        if( !bcm2835_gpio_lev( UP ) && pinUP_state )
        {
            pinUP_state = 0;
            player.move_dir = 1.;
            bcm2835_aux_muart_transfernb( "UP - up" );
        }
        else if( !bcm2835_gpio_lev( UP ) && !pinUP_state )
        {
            pinUP_state = 1;
        }

        if( !bcm2835_gpio_lev( LEFT ) && pinLEFT_state )
        {
            pinLEFT_state = 0;
            player.move_dir = -1.;
            bcm2835_aux_muart_transfernb( "LEFT - down" );
        }
        else if( !bcm2835_gpio_lev( LEFT ) && !pinLEFT_state )
        {
            pinLEFT_state = 1;
        }

        if( !bcm2835_gpio_lev( RIGHT ) )
        {
            pinRIGHT_state = 0;
            player.rot_dir = -1.;
            bcm2835_aux_muart_transfernb( "RIGHT - right" );
        }
        else if( !bcm2835_gpio_lev( RIGHT ) && !pinRIGHT_state )
        {
            pinRIGHT_state = 1;
        }

        move_player();
        cast_rays( draw_vert_line, ili9340_update_display );
    }

    return 0;
}
