## @file
#
#  Copyright (c) 2014-2018, Linaro Limited. All rights reserved.
#  Copyright (c) 2023, Molly Sophia <mollysophia379@gmail.com>
#  Copyright (c) 2023-2024, Mario Bălănică <mariobalanica02@gmail.com>
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = BDY-G98
  PLATFORM_VENDOR                = Rockchip
  PLATFORM_GUID                  = a5022309-24e1-46e0-9d40-dcbc7293e60a
  PLATFORM_VERSION               = 0.2
  DSC_SPECIFICATION              = 0x00010019
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  VENDOR_DIRECTORY               = Platform/$(PLATFORM_VENDOR)
  PLATFORM_DIRECTORY             = $(VENDOR_DIRECTORY)/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Silicon/Rockchip/RK3588/RK3588.fdf
  RK_PLATFORM_FVMAIN_MODULES     = $(PLATFORM_DIRECTORY)/$(PLATFORM_NAME).Modules.fdf.inc

  # GMAC is not exposed
  DEFINE RK3588_GMAC_ENABLE = FALSE

  #
  # HYM8563 RTC support
  # I2C location configured by PCDs below.
  #
  DEFINE RK_RTC8563_ENABLE = TRUE

  #
  # RK3588-based platform
  #
!include Silicon/Rockchip/RK3588/RK3588Platform.dsc.inc

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################

[LibraryClasses.common]
  RockchipPlatformLib|$(PLATFORM_DIRECTORY)/Library/RockchipPlatformLib/RockchipPlatformLib.inf

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform.
#
################################################################################

[PcdsFixedAtBuild.common]
  # SMBIOS platform config
  gRockchipTokenSpaceGuid.PcdPlatformName|"BDY G98"
  gRockchipTokenSpaceGuid.PcdPlatformVendorName|"Rockchip"
  gRockchipTokenSpaceGuid.PcdFamilyName|"BDY"
  gRockchipTokenSpaceGuid.PcdProductUrl|"https://github.com/yifengyou/BDY_G98_RK3588"
  gRockchipTokenSpaceGuid.PcdDeviceTreeName|"rk3588-bdy-g98"

  #
  # I2C / PMIC configuration
  # RK806 master PMIC on SPI bus (no I2C regulators like RK860x)
  #
  gRockchipTokenSpaceGuid.PcdI2cSlaveAddresses|{ 0x42, 0x43 }
  gRockchipTokenSpaceGuid.PcdI2cSlaveBuses|{ 0x0, 0x0 }
  gRockchipTokenSpaceGuid.PcdI2cSlaveBusesRuntimeSupport|{ FALSE, FALSE }
  gRockchipTokenSpaceGuid.PcdRk860xRegulatorAddresses|{ 0x42, 0x43 }
  gRockchipTokenSpaceGuid.PcdRk860xRegulatorBuses|{ 0x0, 0x0 }
  gRockchipTokenSpaceGuid.PcdRk860xRegulatorTags|{ $(SCMI_CLK_CPUB01), $(SCMI_CLK_CPUB23) }

  #
  # Combo PHY configuration
  # Phoenix does NOT use combo PHYs for PCIe/SATA/USB3
  # All combo PHYs are unconnected
  #
  gRK3588TokenSpaceGuid.PcdComboPhy0Switchable|FALSE
  gRK3588TokenSpaceGuid.PcdComboPhy1Switchable|FALSE
  gRK3588TokenSpaceGuid.PcdComboPhy2Switchable|FALSE
  gRK3588TokenSpaceGuid.PcdComboPhy0ModeDefault|$(COMBO_PHY_MODE_UNCONNECTED)
  gRK3588TokenSpaceGuid.PcdComboPhy1ModeDefault|$(COMBO_PHY_MODE_UNCONNECTED)
  gRK3588TokenSpaceGuid.PcdComboPhy2ModeDefault|$(COMBO_PHY_MODE_UNCONNECTED)

  #
  # PCIe 3.0 configuration
  # Phoenix uses NANBNB (x2 + x2) mode for dual NVMe
  # pcie3x4 = 2 lanes, pcie3x2 = 2 lanes
  #
  gRK3588TokenSpaceGuid.PcdPcie30Supported|TRUE
  gRK3588TokenSpaceGuid.PcdPcie30x2Supported|TRUE
  gRK3588TokenSpaceGuid.PcdPcie30PhyModeSwitchable|FALSE
  gRK3588TokenSpaceGuid.PcdPcie30PhyModeDefault|$(PCIE30_PHY_MODE_NANBNB)

  #
  # USB/DP PHY configuration
  # Phoenix does not use USB-C DP (headless)
  #
  gRK3588TokenSpaceGuid.PcdUsbDpPhy0Supported|FALSE
  gRK3588TokenSpaceGuid.PcdUsbDpPhy1Supported|FALSE

  #
  # No display connectors (headless device)
  #

  #
  # GMAC configuration
  # Phoenix uses dual GMAC with RGMII fixed-link to YT9215S DSA switches
  # TX delay values from U-Boot DTS: gmac0 tx_delay=0x44, gmac1 tx_delay=0x42
  #
  gRK3588TokenSpaceGuid.PcdGmac0Supported|TRUE
  gRK3588TokenSpaceGuid.PcdGmac1Supported|TRUE
  gRK3588TokenSpaceGuid.PcdGmac0TxDelay|0x44
  gRK3588TokenSpaceGuid.PcdGmac1TxDelay|0x42

  # No fan output
  gRK3588TokenSpaceGuid.PcdHasOnBoardFanOutput|FALSE

  # No I2S audio
  gRK3588TokenSpaceGuid.PcdI2S0Supported|FALSE

[Components.common]
  # ACPI tables
  $(PLATFORM_DIRECTORY)/AcpiTables/AcpiTables.inf

  # Device tree (vendor)
  $(PLATFORM_DIRECTORY)/DeviceTree/Vendor.inf

  # Splash screen logo
  $(VENDOR_DIRECTORY)/Drivers/LogoDxe/LogoDxe.inf

