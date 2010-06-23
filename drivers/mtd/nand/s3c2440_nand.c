/*
 * (C) Copyright 2006 OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
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

#include <common.h>

#include <nand.h>
#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>

#define S3C2440_NFCONT_ENABLE      (1<<0)
#define S3C2440_NFCONT_nFCE        (1<<1)
#define S3C2440_NFCONT_INITECC     (1<<4)

#define S3C2440_NFCONF_TACLS(x)    ((x)<<12)
#define S3C2440_NFCONF_TWRPH0(x)   ((x)<<8)
#define S3C2440_NFCONF_TWRPH1(x)   ((x)<<4)

#define S3C2440_NFSTAT_READY		(0x01)

#ifdef CONFIG_NAND_SPL

/* in the early stage of NAND flash booting, printf() is not available */
#define printf(fmt, args...)

static void nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i;
	struct nand_chip *this = mtd->priv;

	for (i = 0; i < len; i++)
		buf[i] = readb(this->IO_ADDR_R);
}
#endif

static void s3c2440_select_chip(struct mtd_info *mtd, int chip)
{
	struct s3c2440_nand *nand = s3c2440_get_base_nand();
	
	if (chip == -1){
		writel(readl(&nand->NFCONT) | S3C2440_NFCONT_nFCE,
		       &nand->NFCONT);
	
	} else {
		writel(readl(&nand->NFCONT) & ~S3C2440_NFCONT_nFCE,
			       &nand->NFCONT);
	}
}

static void s3c2440_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct s3c2440_nand *nand = s3c2440_get_base_nand();

	debugX(1, "hwcontrol(): 0x%02x 0x%02x\n", cmd, ctrl);

    if (cmd == NAND_CMD_NONE)
	        return;

	if (ctrl & NAND_CLE)
		writeb(cmd, &nand->NFCMD);
	else
		writeb(cmd, &nand->NFADDR);
}

static int s3c2440_dev_ready(struct mtd_info *mtd)
{
	struct s3c2440_nand *nand = s3c2440_get_base_nand();
	debugX(1, "dev_ready\n");
	return readl(&nand->NFSTAT) & S3C2440_NFSTAT_READY;
}

#ifdef CONFIG_S3C2440_NAND_HWECC
void s3c2440_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	struct s3c2440_nand *nand = s3c2440_get_base_nand();
	debugX(1, "s3c2440_nand_enable_hwecc(%p, %d)\n", mtd, mode);
	writel(readl(&nand->NFCONT) | S3C2440_NFCONT_INITECC, &nand->NFCONT);
}

static int s3c2440_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat,
				      u_char *ecc_code)
{
	struct s3c2440_nand *nand = s3c2440_get_base_nand();
	unsigned long ecc = readl(&nand->NFMECC0);
	
	ecc_code[0] = ecc;
	ecc_code[1] = ecc >> 8;
	ecc_code[2] = ecc >> 16;
	
	debugX(1, "s3c2440_nand_calculate_hwecc(%p,): 0x%02x 0x%02x 0x%02x\n",
	       mtd , ecc_code[0], ecc_code[1], ecc_code[2]);

	return 0;
}

static int s3c2440_nand_correct_data(struct mtd_info *mtd, u_char *dat,
				     u_char *read_ecc, u_char *calc_ecc)
{
	if (read_ecc[0] == calc_ecc[0] &&
	    read_ecc[1] == calc_ecc[1] &&
	    read_ecc[2] == calc_ecc[2])
		return 0;

	printf("s3c2440_nand_correct_data: not implemented\n");
	return -1;
}
#endif

int board_nand_init(struct nand_chip *nand)
{
	u_int32_t cfg, set, mask;
	u_int8_t tacls, twrph0, twrph1;
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();
	struct s3c2440_nand *nand_reg = s3c2440_get_base_nand();

	debugX(1, "board_nand_init()\n");

	/* Enable Control HCLK into NAND Flash Control block */
	writel(readl(&clk_power->CLKCON) | (1 << 4), &clk_power->CLKCON);

	/* initialize hardware */
	twrph0 = 3;
	twrph1 = 1;
	tacls = 1;

	mask = S3C2440_NFCONF_TACLS(3) | S3C2440_NFCONF_TWRPH0(7) | S3C2440_NFCONF_TWRPH1(7);	
	set = S3C2440_NFCONF_TACLS(tacls - 1) |
		S3C2440_NFCONF_TWRPH0(twrph0 - 1) | 
		S3C2440_NFCONF_TWRPH1(twrph1 - 1);
	cfg = readl(&nand_reg->NFCONF);
	cfg &= ~mask;
	cfg |= set;
	writel(cfg, &nand_reg->NFCONF);
	
	/* enable controller */
	writel(S3C2440_NFCONT_ENABLE, &nand_reg->NFCONT);

	/* initialize nand_chip data structure */
	nand->IO_ADDR_R = nand->IO_ADDR_W = (void *)&nand_reg->NFDATA;

	nand->select_chip = s3c2440_select_chip;

	/* read_buf and write_buf are default */
	/* read_byte and write_byte are default */
#ifdef CONFIG_NAND_SPL
	nand->read_buf = nand_read_buf;
#endif

	/* hwcontrol always must be implemented */
	nand->cmd_ctrl = s3c2440_hwcontrol;

	nand->dev_ready = s3c2440_dev_ready;

#ifdef CONFIG_S3C2440_NAND_HWECC
	nand->ecc.hwctl = s3c2440_nand_enable_hwecc;
	nand->ecc.calculate = s3c2440_nand_calculate_ecc;
	nand->ecc.correct = s3c2440_nand_correct_data;
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.size = CONFIG_SYS_NAND_ECCSIZE;
	nand->ecc.bytes = CONFIG_SYS_NAND_ECCBYTES;
#else
	nand->ecc.mode = NAND_ECC_NONE;	//NAND_ECC_SOFT;
#endif

#ifdef CONFIG_S3C2440_NAND_BBT
	nand->options = NAND_USE_FLASH_BBT;
#else
	nand->options = 0;
#endif

	debugX(1, "end of nand_init\n");

	return 0;
}
