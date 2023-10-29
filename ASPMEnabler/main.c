#include <stdbool.h>
#include <efi.h>
#include <efilib.h>
#include "listacpi.h"
#include "fadt.h"
#include "acpi_checksum.h"

// Application entrypoint (must be set to 'efi_main' for gnu-efi crt0 compatibility)
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
#if defined(_GNU_EFI)
	InitializeLib(ImageHandle, SystemTable);
#endif

	Print(L"\n%HASPMEnabler%N\n");
	Print(L"%Hhttps://github.com/0x666690/ASPM%N\n");
	Print(L"%HA modified version of: https://github.com/Jamesits/S0ixEnabler%N\n");
	Print(L"Firmware %s Rev %d\n\n", SystemTable->FirmwareVendor, SystemTable->FirmwareRevision);

	EFI_CONFIGURATION_TABLE* ect = SystemTable->ConfigurationTable;
	EFI_GUID AcpiTableGuid = ACPI_TABLE_GUID;
	EFI_GUID Acpi2TableGuid = ACPI_20_TABLE_GUID;
	EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER* rsdp = NULL;

	bool foundTable = false;
	bool patchSuccessful = false;
	UINT64 ret = EFI_SUCCESS;

	/*
		We want to patch the FADT to force enable ASPM. The FADT is pointed to by an entry in the RSDT.

		From https://wiki.osdev.org/RSDT:
			To find the RSDT you need first to locate and check the RSDP, then use the RsdtPointer for ACPI Version < 2.0 an XsdtPointer for any other case.
			The ACPI standards state that an OS that complies with ACPI version 2.0 or later should use the XSDT instead of the RSDT, however I personally
			doubt that there is a difference on 80x86 computers. AFAIK the XSDT itself was introduced for Itanium's (IA-64) and other 64 bit computers where
			it's likely that the BIOS (and ACPI tables) are above 4 GB. On 80x86 it's likely that the RSDT and the XSDT both point to the same tables below
			4 GB for compatibility reasons (it doesn't make sense to have 2 versions of the same tables) -- Brendan. 
	*/


	// locate RSDP (Root System Description Pointer) 
	for (int SystemTableIndex = 0; SystemTableIndex < SystemTable->NumberOfTableEntries; SystemTableIndex++) {
		Print(L"Table #%d/%d: ", SystemTableIndex + 1, SystemTable->NumberOfTableEntries);

		// Vendor GUID of the table we're currently looking at matches neither the ACPI nor the ACPI Version 2.0 GUID, go to the next table
		if (!CompareGuid(&SystemTable->ConfigurationTable[SystemTableIndex].VendorGuid, &AcpiTableGuid) && !CompareGuid(&SystemTable->ConfigurationTable[SystemTableIndex].VendorGuid, &Acpi2TableGuid)) {
			Print(L"Not ACPI\n");
			goto next_table;
		}

		/*
			From https://wiki.osdev.org/RSDP:
				To find the table, the Operating System has to find the "RSD PTR " string (notice the lastspace character) in one of the two areas.
				This signature is always on a 16 byte boundary. 
		*/
		if (myStrnCmpA((unsigned char*)"RSD PTR ", (CHAR8*)ect->VendorTable, 8)) {
			Print(L"Not RSDP\n");
			goto next_table;
		} 
		
		/*
			RDSP has been found
		*/
		rsdp = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)ect->VendorTable;
		Print(L"RSDP Rev %u @0x%x | ", rsdp->Revision, rsdp);

		/*
			From https://wiki.osdev.org/RSDT:
				To find the RSDT you need first to locate and check the RSDP, then use the RsdtPointer for ACPI Version < 2.0 an XsdtPointer for any other case.
				The ACPI standards state that an OS that complies with ACPI version 2.0 or later should use the XSDT instead of the RSDT, however I personally
				doubt that there is a difference on 80x86 computers. AFAIK the XSDT itself was introduced for Itanium's (IA-64) and other 64 bit computers where
				it's likely that the BIOS (and ACPI tables) are above 4 GB. On 80x86 it's likely that the RSDT and the XSDT both point to the same tables below
				4 GB for compatibility reasons (it doesn't make sense to have 2 versions of the same tables) -- Brendan.
		*/

		/*
		*	https://wiki.osdev.org/XSDT:
			eXtended System Descriptor Table (XSDT) - the 64-bit version of the ACPI RSDT

			"The XSDT provides identical functionality to the RSDT but accommodates physical addresses of DESCRIPTION HEADERs that are larger than 32-bits."
			- ACPI Specification v5.0.
			
			This means all addresses are now 64 bits. If the pointer to the XSDT is valid, the OS MUST use the XSDT. All of the addresses will probably be 32 bits,
			and if you are not using PAE, you couldn't use higher than 32 bits, but the spec says you should use this anyway.

			To find the XSDT you need first to locate and check the RSDP, then use the XsdtPointer (if it exists).
		*/


		/*
			If we're not on ACPI Version 2.0 or above, the XSDT doesn't exist.
		*/
		if (rsdp->Revision < EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER_REVISION) {
			Print(L"No XSDT\n");
			goto next_table;
		}

		/*
			You only need to sum all the bytes in the table and compare the result to 0. 
		*/
		EFI_ACPI_SDT_HEADER* Xsdt = (EFI_ACPI_SDT_HEADER*)(rsdp->XsdtAddress);
		if (myStrnCmpA("XSDT", (CHAR8*)(VOID*)(Xsdt->Signature), 4)) {
			Print(L"Invalid XSDT\n");
			goto next_table;
		}

		// yeah we got XSDT!
		CHAR16 OemStr[20];
		Ascii2UnicodeStr((CHAR8*)(Xsdt->OemId), OemStr, 6);
		UINT32 EntryCount = (Xsdt->Length - sizeof(EFI_ACPI_SDT_HEADER)) / sizeof(UINT64);
		Print(L"%HXSDT OEM ID: %s Tables: %d%N\n", OemStr, EntryCount);

		// iterate XSDT tables
		UINT64* EntryPtr;
		CHAR16 SigStr[20];
		EntryPtr = (UINT64*)(Xsdt + 1);
		for (UINT32 XsdtTableIndex = 0; XsdtTableIndex < EntryCount; XsdtTableIndex++, EntryPtr++) {
			EFI_ACPI_SDT_HEADER* Entry = (EFI_ACPI_SDT_HEADER*)((UINTN)(*EntryPtr));
			Ascii2UnicodeStr((CHAR8*)(Entry->Signature), SigStr, 4);
			Ascii2UnicodeStr((CHAR8*)(Entry->OemId), OemStr, 6);
			Print(L"ACPI table #%d/%d: %s Rev %d OEM ID: %s\n", XsdtTableIndex + 1, EntryCount, SigStr, (int)(Entry->Revision), OemStr);

			// See Advanced Configuration and Power Interface Specification Version 6.2
			// Table 5-30 DESCRIPTION_HEADER Signatures for tables defined by ACPI
			if (!myStrnCmpA((unsigned char *)"FACP", (CHAR8*)(Entry->Signature), 4))
			{
				// check checksum
				Print(L"%HChecking initial checksum... %N");
				if (AcpiChecksum((UINT8*)(Entry), Entry->Length))
				{
					Print(L"%HFAILED%N\n");
					goto next_table;
				}
				Print(L"%HOK%N\n");
				foundTable = true;

				Print(L"%HPatching FADT table...%N\n");
				EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE* FADT = (EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE*)Entry;
				/*
					IaPcBootArch contains what Linux refers to as "boot_flags".
					What we want to patch is ACPI_FADT_NO_ASPM:
					see https://github.com/torvalds/linux/blob/master/include/acpi/actbl.h
				*/

				Print(L"FADT::IaPcBootArch before: 0x%x\n", FADT->IaPcBootArch);
				FADT->IaPcBootArch = FADT->IaPcBootArch & ~(1 << 4);
				Print(L"FADT::IaPcBootArch after: 0x%x\n", FADT->IaPcBootArch);

				// re-calculate checksum
				Print(L"Checksum before: 0x%x\n", Entry->Checksum);
				UINT8 ChkSumDiff = AcpiChecksum((UINT8*)(Entry), Entry->Length);
				Entry->Checksum -= ChkSumDiff;
				Print(L"Checksum after: 0x%x\n", Entry->Checksum);

				// re-check checksum for OCD people
				Print(L"Re-check... ");
				if (AcpiChecksum((UINT8*)(Entry), Entry->Length))
				{
					Print(L"%HFAILED%N\n");
					ret = EFI_CRC_ERROR;
				} else
				{
					Print(L"%HOK%N\n");
					patchSuccessful = true;
				}

				Print(L"%HFADT table patch finished%N\n");
			}

			if (patchSuccessful) {
				break;
			}
		}

		if (patchSuccessful) {
			break;
		}

		next_table:
		ect++;
	}

	

	if (rsdp == NULL) {
		Print(L"%EERROR: RSDP could not be found%N\n");
		ret = EFI_UNSUPPORTED;
	}

	if (!foundTable)
	{
		Print(L"%EERROR: FADT could not be found%N\n");
		ret = EFI_UNSUPPORTED;
	}

	if (!patchSuccessful)
	{
		Print(L"%EERROR: Patch failed%N\n");
		ret = EFI_UNSUPPORTED;
	}

	// if we are running as an EFI driver, then just quit and let other things load
	Print(L"%EASPMEnabler done%N\n\n");

	return ret;
}
