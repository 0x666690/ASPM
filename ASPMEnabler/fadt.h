// ACPI table definition from https://github.com/tianocore/edk2
#pragma once
#ifndef __FADT_H__
#define __FADT_H__

#include <efi.h>
#include <efilib.h>

// edk2/MdePkg/Include/IndustryStandard/Acpi10.h 
// The common ACPI description table header.  This structure prefaces most ACPI tables.
#pragma pack(1)
typedef struct {
    UINT32  Signature;
    UINT32  Length;
    UINT8   Revision;
    UINT8   Checksum;
    UINT8   OemId[6];
    UINT64  OemTableId;
    UINT32  OemRevision;
    UINT32  CreatorId;
    UINT32  CreatorRevision;
} EFI_ACPI_DESCRIPTION_HEADER;

// edk2/MdePkg/Include/IndustryStandard/Acpi50.h 
// ACPI 5.0 Generic Address Space definition
#pragma pack(1)
typedef struct {
    UINT8   AddressSpaceId;
    UINT8   RegisterBitWidth;
    UINT8   RegisterBitOffset;
    UINT8   AccessSize;
    UINT64  Address;
} EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE;

// edk2/MdePkg/Include/IndustryStandard/Acpi50.h 
// Fixed ACPI Description Table Structure (FADT)
#pragma pack(1)
typedef struct {
    EFI_ACPI_DESCRIPTION_HEADER             Header;
    UINT32                                  FirmwareCtrl;
    UINT32                                  Dsdt;
    UINT8                                   Reserved0;
    UINT8                                   PreferredPmProfile;
    UINT16                                  SciInt;
    UINT32                                  SmiCmd;
    UINT8                                   AcpiEnable;
    UINT8                                   AcpiDisable;
    UINT8                                   S4BiosReq;
    UINT8                                   PstateCnt;
    UINT32                                  Pm1aEvtBlk;
    UINT32                                  Pm1bEvtBlk;
    UINT32                                  Pm1aCntBlk;
    UINT32                                  Pm1bCntBlk;
    UINT32                                  Pm2CntBlk;
    UINT32                                  PmTmrBlk;
    UINT32                                  Gpe0Blk;
    UINT32                                  Gpe1Blk;
    UINT8                                   Pm1EvtLen;
    UINT8                                   Pm1CntLen;
    UINT8                                   Pm2CntLen;
    UINT8                                   PmTmrLen;
    UINT8                                   Gpe0BlkLen;
    UINT8                                   Gpe1BlkLen;
    UINT8                                   Gpe1Base;
    UINT8                                   CstCnt;
    UINT16                                  PLvl2Lat;
    UINT16                                  PLvl3Lat;
    UINT16                                  FlushSize;
    UINT16                                  FlushStride;
    UINT8                                   DutyOffset;
    UINT8                                   DutyWidth;
    UINT8                                   DayAlrm;
    UINT8                                   MonAlrm;
    UINT8                                   Century;
    UINT16                                  IaPcBootArch;
    UINT8                                   Reserved1;
    UINT32                                  Flags;
    EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE  ResetReg;
    UINT8                                   ResetValue;
    UINT8                                   Reserved2[3];
    UINT64                                  XFirmwareCtrl;
    UINT64                                  XDsdt;
    EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE  XPm1aEvtBlk;
    EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE  XPm1bEvtBlk;
    EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE  XPm1aCntBlk;
    EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE  XPm1bCntBlk;
    EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE  XPm2CntBlk;
    EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE  XPmTmrBlk;
    EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE  XGpe0Blk;
    EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE  XGpe1Blk;
    EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE  SleepControlReg;
    EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE  SleepStatusReg;
} EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE;

#pragma pack()
#endif