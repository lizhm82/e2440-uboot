/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <garyj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * Configuation settings for the SAMSUNG SMDK2410 board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define MHz(x)		((x) << 20)
#define MiB(x)		((x) << 20)
#define KiB(x)		((x) << 10)

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM920T	1	/* This is an ARM920T Core	*/
#define CONFIG_S3C24X0	1	/* in a SAMSUNG S3C24x0-type SoC	*/
#define CONFIG_S3C2440	1	/* specifically a SAMSUNG S3C2440 SoC	*/
#define CONFIG_E2440	1	/* on a e2440 Board  */

#define CONFIG_SYS_SDRAM_BASE		(0x30000000)	// physical memory start address


/*
 * e2440 board specific data
 */
/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ		MHz(12)		/* the e2440 has 12MHz input clock */

#define CONFIG_SYS_PHY_UBOOT_BASE		(CONFIG_SYS_SDRAM_BASE + 0x03f80000)	// base address for u-boot
#define CONFIG_SYS_UBOOT_SIZE			KiB(512)			// total memory available for uboot


#define USE_920T_MMU		1
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

#define	CONFIG_SYS_HZ			1000
/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */

/*
 * Ethernet support
 */
#define CONFIG_CMD_NET
#define CONFIG_NET_MULTI

#define CONFIG_CS8900		/* we have a CS8900 on-board */
#define CONFIG_CS8900_BASE		0x19000000
#define CONFIG_CS8900_BUS16


/*
 * select serial console configuration
 */
#define CONFIG_S3C24X0_SERIAL
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on e2440 */
#define CONFIG_BAUDRATE		115200
/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Nand Flash support
 */
#define CONFIG_CMD_NAND
#define CONFIG_NAND_S3C2440
#define CONFIG_SYS_MAX_NAND_DEVICE		1	/* Max number of NAND devices */
#define CONFIG_SYS_NAND_BASE 	0x4e000010

//#define CONFIG_S3C2440_NAND_HWECC

#ifdef CONFIG_S3C2440_NAND_HWECC
#define CONFIG_SYS_NAND_ECCSIZE 	256		//??
#define CONFIG_SYS_NAND_ECCBYTES    3		//??
#endif

/************************************************************
 * RTC
 ************************************************************/
#define	CONFIG_RTC_S3C24X0	1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE



/*
 * BOOTP options
 */
//#define CONFIG_BOOTP_BOOTFILESIZE
//#define CONFIG_BOOTP_BOOTPATH
//#define CONFIG_BOOTP_GATEWAY
//#define CONFIG_BOOTP_HOSTNAME

#define CONFIG_BOOTDELAY	3
#define CONFIG_ETHADDR		08:00:3e:26:0a:5b 
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		172.16.17.179
#define CONFIG_SERVERIP		172.16.17.152
#define CONFIG_BOOTFILE		"uImage" 
#define CONFIG_BOOTCOMMAND	"tftpboot; bootm" 
#define	CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x8000)	/* default load address	*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"e2440 # "	/* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x30000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x33F00000	/* 63 MB in DRAM	*/


#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING			/* cmd history */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		(CONFIG_SYS_SDRAM_BASE) /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	MiB(64)	 /* 64 MB */

/*-----------------------------------------------------------------------
 * NAND and environment organization
 */
#define CONFIG_SYS_NO_FLASH


//#define	CONFIG_ENV_IS_IN_NAND	1
#define CONFIG_ENV_IS_NOWHERE		1

#define CONFIG_ENV_OFFSET		0x0040000
#define CONFIG_ENV_SIZE		0x10000	/* Total Size of Environment Sector */

//-----------------Nand SPL ----------------
//
#define CONFIG_SYS_NAND_U_BOOT_OFFS		(4 * 1024)		/* Offset to U-Boot image in Nand*/
#define CONFIG_SYS_NAND_U_BOOT_SIZE		KiB(254)	/* Size of U-Boot image in Nand */

#define CONFIG_SYS_NAND_U_BOOT_DST		CONFIG_SYS_PHY_UBOOT_BASE	/* NUB load-addr in SDRAM */
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST	/* NUB start-addr in SDRAM  */

/*
 * Now the NAND chip has to be defined (no autodetection used!)
 */
#define CONFIG_SYS_NAND_PAGE_SIZE		512		/* NAND chip page size		*/
#define CONFIG_SYS_NAND_BLOCK_SIZE		KiB(16)		/* NAND chip block size		*/
#define CONFIG_SYS_NAND_PAGE_COUNT		32		/* NAND chip page per block count  */
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	5		/* Location of the bad-block label */
#define CONFIG_SYS_NAND_4_ADDR_CYCLE	1		/* Fourth addr used (>32MB)	*/

#define CONFIG_SYS_NAND_ECCSIZE		256
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_SYS_NAND_ECCSTEPS	(CONFIG_SYS_NAND_PAGE_SIZE / CONFIG_SYS_NAND_ECCSIZE)
#define CONFIG_SYS_NAND_OOBSIZE		16
#define CONFIG_SYS_NAND_ECCTOTAL	(CONFIG_SYS_NAND_ECCBYTES * CONFIG_SYS_NAND_ECCSTEPS)
#define CONFIG_SYS_NAND_ECCPOS		{0, 1, 2, 3, 6, 7}

/*
 * Linux Boot
 */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTARGS     "console=ttyS0,115200n8 root=/dev/nfs rw nfsroot=172.16.17.152:/nfsboot ip=dhcp rdinit=/linuxrc mem=128M"

/*
 * U-BOOT commands
 */
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_SAVEENV

#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP

//#define CONFIG_CMD_CACHE
//#define CONFIG_CMD_DATE
//#define CONFIG_CMD_ELF

#endif	/* __CONFIG_H */
