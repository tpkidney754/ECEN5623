#include <stdio.h>
#include <stdlib.h>


// For testing, I will limit the size of the address space to 0 ... 0xFF, 256 bytes,
// but this could be extended for a real implementation given sufficent memory up to 32-bit addressing
#define MEM_SIZE (256)

// structures with bit-fields would be nice, but a realistic exam answer is not likely 
// to use bit-fields

#define P01_BIT (0x01)
#define P02_BIT (0x02)
#define P03_BIT (0x04)
#define P04_BIT (0x08)
#define SYNBITS (0x0F)
#define PW_BIT (0x010)
#define ENCODED_BIT (0x20)
#define UNUSED_BIT_7 (0x40)
#define UNUSED_BIT_8 (0x80)

#define DATA_BIT_1 (0x01)
#define DATA_BIT_2 (0x02)
#define DATA_BIT_3 (0x04)
#define DATA_BIT_4 (0x08)
#define DATA_BIT_5 (0x10)
#define DATA_BIT_6 (0x20)
#define DATA_BIT_7 (0x40)
#define DATA_BIT_8 (0x80)

typedef struct emulated_edac
{
    unsigned char data_memory[MEM_SIZE];

    // code memory is a parallel memory space that has an offset (4 bits, most significant), so here, I will simulate as an array
    // that has a different base address, but exact same size.  In this buffer, I will use only 5-bits
    // of each 8-bit array entry to encode 6 bits of the 8 as pW, p01, p02, p03, p04, encoded=1 or 0 for never written
    unsigned char code_memory[MEM_SIZE];

} edac_t;


// Return codes for read_byte are either no error, a non-recoverable error (negative) or a recoverable error
// indicated by a positive value that indicates to the caller which bit had to be flipped to fix the data.

// In my implementation I chose to fix the data on SBEs for the returned byteRead as well as updating the data
// stored at that adress (automatically).
#define NO_ERROR (0)
#define PW_ERROR (-1)
#define DOUBLE_BIT_ERROR (-2)
#define UNKNOWN_ERROR (-3)

void print_code(unsigned char codeword)
{
    printf("codeword=");
    if(codeword & PW_BIT) printf("1"); else printf("0");
    if(codeword & P01_BIT) printf("1"); else printf("0");
    if(codeword & P02_BIT) printf("1"); else printf("0");
    if(codeword & P03_BIT) printf("1"); else printf("0");
    if(codeword & P04_BIT) printf("1"); else printf("0");

    printf("\n");
}


void print_code_word(edac_t *edac, unsigned char *address)
{
    unsigned int offset = address - edac->data_memory;
    unsigned char codeword = edac->code_memory[offset];

    printf("addr=%p (offset=%d) ", address, offset);

    if(codeword & PW_BIT) printf("1"); else printf("0");
    if(codeword & P01_BIT) printf("1"); else printf("0");
    if(codeword & P02_BIT) printf("1"); else printf("0");
    if(codeword & P03_BIT) printf("1"); else printf("0");
    if(codeword & P04_BIT) printf("1"); else printf("0");

    printf("\n");
}


void print_data_word(edac_t *edac, unsigned char *address)
{
    unsigned int offset = address - edac->data_memory;
    unsigned char dataword = edac->data_memory[offset];

    printf("addr=%p (offset=%d) ", address, offset);

    if(dataword & DATA_BIT_1) printf("1"); else printf("0");
    if(dataword & DATA_BIT_2) printf("1"); else printf("0");
    if(dataword & DATA_BIT_3) printf("1"); else printf("0");
    if(dataword & DATA_BIT_4) printf("1"); else printf("0");
    if(dataword & DATA_BIT_5) printf("1"); else printf("0");
    if(dataword & DATA_BIT_6) printf("1"); else printf("0");
    if(dataword & DATA_BIT_7) printf("1"); else printf("0");
    if(dataword & DATA_BIT_8) printf("1"); else printf("0");

    printf("\n");
}


void print_encoded(edac_t *edac, unsigned char *address)
{
    unsigned int offset = address - edac->data_memory;
    unsigned char codeword = edac->code_memory[offset];
    unsigned char dataword = edac->data_memory[offset];

    printf("addr=%p (offset=%d) ", address, offset);

    if(codeword & PW_BIT) printf("1"); else printf("0");
    if(codeword & P01_BIT) printf("1"); else printf("0");
    if(codeword & P02_BIT) printf("1"); else printf("0");
    if(dataword & DATA_BIT_1) printf("1"); else printf("0");
    if(codeword & P03_BIT) printf("1"); else printf("0");
    if(dataword & DATA_BIT_2) printf("1"); else printf("0");
    if(dataword & DATA_BIT_3) printf("1"); else printf("0");
    if(dataword & DATA_BIT_4) printf("1"); else printf("0");
    if(codeword & P04_BIT) printf("1"); else printf("0");
    if(dataword & DATA_BIT_5) printf("1"); else printf("0");
    if(dataword & DATA_BIT_6) printf("1"); else printf("0");
    if(dataword & DATA_BIT_7) printf("1"); else printf("0");
    if(dataword & DATA_BIT_8) printf("1"); else printf("0");

    printf("\n");
}


unsigned char get_codeword(edac_t *edac, unsigned int offset)
{
    unsigned char codeword=0;

    //printf("CODEWORD offset=%d\n", offset);

    // p01 - per spreadsheet model, compute even parity=0 over 7,5,4,3,2,1 bits
    codeword |= (P01_BIT & (
                            ((edac->data_memory[offset] & DATA_BIT_1) ^ 
                            ((edac->data_memory[offset] & DATA_BIT_2)>>1) ^ 
                            ((edac->data_memory[offset] & DATA_BIT_4)>>3) ^ 
                            ((edac->data_memory[offset] & DATA_BIT_5)>>4) ^ 
                            ((edac->data_memory[offset] & DATA_BIT_7)>>6)) 
                           ) );

    // p02 - per spreadsheet modell, compute even parity=0 over 7,6,4,3,1 bits
    codeword |= (P02_BIT & (
                            (
                             ((edac->data_memory[offset] & DATA_BIT_1) ^ 
                             ((edac->data_memory[offset] & DATA_BIT_3)>>2) ^ 
                             ((edac->data_memory[offset] & DATA_BIT_4)>>3) ^ 
                             ((edac->data_memory[offset] & DATA_BIT_6)>>5) ^ 
                             ((edac->data_memory[offset] & DATA_BIT_7)>>6))<<1) 
                            ) );

    // p03 - per spreadsheet model, compute even parity=0 over 8,4,3,2 bits
    codeword |= (P03_BIT & (
                            ((
                              ((edac->data_memory[offset] & DATA_BIT_2)>>1) ^ 
                              ((edac->data_memory[offset] & DATA_BIT_3)>>2) ^ 
                              ((edac->data_memory[offset] & DATA_BIT_4)>>3) ^ 
                              ((edac->data_memory[offset] & DATA_BIT_8)>>7))<<2) 
                             ) );

    // p04 - per spreadsheet model, compute even parity=0 over 8,7,6,5 bits
    codeword |= (P04_BIT & (
                            ((
                              ((edac->data_memory[offset] & DATA_BIT_5)>>4) ^ 
                              ((edac->data_memory[offset] & DATA_BIT_6)>>5) ^ 
                              ((edac->data_memory[offset] & DATA_BIT_7)>>6) ^ 
                              ((edac->data_memory[offset] & DATA_BIT_8)>>7))<<3) 
                             ) );

    // pW - per spreadsheet model compute even parity=0 over all bits
    codeword |= (PW_BIT & (
                           ((
                              (edac->data_memory[offset] & DATA_BIT_1) ^ 
                             ((edac->data_memory[offset] & DATA_BIT_2)>>1) ^ 
                             ((edac->data_memory[offset] & DATA_BIT_3)>>2) ^ 
                             ((edac->data_memory[offset] & DATA_BIT_4)>>3) ^ 
                             ((edac->data_memory[offset] & DATA_BIT_5)>>4) ^ 
                             ((edac->data_memory[offset] & DATA_BIT_6)>>5) ^ 
                             ((edac->data_memory[offset] & DATA_BIT_7)>>6) ^ 
                             ((edac->data_memory[offset] & DATA_BIT_8)>>7) ^ 
                              (codeword & P01_BIT) ^ 
                             ((codeword & P02_BIT)>>1) ^ 
                             ((codeword & P03_BIT)>>2) ^ 
                             ((codeword & P04_BIT)>>3))<<4) 
                            ) );

    // set the encoded bit
    codeword |= ENCODED_BIT;

    return codeword;

}




int read_byte(edac_t *edac, unsigned char *address, unsigned char *byteRead)
{
    unsigned int offset = address - edac->data_memory;
    unsigned char SYNDROME=0, pW2=0, pW=0, codeword=0;

    //printf("READ :");
    codeword = get_codeword(edac, offset);

    SYNDROME = (codeword & SYNBITS) ^ (edac->code_memory[offset] & SYNBITS);

    pW2 = codeword & PW_BIT; 
    pW = (edac->code_memory[offset]) & PW_BIT;

    //printf("READ :");print_encoded(edac, address);
    //printf("READ :");print_code_word(edac, address);
    //printf("READ :");print_code(codeword);

    // 1) if SYNDROME ==0 and pW == pW2, return NO_ERROR
    if((SYNDROME == 0) && (pW == pW2)) return NO_ERROR;

    // 2) if SYNDROME ==0 and pW != pW2, return PW_ERROR
    if((SYNDROME == 0) && (pW != pW2))
    {
        // restore pW to PW2
        edac->code_memory[offset] |= pW2 & PW_BIT;

        return PW_ERROR;
    }
    
    // 3) if SYNDROME !=0 and pW == pW2, return DOUBLE_BIT_ERROR
    if((SYNDROME != 0) && (pW == pW2))
    {
        return DOUBLE_BIT_ERROR;
    }

    // 4) if SYNDROME !=0 and pW != pW2, SBE, return SYNDROME
    if((SYNDROME != 0) && (pW != pW2))
    {
        return SYNDROME;
    }

    // if we get here, something is seriously wrong like triple bit
    // or worse error, so return UNKNOWN_ERROR    
    return UNKNOWN_ERROR;
}


int write_byte(edac_t *edac, unsigned char *address, unsigned char byteToWrite)
{
    unsigned int offset = address - edac->data_memory;

    // set data
    //*address=byteToWrite;
    edac->data_memory[offset]=byteToWrite;
    //print_data_word(edac, address);

    // encode partiy bits
    //printf("WRITE:");
    edac->code_memory[offset] = get_codeword(edac, offset);

    //printf("WRITE:");print_encoded(edac, address);
    //printf("WRITE:");print_code_word(edac, address);

    return NO_ERROR;
}


int flip_bit(edac_t *edac, unsigned char *address)
{
    unsigned int offset = address - edac->data_memory;
    unsigned char byte;

    // set data
    byte=edac->data_memory[offset];

    byte ^= 1 << 4;

    edac->data_memory[offset]=byte;

    //print_data_word(edac, address);
}


unsigned char *enable_edac_memory(edac_t *edac)
{
    int idx;
    
    // zero the code memory - we could initialize data memory, but this is not likely to be automatic,
    // so to emulate, we set the intial value to all 0's since a read with no prior
    // write to intialize could either return garbage or an MBE - and if the location has not been intialized
    // by a user write, we set encoded=0, so all other bits are meaningless until this is changed to a 1 by a write
    for(idx=0; idx < MEM_SIZE; idx++) edac->code_memory[idx]=0;


    return edac->data_memory;
}



void main(void)
{
    // this is the emulated Error Detection, Correction Memory
    edac_t EDAC;
    unsigned int offset=0;
    int rc;
    unsigned char byteToRead;

    // Any code that manages ECC memory would track the base address - e.g. for a heap
    unsigned char *base_addr=enable_edac_memory(&EDAC);

    // An ECC user should always initialize all memory locations before any
    // read from them (to avoid a false MBE).
    for(offset=0; offset < MEM_SIZE; offset++)
    {
        write_byte(&EDAC, base_addr+offset, (unsigned char)offset);

        flip_bit(&EDAC, base_addr+offset);

        if((rc=read_byte(&EDAC, base_addr+offset, &byteToRead)) != 0)
        {
            if(rc == DOUBLE_BIT_ERROR)
            {
                printf("NON RECOVERABLE DBE\n"); exit(-1);
            }

            if(rc == UNKNOWN_ERROR)
            {
                printf("NON RECOVERABLE UNKNOWN ERROR\n"); exit(-1);
            }

            if(rc == PW_ERROR)
            {
                printf("RECOVERABLE Parity of Encoded Word error\n");
            }

            if(rc > 0)
            {
                printf("SYNDROME=0x%x, correcting bit error at %d\n", rc, rc); 
            }

            printf("\n");
        }
        print_encoded(&EDAC, base_addr+offset);
    }
    
}
