#ifndef ILI9340_H
#define ILI9340_H

#define ILI9340_TFTWIDTH  240
#define ILI9340_TFTHEIGHT 320

// Color definitions
#define	ILI9340_BLACK   0x0000
#define	ILI9340_BLUE    0x001F
#define	ILI9340_RED     0xF800
#define	ILI9340_GREEN   0x07E0
#define ILI9340_CYAN    0x07FF
#define ILI9340_MAGENTA 0xF81F
#define ILI9340_YELLOW  0xFFE0
#define ILI9340_WHITE   0xFFFF


extern void ili9340_write_command(uint8_t command, int param_len, ...);

extern void ili9340_init(void);

extern void ili9340_close(void);

extern void ili9340_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

extern void ili9340_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

extern void ili9340_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

extern void ili9340_draw_line_v(uint16_t x, uint16_t y, uint16_t h, uint16_t color);

extern void ili9340_draw_line_h(uint16_t x, uint16_t y, uint16_t w, uint16_t color);

extern void ili9340_set_rotation(uint8_t m);

extern uint16_t ili9340_get_width(void);

extern uint16_t ili9340_get_height(void);

extern void ili9340_update_display(void);

void ili9340_mkdirty(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

#endif /* ILI9340_H */
