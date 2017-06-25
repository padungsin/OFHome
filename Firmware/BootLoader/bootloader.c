
#include "bootloader.h"
#include "types.h"

/* enable the following line to support lower case charachters in HEX file */
//#define BOOTLOADER_HEX_LOWERCASE

/* the following line enables the checksum check, disable to decrease the */
/* bootloader size */
#define BOOTLOADER_HEX_CHECKSUM

/* pin for booloader activation */
//#define BOOTLOADER_PIN      PIN_A5
//#define LED2				PIN_C4

#define UPGRADE_ADDRESS		0xFF

/* ------------------------------------------------------------------------- */
/* Intel HEX file record types                                               */
/* ------------------------------------------------------------------------- */
#define HEX_DATA_REC        0   /* data record */
#define HEX_EOF_REC         1   /* end of file record */
#define HEX_EXTADDR_REC     4   /* extened linear address record */


/* ------------------------------------------------------------------------- */
/* Intel HEX file section start offsets                                      */
/* ------------------------------------------------------------------------- */
#define HEX_LEN_START       1   /* start of record length */
#define HEX_ADDR_START      3   /* start of address */
#define HEX_TYPE_START      7   /* start of record type */
#define HEX_DATA_START      9   /* start of data */

#define HEX_HEADER_LEN      4   /* lenght of lenght-, address- and type field in byte */

#define HEX_LINE_LEN_MAX    50  /* maximum length a line in the HEX file */


#define ACK                 0x06    /* positive acknowledge (ACK) */
#define NAK                 0x15    /* negative acknowledge (NAK) */


/* ------------------------------------------------------------------------- */
/* Bootloader Address Mapping                                                */
/* ------------------------------------------------------------------------- */
#define BOOTLOADER_START        0x2A
#define BOOTLOADER_END          0x3FF
#define RESET_VECTOR            0x400
#define INTERRUPT_VECTOR_LOW    0x408
#define INTERRUPT_VECTOR_HIGH   0x418
#define APPLICATION_START       RESET_VECTOR




#ORG BOOTLOADER_START, BOOTLOADER_END auto=0 default

/* RS232 configuration */
//xmit=PIN_D6,rcv=PIN_D7 ESP8266
//xmit=PIN_C6,rcv=PIN_C7 WIFI
#use rs232(baud=57600,parity=N,xmit=PIN_D6,rcv=PIN_D7,bits=8)

/* ========================================================================= */
/* get_hexbyte                                                               */
/* returns byte representation of 2 ASCII characters pointed to by *hex      */
/* ========================================================================= */
uint8 get_hexbyte(uint8 *hex)
{
    uint8 i;
    uint8 ret = 0;
    uint8 ch;
    
    for (i=0; i < 2; i++)
    {
        /* take care of HEX files where lower case letters are used */
        #ifdef BOOTLOADER_HEX_LOWERCASE
        ch = toupper(*hex);
        #else
        ch = *hex;
        #endif
        
        /* convert character to integer value */
        if (ch >= 'A')
        {
            ch = ch - 'A' + 10;
        }
        else
        {
            ch = ch - '0';
        }
        ret = (ret << 4) | ch;
        hex++;
    }
    
    return (ret);
}


/* ========================================================================= */
/* check_checksum                                                            */
/* calculates checksum for the received Intel HEX file line                  */
/* return: 0 = checksum ok                                                   */
/*         1 = checksum error                                                */
/* ========================================================================= */
#ifdef BOOTLOADER_HEX_CHECKSUM
uint8 check_checksum(uint8 *hex, uint8 reclen)
{
    uint8 checksum = 0;
    uint8 i;

    // add all byte values, incl. checksum!
    for (i=0; i <= (reclen + HEX_HEADER_LEN); i++)
    {
        checksum += get_hexbyte(hex);
        hex += 2;
    }

    // checksum is zero if checksum is correct
    return (checksum);
}
#endif /* BOOTLOADER_HEX_CHECKSUM */


/* ========================================================================= */
/* bootloader                                                                */
/* the bootloader itself.                                                    */
/* ========================================================================= */
void bootloader(void)
{
    uint32 addr;
    uint16 addr_low;
    uint8 addr_high = 0;
    uint8 reclen;
    uint8 rectype;
    uint8 idx;
    uint8 buffer[HEX_LINE_LEN_MAX];
    uint8 ch;
    bool hexend = 0;
    
    #if getenv("FLASH_ERASE_SIZE")>2
    uint16 next_addr = 0;
    #endif

    
    /* until end of HEX file */
    while(hexend == 0)
    {
        /* get one line of the HEX file via RS232 until we receive CR or */
        /* we reached the end of the buffer */
        idx = 0;
        do
        {
             /* get one byte */
             ch = getch();
             /* save to buffer */
             buffer[idx] = ch;
             /* increment buffer index */
             idx++;
        }
        while(ch != 0x0A);


        /* get record length */
        reclen = get_hexbyte(&buffer[HEX_LEN_START]);

        /* check for proper checksum */
        #ifdef BOOTLOADER_HEX_CHECKSUM
        if (check_checksum(&buffer[HEX_LEN_START], reclen) != 0)
        {
            /* checkum error - send negative acknowledge */
            putc(NAK);
        }
        else
        #endif /* BOOTLOADER_HEX_CHECKSUM */
        {
            /* checkum ok */

            /* get address */
            addr_low = make16(get_hexbyte(&buffer[HEX_ADDR_START]),
                              get_hexbyte(&buffer[HEX_ADDR_START+2]));

            /* make 32 bit address */
            addr = make32(addr_high, addr_low);

            /* get record type */
            rectype = get_hexbyte(&buffer[HEX_TYPE_START]);
            
            if (rectype == HEX_DATA_REC)
            {
                /* only program code memory */
                if ((addr_high < 0x30) && (addr >= RESET_VECTOR))
                {
                    for (idx=0; idx < reclen; idx++)
                    {
                        buffer[idx] = get_hexbyte(&buffer[HEX_DATA_START+(idx*2)]);
                    }

                    #if getenv("FLASH_ERASE_SIZE") > getenv("FLASH_WRITE_SIZE")
                    #if defined(__PCM__)
                    if ((addr_low!=next_addr)&&(addr_low&(getenv("FLASH_ERASE_SIZE")-1)!=0))
                    #else
                    if ((addr_low!=next_addr)&&(addr_low&(getenv("FLASH_ERASE_SIZE")/2-1)!=0))
                    #endif
                        erase_program_eeprom(addr);
                    next_addr = addr_low + 1;
                    #endif
                    write_program_memory(addr, buffer, reclen);
                }
            }
            else if (rectype == HEX_EOF_REC)
            {
                hexend = 1;
            }
            else if (rectype == HEX_EXTADDR_REC)
            {
                /* to save resources, only take lower byte - this */
                /* allows 24 bit addresses => enough for PIC18F */
                //addr_high = make16(get_hexbyte(&buffer[HEX_DATA_START]),
                //                   get_hexbyte(&buffer[HEX_DATA_START+2]));
                addr_high = get_hexbyte(&buffer[HEX_DATA_START+2]);
            }
            
            /* send positive acknowledge */
            putc(ACK);
        }
    }
}


/* ========================================================================= */
/* application                                                               */
/* dummy application.                                                        */
/* ========================================================================= */
#ORG APPLICATION_START, APPLICATION_START+0x100
void application(void)
{
    while(1);
}

/* ========================================================================= */
/* main                                                                      */
/* checks if bootloader shall be invoked or if it shall start the            */
/* application program.                                                      */
/* ========================================================================= */
#ORG BOOTLOADER_START
void main()
{
    setup_adc(ADC_OFF);

/*    printf("\r\nHome Control V3 ...\r\n");
	output_high(BOOTLOADER_PIN);
	delay_ms(1000);
	output_low(BOOTLOADER_PIN);	
*/
   
    if (read_eeprom (UPGRADE_ADDRESS) == 0x31)
    {
	    printf("$+++ Uploading New Firmware +++*");
        bootloader();		
    }

    #asm
    goto RESET_VECTOR
    #endasm
}


/* ========================================================================= */
/* isr_low                                                                   */
/* remap interrupt vector low.                                               */
/* ========================================================================= */
#ORG 0x08, 0x17
void isr_low(void)
{
    #asm
    goto INTERRUPT_VECTOR_LOW
    #endasm
}


/* ========================================================================= */
/* isr_high                                                                  */
/* remap interrupt vector high.                                              */
/* ========================================================================= */
#ORG 0x18, 0x29
void isr_high(void)
{
    #asm
    goto INTERRUPT_VECTOR_HIGH
    #endasm
}
