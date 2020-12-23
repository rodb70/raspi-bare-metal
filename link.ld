OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS
{
    /*
     * First and formost we need the .init section, containing the code to 
     * be run first. We allow room for the ATAGs and stack and conform to 
     * the bootloader's expectation by putting this code at 0x8000.
     */
    .init 0x8000 :
    {
        *(.init)
    }
    
    /* 
    * Next we put the rest of the code.
    */
    .text :
    {
        *(.text)
        *(.text*)
        *(.rodata*)
    }
    
    /* 
    * Next we put the data.
    */
    .data : {
        . = ALIGN( 4 );
        *(.data*)
    }

    __bss_start = .;
    .bss : 
    { 
        . = ALIGN( 4 );
        *(.bss*) 
        *(COMMON*) 
        . = ALIGN( 4 );
    }
    __bss_end__ = . ;

    .stack :
    {
        . = ALIGN( 8 );
        . += 0x10000;
        *(.stack)
        _stack = .;
    }
}
