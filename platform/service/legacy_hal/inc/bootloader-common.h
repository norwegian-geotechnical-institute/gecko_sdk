/***************************************************************************//**
 * @file
 * @brief Common bootloader definitions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef BOOTLOADER_COMMON_H
#define BOOTLOADER_COMMON_H

#include <stdint.h>

#define BLDEBUG(x)
#define BLDEBUG_PRINT(str)

/** @brief Define the bootloader status type.
 */
typedef uint8_t BL_Status;
#ifndef __EMBERSTATUS_TYPE__
#define __EMBERSTATUS_TYPE__
#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef uint8_t EmberStatus;
#endif
#endif // __EMBERSTATUS_TYPE__

#define EBL_MIN_TAG_SIZE                  128U
#define IMAGE_STAMP_SIZE                  8U
#define IMAGE_INFO_MAXLEN_OLD             32U
#define IMAGE_INFO_MAXLEN                 16U
#define AAT_MAX_SIZE                      128U
#define BTL_NONCE_LENGTH                  12U

#define BL_SUCCESS                        0U
#define BL_EBL_CONTINUE                   0x50U

#define BAT_MIN_IBR_VERSION               0x010A
#define BOOTLOADER_ADDRESS_TABLE_TYPE     0x0BA7

#define MFB_BOTTOM          FLASH_BASE
#define MFB_SIZE_B          FLASH_SIZE
#define MFB_SIZE_W          (MFB_SIZE_B / 4)
#define MFB_TOP             (MFB_BOTTOM + MFB_SIZE_B - 1)

/**
 * @brief This structure defines ebl flash function types
 */
typedef struct eblHdr3xx_s {
  uint16_t tag;                 ///< = EBLTAG_HEADER
  uint16_t len;                 ///< =
  uint16_t version;             ///< Version of the ebl format
  uint16_t signature;           ///< Magic signature: 0xE350
  uint32_t flashAddr;           ///< Address where the AAT is stored
  uint32_t aatCrc;              ///< CRC of the ebl header portion of the AAT
  // aatBuff is oversized to account for the potential of the AAT to grow in
  //  the future.  Only the first 128 bytes of the AAT can be referenced as
  //  part of the ebl header, although the AAT itself may grow to 256 total
  uint8_t aatBuff[AAT_MAX_SIZE];         ///< buffer for the ebl portion of the AAT
} eblHdr3xx_t;

/**
 * @brief This structure defines ccm states
 */
typedef struct ccmState_s {
  uint32_t msgLen;                     ///< Length of the encrypted data
  uint8_t nonce[BTL_NONCE_LENGTH];     ///< The random nonce for this message
  uint8_t mac[SECURITY_BLOCK_SIZE];    ///< The full rolling MAC value
  uint32_t blockCount;                 ///< Current AES block we're processing in this message
  uint8_t blockOffset;                 ///< Offset within the current AES block [0, SECURITY_BLOCK_SIZE]
  uint8_t macOffset;                   ///< Last byte written to in the MAC buffer
} ccmState_t;

/**
 * @brief This structure defines ebl data function types
 */
typedef struct {
  BL_Status (*eblGetData)(void *state, uint8_t *dataBytes, uint16_t len);                            ///< ebl Get Data
  BL_Status (*eblDataFinalize)(void *state);       ///< ebl Data Finalize
} EblDataFuncType;

/**
 * @brief This structure defines ebl process state types
 */
typedef struct {
  uint32_t fileCrc;                                ///< file Crc
  bool headerValid;                                ///< header Valid
  eblHdr3xx_t eblHeader;                           ///< ebl Header
  uint16_t encType;                                ///< enc Type
  uint8_t encEblStateMachine;                      ///< enc Ebl State Machine
  bool decryptEnabled;                             ///< decrypt Enabled
  uint32_t byteCounter;                            ///< byte Counter
  ccmState_t encState;                             ///< enc State
} EblProcessStateType;

/**
 * @brief This structure defines ebl config types
 */
typedef struct {
  void *dataState;                                 ///< data State
  uint8_t *tagBuf;                                 ///< tag Buf
  uint16_t tagBufLen;                              ///< tag Buf Len
  bool returnBetweenBlocks;                        ///< return Between Blocks
  EblProcessStateType eblState;                    ///< ebl State
} EblConfigType;

typedef uint8_t (*flashReadFunc)(uint32_t address);

/**
 * @brief This structure defines ebl flash function types
 */
typedef struct {
  uint8_t (*erase)(uint8_t eraseType, uint32_t address);                                             ///< erase
  uint8_t (*write)(uint32_t address, uint16_t * data, uint32_t length);                              ///< write
  flashReadFunc read;                              ///< read
} EblFlashFuncType;

typedef tVectorEntry HalVectorTableType;

/**
 * @brief This structure defines hal base address table types
 */
typedef struct {
  void *topOfStack;                                ///< top Of Stack
  void (*resetVector)(void);                       ///< reset Vector
  void (*nmiHandler)(void);                        ///< nmi Handler
  void (*hardFaultHandler)(void);                  ///< hardFault Handler
  uint16_t type;                                   ///< type
  uint16_t version;                                ///< version
  const HalVectorTableType *vectorTable;           ///< vector Table
  // Followed by more fields depending on the specific address table type
} HalBaseAddressTableType;

typedef uint8_t AatPageNumber8bit_t;

/**
 * @brief This structure defines flash 8 bits page range
 */
typedef struct      pageRange8bit_s {
  AatPageNumber8bit_t begPg;            ///< First flash page in range
  AatPageNumber8bit_t endPg;            ///< Last  flash page in range
}pageRange8bit_t;

#define NUM_AAT_PAGE_RANGES 6

/**
 * @brief This structure defines hal application address table types
 */
typedef struct {
  HalBaseAddressTableType baseTable;               ///< base Table
  // The following fields are used for ebl and bootloader processing.
  //   See the above description for more information.
  uint8_t platInfo;   ///< type of platform, defined in micro.h
  uint8_t microInfo;  ///< type of micro, defined in micro.h
  uint8_t phyInfo;    ///< type of phy, defined in micro.h
  uint8_t aatSize;    ///< size of the AAT itself
  uint16_t softwareVersion;  ///< EmberZNet SOFTWARE_VERSION
  uint16_t softwareBuild;  ///< EmberZNet EMBER_BUILD_NUMBER
  uint32_t timestamp; ///< Unix epoch time of .ebl file, filled in by ebl gen

  // String, filled in by ebl generation. Used to be larger, reduced so that the
  // app properties struct could be placed in the same word for apps regardless
  // of whether they had an AAT (which Bluetooth and MCU apps do not).
  uint8_t imageInfo[IMAGE_INFO_MAXLEN];            ///< imageInfo[IMAGE_INFO_MAXLEN]
  const void *appProps; ///< ptr to app properties struct for Gecko bootloader
  // eat up the remainder of the previous imageInfo space
  uint8_t reserved[IMAGE_INFO_MAXLEN_OLD - IMAGE_INFO_MAXLEN - sizeof(void*)];                       ///< reserved
  uint32_t imageCrc;  ///< CRC over following pageRanges, filled in by ebl gen
  pageRange8bit_t pageRanges[NUM_AAT_PAGE_RANGES];  ///< Flash pages used by app.
                                                    // Filled in by ebl gen.
                                                    // 2 bytes per struct

  void *simeeBottom;  ///< assumed to be 4 bytes on Cortex M3

  uint32_t customerApplicationVersion; ///< a version field for the customer

  void *internalStorageBottom;  ///< assumed to be 4 bytes on Cortex M3

  // A non-cryptographic hash of the entire on-chip image,
  // including AAT.  It uses AES-MMO, which is a cryptographic
  // hash, but because the length is only 64-bit there is a
  // greater chance of collisions.  It is not recommended to
  // use this to prove integrity in a cryptographic sense.
  // It is provided as a simple way to verify an EBL file
  // is the same as the one on-chip.
  uint8_t imageStamp[IMAGE_STAMP_SIZE];            ///< imageStamp[IMAGE_STAMP_SIZE]

  uint8_t familyInfo; ///< type of family, defined in micro.h

  // reserve the remainder of the first 128 bytes of the AAT in case we need
  //  to go back and add any values that the bootloader will need to reference,
  //  since only the first 128 bytes of the AAT become part of the EBL header.
  uint8_t bootloaderReserved[35 - (NUM_AAT_PAGE_RANGES * sizeof(pageRange8bit_t))]; ///< Reserve the remainder of the first 128 bytes of the AAT

  //////////////
  // Any values after this point are still part of the AAT, but will not
  //   be included as part of the ebl header

  void *debugChannelBottom;                        ///< debug Channel Bottom
  void *noInitBottom;                              ///< no Init Bottom
  void *appRamTop;                                 ///< app Ram Top
  void *globalTop;                                 ///< global Top
  void *cstackTop;                                 ///< cstack Top
  void *initcTop;                                  ///< initc Top
  void *codeTop;                                   ///< code Top
  void *cstackBottom;                              ///< cstack Bottom
  void *heapTop;                                   ///< heap Top
  void *simeeTop;                                  ///< simee Top
  void *debugChannelTop;                           ///< debug Channel Top
} HalAppAddressTableType;

// Description of the Bootloader Address Table (BAT)
/**
 * @brief This structure defines hal bootloader address table types
 */
typedef struct {
  HalBaseAddressTableType baseTable;               ///< base Table
  uint16_t bootloaderType;                         ///< bootloader Type
  uint16_t bootloaderVersion;                      ///< bootloader Version
  const HalAppAddressTableType *appAddressTable;   ///< appAddress Table

  // plat/micro/phy info added in version 0x0104
  uint8_t platInfo;   ///< type of platform, defined in micro.h
  uint8_t microInfo;  ///< type of micro, defined in micro.h
  uint8_t phyInfo;    ///< type of phy, defined in micro.h
  uint8_t reserved;   ///< reserved for future use

  // moved to this location after plat/micro/phy info added in version 0x0104
  void (*eblProcessInit)(EblConfigType *config,
                         void *dataState,
                         uint8_t *tagBuf,
                         uint16_t tagBufLen,
                         bool returnBetweenBlocks);  ///< ebl ProcessInit
  BL_Status (*eblProcess)(const EblDataFuncType *dataFuncs,
                          EblConfigType *config,
                          const EblFlashFuncType *flashFuncs);  ///< ebl Process
  EblDataFuncType *eblDataFuncs;                   ///< ebl Data Funcs

  // these eeprom routines happen to be app bootloader specific
  // added in version 0x0105
  uint8_t (*eepromInit)(void);                     ///< eeprom Init
  uint8_t (*eepromRead)(uint32_t address, uint8_t *data, uint16_t len);                              ///< eeprom Read
  uint8_t (*eepromWrite)(uint32_t address, uint8_t const *data, uint16_t len);                       ///< eeprom Write
  void (*eepromShutdown)(void);                    ///< eeprom Shutdown
  const void *(*eepromInfo)(void);                 ///< eeprom Info
  uint8_t (*eepromErase)(uint32_t address, uint32_t len);                                            ///< eeprom Erase
  bool (*eepromBusy)(void);                        ///< eeprom Busy

  // These variables hold extended version information
  // added in version 0x0109
  uint16_t bootloaderBuild; ///< the build number associated with bootloaderVersion
  uint16_t reserved2;       ///< reserved for future use
  uint32_t customerBootloaderVersion; ///< hold a customer specific bootloader version
} HalBootloaderAddressTableType;

extern const HalBootloaderAddressTableType *halBootloaderAddressTable;
extern const HalAppAddressTableType halAppAddressTable;

#endif // BOOTLOADER_COMMON_H
