/** @file
*
*  Phoenix RK3588 Platform Library
*
*  Board-specific GPIO, PMIC, PCIe, USB, and GMAC configuration.
*
*  Hardware summary:
*    - Dual NVMe via PCIe 3.0 NANBNB (pcie3x4 = 2 lanes, pcie3x2 = 2 lanes)
*    - Dual GMAC with fixed-link to YT9215S DSA switches
*    - USB 2.0 host only (EHCI/OHCI)
*    - SPI NOR flash on fspim0
*    - Headless (no HDMI/DP)
*
*  GPIO map (from Phoenix DTS):
*    PCIe 3.0 power enable:  GPIO2_PB6
*    PCIe 3x4 reset:         GPIO4_PB6
*    PCIe 3x2 reset:         GPIO3_PD4
*    USB host power:          GPIO3_PD5
*    GMAC0 PHY reset:         GPIO3_PD0
*    GMAC1 PHY reset:         GPIO4_PB3
*
*  Copyright (c) 2021, Rockchip Limited. All rights reserved.
*  Copyright (c) 2023-2024, Mario Balanica <mariobalanica02@gmail.com>
*  Copyright (c) 2026, Phoenix Project
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/GpioLib.h>
#include <Library/TimerLib.h>
#include <Library/RK806.h>
#include <Library/Rk3588Pcie.h>
#include <Soc.h>
#include <VarStoreData.h>

static struct regulator_init_data  rk806_init_data[] = {
  /* Master PMIC */
  RK8XX_VOLTAGE_INIT (MASTER_BUCK1,  750000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK3,  750000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK4,  750000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK5,  850000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK7,  2000000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK8,  3300000),
  RK8XX_VOLTAGE_INIT (MASTER_BUCK10, 1800000),

  RK8XX_VOLTAGE_INIT (MASTER_NLDO1,  750000),
  RK8XX_VOLTAGE_INIT (MASTER_NLDO2,  850000),
  RK8XX_VOLTAGE_INIT (MASTER_NLDO3,  750000),
  RK8XX_VOLTAGE_INIT (MASTER_NLDO4,  850000),
  RK8XX_VOLTAGE_INIT (MASTER_NLDO5,  750000),

  RK8XX_VOLTAGE_INIT (MASTER_PLDO1,  1800000),
  RK8XX_VOLTAGE_INIT (MASTER_PLDO2,  1800000),
  RK8XX_VOLTAGE_INIT (MASTER_PLDO3,  1200000),
  RK8XX_VOLTAGE_INIT (MASTER_PLDO4,  3300000),
  RK8XX_VOLTAGE_INIT (MASTER_PLDO5,  3300000),
  RK8XX_VOLTAGE_INIT (MASTER_PLDO6,  1800000),
};

VOID
EFIAPI
SdmmcIoMux (
  VOID
  )
{
  /* sdmmc0 iomux (microSD socket) */
  BUS_IOC->GPIO4D_IOMUX_SEL_L  = (0xFFFFUL << 16) | (0x1111);
  BUS_IOC->GPIO4D_IOMUX_SEL_H  = (0x00FFUL << 16) | (0x0011);
  PMU1_IOC->GPIO0A_IOMUX_SEL_H = (0x000FUL << 16) | (0x0001);
}

VOID
EFIAPI
SdhciEmmcIoMux (
  VOID
  )
{
  /* sdhci0 iomux (eMMC) */
  BUS_IOC->GPIO2A_IOMUX_SEL_L = (0xFFFFUL << 16) | (0x1111);
  BUS_IOC->GPIO2D_IOMUX_SEL_L = (0xFFFFUL << 16) | (0x1111);
  BUS_IOC->GPIO2D_IOMUX_SEL_H = (0xFFFFUL << 16) | (0x1111);
}

#define NS_CRU_BASE       0xFD7C0000
#define CRU_CLKSEL_CON59  0x03EC
#define CRU_CLKSEL_CON78  0x0438

VOID
EFIAPI
Rk806SpiIomux (
  VOID
  )
{
  /* io mux for RK806 PMIC SPI */
  PMU1_IOC->GPIO0A_IOMUX_SEL_H = (0x0FF0UL << 16) | 0x0110;
  PMU1_IOC->GPIO0B_IOMUX_SEL_L = (0xF0FFUL << 16) | 0x1011;
  MmioWrite32 (NS_CRU_BASE + CRU_CLKSEL_CON59, (0x00C0UL << 16) | 0x0080);
}

VOID
EFIAPI
Rk806Configure (
  VOID
  )
{
  UINTN  RegCfgIndex;

  RK806Init ();

  RK806PinSetFunction (MASTER, 1, 2);

  for (RegCfgIndex = 0; RegCfgIndex < ARRAY_SIZE (rk806_init_data); RegCfgIndex++) {
    RK806RegulatorInit (rk806_init_data[RegCfgIndex]);
  }
}

VOID
EFIAPI
SetCPULittleVoltage (
  IN UINT32  Microvolts
  )
{
  struct regulator_init_data  Rk806CpuLittleSupply =
    RK8XX_VOLTAGE_INIT (MASTER_BUCK2, Microvolts);

  RK806RegulatorInit (Rk806CpuLittleSupply);
}

VOID
EFIAPI
NorFspiIomux (
  VOID
  )
{
  /*
   * Phoenix uses FSPI M0 (fspim0) for SPI NOR flash.
   * This matches the U-Boot DTS: pinctrl-0 = <&fspim0_pins>
   */
  MmioWrite32 (
    NS_CRU_BASE + CRU_CLKSEL_CON78,
    (((0x3 << 12) | (0x3f << 6)) << 16) | (0x0 << 12) | (0x3f << 6)
    );

  /* FSPI M0 */
  BUS_IOC->GPIO2A_IOMUX_SEL_L = ((0xF << 0) << 16) | (2 << 0);
  BUS_IOC->GPIO2D_IOMUX_SEL_L = (0xFFFFUL << 16) | (0x2222);
  BUS_IOC->GPIO2D_IOMUX_SEL_H = ((0xF << 8) << 16) | (0x2 << 8);
}

VOID
EFIAPI
GmacIomux (
  IN UINT32  Id
  )
{
  switch (Id) {
    case 0:
      /* GMAC0 RGMII iomux */
      BUS_IOC->GPIO4A_IOMUX_SEL_L = (0xFFFFUL << 16) | (0x1111);
      BUS_IOC->GPIO4A_IOMUX_SEL_H = (0xFFFFUL << 16) | (0x1111);
      BUS_IOC->GPIO4B_IOMUX_SEL_L = (0x0FFFUL << 16) | (0x0111);
      BUS_IOC->GPIO2B_IOMUX_SEL_H = (0x0FF0UL << 16) | (0x0110);
      BUS_IOC->GPIO2C_IOMUX_SEL_L = (0xFFFFUL << 16) | (0x1111);

      /* GMAC0 PHY reset pin: GPIO3_PD0, configure as output */
      GpioPinSetDirection (3, GPIO_PIN_PD0, GPIO_PIN_OUTPUT);
      break;

    case 1:
      /* GMAC1 RGMII iomux */
      BUS_IOC->GPIO3B_IOMUX_SEL_L = (0xFFFFUL << 16) | (0x1111);
      BUS_IOC->GPIO3B_IOMUX_SEL_H = (0xFFFFUL << 16) | (0x1111);
      BUS_IOC->GPIO3C_IOMUX_SEL_L = (0x0FFFUL << 16) | (0x0111);
      BUS_IOC->GPIO3A_IOMUX_SEL_H = (0x0FF0UL << 16) | (0x0110);
      BUS_IOC->GPIO3A_IOMUX_SEL_L = (0xFFFFUL << 16) | (0x1111);

      /* GMAC1 PHY reset pin: GPIO4_PB3, configure as output */
      GpioPinSetDirection (4, GPIO_PIN_PB3, GPIO_PIN_OUTPUT);
      break;

    default:
      break;
  }
}

VOID
EFIAPI
NorFspiEnableClock (
  UINT32  *CruBase
  )
{
  UINTN  BaseAddr = (UINTN)CruBase;

  MmioWrite32 (BaseAddr + 0x087C, 0x0E000000);
}

VOID
EFIAPI
GmacIoPhyReset (
  IN UINT32   Id,
  IN BOOLEAN  Enable
  )
{
  switch (Id) {
    case 0:
      /* GMAC0 PHY reset: GPIO3_PD0, active low */
      GpioPinWrite (3, GPIO_PIN_PD0, !Enable);
      break;
    case 1:
      /* GMAC1 PHY reset: GPIO4_PB3, active low */
      GpioPinWrite (4, GPIO_PIN_PB3, !Enable);
      break;
    default:
      break;
  }
}

VOID
EFIAPI
I2cIomux (
  UINT32  id
  )
{
  switch (id) {
    case 0:
      GpioPinSetFunction (0, GPIO_PIN_PD1, 3);
      GpioPinSetFunction (0, GPIO_PIN_PD2, 3);
      break;
    default:
      break;
  }
}

VOID
EFIAPI
UsbPortPowerEnable (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "Phoenix: UsbPortPowerEnable\n"));

  /*
   * USB host power: GPIO3_PD5 (vcc5v0_host)
   * From Phoenix DTS: enable-active-high, gpio = <&gpio3 RK_PD5>
   */
  GpioPinWrite (3, GPIO_PIN_PD5, TRUE);
  GpioPinSetDirection (3, GPIO_PIN_PD5, GPIO_PIN_OUTPUT);
}

VOID
EFIAPI
Usb2PhyResume (
  VOID
  )
{
  /* Resume all USB 2.0 PHYs */
  MmioWrite32 (0xfd5d0008, 0x20000000);
  MmioWrite32 (0xfd5d4008, 0x20000000);
  MmioWrite32 (0xfd5d8008, 0x20000000);
  MmioWrite32 (0xfd5dc008, 0x20000000);
  MmioWrite32 (0xfd7f0a10, 0x07000700);
  MmioWrite32 (0xfd7f0a10, 0x07000000);
}

VOID
EFIAPI
PcieIoInit (
  UINT32  Segment
  )
{
  switch (Segment) {
    case PCIE_SEGMENT_PCIE30X4:
      /*
       * PCIe 3x4 (used as 2-lane in NANBNB mode)
       * Reset: GPIO4_PB6
       * Power: GPIO2_PB6 (vcc3v3_pcie30, shared with 3x2)
       */
      GpioPinSetDirection (4, GPIO_PIN_PB6, GPIO_PIN_OUTPUT);
      GpioPinSetDirection (2, GPIO_PIN_PB6, GPIO_PIN_OUTPUT);
      break;

    case PCIE_SEGMENT_PCIE30X2:
      /*
       * PCIe 3x2 (used as 2-lane in NANBNB mode)
       * Reset: GPIO3_PD4
       * Power: shared vcc3v3_pcie30
       */
      GpioPinSetDirection (3, GPIO_PIN_PD4, GPIO_PIN_OUTPUT);
      break;

    default:
      break;
  }
}

VOID
EFIAPI
PciePowerEn (
  UINT32   Segment,
  BOOLEAN  Enable
  )
{
  switch (Segment) {
    case PCIE_SEGMENT_PCIE30X4:
      /* vcc3v3_pcie30: GPIO2_PB6, active high */
      GpioPinWrite (2, GPIO_PIN_PB6, Enable);
      break;

    case PCIE_SEGMENT_PCIE30X2:
      /* Shares the same vcc3v3_pcie30 regulator, already enabled by 3x4 */
      break;

    default:
      break;
  }
}

VOID
EFIAPI
PciePeReset (
  UINT32   Segment,
  BOOLEAN  Enable
  )
{
  switch (Segment) {
    case PCIE_SEGMENT_PCIE30X4:
      /* Reset: GPIO4_PB6, active high in DTS -> invert for PERST# */
      GpioPinWrite (4, GPIO_PIN_PB6, !Enable);
      break;

    case PCIE_SEGMENT_PCIE30X2:
      /* Reset: GPIO3_PD4 */
      GpioPinWrite (3, GPIO_PIN_PD4, !Enable);
      break;

    default:
      break;
  }
}

VOID
EFIAPI
HdmiTxIomux (
  IN UINT32  Id
  )
{
  /* Phoenix is headless - no HDMI */
}

VOID
EFIAPI
PwmFanIoSetup (
  VOID
  )
{
  /* No PWM fan on Phoenix */
}

VOID
EFIAPI
PwmFanSetSpeed (
  IN UINT32  Percentage
  )
{
  /* No PWM fan on Phoenix */
}

VOID
EFIAPI
PlatformInitLeds (
  VOID
  )
{
  /* No status LED defined for Phoenix */
}

VOID
EFIAPI
PlatformSetStatusLed (
  IN BOOLEAN  Enable
  )
{
  /* No status LED defined for Phoenix */
}

CONST EFI_GUID *
EFIAPI
PlatformGetDtbFileGuid (
  IN UINT32  CompatMode
  )
{
  STATIC CONST EFI_GUID  VendorDtbFileGuid = {
    // DeviceTree/Vendor.inf
    0xAABBCC01, 0x1234, 0x5678, { 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78 }
  };

  switch (CompatMode) {
    case FDT_COMPAT_MODE_VENDOR:
      return &VendorDtbFileGuid;
  }

  return NULL;
}

VOID
EFIAPI
PlatformEarlyInit (
  VOID
  )
{
  /* Phoenix-specific early init */
}
