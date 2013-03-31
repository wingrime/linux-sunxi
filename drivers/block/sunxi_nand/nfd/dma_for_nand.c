/*
 * drivers/block/sunxi_nand/nfd/dma_for_nand.c
 *
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

//#include "nand_oal.h"
#include "nand_private.h"
#include <mach/dma.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/cacheflush.h>

#define DMA_HALF_INT_MASK       (1<<0)
#define DMA_END_INT_MASK        (1<<1)

#define NAND_DMA_TIMEOUT 20000 /*20 sec*/
//__hdle          hNandDmaHdle;
static int nanddma_completed_flag = 1;
static DECLARE_WAIT_QUEUE_HEAD(DMA_wait);

__s32  NAND_DmaInit(void)
{
	return 0;
}

__s32  NAND_DmaExit(void)
{
	return 0;
}

struct sw_dma_client nand_dma_client = {
	.name="NAND_DMA",
};


void nanddma_buffdone(struct sw_dma_chan * ch, void *buf, int size,enum sw_dma_buffresult result){
	nanddma_completed_flag = 1;
	wake_up( &DMA_wait );
	//printk("buffer done. nanddma_completed_flag: %d\n", nanddma_completed_flag);
}
int  nanddma_opfn(struct sw_dma_chan * ch,   enum sw_chan_op op_code){
	if(op_code == SW_DMAOP_START)
		nanddma_completed_flag = 0;

	//printk("buffer opfn: %d, nanddma_completed_flag: %d\n", (int)op_code, nanddma_completed_flag);

	return 0;
}

__hdle NAND_RequestDMA  (__u32 dmatype)
{
	__hdle ch;

	ch = sw_dma_request(DMACH_DNAND, &nand_dma_client, NULL);
	if(ch < 0)
		return ch;

	sw_dma_set_opfn(ch, nanddma_opfn);
	sw_dma_set_buffdone_fn(ch, nanddma_buffdone);

	return ch;
}


__s32  NAND_ReleaseDMA  (__hdle hDma)
{
	return 0;
}


__s32 NAND_SettingDMA(__hdle hDMA, void * pArg)
{
	sw_dma_setflags(hDMA, SW_DMAF_AUTOSTART);
	return sw_dma_config(hDMA, (struct dma_hw_conf*)pArg);
}

__s32 NAND_StartDMA(__u8 rw,__hdle hDMA, __u32 saddr, __u32 daddr, __u32 bytes)
{
	return 0;
}



int seq=0;
__s32 NAND_DMAEqueueBuf(__hdle hDma,  __u32 buff_addr, __u32 len)
{

	nanddma_completed_flag = 0;
	return sw_dma_enqueue((int)hDma, (void*)(seq++), buff_addr, len);
}

__s32 NAND_RestartDMA(__hdle hDma)
{
	return 0;
}

__s32 NAND_StopDMA(__hdle hDma)
{
	return 0;
}

__s32 NAND_EnableDmaINT(__hdle hDma, __u16 xDma, __u32 mode)
{
	return 0;
}

__s32 NAND_DisableDmaINT(__hdle hDma, __u16 xDma)
{
	return 0;
}

__u32 NAND_QueryDmaINT(__hdle hDma)
{
	return 0;
}

__u32 NAND_QueryDmaStat(__hdle hDma)
{
	return 0;
}

__u32 NAND_QueryDmaSrc (__hdle hDma)
{
	return 0;

}

__u32 NAND_QueryDmaDst(__hdle hDma)
{
	return 0;
}


__s32 NAND_WaitDmaFinish(void)
{
	int ret = wait_event_timeout(DMA_wait, nanddma_completed_flag,\
			msecs_to_jiffies(NAND_DMA_TIMEOUT));
	if (!ret)
		pr_err("sunxi:nand: Dma operation finish timeout\n");
	return ret;
}

