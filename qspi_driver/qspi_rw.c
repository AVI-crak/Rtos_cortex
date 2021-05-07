/// qspi_rw.c
#include "stm32f7xx.h"
#include "DMA_ONE32F746BG.h"    // asssssaa
#include "gpio_one.h"

#include "qspi_rw.h"

#pragma GCC push_options
#pragma GCC optimize ("Os")


#define SDRAM           __attribute__((section(".qsdram")))
__attribute__( ( always_inline ) ) static inline void delay( uint32_t volatile time_tmp){do {time_tmp--;} while ( time_tmp );};

 /// array[b/8] |= (1 << (b%8));

void qspi_reset (void)
{
    while(QUADSPI->SR & QUADSPI_SR_BUSY) delay(16);
    QUADSPI->CCR = _VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0x66)      /// Software Reset (RST 99h)
                  |_VAL2FLD(QUADSPI_CCR_IMODE, 1);              // Instruction on a single line
    while(!(QUADSPI->SR & QUADSPI_SR_TCF));                     // Transfer Complete Flag
    QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags

    QUADSPI->CCR = _VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0x99)      /// Software Reset (RSTEN 66h)
                  |_VAL2FLD(QUADSPI_CCR_IMODE, 1);              // Instruction on a single line
    while(!(QUADSPI->SR & QUADSPI_SR_TCF));                     // Transfer Complete Flag
    QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags
}

/// Quad spi S25FL256S
/// Initialization is used in the SystemInit function
void qspi_install (void)
{
    gpio_one_pin(zap_gpio.B.pin02.v_af09_quadspi_clk.speed4.pull_down.lock_on);
    gpio_one_pin(zap_gpio.B.pin06.v_af10_quadspi_bk1_ncs.speed4.pull_up.lock_on);
    gpio_one_pin(zap_gpio.F.pin08.v_af10_quadspi_bk1_io0.speed4.pull_down.lock_on);
    gpio_one_pin(zap_gpio.F.pin09.v_af10_quadspi_bk1_io1.speed4.pull_down.lock_on);
    gpio_one_pin(zap_gpio.F.pin07.v_af09_quadspi_bk1_io2.speed4.pull_down.lock_on);
    gpio_one_pin(zap_gpio.F.pin06.v_af09_quadspi_bk1_io3.speed4.pull_up.lock_on);

    RCC->AHB3ENR |= RCC_AHB3ENR_QSPIEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    RCC->AHB3RSTR = RCC_AHB3RSTR_QSPIRST;
    delay(50); // 50 HCLK
    RCC->AHB3RSTR &= ~RCC_AHB3RSTR_QSPIRST;
    delay(50); // 50 HCLK

    QUADSPI->DCR = _VAL2FLD(QUADSPI_DCR_FSIZE, 27)              // Flash memory size number of address lines (27 = 256mb)
                    |_VAL2FLD(QUADSPI_DCR_CSHT, 4);             // Chip select high time
    QUADSPI->PIR = _VAL2FLD(QUADSPI_PIR_INTERVAL, 32);          // Interval of automatic polling of status registers
    QUADSPI->PSMKR = 0x03;                                      // Polling status match register
    QUADSPI->LPTR = _VAL2FLD(QUADSPI_LPTR_TIMEOUT, 128);        // Keep the old address
    QUADSPI->DLR = 1;
    QUADSPI->ABR = 0xA5;                                        // Alternate bytes
    QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags
    QUADSPI->CR = _VAL2FLD(QUADSPI_CR_PRESCALER, 3)             // Clock prescaler AHB/4 54Mgz
                 |_VAL2FLD(QUADSPI_CR_APMS, 1)                  // Automatic poll mode stop
                 |_VAL2FLD(QUADSPI_CR_TCEN, 1)                  // Timeout counter enable memory-mapped mode
                 |_VAL2FLD(QUADSPI_CR_FTHRES, 1)                // FIFO threshold level 4Bute
                 |_VAL2FLD(QUADSPI_CR_EN, 1);                   // Enable the QUADSPI

    uint32_t tmp;
    while(QUADSPI->SR & QUADSPI_SR_FLEVEL)tmp = QUADSPI->DR;
    qspi_reset();

    QUADSPI->CCR = _VAL2FLD(QUADSPI_CCR_FMODE, 1)               // Indirect read mode
                    |_VAL2FLD(QUADSPI_CCR_DMODE, 1)             // Data on four lines
                    |_VAL2FLD(QUADSPI_CCR_IMODE, 1)             // Instruction on a single line
                    |_VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0x35);   /// Read Configuration Register
    while(!(QUADSPI->SR & QUADSPI_SR_TCF));                     // Transfer Complete Flag
    tmp = QUADSPI->DR;
    QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags

    if (tmp != 0xc2c2)
    {
        QUADSPI->CCR = _VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0x06)  /// Write Enable (06h)
                        |_VAL2FLD(QUADSPI_CCR_IMODE, 1);        // Instruction on a single line
        while(!(QUADSPI->SR & QUADSPI_SR_TCF));                 // Transfer complete flag
        QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags

        QUADSPI->CCR = _VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0x01)  /// Write Register Configuration1+Status
                        |_VAL2FLD(QUADSPI_CCR_IMODE, 1)         // Instruction on a single line
                        |_VAL2FLD(QUADSPI_CCR_DMODE, 1);        // Data on a single line
        QUADSPI->DR = 0xC200;                                   // LC + QUAD (50Mgz < CLK < 66Mgz)
        while(!(QUADSPI->SR & QUADSPI_SR_TCF));                 // Transfer complete flag
        QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags

        QUADSPI->PSMAR = 0;                                     // Status mask
        QUADSPI->CCR =  _VAL2FLD(QUADSPI_CCR_FMODE, 2)          // Automatic polling mode
                        |_VAL2FLD(QUADSPI_CCR_DMODE, 1)         // Data on four lines
                        |_VAL2FLD(QUADSPI_CCR_IMODE, 1)         // Instruction on a single line
                        |_VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0x05);   /// Read Status Register-1
        while(!(QUADSPI->SR & QUADSPI_SR_SMF));                 // Status Match Flag
        QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags
    };

__DSB();
    QUADSPI->CCR =  _VAL2FLD(QUADSPI_CCR_SIOO, 1)               // Send instruction only for the first command
                    |_VAL2FLD(QUADSPI_CCR_FMODE, 3)             // Memory-mapped mode
                    |_VAL2FLD(QUADSPI_CCR_DDRM, 1)              // DDR Mode enabled
                    |_VAL2FLD(QUADSPI_CCR_DHHC, 1)              // DDR hold
                    |_VAL2FLD(QUADSPI_CCR_DMODE, 3)             // Data on four lines
                    |_VAL2FLD(QUADSPI_CCR_DCYC, 3)              // Number of dummy cycles
                    |_VAL2FLD(QUADSPI_CCR_ABSIZE, 0)            // 8-bit alternate byte
                    |_VAL2FLD(QUADSPI_CCR_ABMODE, 3)            // Alternate bytes on four lines
                    |_VAL2FLD(QUADSPI_CCR_ADSIZE, 3)            // 32-bit address
                    |_VAL2FLD(QUADSPI_CCR_ADMODE, 3)            // Address on four lines
                    |_VAL2FLD(QUADSPI_CCR_IMODE, 1)             // Instruction on a single line
                    |_VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0xEE);   /// DDR Quad I/O Read(4-byte Address 0xEE)
__DSB();
    while(QUADSPI->SR & QUADSPI_SR_BUSY);

};







/// Recording a single sector of memory
/// The size of the data block is equal to the size of the sector, the address of the sector is direct
void qspi_sektor_wire (uint8_t* data, uint32_t adress)
{
    uint32_t  adres, temp, temp2;
    adres = adress & ((~(QSPI_SEKTOR_SIZE - 1)) & 0x0FFFFFFF);
    qspi_reset(); delay(100);
/// Erasing a single memory sector
    QUADSPI->CCR = _VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0x06)      /// Write Enable (06h)
                    |_VAL2FLD(QUADSPI_CCR_IMODE, 1);            // Instruction on a single line
    while(!(QUADSPI->SR & QUADSPI_SR_TCF))temp2 = QUADSPI->DR;                     // Transfer complete flag
    while(QUADSPI->SR & QUADSPI_SR_BUSY);
    QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags
    delay(10);


    QUADSPI->CCR =  _VAL2FLD(QUADSPI_CCR_ADMODE, 1)             // Address on four lines
                    |_VAL2FLD(QUADSPI_CCR_FMODE, 0)             // read
                    |_VAL2FLD(QUADSPI_CCR_ADSIZE, 3)            // 32-bit address
                    |_VAL2FLD(QUADSPI_CCR_IMODE, 1)             // Instruction on a single line
                    |_VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0xDC);   ///DCh Sector Erase a 4-byte address (A31-A0)
    QUADSPI->AR = adres;                                        // ctart comand
    while(!(QUADSPI->SR & QUADSPI_SR_TCF));                     // Transfer complete flag
    while(QUADSPI->SR & QUADSPI_SR_BUSY);
    QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags
    delay(100);

    QUADSPI->CCR =  _VAL2FLD(QUADSPI_CCR_FMODE, 2)              // Automatic polling mode
                        |_VAL2FLD(QUADSPI_CCR_DMODE, 1)         // Data on four lines
                        |_VAL2FLD(QUADSPI_CCR_IMODE, 1)         // Instruction on a single line
                        |_VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0x05);   /// Read Status Register-1
    while(!(QUADSPI->SR & QUADSPI_SR_SMF));                     // Status Match Flag
    while(QUADSPI->SR & QUADSPI_SR_FTF)temp2 = QUADSPI->DR;
    QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags
    delay(100);

/// Memory block record
    temp = 0;
 do{
    QUADSPI->CCR = _VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0x06)      /// Write Enable (06h)
                        |_VAL2FLD(QUADSPI_CCR_IMODE, 1);        // Instruction on a single line
    while(!(QUADSPI->SR & QUADSPI_SR_TCF))temp2 = QUADSPI->DR;                     // Transfer complete flag
    while(QUADSPI->SR & QUADSPI_SR_BUSY);
    QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags
    delay(100);

    QUADSPI->AR = adres + temp;
    QUADSPI->DLR = QSPI_BLOCK_SIZE- 1;
    QUADSPI->CR = _VAL2FLD(QUADSPI_CR_PRESCALER, 3)             // Clock prescaler AHB/4 54Mgz
                 |_VAL2FLD(QUADSPI_CR_APMS, 1)                  // Automatic poll mode stop
                 |_VAL2FLD(QUADSPI_CR_FTHRES, 15)               // FIFO threshold level 16Bute
                 |_VAL2FLD(QUADSPI_CR_DMAEN, 1)                 // DMA enable
                 |_VAL2FLD(QUADSPI_CR_EN, 1);                   // Enable the QUADSPI

    QUADSPI->CCR =  _VAL2FLD(QUADSPI_CCR_DMODE, 3)              // Data on four lines
                    |_VAL2FLD(QUADSPI_CCR_ADSIZE, 3)            // 32-bit address
                    |_VAL2FLD(QUADSPI_CCR_ADMODE, 1)            // Address on four lines
                    |_VAL2FLD(QUADSPI_CCR_IMODE, 1)             // Instruction on a single line
                    |_VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0x34);   /// Quad Page Program (4-byte Address)

__DSB();
    DMA2->HIFCR = (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7);
    DMA2_Stream7->CR = 0;
    DMA2_Stream7->FCR = 0;
    DMA2_Stream7->NDTR = (QSPI_BLOCK_SIZE / 4);                         // Block size 256B/4
    DMA2_Stream7->M0AR = ((uint32_t) data) + temp;
    DMA2_Stream7->PAR = (uint32_t) &(QUADSPI->DR);
    DMA2_Stream7->FCR = DMA_SxFCR_DMDIS | sDMA_SxFCR_FTH.full_FIFO_4_4;
__DSB();
    DMA2_Stream7->CR = sdma_line.dma2.stream_7.ch3_QUADSPI              /// channe - tmigger
                        |sDMA_SxCR_DIR.memory_to_peripheral             /// data transfer direction
                        |sDMA_SxCR_MBURST.incremental_burst_of_4_beats  /// memory burst transfer configuration
                        |sDMA_SxCR_PBURST.incremental_burst_of_4_beats              /// peripheral burst transfer configuration
                        |swDMA_SxCR_PL(3)                               /// priority level 0-Low, 1-Medium, 2-High, 3-Very high
                        |sDMA_SxCR_MSIZE.t32_bit                        /// memory data size
                        |DMA_SxCR_MINC                                  /// memory increment mode
                        |sDMA_SxCR_PSIZE.t32_bit                        /// peripheral data size
                        |DMA_SxCR_EN;

__DSB();
    while(QUADSPI->SR & QUADSPI_SR_BUSY)delay(50);
    while(!(QUADSPI->SR & QUADSPI_SR_TCF))delay(50);


    QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags


delay(30);
    QUADSPI->CR = _VAL2FLD(QUADSPI_CR_PRESCALER, 3)             // Clock prescaler AHB/4 54Mgz
                 |_VAL2FLD(QUADSPI_CR_APMS, 1)                  // Automatic poll mode stop
                 |_VAL2FLD(QUADSPI_CR_TCEN, 1)                  // Timeout counter enable memory-mapped mode
                 |_VAL2FLD(QUADSPI_CR_FTHRES, 2)                // FIFO threshold level 4Bute
                 |_VAL2FLD(QUADSPI_CR_EN, 1);                   // Enable the QUADSPI
__DSB();
    QUADSPI->DLR = 1;
    QUADSPI->PSMAR = 0;
    QUADSPI->CCR =  _VAL2FLD(QUADSPI_CCR_FMODE, 2)              // Automatic polling mode
                    |_VAL2FLD(QUADSPI_CCR_DMODE, 1)             // Data on four lines
                    |_VAL2FLD(QUADSPI_CCR_IMODE, 1)             // Instruction on a single line
                    |_VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0x05);   /// Read Status Register-1

__DSB();
    while(!(QUADSPI->SR & QUADSPI_SR_SMF));                     // Status Match Flag
    QUADSPI->FCR = QUADSPI_FCR_CSMF|QUADSPI_FCR_CTCF|QUADSPI_FCR_CTEF|QUADSPI_FCR_CTOF; // reset flags
    temp += QSPI_BLOCK_SIZE;

  }while(temp < QSPI_SEKTOR_SIZE);

/// Switch to hardware memory access mode
    while(QUADSPI->SR & QUADSPI_SR_BUSY);
__DSB();
    QUADSPI->CCR =  _VAL2FLD(QUADSPI_CCR_SIOO, 1)               // Send instruction only for the first command
                    |_VAL2FLD(QUADSPI_CCR_FMODE, 3)             // Memory-mapped mode
                    |_VAL2FLD(QUADSPI_CCR_DDRM, 1)              // DDR Mode enabled
                    |_VAL2FLD(QUADSPI_CCR_DHHC, 1)              // DDR hold
                    |_VAL2FLD(QUADSPI_CCR_DMODE, 3)             // Data on four lines
                    |_VAL2FLD(QUADSPI_CCR_DCYC, 3)              // Number of dummy cycles
                    |_VAL2FLD(QUADSPI_CCR_ABSIZE, 0)            // 8-bit alternate byte
                    |_VAL2FLD(QUADSPI_CCR_ABMODE, 3)            // Alternate bytes on four lines
                    |_VAL2FLD(QUADSPI_CCR_ADSIZE, 3)            // 32-bit address
                    |_VAL2FLD(QUADSPI_CCR_ADMODE, 3)            // Address on four lines
                    |_VAL2FLD(QUADSPI_CCR_IMODE, 1)             // Instruction on a single line
                    |_VAL2FLD(QUADSPI_CCR_INSTRUCTION, 0xEE);   /// DDR Quad I/O Read(4-byte Address 0xEE)
__DSB();
delay(100);
};



 union qspi_dat
{
     uint32_t    dat32[(QSPI_SEKTOR_SIZE / 4)];
     uint8_t     dat8[QSPI_SEKTOR_SIZE];
}qspi_dat_buf;


void qspi_data_wire ( uint8_t* data, uint32_t data_size, uint32_t flash_adres)
{
    uint32_t adres_in, adres_out;
    uint32_t *ptr32;
    adres_in = 0;
    do
    {
        ptr32 = (uint32_t*) ((flash_adres + adres_in) & (~(QSPI_SEKTOR_SIZE - 1))); adres_out = 0;
        while (adres_out < (QSPI_SEKTOR_SIZE / 4))
            {qspi_dat_buf.dat32[adres_out] = ptr32[adres_out]; delay(100); adres_out++;};
        adres_out = (flash_adres + adres_in) & (QSPI_SEKTOR_SIZE - 1);
        while ((adres_out < QSPI_SEKTOR_SIZE)&&(adres_in < data_size))
        {
            qspi_dat_buf.dat8[adres_out++] = data[adres_in++] ;
        };
        adres_out = (flash_adres + adres_in - 1) & (~(QSPI_SEKTOR_SIZE - 1));
        qspi_sektor_wire(&qspi_dat_buf.dat8[0], adres_out);

    }while (adres_in < data_size);
};

#pragma GCC pop_options
