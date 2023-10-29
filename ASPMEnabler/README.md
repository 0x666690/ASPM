# ASPMEnabler

Patches the FADT ACPI table and overrides the bit which signals that the mainboard does not correctly support ASPM. 

This tool is based on [S0ixEnabler](https://github.com/Jamesits/S0ixEnabler) by James Swineson.

## Requirements

* UEFI firmware (some old EFI firmwares might be supported as well)

## Usage

Run `ASPMEnabler.efi` either manually or automatically before your OS loads. 

## Building

Requirements:

* Windows and Visual Studio 2017 or higher
* C++ desktop development tools
* MSVC C++ build tools (for the architecture you need)
* MSVC C++ Spectre-mitigated libs (for the architecture you need)

Open `ASPMEnabler.sln` in Visual Studio, make sure that your build is set to "Release" and click "Build Solution".