/*
 * TVIN Canvas
 *
 * Author: Bobby Yang <bo.yang@amlogic.com>
 *
 * Copyright (C) 2010 Amlogic Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <mach/dmc.h>
#include <linux/amports/canvas.h>
#include <linux/amports/vframe.h>
#include <linux/mm.h>

#include "../tvin_format_table.h"
#include "vdin_drv.h"
#include "vdin_canvas.h"

const unsigned int vdin_canvas_ids[2][24] = {
	{
		128, 129, 130, 131, 132, 133,
		134, 135, 136, 137, 138, 139,
		140, 141, 142, 143, 144, 145,
		146, 147, 148, 149, 150, 151,

	},
	{
		36, 37, 38, 39, 40, 41,
		42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53,
		54, 55, 56, 57, 58, 59,
	},

};

/*switch addr to integer 32 timers*/
static inline void addr2int32(unsigned int *addr)
{
         if((*addr)&0x1f){ 
                (*addr)&=(~0x1f); 
                (*addr) +=0x20;
          }         
}

inline void vdin_canvas_init(struct vdin_dev_s *devp)
{
	int i, canvas_id;
	unsigned int canvas_addr;
	int canvas_max_w = VDIN_CANVAS_MAX_WIDTH << 1;
	int canvas_max_h = VDIN_CANVAS_MAX_HEIGH;
	devp->canvas_max_size = PAGE_ALIGN(canvas_max_w*canvas_max_h);
	devp->canvas_max_num  = devp->mem_size / devp->canvas_max_size;
	if (devp->canvas_max_num > VDIN_CANVAS_MAX_CNT)
		devp->canvas_max_num = VDIN_CANVAS_MAX_CNT;
        
        addr2int32(&devp->mem_start);
	pr_info("vdin.%d cnavas initial table:\n", devp->index);
	for ( i = 0; i < devp->canvas_max_num; i++)
	{
		canvas_id = vdin_canvas_ids[devp->index][i];
		canvas_addr = devp->mem_start + devp->canvas_max_size * i;
                
		canvas_config(canvas_id, canvas_addr, canvas_max_w, canvas_max_h,
				CANVAS_ADDR_NOWRAP, CANVAS_BLKMODE_LINEAR);
		pr_info("\t%d: 0x%x-0x%x  %dx%d (%d KB)\n",
				canvas_id, canvas_addr, (canvas_addr + devp->canvas_max_size),
				canvas_max_w, canvas_max_h, (devp->canvas_max_size >> 10));
	}
}

inline void vdin_canvas_start_config(struct vdin_dev_s *devp)
{
	int i, canvas_id;
	unsigned long canvas_addr;
        unsigned int canvas_max_w = VDIN_CANVAS_MAX_WIDTH << 1;
        unsigned int canvas_max_h = VDIN_CANVAS_MAX_HEIGH;
        unsigned int canvas_num = VDIN_CANVAS_MAX_CNT;

	if ((devp->format_convert == VDIN_FORMAT_CONVERT_YUV_YUV444) ||
	    (devp->format_convert == VDIN_FORMAT_CONVERT_YUV_RGB   ) ||
	    (devp->format_convert == VDIN_FORMAT_CONVERT_RGB_YUV444) ||
	    (devp->format_convert == VDIN_FORMAT_CONVERT_RGB_RGB   ))
		devp->canvas_w = devp->h_active * 3;
	else if((devp->prop.dest_cfmt == TVIN_NV12) ||
                (devp->prop.dest_cfmt == TVIN_NV21))
        {
                canvas_max_w = VDIN_CANVAS_MAX_WIDTH;
		canvas_max_h = VDIN_CANVAS_MAX_HEIGH>>1;
		canvas_num >>= 1;
                devp->canvas_w = devp->h_active;
	}
        else
		devp->canvas_w = devp->h_active * 2;

#if 0
	const struct tvin_format_s *fmt_info = tvin_get_fmt_info(devp->parm.info.fmt);
	if(fmt_info->scan_mode == TVIN_SCAN_MODE_INTERLACED)
		devp->canvas_h = devp->v_active * 2;
	else
		devp->canvas_h = devp->v_active;
#else
	devp->canvas_h = devp->v_active;
#endif
	devp->canvas_max_size = PAGE_ALIGN(canvas_max_w*canvas_max_h);
        devp->canvas_max_num  = devp->mem_size / devp->canvas_max_size;
	devp->canvas_max_num = min(devp->canvas_max_num,canvas_num);
        addr2int32(&devp->canvas_w);
        addr2int32(&devp->mem_start);
	#ifdef VDIN_DEBUG	
	pr_info("vdin.%d cnavas configuration table:\n", devp->index);
	#endif
	for (i = 0; i < devp->canvas_max_num; i++)
	{
		canvas_id = vdin_canvas_ids[devp->index][i];
		//canvas_addr = canvas_get_addr(canvas_id);
		/*reinitlize the canvas*/
		canvas_addr = devp->mem_start + devp->canvas_max_size * i;
		canvas_config(canvas_id, canvas_addr, devp->canvas_w, devp->canvas_h,
				CANVAS_ADDR_NOWRAP, CANVAS_BLKMODE_LINEAR);
		
		#ifdef VDIN_DEBUG
		pr_info("\t0x%2x: 0x%lx-0x%lx %ux%u\n",
				canvas_id, canvas_addr, canvas_addr + devp->canvas_max_size, devp->canvas_w, devp->canvas_h);
		#endif
	}

}
/*
*this function used for configure canvas base on the input format
*also used for input resalution over 1080p such as camera input 200M,500M
*/
void vdin_canvas_auto_config(struct vdin_dev_s *devp)
{
	int i = 0;
	int canvas_id;
	unsigned long canvas_addr;

	if ((devp->format_convert == VDIN_FORMAT_CONVERT_YUV_YUV444) ||
	    (devp->format_convert == VDIN_FORMAT_CONVERT_YUV_RGB   ) ||
	    (devp->format_convert == VDIN_FORMAT_CONVERT_RGB_YUV444) ||
	    (devp->format_convert == VDIN_FORMAT_CONVERT_RGB_RGB   ))
		devp->canvas_w = devp->h_active * 3;
        else if((devp->prop.dest_cfmt == TVIN_NV12) ||
                (devp->prop.dest_cfmt == TVIN_NV21))
                devp->canvas_w = devp->h_active;
	else
		devp->canvas_w = devp->h_active * 2;
        addr2int32(&devp->canvas_w);
	devp->canvas_h = devp->v_active;
	devp->canvas_max_size = PAGE_ALIGN(devp->canvas_w*devp->canvas_h);
	devp->canvas_max_num  = devp->mem_size / devp->canvas_max_size;
	if (devp->canvas_max_num > VDIN_CANVAS_MAX_CNT)
		devp->canvas_max_num = VDIN_CANVAS_MAX_CNT;
#ifdef CONFIG_ARCH_MESON6
        /*the max buffer in mid is 6*/
        if(devp->canvas_max_num > 6)
                devp->canvas_max_num = 6;
#endif
        addr2int32(&devp->mem_start);
	#ifdef VDIN_DEBUG
	pr_info("vdin%d cnavas auto configuration table:\n", devp->index);
	#endif
	for (i = 0; i < devp->canvas_max_num; i++)
	{
		canvas_id = vdin_canvas_ids[devp->index][i];
		canvas_addr = devp->mem_start + devp->canvas_max_size * i;
                canvas_config(canvas_id, canvas_addr, devp->canvas_w, devp->canvas_h,
				CANVAS_ADDR_NOWRAP, CANVAS_BLKMODE_LINEAR);

		#ifdef VDIN_DEBUG
		pr_info("\t%3d: 0x%lx-0x%lx %ux%u\n",
				canvas_id, canvas_addr, canvas_addr + devp->canvas_max_size,
				devp->canvas_w, devp->canvas_h);
		#endif
	}
}

