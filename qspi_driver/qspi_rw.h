/// #include "qspi_rw.h"

/// Sector size unsigned integer 65536 = 0x0001000U
/// in binary representation has one bit equal to one
/// 0b00000000000000010000000000000000
#define QSPI_SEKTOR_SIZE    65536
#define QSPI_BLOCK_SIZE     256
void qspi_reset (void);
void qspi_install (void);

void qspi_sektor_wire (uint8_t* data, uint32_t adress);
void qspi_data_wire ( uint8_t* data, uint32_t data_size, uint32_t flash_adres);



