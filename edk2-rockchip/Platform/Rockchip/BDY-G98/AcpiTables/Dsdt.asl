/** @file
 *
 *  Differentiated System Definition Table (DSDT)
 *  Phoenix RK3588 - Headless device with dual NVMe + dual GMAC (YT9215S DSA)
 *
 *  Copyright (c) 2020, Pete Batard <pete@akeo.ie>
 *  Copyright (c) 2021, ARM Limited. All rights reserved.
 *  Copyright (c) 2026, Phoenix Project
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include "AcpiTables.h"

DefinitionBlock ("Dsdt.aml", "DSDT", 2, "RKCP  ", "RK3588  ", 2)
{
  Scope (\_SB_)
  {
    include ("DsdtCommon.asl")

    include ("Cpu.asl")

    include ("Pcie.asl")
    include ("Emmc.asl")
    include ("Sdhc.asl")
    include ("Dma.asl")
    include ("Gmac0.asl")
    include ("Gmac1.asl")
    include ("Gpio.asl")
    include ("I2c.asl")
    include ("Uart.asl")

    include ("Usb2Host.asl")
  }
}
