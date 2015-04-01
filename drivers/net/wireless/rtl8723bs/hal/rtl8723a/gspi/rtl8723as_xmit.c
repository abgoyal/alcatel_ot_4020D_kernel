/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#define _RTL8723AS_XMIT_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <gspi_ops.h>
#include <rtl8723a_hal.h>

/*
 * Description
 *	Transmit xmitbuf to hardware tx fifo
 *
 * Return
 *	_SUCCESS	ok
 *	_FAIL		something error
 */
s32 rtl8723as_xmit_buf_handler(PADAPTER padapter)
{
	PHAL_DATA_TYPE phal;
	struct mlme_priv *pmlmepriv;
	struct xmit_priv *pxmitpriv;
	struct dvobj_priv *pdvobjpriv;
	struct xmit_buf *pxmitbuf;
	struct xmit_frame *pframe;
	u8 *freePage;
	u32 deviceId;
	u32 requiredPage;
	u8 PageIdx;
	_irqL irql;
	u32 n;
	s32 ret;

	_queue *pqueue = &padapter->xmitpriv.pending_xmitbuf_queue;

	phal = GET_HAL_DATA(padapter);
	pmlmepriv = &padapter->mlmepriv;
	pxmitpriv = &padapter->xmitpriv;
	pdvobjpriv = adapter_to_dvobj(padapter);
	freePage = phal->SdioTxFIFOFreePage;

	ret = _rtw_down_sema(&pxmitpriv->xmit_sema);
	if (_FAIL == ret) {
		RT_TRACE(_module_hal_xmit_c_, _drv_emerg_,
				 ("%s: down SdioXmitBufSema fail!\n", __FUNCTION__));
		return _FAIL;
	}

	ret = (padapter->bDriverStopped == _TRUE) || (padapter->bSurpriseRemoved == _TRUE);
	if (ret) {
		RT_TRACE(_module_hal_xmit_c_, _drv_err_,
				 ("%s: bDriverStopped(%d) bSurpriseRemoved(%d)!\n",
				  __FUNCTION__, padapter->bDriverStopped, padapter->bSurpriseRemoved));
		return _FAIL;
	}

#ifdef CONFIG_LPS_LCLK
	if(_rtw_queue_empty(pqueue)) {
		//DBG_871X("+%s: pending_xmitbuf_queue is Empty!\n", __func__);
		return _SUCCESS;
	}

	ret = rtw_register_tx_alive(padapter);
	if (ret != _SUCCESS) {
		return _SUCCESS;
	}
#endif

	do {
		ret = check_fwstate(pmlmepriv, _FW_UNDER_SURVEY);
		if (_TRUE == ret)
			pxmitbuf = dequeue_pending_xmitbuf_under_survey(pxmitpriv);
		else
			pxmitbuf = dequeue_pending_xmitbuf(pxmitpriv);
		if (pxmitbuf == NULL) break;
		pframe = (struct xmit_frame*)pxmitbuf->priv_data;
		requiredPage = pframe->pg_num;

		//translate tx queue index to sdio fifo addr
		deviceId = pdvobjpriv->Queue2Pipe[pxmitbuf->ff_hwaddr];

		//translate sdio fifo addr to tx fifo page index
		switch (deviceId)
		{
			case WLAN_TX_HIQ_DEVICE_ID:
					PageIdx = HI_QUEUE_IDX;
					break;

			case WLAN_TX_MIQ_DEVICE_ID:
					PageIdx = MID_QUEUE_IDX;
					break;

			case WLAN_TX_LOQ_DEVICE_ID:
					PageIdx = LOW_QUEUE_IDX;
					break;
		}

		// check if hardware tx fifo page is enough
		n = 0;
//		_enter_critical_bh(&phal->SdioTxFIFOFreePageLock, &irql);
		do {
			if (requiredPage <= freePage[PageIdx]) {
				freePage[PageIdx] -= requiredPage;
				break;
			}

			// The number of page which public page included is available.
			if ((freePage[PageIdx] + freePage[PUBLIC_QUEUE_IDX]) > (requiredPage + 1))
			{
				u8 requiredPublicPage;

				requiredPublicPage = requiredPage - freePage[PageIdx];
				freePage[PageIdx] = 0;
				freePage[PUBLIC_QUEUE_IDX] -= requiredPublicPage;
				break;
			}
//			_exit_critical_bh(&phal->SdioTxFIFOFreePageLock, &irql);

			ret = (padapter->bDriverStopped == _TRUE) || (padapter->bSurpriseRemoved == _TRUE);
			if (ret) {
				RT_TRACE(_module_hal_xmit_c_, _drv_notice_,
						 ("%s: updating TX FIFO page, bDriverStopped(%d) bSurpriseRemoved(%d)!\n",
						  __FUNCTION__, padapter->bDriverStopped, padapter->bSurpriseRemoved));
				goto free_xmitbuf;
			}

			n++;
			if ((n & 0xFF) == 0) {
				if (n > 5000) {
					DBG_8192C(KERN_NOTICE "%s: FIFO starvation!(%d) len=%d agg=%d page=(R)%d(A)%d\n",
						__func__, n, pxmitbuf->len, pframe->agg_num, pframe->pg_num, freePage[PageIdx] + freePage[PUBLIC_QUEUE_IDX]);
				} else {
					RT_TRACE(_module_hal_xmit_c_, _drv_notice_,
						("%s: FIFO starvation!(%d) len=%d agg=%d page=(R)%d(A)%d\n",
						__FUNCTION__, n, pxmitbuf->len, pframe->agg_num, pframe->pg_num, freePage[PageIdx] + freePage[PUBLIC_QUEUE_IDX]));
				}
				rtw_yield_os();
			}

			// Total number of page is NOT available, so update current FIFO status
			HalQueryTxBufferStatus8723ASdio(padapter);

//			_enter_critical_bh(&phal->SdioTxFIFOFreePageLock, &irql);
		} while (1);
//		_exit_critical_bh(&phal->SdioTxFIFOFreePageLock, &irql);

		if (padapter->bSurpriseRemoved == _TRUE) {
			RT_TRACE(_module_hal_xmit_c_, _drv_err_,
				 ("%s: bSurpriseRemoved(wirte port)\n", __FUNCTION__));
			goto free_xmitbuf;
		}
		rtw_write_port(padapter, deviceId, pxmitbuf->len, pxmitbuf->pdata);

free_xmitbuf:
		rtw_free_xmitframe(pxmitpriv, pframe);
		pxmitbuf->priv_data = NULL;
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
	} while (1);

#ifdef CONFIG_LPS_LCLK
	rtw_unregister_tx_alive(padapter);
#endif

	return _SUCCESS;
}


/*
 * Description:
 *	Aggregation packets and send to hardware
 *
 * Return:
 *	0	Success
 *	-1	Hardware resource(TX FIFO) not ready
 *	-2	Software resource(xmitbuf) not ready
 */
static s32 xmit_xmitframes(PADAPTER padapter, struct xmit_priv *pxmitpriv)
{
	s32 err, ret;
	u32 k;
	struct hw_xmit *hwxmits;
	u8 no_res, idx, hwentry;
	_irqL irql;
	struct tx_servq *ptxservq;
	_list *sta_plist, *sta_phead, *frame_plist, *frame_phead, *tmp_sta_list;
	struct xmit_frame *pxmitframe;
	_queue *pframe_queue;
	struct xmit_buf *pxmitbuf;
	u32 txlen;


	err = 0;
	no_res = _FALSE;
	hwxmits = pxmitpriv->hwxmits;
	hwentry = pxmitpriv->hwxmit_entry;
	ptxservq = NULL;
	pxmitframe = NULL;
	pframe_queue = NULL;
	pxmitbuf = NULL;

	// 0(VO), 1(VI), 2(BE), 3(BK)
	for (idx = 0; idx < hwentry; idx++, hwxmits++)
	{
		//because stop_sta_xmit may delete sta_plist at any time
		//so we should add lock here, or while loop can not exit
		_enter_critical_bh(&pxmitpriv->lock, &irql);

		sta_phead = get_list_head(hwxmits->sta_queue);
		sta_plist = get_next(sta_phead);
		while (rtw_end_of_queue_search(sta_phead, sta_plist) == _FALSE)
		{
			ptxservq = LIST_CONTAINOR(sta_plist, struct tx_servq, tx_pending);
			tmp_sta_list = sta_plist;
			sta_plist = get_next(sta_plist);
			if(tmp_sta_list == sta_plist)
			{
				DBG_871X_LEVEL(_drv_err_, "%s: ERROR!sta_plist is equal to its next!\n", __FUNCTION__);
				break;
			}

			pframe_queue = &ptxservq->sta_pending;

			frame_phead = get_list_head(pframe_queue);

			while (rtw_is_list_empty(frame_phead) == _FALSE)
			{
				frame_plist = get_next(frame_phead);
				pxmitframe = LIST_CONTAINOR(frame_plist, struct xmit_frame, list);

				// check xmit_buf size enough or not
				txlen = TXDESC_SIZE + rtw_wlan_pkt_size(pxmitframe);
				if ((NULL == pxmitbuf) ||
					((pxmitbuf->ptail + txlen) > pxmitbuf->pend)
#ifdef SDIO_TX_AGG_MAX
					|| (k >= SDIO_TX_AGG_MAX)
#endif
					)
				{
					if (pxmitbuf) {
						//pxmitbuf->priv_data will be NULL, and will crash here
						if (pxmitbuf->len > 0 && pxmitbuf->priv_data) {
						        struct xmit_frame *pframe;
						        pframe = (struct xmit_frame*)pxmitbuf->priv_data;
						        pframe->agg_num = k;
						        rtl8723a_update_txdesc(pframe, pframe->buf_addr);
						        enqueue_pending_xmitbuf(pxmitpriv, pxmitbuf);
							//can not yield under lock
						        //rtw_yield_os();
						} else {
						        rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
						}
					}

					pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
					if (pxmitbuf == NULL) {
						RT_TRACE(_module_hal_xmit_c_, _drv_err_, ("%s: xmit_buf is not enough!\n", __FUNCTION__));
						err = -2;
						break;
					}
					k = 0;
				}

				// ok to send, remove frame from queue
#ifdef CONFIG_AP_MODE
				if (check_fwstate(&padapter->mlmepriv, WIFI_AP_STATE) == _TRUE)
				{
					if ((pxmitframe->attrib.psta->state & WIFI_SLEEP_STATE) &&
						(pxmitframe->attrib.triggered == 0))
					{
						DBG_8192C("%s: one not triggered pkt in queue when STA sleep\n", __func__);
						break;
					}
				}
#endif
				rtw_list_delete(&pxmitframe->list);
				ptxservq->qcnt--;
				hwxmits->accnt--;

				if (k == 0) {
					pxmitbuf->ff_hwaddr = rtw_get_ff_hwaddr(pxmitframe);
					pxmitbuf->priv_data = (u8*)pxmitframe;
				}

				// coalesce the xmitframe to xmitbuf
				pxmitframe->pxmitbuf = pxmitbuf;
				pxmitframe->buf_addr = pxmitbuf->ptail;

				ret = rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
				if (ret == _FAIL) {
					RT_TRACE(_module_hal_xmit_c_, _drv_err_, ("%s: coalesce FAIL!", __FUNCTION__));
					// Todo: error handler
				} else {
					k++;
					if (k != 1)
						rtl8723a_update_txdesc(pxmitframe, pxmitframe->buf_addr);
					rtw_count_tx_stats(padapter, pxmitframe, pxmitframe->attrib.last_txcmdsz);

					txlen = TXDESC_SIZE + pxmitframe->attrib.last_txcmdsz;
					pxmitframe->pg_num = (txlen + 127)/128;
					if (k != 1)
						((struct xmit_frame*)pxmitbuf->priv_data)->pg_num += pxmitframe->pg_num;
					pxmitbuf->ptail += _RND(txlen, 8); // round to 8 bytes alignment
					pxmitbuf->len = _RND(pxmitbuf->len, 8) + txlen;
				}

				if (k != 1)
					rtw_free_xmitframe(pxmitpriv, pxmitframe);
				pxmitframe = NULL;
			}

			if (_rtw_queue_empty(pframe_queue) == _TRUE)
				rtw_list_delete(&ptxservq->tx_pending);

			if (err) break;
		}
		_exit_critical_bh(&pxmitpriv->lock, &irql);

		// dump xmit_buf to hw tx fifo
		if (pxmitbuf)
		{
			RT_TRACE(_module_hal_xmit_c_, _drv_notice_, ("pxmitbuf->len=%d enqueue\n",pxmitbuf->len));

			if (pxmitbuf->len > 0) {
				struct xmit_frame *pframe;
				pframe = (struct xmit_frame*)pxmitbuf->priv_data;
				pframe->agg_num = k;
				rtl8723a_update_txdesc(pframe, pframe->buf_addr);
				enqueue_pending_xmitbuf(pxmitpriv, pxmitbuf);
				rtw_yield_os();
			}
			else
				rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
			pxmitbuf = NULL;
		}

		if (err) break;
	}

	return err;
}

/*
 * Description
 *	Transmit xmitframe from queue
 *
 * Return
 *	_SUCCESS	ok
 *	_FAIL		something error
 */
s32 rtl8723as_xmit_handler(PADAPTER padapter)
{
	struct xmit_priv *pxmitpriv;
	PHAL_DATA_TYPE phal;
	s32 ret;
	_irqL irql;


	pxmitpriv = &padapter->xmitpriv;
	phal = GET_HAL_DATA(padapter);

wait:
	ret = _rtw_down_sema(&pxmitpriv->SdioXmitSema);
	if (_FAIL == ret) {
		RT_TRACE(_module_hal_xmit_c_, _drv_emerg_, ("%s: down sema fail!\n", __FUNCTION__));
		return _FAIL;
	}

next:
	if ((padapter->bDriverStopped == _TRUE) ||
		(padapter->bSurpriseRemoved == _TRUE)) {
		RT_TRACE(_module_hal_xmit_c_, _drv_notice_,
				 ("%s: bDriverStopped(%d) bSurpriseRemoved(%d)\n",
				  __FUNCTION__, padapter->bDriverStopped, padapter->bSurpriseRemoved));
		return _FAIL;
	}

	_enter_critical_bh(&pxmitpriv->lock, &irql);
	ret = rtw_txframes_pending(padapter);
	_exit_critical_bh(&pxmitpriv->lock, &irql);
	if (ret == 0) {
		return _SUCCESS;
	}

	// dequeue frame and write to hardware

	ret = xmit_xmitframes(padapter, pxmitpriv);
	if (ret == -2) {
		//here sleep 1ms will cause big TP loss of TX
		//from 50+ to 40+
		//rtw_msleep_os(1);
		rtw_yield_os();
		goto next;
	}

	_enter_critical_bh(&pxmitpriv->lock, &irql);
	ret = rtw_txframes_pending(padapter);
	_exit_critical_bh(&pxmitpriv->lock, &irql);
	if (ret == 1) {
		//rtw_msleep_os(1);
		goto next;
	}

	return _SUCCESS;
}


thread_return rtl8723as_xmit_thread(thread_context context)
{
	s32 ret;
	PADAPTER padapter;
	struct xmit_priv *pxmitpriv;

	ret = _SUCCESS;
	padapter = (PADAPTER)context;
	pxmitpriv = &padapter->xmitpriv;

	thread_enter("RTWHALXT");

	DBG_871X("start %s\n", __FUNCTION__);

	_rtw_up_sema(&pxmitpriv->SdioXmitTerminateSema);

	do {
		ret = rtl8723as_xmit_handler(padapter);
		if (signal_pending(current)) {
			flush_signals(current);
		}
	} while (_SUCCESS == ret);

	_rtw_up_sema(&pxmitpriv->SdioXmitTerminateSema);

	RT_TRACE(_module_hal_xmit_c_, _drv_notice_, ("-%s\n", __FUNCTION__));

	thread_exit();
}

s32 rtl8723as_mgnt_xmit(PADAPTER padapter, struct xmit_frame *pmgntframe)
{
	s32 ret = _SUCCESS;
	struct pkt_attrib *pattrib;
	struct xmit_buf *pxmitbuf;


	RT_TRACE(_module_hal_xmit_c_, _drv_info_, ("+%s\n", __FUNCTION__));

	pattrib = &pmgntframe->attrib;
	pxmitbuf = pmgntframe->pxmitbuf;

	rtl8723a_update_txdesc(pmgntframe, pmgntframe->buf_addr);

	pxmitbuf->len = TXDESC_SIZE + pattrib->last_txcmdsz;
	pmgntframe->pg_num = (pxmitbuf->len + 127)/128; // 128 is tx page size
	pxmitbuf->ptail = pmgntframe->buf_addr + pxmitbuf->len;
	pxmitbuf->ff_hwaddr = rtw_get_ff_hwaddr(pmgntframe);

	enqueue_pending_xmitbuf(&padapter->xmitpriv, pxmitbuf);

	rtw_count_tx_stats(padapter, pmgntframe, pattrib->last_txcmdsz);

	if  (ret != _SUCCESS)
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_UNKNOWN);

	return ret;
}

/*
 * Description:
 *	Handle xmitframe(packet) come from rtw_xmit()
 *
 * Return:
 *	_TRUE	dump packet directly ok
 *	_FALSE	enqueue, temporary can't transmit packets to hardware
 */
s32 rtl8723as_hal_xmit(PADAPTER padapter, struct xmit_frame *pxmitframe)
{
	struct xmit_priv *pxmitpriv;
	PHAL_DATA_TYPE phal;
	_irqL irql;
	s32 err;


	pxmitframe->attrib.qsel = pxmitframe->attrib.priority;
	pxmitpriv = &padapter->xmitpriv;
	phal = GET_HAL_DATA(padapter);

#ifdef CONFIG_80211N_HT
	if ((pxmitframe->frame_tag == DATA_FRAMETAG) &&
		(pxmitframe->attrib.ether_type != 0x0806) &&
		(pxmitframe->attrib.ether_type != 0x888e) &&
		(pxmitframe->attrib.dhcp_pkt != 1))
	{
		if (padapter->mlmepriv.LinkDetectInfo.bBusyTraffic == _TRUE)
			rtw_issue_addbareq_cmd(padapter, pxmitframe);
	}
#endif

	_enter_critical_bh(&pxmitpriv->lock, &irql);
	err = rtw_xmitframe_enqueue(padapter, pxmitframe);
	_exit_critical_bh(&pxmitpriv->lock, &irql);
	if (err != _SUCCESS) {
		RT_TRACE(_module_hal_xmit_c_, _drv_err_, ("rtl8723as_hal_xmit: enqueue xmitframe fail\n"));
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		// Trick, make the statistics correct
		pxmitpriv->tx_pkts--;
		pxmitpriv->tx_drop++;
		return _TRUE;
	}

	_rtw_up_sema(&pxmitpriv->SdioXmitSema);

	return _FALSE;
}

/*
 * Return
 *	_SUCCESS	start thread ok
 *	_FAIL		start thread fail
 *
 */
s32 rtl8723as_init_xmit_priv(PADAPTER padapter)
{
	PHAL_DATA_TYPE phal;
	struct xmit_priv *pxmitpriv;

	pxmitpriv = &padapter->xmitpriv;
	phal = GET_HAL_DATA(padapter);

	_rtw_spinlock_init(&phal->SdioTxFIFOFreePageLock);
	_rtw_init_sema(&pxmitpriv->SdioXmitSema, 0);
	_rtw_init_sema(&pxmitpriv->SdioXmitTerminateSema, 0);

	return _SUCCESS;
}

void rtl8723as_free_xmit_priv(PADAPTER padapter)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);

	_rtw_spinlock_free(&pHalData->SdioTxFIFOFreePageLock);
}

