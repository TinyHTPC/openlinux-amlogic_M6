/*
 * Copyright (c) 2004-2012 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "core.h"
#include "debug.h"
#include "acs.h"
static void acs_flush_bss_info(struct acs *acs)
{
	struct acs_bss_info *acs_bss, *tmp;
	
	spin_lock(&acs->bss_info_lock);

	list_for_each_entry_safe(acs_bss, tmp, &acs->bss_info_list, list) {
		list_del(&acs_bss->list);
		kfree(acs_bss->raw_frame);
		kfree(acs_bss);
	}

	spin_unlock(&acs->bss_info_lock);

	return;
}

struct acs *ath6kl_acs_init(struct ath6kl_vif *vif)
{
	struct acs *acs;

	acs = kzalloc(sizeof(struct acs), GFP_KERNEL);
	if (!acs) {
		ath6kl_err("failed to alloc memory for acs\n");
		return NULL;
	}

	acs->vif = vif;
	acs->request.dev = vif->net_dev;
	acs->request.wiphy = vif->ar->wiphy;

	spin_lock_init(&acs->bss_info_lock);
	INIT_LIST_HEAD(&acs->bss_info_list);

	ath6kl_dbg(ATH6KL_DBG_ACS,
		   "htcoex init (vif %p)\n",
		   vif);

	return acs;
}

void ath6kl_acs_deinit(struct ath6kl_vif *vif)
{
	struct acs *acs = vif->acs_ctx;

	if (acs) {
		//if (acs->flags & ATH6KL_HTCOEX_FLAGS_START)
		//	del_timer(&acs->scan_timer);

		acs_flush_bss_info(acs);

		kfree(acs);
	}

	vif->acs_ctx = NULL;

	ath6kl_dbg(ATH6KL_DBG_ACS,
		   "acs deinit (vif %p)\n",
		   vif);

	return;
}

void ath6kl_acs_bss_info(struct ath6kl_vif *vif, 
							struct ieee80211_mgmt *mgmt, 
							int len,
							struct ieee80211_channel *channel,u8 snr)
{
	struct acs *acs = vif->acs_ctx;
	struct acs_bss_info *acs_bss, *tmp;
	int match = 0;

	/* Add bss even scan not issue by htcoex. */

	if (memcmp(mgmt->bssid, vif->bssid, ETH_ALEN) == 0)
		return;	

	
	ath6kl_dbg(ATH6KL_DBG_ACS,
		   "acs bssinfo (vif %p) BSSID %02x:%02x:%02x:%02x:%02x:%02x \n",
		   vif, 
		   mgmt->bssid[0], mgmt->bssid[1], mgmt->bssid[2],
		   mgmt->bssid[3], mgmt->bssid[4], mgmt->bssid[5]);

	if (!list_empty(&acs->bss_info_list)) {
		list_for_each_entry_safe(acs_bss, tmp, &acs->bss_info_list, list) {
			if (memcmp(acs_bss->bssid, mgmt->bssid, ETH_ALEN) == 0) {
				match = 1;
				break;
			}
		}
	}
	//printk("%s:%d,match=%d,snr=%d\n\r", __func__,__LINE__,match,snr);
	if (!match) {
		acs_bss = kzalloc(sizeof(struct acs_bss_info), GFP_KERNEL);
		if (!acs_bss) {
			ath6kl_err("failed to alloc memory for acs_bss\n");
			return;
		}

		acs_bss->raw_frame = kmalloc(len, GFP_KERNEL);
		if (!acs_bss->raw_frame) {
			kfree(acs_bss);
			ath6kl_err("failed to alloc memory for acs_bss->raw_frame\n");
			return;
		}

		/* Add BSS info. */
		acs_bss->frame_len = len;
		memcpy(acs_bss->raw_frame, mgmt, len);
		memcpy(acs_bss->bssid, mgmt->bssid, ETH_ALEN);
		acs_bss->channel = channel;
		acs_bss->snr = snr;
		spin_lock(&acs->bss_info_lock);
		//printk("%s:%d\n", __func__,__LINE__);
		list_add_tail(&acs_bss->list, &acs->bss_info_list);
		spin_unlock(&acs->bss_info_lock);
	}

	return;
}

int ath6kl_acs_scan_complete_event(struct ath6kl_vif *vif, bool aborted)
{
	struct acs *acs = vif->acs_ctx;
	int ret = ACS_PASS_SCAN_DONE;

	if (aborted == 1)
		return ret;
	/* Send Action frame even scan issue by user. */
	if (!vif->scan_req)
		goto done;

	ath6kl_dbg(ATH6KL_DBG_ACS,
		   "acs scan done (vif %p aborted %d) \n",
		   vif,
		   aborted);

done:
	/* don't update it if single channel scan. */
	if(vif->scan_band != ATHR_CMD_SCANBAND_CHAN_ONLY) {
		ath6kl_acs_find_info(vif);
	}
	/* Always flush it. */
	acs_flush_bss_info(acs);

	return ret;
}

/* 
 * Trace all entries to record the max rssi found for every channel.
 */
static int
ieee80211_acs_get_channel_maxrssi(void *arg, struct acs_bss_info *coex_bss)
{
    ieee80211_acs_t acs = (ieee80211_acs_t) arg;
    int i,j;
    struct ieee80211_channel *channel_se = coex_bss->channel;
    struct ieee80211_channel *channel;
    u_int8_t rssi_se = coex_bss->snr;
    u_int8_t channel_ieee_se = ieee80211_frequency_to_channel(channel_se->center_freq);

    for (i = 0; i < acs->acs_nchans; i++) {
        channel = acs->acs_chans[i];


        if (channel_ieee_se == ieee80211_frequency_to_channel(channel->center_freq)) {
            j = acs->acs_chan_maps[ieee80211_frequency_to_channel(channel->center_freq)];
            if (rssi_se > acs->acs_chan_maxrssi[j]) {
				    ath6kl_dbg(ATH6KL_DBG_ACS,"%s chan %d rssi from %d to %d \n", 
                      __func__, channel_ieee_se, acs->acs_chan_maxrssi[j],rssi_se);
                acs->acs_chan_maxrssi[j] = rssi_se;
            }
            break;
        }
    }

    return 0;
}
#if 0
/*
 * Check for channel interference.
 */
static int
ieee80211_acs_check_interference(struct ieee80211_channel *chan, struct ieee80211vap *vap)
{
    struct ieee80211com *ic = vap->iv_ic;

    /*
   * (1) skip static turbo channel as it will require STA to be in
   * static turbo to work.
   * (2) skip channel which's marked with radar detection
   * (3) WAR: we allow user to config not to use any DFS channel
   * (4) skip excluded 11D channels. See bug 31246 
   */
    if ( IEEE80211_IS_CHAN_STURBO(chan) || 
         IEEE80211_IS_CHAN_RADAR(chan) ||
         (IEEE80211_IS_CHAN_DFSFLAG(chan) && ieee80211_ic_block_dfschan_is_set(ic)) ||
         IEEE80211_IS_CHAN_11D_EXCLUDED(chan) ) {
        return (1);
    } else {
        return (0);
    }
}
#endif

/*
 * In 5 GHz, if the channel is unoccupied the max rssi
 * should be zero; just take it.Otherwise
 * track the channel with the lowest rssi and
 * use that when all channels appear occupied.
 */
static struct ieee80211_channel *
ieee80211_acs_find_best_11na_centerchan(struct ath6kl_vif *vif,ieee80211_acs_t acs)
{
    struct ieee80211_channel *channel;
    int best_center_chanix = -1, i;

    acs->acs_minrssi_11na = 0xffffffff;

    for (i = 0; i < acs->acs_nchans; i++) {
        channel = acs->acs_chans[i];
#if 0
        /* Check for channel interference. If found, skip the channel */
        if (ieee80211_acs_check_interference(channel, acs->acs_vap)) {
            continue;
        }
#endif

        if (ieee80211_is_support_band(vif->wdev->wiphy,channel->center_freq,IEEE80211_BAND_5GHZ)) {

            ath6kl_dbg(ATH6KL_DBG_ACS,
                "%s chan: %d maxrssi: %d\n", __func__, ieee80211_frequency_to_channel(channel->center_freq), acs->acs_chan_maxrssi[i]);

            /* look for max rssi in beacons found in this channel */
            if (acs->acs_chan_maxrssi[i] == 0) {
                acs->acs_minrssi_11na = 0;
                best_center_chanix = i;
                break;
            }

            if (acs->acs_chan_maxrssi[i] < acs->acs_minrssi_11na) {
                acs->acs_minrssi_11na = acs->acs_chan_maxrssi[i];
                best_center_chanix = i;
            }
        } else {
            /* Skip non-5GHZ channel */
            continue;
        }
    }

    if (best_center_chanix != -1) {
        channel = acs->acs_chans[best_center_chanix];
        ath6kl_dbg(ATH6KL_DBG_ACS,
            "%s found best 11na center chan: %d rssi: %d\n", 
            __func__, ieee80211_frequency_to_channel(channel->center_freq), acs->acs_minrssi_11na);
    } else {
		
		channel = __ieee80211_get_channel_ath6kl(vif->wdev->wiphy,4190);
        //channel = ieee80211_find_dot11_channel(acs->acs_ic, 0, acs->acs_vap->iv_des_mode);
        /* no suitable channel, should not happen */
        ath6kl_dbg(ATH6KL_DBG_ACS,
            "%s: no suitable channel! (should not happen)\n", __func__);
    }

    return channel;
}

static u_int32_t ieee80211_acs_find_average_rssi(ieee80211_acs_t acs, const int *chanlist, int chancount, u_int8_t centChan)
{
    u_int8_t chan;
    int i, j;
    u_int32_t average = 0;
    u_int32_t totalrssi = 0; /* total rssi for all channels so far */

    if (chancount <= 0) {
        /* return a large enough number for not to choose this channel */
        return 0xffffffff;
    }

    for (i = 0; i < chancount; i++) {
        chan = chanlist[i];

        j = acs->acs_chan_maps[chan];
        if (j != 0xff) { 
            totalrssi += acs->acs_chan_maxrssi[j];

            ath6kl_dbg(ATH6KL_DBG_ACS,
                "%s chan: %d maxrssi: %d\n", __func__, chan, acs->acs_chan_maxrssi[j]);
        }
    }
    
    average = totalrssi/chancount;
   
    ath6kl_dbg(ATH6KL_DBG_ACS,
       "Channel %d average beacon RSSI %d\n", 
       centChan, average);
    
    /* add the weighted noise floor */
    //average += acs->acs_noisefloor[centChan] * NF_WEIGHT_FACTOR;
//#if ATH_SUPPORT_SPECTRAL
    /* If channel loading is greater, add RSSI factor to the average_rssi for that channel */
//    average +=  ((acs->acs_channel_loading[centChan] * CHANLOAD_INCREASE_AVERAGE_RSSI) / 100);
//#endif    
    return (average);
}

//coded for dyn acs feature
static struct ieee80211_channel *
ieee80211_acs_find_best_11ng_channel(struct ath6kl_vif *vif,ieee80211_acs_t acs)
{
    struct ieee80211_channel *channel;
    int best_center_chanix = -1, i, j;
    u_int8_t chan;//band;
    u_int32_t avg_rssi;
    u_int chanlist[5];
    int count = 0;

    /*
   * The following center chan data structures are invented to allow calculating
   * the average rssi in 20Mhz band for a certain center chan. 
   *
   * We would then pick the band which has the minimum rsi of all the 20Mhz bands.
   */
#if 0
    /* For 20Mhz band with center chan 1, we would see beacons only on channels 1,2. */
    static const u_int center1[] = { 1, 2 };

    /* For 20Mhz band with center chan 6, we would see beacons on channels 4,5,6 & 7. */
    static const u_int center6[] = { 4, 5, 6, 7 };

    /* For 20Mhz band with center chan 11, we would see beacons on channels 9,10 & 11. */
    static const u_int center11[] = { 9, 10, 11 };
#endif
    struct centerchan {
        int count;               /* number of chans to average the rssi */
        const u_int *chanlist;   /* the possible beacon channels around center chan */
    };

//#define X(a)    sizeof(a)/sizeof(a[0]), a

    /*struct centerchan centerchans[] = {
        { X(center1) },
        { X(center6) },
        { X(center11) }
    };*/

    acs->acs_avgrssi_11ng = 0xffffffff;

    //printk("%s: acs_nchans = %d \n",__func__,acs->acs_nchans);
    for (i = 0; i < acs->acs_nchans; i++) {
        channel = acs->acs_chans[i];
        chan = ieee80211_frequency_to_channel(channel->center_freq);

        if(chan < 1 || chan > 11)
            continue;

        count = 0;
        chanlist[count++] = chan;

        for(j=1;j<=2;j++) {
            if(chan -j >= 1)
                chanlist[count++] = chan - j;
            if(chan +j <= 11)
                chanlist[count++] = chan + j;
        }

        /*printk("%s: count = %d, center channels for %d: (",__func__,count,chan);

        for(j=0;j<count;j++) {
            printk("%d,",chanlist[j]);
        }
        printk(")");*/

        /* find the average rssi for this 20Mhz band */
        avg_rssi = ieee80211_acs_find_average_rssi(acs, chanlist, count, chan);

        ath6kl_dbg(ATH6KL_DBG_ACS,
            "%s chan: %d beacon RSSI + weighted noisefloor: %d\n", __func__, chan, avg_rssi);

        if (avg_rssi == 0) {
            acs->acs_avgrssi_11ng = avg_rssi;
            best_center_chanix = i;
            break;
        }

        if (avg_rssi < acs->acs_avgrssi_11ng) {
            acs->acs_avgrssi_11ng = avg_rssi;
            best_center_chanix = i;
        }
    }
    if (best_center_chanix != -1) {
        channel = acs->acs_chans[best_center_chanix];
        ath6kl_dbg(ATH6KL_DBG_ACS,
            "%s found best 11ng center chan: %d rssi: %d\n",
            __func__, ieee80211_frequency_to_channel(channel->center_freq), acs->acs_avgrssi_11ng);

    } else {
		channel = __ieee80211_get_channel_ath6kl(vif->wdev->wiphy,2412);
        //channel = ieee80211_find_dot11_channel(acs->acs_ic, 0, acs->acs_vap->iv_des_mode);
        /* no suitable channel, should not happen */
        ath6kl_dbg(ATH6KL_DBG_ACS,
            "%s: no suitable channel! (should not happen)\n", __func__);
    }

    return channel;
}

int ath6kl_acs_find_info(struct ath6kl_vif *vif) 
{
	struct acs *acs = vif->acs_ctx;
	struct acs_bss_info *acs_bss, *tmp;
	ieee80211_acs_t temp_acs;
	struct ieee80211_channel *best_11na = NULL;
    struct ieee80211_channel *best_11ng = NULL;	
	struct ieee80211_channel *best_overall = NULL;
	int retv = 0;

	temp_acs = (ieee80211_acs_t) kmalloc(sizeof(struct ieee80211_acs), GFP_ATOMIC);	
    if (temp_acs) {
        memset(temp_acs, 0x00,sizeof(struct ieee80211_acs));
    } else {
       ath6kl_dbg(ATH6KL_DBG_ACS,"%s: malloc failed \n", __func__);
       return -1;
    }	
	
	temp_acs->acs_nchans = 0;
	
	ieee80211_acs_construct_chan_list(vif->wdev->wiphy,temp_acs);
	
	/* find max rssi for each channel */
	if (!list_empty(&acs->bss_info_list)){
		list_for_each_entry_safe(acs_bss, tmp, &acs->bss_info_list, list) {	

			ieee80211_acs_get_channel_maxrssi(temp_acs, acs_bss);
		}
	}

    best_11na = ieee80211_acs_find_best_11na_centerchan(vif,temp_acs);
    best_11ng = ieee80211_acs_find_best_11ng_channel(vif,temp_acs);	

    if (temp_acs->acs_minrssi_11na > temp_acs->acs_avgrssi_11ng) {
        best_overall = best_11ng;
    } else {
        best_overall = best_11na;
    }
    if ((best_11na == NULL && best_11ng == NULL) || best_overall == NULL) {
        ath6kl_dbg(ATH6KL_DBG_ACS,"%s: null channel \n", __func__);
        retv = -1;
        goto err;
    }
    if (best_11na == NULL)
        vif->best_chan[0] = 5200;
    else
	    vif->best_chan[0] = (int) best_11na->center_freq;
    if (best_11ng == NULL)
        vif->best_chan[1] = 2412;
    else
	    vif->best_chan[1] = (int) best_11ng->center_freq;
	vif->best_chan[2] = (int) best_overall->center_freq;
  
    ath6kl_dbg(ATH6KL_DBG_ACS,"%s:** 11na = %d,11ng = %d, Overall best "
                      "freq = %d", __func__, vif->best_chan[0],vif->best_chan[1], vif->best_chan[2]);	
err:	
	kfree(temp_acs);
	return retv;
}
void ath6kl_acs_get_info(struct ath6kl_vif *vif,int *best_chan) 
{
	best_chan[0] = vif->best_chan[0];
	best_chan[1] = vif->best_chan[1];
	best_chan[2] = vif->best_chan[2];

	printk("%s:** ", __func__);
	if(ieee80211_is_support_band(vif->wdev->wiphy, best_chan[0], IEEE80211_BAND_5GHZ)) 
		printk("11na = %d, ", best_chan[0]);
	else
		printk("11na = N/A, ");
	if(ieee80211_is_support_band(vif->wdev->wiphy, best_chan[1], IEEE80211_BAND_2GHZ)) 
		printk("11ng = %d, ", best_chan[1]);
	else
		printk("11ng = N/A, ");
    printk("Overall best freq = %d\n", best_chan[2]);
	
	return;
}
