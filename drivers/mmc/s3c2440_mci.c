/*
 * Driver for S3C2440 MMC controller
 *
 * jimmy.li <lizhm82@gmail.com>
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <part.h>
#include <mmc.h>

#include <asm/io.h>
#include <asm/errno.h>
#include <asm/byteorder.h>

#include <asm/arch/s3c24x0_cpu.h>

#define sdi_error		printf
#define sdi_debug		//printf

/* SDI Control Register */
#define S3C2440_SDICON_SDRESET        (1<<8)
#define S3C2440_SDICON_MMCCLOCK       (1<<5)
#define S3C2440_SDICON_BYTEORDER      (1<<4)
#define S3C2440_SDICON_SDIOIRQ        (1<<3)
#define S3C2440_SDICON_RWAITEN        (1<<2)
#define S3C2440_SDICON_FIFORESET      (1<<1)
#define S3C2440_SDICON_CLOCKTYPE      (1<<0)

/* SDI Command Control Register */
#define S3C2440_SDICMDCON_ABORT       (1<<12)
#define S3C2440_SDICMDCON_WITHDATA    (1<<11)
#define S3C2440_SDICMDCON_LONGRSP     (1<<10)
#define S3C2440_SDICMDCON_WAITRSP     (1<<9)
#define S3C2440_SDICMDCON_CMDSTART    (1<<8)
#define S3C2440_SDICMDCON_SENDERHOST  (1<<6)
#define S3C2440_SDICMDCON_INDEX       (0x3f)

/* SDI Command Status Register */
#define S3C2440_SDICMDSTAT_CRCFAIL    (1<<12)
#define S3C2440_SDICMDSTAT_CMDSENT    (1<<11)
#define S3C2440_SDICMDSTAT_CMDTIMEOUT (1<<10)
#define S3C2440_SDICMDSTAT_RSPFIN     (1<<9)
#define S3C2440_SDICMDSTAT_XFERING    (1<<8)
#define S3C2440_SDICMDSTAT_INDEX      (0xff)

/* SDI Data Control Register */
#define S3C2440_SDIDCON_BURST4EN      (1<<24)
#define S3C2440_SDIDCON_DS_BYTE       (0<<22)
#define S3C2440_SDIDCON_DS_HALFWORD   (1<<22)
#define S3C2440_SDIDCON_DS_WORD       (2<<22)
#define S3C2440_SDIDCON_IRQPERIOD     (1<<21)
#define S3C2440_SDIDCON_TXAFTERRESP   (1<<20)
#define S3C2440_SDIDCON_RXAFTERCMD    (1<<19)
#define S3C2440_SDIDCON_BUSYAFTERCMD  (1<<18)
#define S3C2440_SDIDCON_BLOCKMODE     (1<<17)
#define S3C2440_SDIDCON_WIDEBUS       (1<<16)
#define S3C2440_SDIDCON_DMAEN         (1<<15)
#define S3C2440_SDIDCON_STOP          (1<<14)
#define S3C2440_SDIDCON_DATSTART      (1<<14)
#define S3C2440_SDIDCON_DATMODE       (3<<12)
//#define S3C2440_SDIDCON_BLKNUM        (0x7ff)
#define S3C2440_SDIDCON_BLKNUM_MASK   (0xFFF)
#define S3C2440_SDIDCNT_BLKNUM_SHIFT  (12)

#define S3C2440_SDIDCON_XFER_READY    (0<<12)
#define S3C2440_SDIDCON_XFER_CHKSTART (1<<12)
#define S3C2440_SDIDCON_XFER_RXSTART  (2<<12)
#define S3C2440_SDIDCON_XFER_TXSTART  (3<<12)

/* SDI Data Status Register */
#define S3C2440_SDIDSTA_NOBUSY		  (1<<11)
#define S3C2440_SDIDSTA_RDYWAITREQ    (1<<10)
#define S3C2440_SDIDSTA_SDIOIRQDETECT (1<<9)
#define S3C2440_SDIDSTA_CRCFAIL       (1<<7)
#define S3C2440_SDIDSTA_RXCRCFAIL     (1<<6)
#define S3C2440_SDIDSTA_DATATIMEOUT   (1<<5)
#define S3C2440_SDIDSTA_XFERFINISH    (1<<4)
#define S3C2440_SDIDSTA_BUSYFINISH    (1<<3)
#define S3C2440_SDIDSTA_SBITERR       (1<<2)    /* reserved on 2440a/2440 */
#define S3C2440_SDIDSTA_TXDATAON      (1<<1)
#define S3C2440_SDIDSTA_RXDATAON      (1<<0)

/* SDI FIFO Status Register */
#define S3C2440_SDIFSTA_FIFORESET      (1<<16)
#define S3C2440_SDIFSTA_FIFOFAIL       (3<<14)  /* 3 is correct (2 bits) */
#define S3C2440_SDIFSTA_TFDET          (1<<13)
#define S3C2440_SDIFSTA_RFDET          (1<<12)
#define S3C2440_SDIFSTA_TFHALF         (1<<11)
#define S3C2440_SDIFSTA_TFEMPTY        (1<<10)
#define S3C2440_SDIFSTA_RFLAST         (1<<9)
#define S3C2440_SDIFSTA_RFFULL         (1<<8)
#define S3C2440_SDIFSTA_RFHALF         (1<<7)
#define S3C2440_SDIFSTA_COUNTMASK      (0x7f)

static void s3c2440_mci_reset(struct mmc* mmc)
{
	volatile struct s3c2440_sdi * const sdi = s3c2440_get_base_sdi();

	sdi->SDICON |= S3C2440_SDICON_SDRESET;
}

static int mci_setup_data(struct mmc* mmc, struct mmc_data *mmc_data)
{
	volatile struct s3c2440_sdi * const sdi = s3c2440_get_base_sdi();
	int retries = 20;
	u32 dcon;

	/* Don't support write yet. */
	if (mmc_data->flags & MMC_DATA_WRITE)
		return UNUSABLE_ERR;
	
	while (sdi->SDIDSTA & (S3C2440_SDIDSTA_TXDATAON | S3C2440_SDIDSTA_RXDATAON)) {
		sdi_debug("mci_setup_data : transfer still in progress!\n");

		sdi->SDIDCON = S3C2440_SDIDCON_STOP;

		s3c2440_mci_reset(mmc);
		udelay(2);

		if (retries-- == 0) {
			sdi_error("transfer still in progress, reset failed!\n");
			return COMM_ERR;
		}
	}
	
	sdi_debug("%s : blocks %d, blocksize %d\n", __FUNCTION__, mmc_data->blocks, mmc_data->blocksize);
	
	dcon = mmc_data->blocks & S3C2440_SDIDCON_BLKNUM_MASK;  
	
	if (mmc->bus_width == 4)
		dcon |= S3C2440_SDIDCON_WIDEBUS;

	dcon |= S3C2440_SDIDCON_BLOCKMODE;
	
	if (mmc_data->flags & MMC_DATA_WRITE) {
		dcon |= S3C2440_SDIDCON_TXAFTERRESP;
		dcon |= S3C2440_SDIDCON_XFER_TXSTART;
	}

	if (mmc_data->flags & MMC_DATA_READ) {
		dcon |=	S3C2440_SDIDCON_RXAFTERCMD;
		dcon |= S3C2440_SDIDCON_XFER_RXSTART;
	}
	
	dcon |= S3C2440_SDIDCON_DS_WORD;
	dcon |= S3C2440_SDIDCON_DATSTART;

	/* write DataControl register */
	sdi->SDIDCON = dcon;

	/* write BSIZE register */
	sdi->SDIBSIZE = mmc_data->blocksize;

	/* write TIMER register */
	sdi->SDIDTIMER = 0x007fffff;
	
	return 0;
}


static u32 fifo_count(struct mmc* mmc)
{
	volatile struct s3c2440_sdi * const sdi = s3c2440_get_base_sdi();
    u32 fifostat = sdi->SDIFSTA;

	fifostat &= S3C2440_SDIFSTA_COUNTMASK;
	return fifostat;
}

static int sdi_wait_data_done(struct mmc* mmc, struct mmc_data *mmc_data)
{
	volatile struct s3c2440_sdi * const sdi = s3c2440_get_base_sdi();
	u32 dsta, fsta;
	int wait_times = 0x10000;
	int ret = 0;
	
	u32 fifo;
	u32 fifo_words;
	u32 *ptr = (u32*)mmc_data->dest;

	do {
		fsta = sdi->SDIFSTA;

		if (fsta & S3C2440_SDIFSTA_RFDET) {
			while (fifo = fifo_count(mmc)) {
				fifo_words = fifo >> 2;
				sdi_debug("%s : fifo %d, fifo_words %d\n", __FUNCTION__, fifo, fifo_words);
				while(fifo_words--)
					*ptr++ = sdi->SDIDAT;
				udelay(10);
			}
		}


		dsta = sdi->SDIDSTA;
		
		if (dsta & S3C2440_SDIDSTA_RXCRCFAIL)
			ret = COMM_ERR;
		else if (dsta & S3C2440_SDIDSTA_DATATIMEOUT)
			ret = TIMEOUT;
		
		if (ret) {
			sdi_error("rs : error %d\n", __FUNCTION__, ret);
			break;
		}

		if (dsta & S3C2440_SDIDSTA_XFERFINISH) {
			while (fifo = fifo_count(mmc)) {
				fifo_words = fifo >> 2;
				sdi_debug("##%s : fifo %d, fifo_words %d\n", __FUNCTION__, fifo, fifo_words);
				while(fifo_words--)
					*ptr++ = sdi->SDIDAT;
			}
	
			break;
		}
	} while(wait_times--);

	if (wait_times <= 0) {
		sdi_error("%s : wait data done timeout!\n", __FUNCTION__);
		ret = TIMEOUT;
	}


	//clear status bits
	sdi->SDIFSTA = S3C2440_SDIFSTA_FIFOFAIL | S3C2440_SDIFSTA_RFLAST;
	sdi->SDIDSTA = S3C2440_SDIDSTA_NOBUSY | S3C2440_SDIDSTA_RDYWAITREQ | S3C2440_SDIDSTA_SDIOIRQDETECT |
			S3C2440_SDIDSTA_CRCFAIL | S3C2440_SDIDSTA_RXCRCFAIL | S3C2440_SDIDSTA_DATATIMEOUT |
			S3C2440_SDIDSTA_XFERFINISH | S3C2440_SDIDSTA_BUSYFINISH;
	
	return ret;
}

static int mci_send_cmd(struct mmc* mmc, struct mmc_cmd *mmc_cmd)
{
	volatile struct s3c2440_sdi * const sdi = s3c2440_get_base_sdi();
	int cmd = mmc_cmd->cmdidx;
	int flags = mmc_cmd->resp_type;
	int arg = mmc_cmd->cmdarg;
	u32 ccon;
	u32 csta;
	int ret = 0;
	int wait_times = 1000;

	/* write command argument */
	sdi->SDICARG = arg;

	ccon = cmd & S3C2440_SDICMDCON_INDEX;
	ccon |= S3C2440_SDICMDCON_SENDERHOST | S3C2440_SDICMDCON_CMDSTART;

	if (flags & MMC_RSP_PRESENT)
		ccon |= S3C2440_SDICMDCON_WAITRSP;
	if (flags & MMC_RSP_136)
		ccon |= S3C2440_SDICMDCON_LONGRSP;

	/* set command types, and start transfer */
	sdi->SDICCON = ccon;


	/* wait cmd done and response received if need */
	do {
		udelay(1);
		csta = sdi->SDICSTA;
		
		if ( csta & (S3C2440_SDICMDSTAT_CRCFAIL | S3C2440_SDICMDSTAT_CMDSENT | 
				S3C2440_SDICMDSTAT_CMDTIMEOUT | S3C2440_SDICMDSTAT_RSPFIN) ) {
			
			if (csta & S3C2440_SDICMDSTAT_CMDTIMEOUT)
				ret = TIMEOUT;
			else if ((csta & S3C2440_SDICMDSTAT_CRCFAIL) && (flags & MMC_RSP_CRC))
				ret = COMM_ERR;

			if (ret)
				break;

			if (flags & MMC_RSP_PRESENT) {
				if (csta & S3C2440_SDICMDSTAT_RSPFIN) {
					sdi_debug("command send complete (have response)!\n");
					mmc_cmd->response[0] = sdi->SDIRSP0;
					if (flags & MMC_RSP_136) {
						mmc_cmd->response[1] = sdi->SDIRSP1;
						mmc_cmd->response[2] = sdi->SDIRSP2;
						mmc_cmd->response[3] = sdi->SDIRSP3;
					}
					break;
				}
			}else {
			 	if (csta & S3C2440_SDICMDSTAT_CMDSENT) {
					sdi_debug("command send complete (no response)!\n");
					break;
				}
			}
			
		}
		
	}while (wait_times--);

	if (wait_times <= 0) {
		sdi_error("wait command done timeout!\n");
		ret = TIMEOUT;
	}
	
	// clear status
	sdi->SDICSTA = (S3C2440_SDICMDSTAT_CRCFAIL | S3C2440_SDICMDSTAT_CMDSENT | 
			S3C2440_SDICMDSTAT_CMDTIMEOUT | S3C2440_SDICMDSTAT_RSPFIN);

	return ret;
}

static int s3c2440_mci_request(struct mmc *mmc, struct mmc_cmd *cmd,
		struct mmc_data *data)
{
	volatile struct s3c2440_sdi * const sdi = s3c2440_get_base_sdi();
	int ret = 0;

	sdi_debug("%s : cmd_idx=%d\n", __FUNCTION__, cmd->cmdidx);
	
	/* Clear command, data and fifo status registers */
	sdi->SDICSTA = 0xffffffff;
	sdi->SDIDSTA = 0xffffffff;
	sdi->SDIFSTA = 0xffffffff;

	
	if (data)
		mci_setup_data(mmc, data);

	ret = mci_send_cmd(mmc, cmd);
	
	if (data)
		ret = sdi_wait_data_done(mmc, data);
	
	return ret;
}

static void mci_set_clock(uint clock)
{
	volatile struct s3c2440_sdi * const sdi = s3c2440_get_base_sdi();
    u32 mci_psc;
	u32	clk_ctl = get_PCLK();
	u32 real_rate;

	/* Set clock */
	for (mci_psc = 0; mci_psc < 255; mci_psc++) {
		real_rate = clk_ctl / (mci_psc+1);
		
		if (real_rate <= clock)
			break;
	}

	if (mci_psc > 255)
		mci_psc = 255;

	sdi->SDIPRE = mci_psc;
	
	/* Set CLOCK_ENABLE */
	if (clock)
		sdi->SDICON |= S3C2440_SDICON_CLOCKTYPE;
	else
		sdi->SDICON &= ~S3C2440_SDICON_CLOCKTYPE;
}

static void s3c2440_mci_set_ios(struct mmc *mmc)
{
	mci_set_clock(mmc->clock);
}

static int s3c2440_mci_init(struct mmc *mmc)
{
	volatile struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();
	volatile struct s3c24x0_clock_power * const clk_power = s3c24x0_get_base_clock_power();
	volatile struct s3c2440_sdi * const sdi = s3c2440_get_base_sdi();

	// set up the MCI IO ports : GPE5 - GPE10
	gpio->GPECON &= ~0x003ffc00;
	gpio->GPECON |= 0x002aa800;

	// SDI clock enable
	clk_power->CLKCON |= (1<<9);

	sdi->SDIIMSK = 0x0;	//disable all interrupt
	return 0;
}


int s3c2440_mmc_init(bd_t *bis)
{
	struct mmc *mmc = NULL;
	
	mmc = malloc(sizeof(struct mmc));

	if (!mmc)
		return -ENOMEM;
	sprintf(mmc->name, "s3c2440 MMC");
	mmc->send_cmd = s3c2440_mci_request;
	mmc->set_ios = s3c2440_mci_set_ios;
	mmc->init = s3c2440_mci_init;
	mmc->host_caps = MMC_MODE_4BIT;

	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->f_max = get_PCLK();
	mmc->f_min = mmc->f_max / 256;
	mmc->block_dev.part_type = PART_TYPE_DOS;

	mmc_register(mmc);

	return 0;
}
