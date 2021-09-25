/******************************************************************************
*
*  COPYRIGHT LeadCore Technology CO.,LTD
*
******************************************************************************/

/******************************************************************************
*
* FILE
*
*    <l1cc_gsm_fcp.c>
*
* DESCRIPTION
*
*    <describing what this file is to do>
*
* NOTE
*
*    <the limitations to use this file>
*
******************************************************************************/


/*********************************************************************************************************************************************************
*  HISTORY OF CHANGES
**********************************************************************************************************************************************************
*
*   <date>          <author>                <CR_ID>             <DESCRIPTION>
*   2010-01-05      guxiaobo                L2000-Bug00000734   L2000 : fix 0x207a7 for lack of MIPS at certain cases  
*   2010-01-12      liujiaheng              L2000-Bug00000657   Reduce AB sending times from 5 to 4 when handover from 3G to 2G in CS call.
*   2010-01-12      liujiaheng              L2000-Bug00000687   Delete all frame task and current executive task to avoid DMA execption under TDD mode when mode change from GSM to TDD.
*   2010-01-12      ningyaojun              A2KPH-Enh00000876   NACC/PCCO: L1 replys MPAL_RMPC_RACH_CONF(status = 0:FAIL) to HLS instead of call oal_error_handler() to step into exception,
                                                                           because too early an improper RACH to the new cell may be initiated by HLS while L1 is still in synchronization with the old cell,
                                                                           when NACC/PCCO provokes an cell-reselect to a new cell directly from packet transfer state.
*   2010-01-13      guxiaobo                L2000-Enh00000325   modify last symbol of 3rd timeslot(3rd ts has 157symbols) from 0 to 1, concerned with PVT
*   2010-01-08      wanghuan                PHYFA-Req00000481   output ATC&AFC information
*   2010-01-21      wanghuan                L2000-Enh00000359   modify the bug of L2000-Bug00000734 for 0x20a79
*   2010-02-07      wuxiaorong              L8132-Bug00000641   modify gsm_fcp_cell_reselect for handover.
*   2010-02-26      kehaidong               PhyFA-Req00000510   L2000: test code, PL not sending RACH_CONF to HLS as there's no _RACH_CONF msg in the TT LOG.
*   2010-03-12      kehaidong               L2000-Bug00000991   L2000:  Put the initialization of NC MEAS and 3G MEAS in front of State Changing() to avoid 0X10531 dump due to time of one frame exhausted before 3G MEAS task configration by some precedue, such as memory reallocation, deleting frame task and event table 
*   2010-03-20      guxiaobo                L2000-Bug00001023   fix the error when PDTCH_RX task is omitted at certain cases.
*   2010-03-22      ningyaojun              A2KPH-Bug00003516   Priority&Conflic:A new tsntask with higher priority cannot be configured due to its conflict with a earlier tsntak that has been download to OAL;
                                                                                 Prblm: this may results in SCELL BCCH lost(Shanxi late SI2quter  in Transfer prblm), or BCCH and PDTCH collision Dump(Shenyang ID_2009-11-09_456), or Single Block failure(Beijing [ID_2010-03-16_689] Single Block Dump);
                                                                                 Solution: L1cc should find out the conflict task to delete it if necessary, event it has been downloaded to OAL HWIF; and RRBP priority is degraded lower than BCCH priority(GSM_TSNTASK_COMM_CH_PRIOR).
*   2010-03-25      liujiaheng              PHYFA-Req00000422   Requirement of dynamic control accelerator, TH,LC3,LC2,RELATE.
*   2010-04-01      wuxiaorong              L8132-Bug00000760   add TSNTASK_BG_BSIC_RX(BG TASKS and NC_MEAS TASKS) to g_u16_del_tstask_permitted[].
*   2010-04-03      sunzhiqiang             PhyFA-Bug00001843   ccch type is reorg,if BCCH type didn't include E_BCCH,the block of B(6-9) is not received
*   2010-03-22      ningyaojun              A2KPH-Bug00003516   Incremental modification of A2KPH-Bug00003516.
*   2010-04-07      wuxiaorong              A2KPH-Enh00001024   do bsic recofirming when handover from TD to GSM..
*   2010-04-19      guxiaobo                L2000-Bug00001070   optimize codes to avoid 0x10509
*   2010-04-20      gaochunsheng            AM-Bug00000002      去掉可删除时隙任务表g_u16_del_tstask_permitted[]，仅根据任务优先级进行冲突判决. 
*   2010-04-26      liujiaheng              PhyFA-Enh00000670   Do some code enhancement.
*   2010-04-27      ningyaojun              A2KPH-Bug00003516   Incremental modification for LG metro problems 0x20AC1,0X20A1B in Beijing metro:
                                                                                         (1) Measurement report expiry due to PCH lost, because TSNTASK_GSM_RX_TD_11DELTA_128CHIP is too long to be deleted;
                                                                                         (2) A pseduo that is pre-set for PDTCH_TX should be allowed to be replaced by other tasks(such as BCCH_RX) with higher priority.
                                                                                         (3) The failure procedure is perfected when oal_rtx_task_del() returns a FAILURE to delete a downloaded event.
*   2010-04-28      sunzhiqiang             AM-Enh00000006      frefr codec should be changed after MS received PI.
*   2010-04-28      wuxiaorong              A2KPH-Bug00002653   Optimize search RF strategy in nocell state with big-little alternant searching period. 
*   2010-04-29      wanghuan                1708A-Enh00000065   1708A: test code, fcp task is runned for too long time.
*   2010-04-29      wanghuan                1708A-Enh00000067   1708A: 1708A-PHASE1 can not support EDGE,when receiving edge tbf connecting req,should return "connect fail".
*   2010-05-01      wuxiaorong              AM-Bug00000023      pseudo task sholud be deleted( in gsm_fcp_del_rtx_conflict),if conflict with other tasks. 
*   2010-05-07      guxiaobo                L2000-Bug00001296   L2000: Fix 0x720 and 0x10509. Delay one frame when timing adjustment takes effect next next fn
*   2010-05-10      wuxiaorong              AM-Bug00000031      maintian event counter and clear set flag  when TD cs task deleted,
*   2010-05-13      guxiaobo                L2000-Enh00000694   L2000: incorrectly cell reselection process when timing adjustment is 0Qbit
*   2010-05-18      guxiaobo                L2000-Enh00000694   verify fail, rework 
*   2010-05-20      wanghuan                L2000-Enh00000667   L2000: the adjustment of agc in transfer mode should be optimized for some special situations.
*   2010-05-21      songzhiyuan             L2000-Bug00001392   fix the bug of blind handover that after blind handover fail,time info is wrong
*   2010-05-20      sunzhiqiang             AM-Enh00000025      按照邻区同步结果进行频偏软校准，重选/切换时对目标小区进行频偏校准。
*   2010-05-24      wuxiaorong              AM-Bug00000012      delete g_stp_nc_meas_info.
*   2010-05-27      guxiaobo                PHYFA-Enh00000707   消除L1CC GSM中使用PC LINT产生的数组越界Warning
*   2010-05-28      guxiaobo                L2000-Enh00000728   AGC optimization
*   2010-06-02      wuxiaorong              AM-Bug00000068      set reporting state (ENTER_NEW_PERIOD) when start nc meas.
*   2010-06-03      kehaidong               PhyFA-Req00000630   L2000: Add test code to detect 3G measurement error of no 3G measurement report.
*   2010-06-04      wanghuan                1708A-Bug00000379   USF decoding is not needed for every slots in  EXT_DYNAMIC_MODE  mac mode,which can be helpful for saving mips.
*   2010-06-07      wanghuan                1708A-Bug00000436   1708A:the g_u16_gsm_enc_parmsBuffer which was used in the last tch_enc should be cleared.
*   2010-06-17      fushilong               AM_Enh00000057      when handover sync process fail from 3g to 2g ,L1 enter GSM_BGS_HANDOVER_SYNC_FAIL state.
*   2010-06-18      guxiaobo                PHYFA-Enh00000737   modify agc address of GPRS interference measurement
*   2010-06-28      wuxiaorong              AM-Bug00000109      modify gsm_fcp_check_rtx_conflict() &gsm_fcp_seek_idle_window_in_traffic_frame().
*   2010-06-29      songzhiyuan             PhyFA-Enh00000748   L平台: 限制定时调整量 
*   2010-06-22      wanghuan                PHYFA-Enh00000742   initialize the local variable before it is used and clear up the warnings produced by pc lint. 
*   2010-07-08      songzhiyuan             PhyFa-Enh00000728   after handover,inherit different band timeinfo.
*   2010-07-12      wuxiaorong              AM-Enh00000083      CC_DEC/EQ del tsntasks by sending msg to FCP,do not try to pre-decode PCH in TRANSFER.
*   2010-07-13      guxiaobo                LM-Bug00000016      modify rx_bitmap when TBF mode is EXT_DYNAMIC_MODE
*   2010-07-16      wanghuan                AM-Bug00000151      ccc_rx in transfer mode may delet the rrbp_tx,so the priority of the task of ccc_rx should be knocked down for
                                                                               better PS service
*   2010-07-21      dujianzhong             LM-Enh00000014      add test code to record time used for memory initialization when state changed   
*   2010-07-28      wanghuan                LM-Bug00000041      add test code to record time used for memory initialization when state changing for ULC_YULONG  
*   2010-07-28      songzhiyuan             LM-Bug00000036      when starting time in handover command is vaild,the first AB transmission time is later than order time.  
*   2010-07-31      songzhiyuan             PHYFA-Enh00000770   clear up the warnings of divide zero or mod zero produced by pc lint. 
*   2010-08-04      liujiaheng              PhyFA-Enh00000767   Use target arfcn to adjust frequency correction when handover or reselect.
*   2010-08-09      guxiaobo                LM-Enh00000026      reset rf register when detect rx data is abnormal.
*   2010-08-15      wuxiaorong              AM-Enh00000066      Optimize search RF strategy in nocell state for saving power reason.
*   2010-08-30      songzhiyuan             LM-Bug00000098      FACCH_H的第一次发送可能会少发2个或4个burst
*   2010-08-28      liujiaheng              PHYFA-Enh00000795   Power optimization in IDLE state. 
*   2010-08-28      liujiaheng              LM-Bug000000100     Fix 0x1050a execption use power optimize vesrion. 
*   2010-09-10      songzhiyuan             LM-Bug00000104      TTCN 8.3.11.12 FAIL FCB搜索成功后，如果定时调整值是负值且定时调整生效帧号是下下帧，配置的下下帧的SB接收的时间点就是错误的 
*   2010-09-14      wanghuan                LM-Bug00000130      agc optimization for CBCH                         
*   2010-09-15      gaochunsheng            AM-Bug00000248      When received unexpected_local_msg in rach state, PL will'not  dump.
*   2010-09-17      wuxiaorong              PhyFA-Enh00000829   when cell search,try to receive SB immediately if find FCB,rather than do time adjusting. Do time adjusting when SB decoded successfully.
                                                                remove inconsistent part of LM-Bug00000104.
*   2010-09-23      guxiaobo                LM-Enh00000057      add condition before delete operation
*   2010-09-26      gaochunsheng            AM-Bug00000229      When configureing oal_rtx_event_req() in idle state,The value of wakeup_frame_no should avoid error
*   2010-09-23      songzhiyuan             LM-Enh00000058      [ID_2010-09-22_2545]高优先级任务打断低优先级任务的位置是变化的，导致一个全局结构体内容前后不一致，在参数检查时dump下来                       
*   2010-09-09      sunzhiqiang             PHYFA-Req00000737   add NST for facility of verifing AMT.
*   2010-09-28      wuxiaorong              AM-Enh00000178      receiving MONITOR_BCCH_REQ and MONITOR_CCCH_REQ  simultaneously,do pending MONITOR_CCCH_REQ when g_u16_gsm_state < GSM_SYNC.
*   2010-09-28      songzhiyuan             PhyFA-Req00000767   add function of CSD F144
*   2010-10-28      guxiaobo                HSDPA-Bug00001249   use macro to define the max checked tsn, for different RF protection gap
*   2010-10-28      songzhiyuan             LM-Bug00000184      [ID_2010-10-18_1842]切换时对于邻小区的定时转换有bug，导致下一次切换失败                        
*   2010-11-03      songzhiyuan             PHYFA-Req00000773   add test code for abnormity of mode change
*   2010-11-08      fushilong               AM-Bug00000304      当MAC模式改变时，不清除缓存中的数据。                                                 
*   2010-11-09      sunzhiqiang             PhyFA-Enh00000876   in AMT mode, search sb offset is not correct
*   2010-11-09      wuxiaorong              AM-Bug00000310      modify GSM_RX_TD_OFFSET_QBIT_BEFORE_RTX(unit qbits)，changed from 130qbits to 233qbits.
                                                                修改cs td的事件保护与gsm测量td的一致，原来用gsm事件之间的保护间隔，比实际要求的大
*   2010-11-16      fushilong               AM-Bug00000321      RACH 状态下，如果CCCH已经停止了，降低DUMP等级，不进行RACH任务配置。
*   2010-11-16      wuxiaorong              AM-Bug00000320      g_u16_gsm_fcp_3rd_state initialized with value 0xFF in gsm_fcp_init().
*   2010-11-15      wanghuan                LM-Bug00000204      starting fn should be current_block fn when tbf is imm_active.
*   2010-11-18      fushilong               AM-Enh00000222      终端无数据可发送时，dummy_block每8次发一个。
*   2010-12-1       songzhiyuan             LM-Enh00000096      G网待机功耗优化：CS IDLE下，测量策略的优化 
*   2010-12-02      dujianzhong             PhyFA-Enh00000884   delete the varible g_u16_gsm_cs_tdd_5frame_128chip
*   2010-12-03      dujianzhong             LM-Bug00000280      when check conflict,we should check both g_st_fcp_set_fcb_task and g_stp_gsm_cs_proc_tb;
*   2010-12-17      wanghuan                LM-Bug00000282      change the mechanism of reporting the rf_scan result in the state of GSM_SEARCH_RF,avoiding of no reporting for ever.
*   2010-12-22      wanghuan                LM-Bug00000308      DSP should not sleep in the frame when the frame number and the l1cc state have just be changed .
*   2010-12-28      dujianzhong             PhyFA-Req00000812   add code setion limit for GSM
*   2010-12-29      dujianzhong             LM-Bug00000319      we shall confirm that the tsn to be delete is just the one we expect.
*   2010-12-29      wanghuan                LM-Bug00000324      nc_meas should be started just when the reporting is not processing;
*   2011-01-04      fushilong               LM-Bug00000349      PS idle下，睡眠的唤醒帧号错误；CR AM-Bug00000229 修改时引入。
*   2011-01-05      wanghuan                PhyFA-Req00000822   test code for all 0 in nc_meas_report.
*   2011-01-06      wuxiaorong              AM-Bug00000325      modify the calculation of s_u32_latest_rtx_fn in GSM_SYNC state,when PBCCH exists .
*   2011-01-09      wuxiaorong              PhyFA-Enh00000919   support the process of rssi  with 4bits fraction(precision = 1/16).
*   2011-01-12      shanxin                 Phyfa-Req00000823   requirement from CPRM00030178
*   2011-01-06      wuxiaorong              AM-Enh00000234      support the case of TARGET_CELL_MONITOR_BCCH_REQ :target cell's bsic_valid is true, but not in neighbour cells'list. 
*   2011-01-10      wuxiaorong              PhyFA-Req00000825   implement PECKER function on L platform,using switch of PECKER_SWITCH.
*   2011-01-14      sunzhiqiang             LM-Req00000128      ADD RRTL function
*   2011-01-20      fushilong               LM-Bug00000351      GSM下HLP进行相同配置的TBF配置表更新时，运行到特定的指令时，FCP任务抢占，导致配置表操作异常。
*   2011-01-20      fushilong               LM-Enh00000145      回退AM-Enh00000222，去掉DUMMY CTRL BLOCK的发送优化
*   2011-01-21      dujianzhong             LM-Enh00000157      Init the data in last used enc buffer to SID to avoid noise in case there is no data from ARM1 later.
*   2011-01-25      wanghuan                LM-Bug00000380      the FCB_SUCCESS_IND which is received before the fcb_start_fn in bg_fcb state when handovering without timeinfor should be discarded.
*   2011-02-12      wanghuan                LM-Bug00000379       modify the way of deleting tasks which were already downloaded to OAL,in avoid of lossing deleting.
*   2011-02-18      wanghuan                LM-Bug00000400      TA value should also be updated when the ptcch_index is not valid but the ta value is valid.
*   2011-02-21      fushilong               LM-Bug00000405      SAIC影响到PRACH的发送时序，代码对这种延迟严重的时序考虑不周
*   2011-01-18      wanghuan                LM-Bug00000398      the return result of function oal_rtx_task_del should be cared before deleteing the tsntask in l1cc frametask table.
*   2011-02-26      sunzhiqiang             LM-Req00000138      add gsm-amr for LM
*   2011-03-03      sunzhiqiang             LM-Req00000192      add fast calibration function for amt
*   2011-03-07      ChenLei                 LM-Bug00000422      Don't do inter-RAT measurement during CCO procedure
*   2011-03-08      wanghuan                PhyFA-Enh00000953   optimization of power consumption in idle state.
*   2011-03-18      dujianzhong             LM-Bug00000435      the frame number used for configuring BG BSIC RX is wrong.
*   2011-03-25      dujianzhong             LM-Enh00000211      delete test code added by PhyFA-Req00000822
*   2011-03-25      wanghuan                LM-Bug00000447      usf_ind which is too late because of normal reason should be discarded.
*   2011-03-28      dujianzhong             LM-Req00000214      when handover,frefr should also be triggered to improve audio performance.
*   2011-03-31      sunzhiqiang             PhyFA-Enh00000988   modify test depth strategy in FDT
*   2011-04-01      sunzhiqiang             LM-Req00000231      AMT: L1 receives and transmit at a certain ARFCN frequency continuously,it is a new requirment. 
*   2011-04-06      dujianzhong             LM-Bug00000461      When measurement report is overtime,check whether it is caused by unupdated parameter.
*   2011-04-12      wanghuan                LM-Req00000237      物理层对BB和RF自适应
*   2011-04-12      dujianzhong             LM-Bug00000479      modify the way of using gsm_get_efrenc_ch_mode() to avoid unexpected g_u16_fcp_gsm_new_state value.
*   2011-04-14      wanghuan                LM-Bug00000474      tsntasks with different burst type(ex.AB and NB) and different rtx_offset should not be jointed.
*   2011-04-21      sunzhiqiang             Phyfa-Enh00000985   eliminate warning from famt and amr function
*   2011-04-22      sunzhiqiang             LM-Bug00000489      when starting_time is valid,if before_time is valid, ms should access cell used before_time.
*   2011-04-25      wanghuan                AM-Bug00000386      transfer_ccch should not be monitored when either ul rlc mode or dl rlc mode is uack mode.
*   2011-05-05      wanghairong             LM-Bug00000525      save source cell freqoffset when handover,and used for handover fail 
*   2011-05-06      sunzhiqiang             LM-Bug00000505      eliminate noise in amr
*   2011-05-14      sunzhiqiang             PhyFA-Enh00001015   sb_next_fn should not modulo 51 in nst sync mode.
*   2011-05-18      sunzhiqiang             PHYFA-Req00000925   eliminate error in pc_lint
*   2011-05-18      wanghuan                LM-Bug00000551      L1 report the latest c_value to HLS across the message MPAL_RR_CONNECT_IND in two phase access TBF establishment process .
*   2011-05-14      wanghuan                LM-Bug00000526      AGC optimization for handover procedure(include handover_success procedure and handover_fail procedure).
*   2011-06-09      ningyaojun              PhyFA-Req00000936   DualCard:code added or modified for DualCard.
*   2011-06-20      ningyaojun              PhyFA-Req00000936   DualCard:(1)code updated;
                                                                         (2)u16_afc_offset is corrected in gsm_pt_rf_rx_tb_t,gsm_pt_rf_tx_tb_t,hls_gsm_pt_rf_rx_req_t and hls_gsm_pt_rf_drx_req_t
*   2011-07-08      sunzhiqiang             LM-Bug00000613      NST sync is not correct
*   2011-07-11      ningyaojun              PhyFA-Req00000936   DualCard:code updated.
*   2011-07-18      wanghuan                LM-Bug00000634      fcb_rx task should be deleted if it exists when entering GSM_HANDOVER state.
*   2011-07-19      dujianzhong             PhyFA-Req00000974   replace the platform switch for L PLATFORM from _LC_ZSP540_ to _L_PLATFORM_。
*   2011-07-21      sunzhiqiang             LM-Bug00000628      modify rrtl tx/crx error
*   2011-07-21      ningyaojun              PhyFA-Bug00002713   DualCard: modification for gsm_fcp_add_tsntask,gsm_fcp_state_change/gsm_fcp_state_changing,etc.
*   2011-07-20      yuzhengfeng             PhyFA- Req00000975  add code setion 
*   2011-07-25      wuxiaorong              PhyFA-Req00000985   DualCard:l1cc_gsm:完善待机及无网络下的睡眠控制.
*   2011-08-01      linlan                  PhyFA-Bug00002758   DualCard: delete RX_GSM_89SLOT when search fcb fail,and modify SCH buffer len .
*   2011-08-01      fushilong               PhyFA-Bug00002757   [DualCard]FCP调定时参数
*   2011-08-01      ningyaojun              PhyFA-Bug00002713   DualCard: modification for gsm_fcp_compare_prior, gsm_fcp_state_changing; add GSM_INVALID_STATE for g_u16_gsm_state, etc.
*   2011-08-02      ningyaojun              PhyFA-Bug00002713   DualCard: s16_rtx_offset in gsm_slot_rtx_t should not be set 0 for for TSNTASK_SCAN_RF in gsm_fcp_add_tsntask.
*   2011-08-04      ningyaojun              PhyFA-Bug00002713   DualCard: the stp_slot_rtx->u16_arfcn is temporarily masked in TSNTASK_NC_RSSI_RX in function gsm_fcp_add_tsntask().
*   2011-08-08      yuzhengfeng             PhyFA-Req00001004   Dualcard: 修改GSM下APC地址传递时的索引 
*   2011-08-08      yuzhengfeng             PhyFA-Enh00001076   Dualcard: CCCH下发时，如果该卡在st_normal_search里刚 INIT_SEARCH搜网成功，需要删除 
*   2011-08-08      ningyaojun              PhyFA-Bug00002713   DualCard: the stp_slot_rtx->u16_arfcn is temporarily masked in TSNTASK_NC_RSSI_RX in function gsm_fcp_add_tsntask().
*   2011-08-08      ningyaojun              PhyFA-Bug00002773   DualCard: modification for adding a affair.
*   2011-08-08      ningyaojun              PhyFA-Bug00002704   DualCard: modification for Background Card (P)BCCH/CCCH_RX.
*   2011-08-12      wuxiaorong              PhyFA-Bug00002799   DualCard: gsm_fcp_cs_idle添加任务计算s_u32_latest_rtx_fn不对 .
*   2011-08-12      wuxiaorong              PhyFA-Bug00002795   DualCard:L1CC GSM HLP_MEAS :gsm_hlp_init_timeinfo函数中，定时继承时会错误的覆盖服务小区的定时信息 .
*   2011-08-15      ningyaojun              PhyFA-Bug00002796   DualCard: (1)add gsm_fcp_time_adj() calling in gsm_fcp_foreground_change_req() 
                                                                          (2)modify NC_BCCH_RX/SACCH_RX branches in gsm_fcp_add_tsntask()
                                                                          (3)correct the type of u32_ch_type in gsm_fcp_add_tsntask() to eliminate PC-LINT warnings
*   2011-08-15      yuzhengfeng             PhyFA-Bug00002819   DualCard:MSG_GSM_FCP_ENC_DATA_REQ消息结构里，没填sim id
*   2011-08-20      sunzhiqiang             L8132-Bug00001581   rx_Qual is not correct in pecker mode
*   2011-07-21      dujianzhong             LM-Bug00000639      when the tsntask has been set expired,we need not to delete it.
*   2011-08-15      yuzhengfeng             PhyFA-Bug00002818   [DualCard]gsm_fcp_check_tsntask等修改 
*   2011-08-08      wanghuan                LM-Bug00000661      supporting discontiguous pdtch tx_tsntask finely for L1CC.
*   2011-08-15      linlan                  PhyFA-Bug00002820   DualCard:modify the configuration of the handover process without existing timeinfo 
*   2011-08-22      wuxiaorong              LM-Bug00000688      DualCard: 双卡双待版本，手动GSM模式, 物理层只上报SI3和SI4 
*   2011-08-22      wuxiaorong              PhyFA-Req00001016   DualCard: oal_rtx_event_del的存在特殊事件拖尾删不掉,加返回值区分 
*   2011-08-22      wuxiaorong              LM-Bug00000707      [DualCard]双卡双待版本，手动GSM模式开机物理层DUMP 0x1050a 
*   2011-08-24      yuzhengfeng             PhyFA-Bug00002849   [Dualcard]fcp任务删除相关内容修改 
*   2011-08-26      wuxiaorong              PhyFA-Bug00002856   DualCard: gsm_hlp_init_fcb_bsic_proc()进不了g_st_fcb_list等表的清0操作. 
*   2011-08-27      sunzhiqiang             LM-Enh00000369      sp flag of init audio data should be set 0.
*   2011-08-27      linlan                  PhyFA-Bug00002859   [DualCard]modify the configuration of foreground td cs in GSM mode
*   2011-08-29      yuzhengfeng             LM-Bug00000729      双卡双待:GSM 电话接通1分钟后物理层dump,错误原因值为105D2，原因是NBITS事件删除错误。这个CR用来统一事件删除接口，去掉NO_RTX。
*   2011-08-29      yuzhengfeng             PhyFA-Bug00002857   [DualCard]背景卡CCCH接收点计算不正确.
*   2011-09-01      yuzhengfeng             PhyFA-Bug00002868   [DualCard]GSM背景卡的timeinfo、tsntask添加需要修改
*   2011-09-05      yuzhengfeng             PhyFA-Bug00002870   [DualCard]背景搜网时，高层下发BCCH等接收且立马配置，需要删除进行中的搜网任务
*   2011-09-05      wuxiaorong              PhyFA-Req00001040   [DualCard]GSM多卡下cellserach控制过程需要支持TD搜网. 
*   2011-09-06      fushilong               PhyFA-Bug00002891   [DualCard]callback的GSM数据_eq_gmsk_freq_correct频偏软校准方式不对
*   2011-09-08      wuxiaoorng              PhyFA-Bug00002928   [DualCard]GSM多卡下,SIM1的TD测量无法启动
*   2011-09-08      sunzhiqiang             PhyFA-Enh00001108   [DualCard]support amt function
*   2011-09-09      wuxiaorong              PhyFA-Bug00002938   [DualCard]L1CC_GSM睡眠代码修改
*   2011-09-09      ningyaojun              PhyFA-Req00001045   [DualCard](1)modify gsm_fcp_get_undownloaded_slot_range() to skip pseudo gsm_slot_rtx_t(pseudo PDTCH_TX);
                                                                          (2)correct the error of input parameter for gsm_fcp_get_undownloaded_slot_range() in gsm_fcp_transfer();
                                                                          (3)temporarily modify gsm_fcp_check_rtx_conflict() to avoid the conflit of RRBP and PDTCH_TX in DD(0x1050A),whom should be downloaded in one event.
*   2011-09-11      wanghuan                LM-Bug00000750      correct the TA caculation method in sync handover process. 
*   2011-09-13      wuxiaorong              LM-Bug00000777      [DualCard]双卡双待版本：GSM下丢网，唤醒帧号出错
*   2011-09-16      ningyaojun              PhyFA-Req00001045   [DualCard]modification for  gsm_fcp_state_changing()
*   2011-09-16      yuzhengfeng             LM-Bug00000796      双卡双待版本： 模式切换过程中，帧任务表中，删除特殊事件处理有误
*   2011-09-16      fushilong               PhyFA-Bug00002979   [DualCard]GSM 系统间切换重选功能                                                                      
*   2011-09-16      wanghuan                LM-Bug00000775      inform HLS to modify the EDEN parameters in handover process.
*   2011-09-22      sunzhiqiang             LM-Bug00000814      [DualCard]solve 0x1050a dump
*   2011-09-22      linlan                  LM-Bug00000839      双卡双待：L1CC GSM定时调整在临界值-2500QBIT处理有问题
*   2011-09-22      wuxiaorong              PhyFA-Bug00003005   [DualCard]GSM双卡在同小区发生前背景卡转换时刚好跨帧，帧号回退了1帧
*   2011-09-23      linlan                  LM-Req00000372      双卡双待版本，物理层完善一卡业务期间另外一卡的BCCH及PCH读取流程
*   2011-09-24      linlan                  LM-Bug00000862      DualCard  L1CC GSM PDTCH功率发送参数更新时隙错误 
*   2011-09-26      wuxiaorong              LM-Bug00000870      双卡双待：由TRANSFER转换至IDLE状态时,残留的FCB任务触发后续流程在跨帧时会导致配置的睡眠帧号异常 
*   2011-09-28      wuxiaorong              LM-Bug00000888      双卡双待 停止测量的处理，还是要放在帧头处理函数gsm_fcp_frametimer_pre_handle()，否则3G重选时，可能引发异常 
*   2011-09-29      linlan                  LM-Bug00000868      双卡双待，GSM APC数组在越界问题，导致睡眠状态改写同时也影响发送功率 
*   2011-09-29      sunzhiqiang             LM-Bug00000900      mofidy calc otd error in pre-sync handover
*   2011-09-29      linlan                  LM-Bug00000911      双卡双待： 连接下业务和BCCH/CCCH接收冲突检测有问题。 
*   2011-10-08      wanghairong             LM-Bug00000923      [双卡双待]高层下发对非同步小区的定时重选后物理层处理存在异常，无法进行目标小区的bcch读取，从而引起后续流程的异常 
*   2011-10-08      wanghairong             LM-Enh00000405      双卡双待：gsm下搜索td过程中RSA特殊事件进行任务删除时处理存在异常
*   2011-10-08      wanghuan                LM-Bug00000918      process correction for Intra-cell handover.
*   2011-10-08      wanghairong             LM-Bug00000928      双卡双待：IDLE态且TBF即将生效转状态前，s_u32_latest_rtx_fn没有更新，导致拦截死机 
*   2011-10-08      wanghairong             LM-Bug00000934      双卡双待：G到T模式切换与睡眠相关的系列异常（ASSERT: tdif tm0 HW abnoramlly，wakeup mode error!）
*   2011-10-09      wuxiaorong              LM-Bug00000955      双卡双待，测量停止时，该卡的上报reporting_bitmap要清0
*   2011-10-12      wuxiaorong              LM-Bug00000986      双卡双待：G+G主次卡待机中丢网后不可恢复
*   2011-10-13      sunzhiqiang             LM-Bug00000969      [DualCard]when find fcb and SB in handover state, agc is error
*   2011-10-13      sunzhiqiang             LM-Req00000395      [DualCard]modify nst_function and do not sleep in amt mode
*   2011-10-13      fushilong               LM-Bug00000971      双卡双待：EDGE:PCCH下，IDLE转TRANSFER过程中，不能睡眠
*   2011-10-13      sunzhiqiang             LM-Bug00000957      [DualCard]when normal sleep wake up, it should send del msg to FCP.
*   2011-10-13      wanghairong             LM-Bug00000994      双卡双待：物理层在收到读取scell_bcch_req之后，在没有找到定时的情况下应该清空原服务小区的定时 
*   2011-10-13      fushilong               LM-Bug00000987      双卡双待：在PRR事件与TD寻呼存在冲突时，会导致PRR请求MMC资源失败，PRR需要进行预占 
*   2011-10-13      fushilong               LM-Bug00000956      双卡双待:SIM1发起接入接入之前，SIM0如果已经 配置RX_TDD_5FRAME_128CHIP事件，需要进行删除操作
*   2011-10-13      fushilong               LM-Bug00000992      双卡双待：SIM0 开机进入NOCELL，之后SIM1开机时，SIM0正在进行FCB过程，SIM1进行搜网时需要考虑已经存在的SIM0的89SLOT冲突的情况
*   2011-10-14      linlan                  LM-Bug00001007      双卡双待：GSM同步切换过程中，L1CC没有转换状态 
*   2011-10-17      wuxiaorong              LM-Enh00000419      双卡双待：TD业务下，GSM不配系统内测量和搜网
*   2011-10-17      wanghairong             LM-Bug00001011      双卡双待：gsm下切换时物理层已收到PI，后续高层下发P_RR_MPAL_HANDOVER_FAIL_REQ后，物理层处理异常  
*   2011-10-19      wuxiaorong              LM-Bug00001044      双卡双待：IDLE下测量配置FCB接收时，如果也存在搜网，需要修改s_u32_latest_rtx_fn，使之不满足睡眠条件 
*   2011-10-20      linlan                  LM-Bug00001069      双卡双待：GSM nc mode等于0时，开启测量时需要有效g_st_serv_rssi_meas的valid flag  
*   2011-10-21      wuxiaorong              LM-Bug00001078      双卡双待，有卡在GSM下进NOCELL，另外一卡模式（重选/切换）转到gsm,all_in_nocell 标志要清掉 
*   2011-10-26      wuxiaorong              LM-Bug00001120      双卡双待：一卡经过TD模式或飞行模式再回到GSM，GSM系统状态管理的完善
*   2011-10-20      linlan                  LM-Bug00001045      双卡双待：GPRS TBF生效前需要支持发送RRBP 
*   2011-10-28      sunzhiqiang             LM-Bug00001124      双卡双待: SABM没有编码导致N200超时
*   2011-10-27      fushilong               LM-Bug00001145      双卡双待：GSM 寻呼预占位置计算错误
*   2011-10-31      wuxiaorong              LM-Bug00001158      双卡双待：一张卡在NOCELL，另一张从GSM模式切走，转状态前被帧中断打断，异常
*   2011-11-03      yuzhengfeng             LM-Bug00001181      双卡双待：transfer下ccch配置时，数组越界导致漏寻呼
*   2011-11-08      wanghairong             LM-Bug00001198      双卡双待：gsm时connect下盲切时需要添加新的事务 
*   2011-11-08      wanghairong             LM-Bug00001221      双卡双待：GSM的代码中每次给高层发消息都是发送两次，第二次都是空消息导致任务切换增加一倍 
*   2011-11-09      wuxiaorong              LM-Enh00000462      双卡双待，物理层GSM_CS状态也要支持进行无网络搜网
*   2011-11-10      sunzhiqiang             LM-Bug00001199      双卡双待: 配置SACCH_TX时offset被设置为0而不是TA.
*   2011-11-10      linlan                  LM-Bug00001204      双卡双待：添加TSNTASK_PDTCH_RX失败不能影响拼接上行发送数据 
*   2011-11-10      linlan                  LM-Bug00001212      双卡双待：预占下一个CCCH接收位置的时候，当帧号转圈时，u32_next_pos_fn计算有误  
*   2011-11-10      sunzhiqiang             LM-Bug00001240      双卡双待: 删除帧任务表时不应该用全局变量g_u32_gsm_fn
*   2011-11-10      wanghuan                PhyFA-Req00001111   eliminate pc-lint messages of warning 415 and warning 416.
*   2011-11-10      fushilong               LM-Bug00001225      双卡双待：IDLE下， RX_TDD_5FRAME_128CHIP事件与NB冲突时，需要删除冲突的事件
*   2011-11-12      wanghairong             LM-Bug00001261      双卡双待：FCP中发送pending 消息时需要填写消息头的sim id
*   2011-11-14      wuxiaorong              LM-Bug00001266      双卡双待：SIM1在GSM_CS收BCCH，SIM0 3到2重选没有收SI 
*   2011-11-15      sunzhiqiang             PhyFA-Req00001107   双卡双待: 支持RRTL功能
*   2011-11-15      wuxiaorong              LM-Bug00001281      双卡双待，GSM一卡待机，另一卡NOCELL需要FCB搜网时不睡眠  
*   2011-11-17      sunzhiqiang             LM-Enh00000468      双卡双待: 定时转换时无网络睡眠时间初始化为5s。
*   2011-11-17      sunzhiqiang             LM-Bug00001293      双卡双待: CS建链时将PDIBUFF清0.
*   2011-11-17      sunzhiqiang             LM-Bug00001296      双卡双待: AMT发送数据应该放在片内
*   2011-11-17      wuxiaorong              LM-Bug00001213      双卡双待，PCCO重选时在帧头调整还没生效时，业务接收的EQ拖尾处理也刚好要调帧头，导致调整错误而坏帧
*   2011-11-18      linlan                  LM-Bug00001262      双卡双待：在RRBP和RLC数据块连续发送的时候，RRBP发送地址配置错误 
*   2011-11-17      fushilong               LM-Req00000393      双卡双待  接入过程中，要区分Paging、CS、PS业务接入的优先级，确保CS业务优先
*   2011-11-17      fushilong               LM-Bug00001280      双卡双待：两卡驻留同一小区时，辅卡业务释放后，辅卡寻呼消息的上报的存在问题
*   2011-11-22      wuxiaorong              LM-Bug00001324      双卡双待，切换是同小区信道重配，邻区测量定时不要从备份恢复
*   2011-11-27      sunzhiqiang             LM-Bug00001335      双卡双待: NST下需要发送固定数据
*   2011-11-28      wuxiaorong              LM-Bug00001371      双卡双待：主卡TD业务，副卡NOCELL时，搜网配不了，子状态置为为GSM_BGS_SLEEP
*   2011-11-30      sunzhiqiang             LM-Bug00001353      双卡双待: 前背景转换或者前景卡小区重选时需判断是否更新g_st_gsm_cellsearch_ctrl的sb_rx_fn
*   2011-11-30      sunzhiqiang             LM-Bug00001361      双卡双待: RACH状态下先配置前景卡的任务
*   2011-11-30      linlan                  LM-Bug00001262      双卡双待: fix again. dd can't recognise TX_GSM_GMSK_NB and TX_GSM_8PSK_NB when call event delete interface
*   2011-11-30      wanghairong             LM-Bug00001339      双卡双待：GSM_CS状态下，scell频点没有定时不直接配置BCCH消息的接收 
*   2011-12-03      dujianzhong             PhyFA-Enh00001154   modify the processions of nci, skip indicator and TA for handover.
*   2011-12-06      linlan                  LM-Bug00001412      双卡双待：将g_st_l1cc_mmc_mode_ind扩展为数组，支持处理同一帧下发不同卡的mode change req消息
*   2011-12-06      sunzhiqiang             LM-Bug00001402      双卡双待: RACH状态下按照任务优先级配置任务
*   2011-12-06      wuxiaorong              LM-Bug00001404      双卡双待：gsm_fcp_state_change()判断目标状态和g_u16_fcp_gsm_new_state相等，不需要ASSERT  
*   2011-12-06      wuxiaorong              LM-Bug00001407      双卡双待：最后一张GSM卡模式切走的时候，把TD接收通道关闭  
*   2011-12-06      wanghairong             LM-Bug00001418      双卡双待,GSM前背景卡倒换时,双模卡系统间测量各个频点PSA(成功)/RSCP接收点也要倒换
*   2011-12-08      sunzhiqiang             LM-Bug00001415      双卡双待: 计算POS的帧号时，需考虑当帧号为2715648的处理 
*   2011-12-08      linlan                  LM-Bug00001439      双卡双待：fcp在给MMC发送mode_change_ind消息之后，允许配置任务并且清除10帧之前的帧任务表 
*   2011-12-14      sunzhiqiang             LM-Bug00001426      双卡双待: RACH状态下收到任何消息都需要更新睡眠唤醒帧号。
*   2011-12-14      sunzhiqiang             LM-Bug00001436      双卡双待: CS状态下配置扫频任务时需先判断FCB是否存在
*   2011-12-14      wanghairong             LM-Bug00001443      双卡双待：背景卡收CCCH时候，存在系统帧号计算差1帧而无MPAL_PAG_CH信道数据上报 
*   2011-12-14      wuxiaorong              LM-Bug00001471      双卡双待,gsm_fcp_handover配任务时，先判断表是否有效
*   2011-12-14      wuxiaorong              LM-Bug00001450      双卡双待：主卡进PS业务，副卡的FULL CCCH不配置接收 
*   2011-12-14      wuxiaorong              LM-Bug00001469      双卡双待,IDLE下要NOCELL搜网时,先要判有没有其它卡刚好进行FCB搜网
*   2011-12-14      linlan                  LM-Bug00001449      双卡双待：无网络搜FCB失败删除和转状态删除FCB同时发生，需要保护 
*   2011-12-14      linlan                  LM-Bug00001464      双卡双待：扫频过程的最后几个频点接收事件被意外删掉，后续流程没恢复 
*   2011-12-15      sunzhiqiang             LM-Bug00001470      双卡双待: CS状态时需按search_type正确处理FCBSUCC_IND消息
*   2011-12-22      linlan                  LM-Bug00001495      双卡双待：无网络搜TD在粗同步阶段被模式切换打断后,5FRAME_128CHIP事件标志没有清除，导致从TD再切换回来后无法配置搜网任
*   2011-12-22      fushilong               LM-Bug00001505      双卡双待：FCBSUCC和信道配置转换（会进行89slot的删除）同时发生时，存在89SLOT的重复删除，此种临界情况下，GSM L1CC可以不作拦截
*   2011-12-24      caisiwen                LM-Bug00001503      correction for possibly not receiving CCCH data in successive 51 frames when state is changing too quickly.
*   2011-12-27      sunzhiqiang             LM-Bug00001481      双卡双待: 在handover状态下也要处理背景卡的FCBSUCC_IND消息。
*   2011-12-28      yuzhengfeng             LM-Bug00001527      双卡双待: 接收位置发生前背景转换调定时可能导致漏收CCCH
*   2011-12-27      linlan                  LM-Enh00000503      双卡双待：修改PCH和BCCH对应的任务优先级，并提高BCCH优先级高于PCH FCP修改任务冲突检测的bug 部分回退LM-Bug00001361
*   2011-12-29      linlan                  LM-Bug00001524      双卡双待：在PTCCH还没有发送的时候，收到下行的PTCCH也不能更新TA 
*   2012-01-04      fushilong               LM-Bug00001544      双卡双待：GSM 打电话过程中，FCBSUCC中已经删除成功，后续gsm_hlp_expired_nc_meas重复删除出错
*   2012-01-10      sunzhiqiang             LM-Enh00000536      双卡双待: 规避仪表信号不稳定和帧号不同步造成异步AGC失败
*   2012-01-18      sunzhiqiang             LM-Bug00001589      双卡双待: 切换状态下配置FCB事件的事物ID应该是MMC_AFF_GSM_CS_CNNT
*   2012-01-18      sunzhiqiang             LM-Bug00001598      双卡双待: CS_IDLE下配置BG任务前需要判断是否有cellsearch的FCB
*   2012-01-21      linlan                  LM-Bug00001585      双卡双待：双卡双待：保存需要被删除任务的数组的大小需要扩大 
*   2012-02-01      sunzhiqiang             LM-Enh00000435      双卡双待: 在normal_pging或extend_pging下如果寻呼冲突需要上报高层重选
*   2012-02-09      wuxiaorong              PhyFA-Req00001175   Eliminate compile Errors on 0.60dev.
*   2012-02-06      ningyaojun              PhyFA-Req00001156   [DualCard]Support the mobility of a simcard in IDLE state in GSM(or TD) mode when another simcard is in PS_CONNECT state in TD(or GSM) mode.
*   2012-02-06      fushilong               PhyFA-Req00001156   [DualCard]Support the mobility of a simcard in IDLE state in GSM(or TD) mode when another simcard is in PS_CONNECT state in TD(or GSM) mode.
*   2011-07-28      wanghairong             LM-Enh00000199      read utran cell bch in GSM mode when receive hls's MSG_ID_RR_MPAL_MONITOR_UTRAN_BCH_REQ message
*   2011-08-31      wanghairong             LM-Bug00000742      在和td邻区失步的情况下收到高层读utran mib请求后,立即回复读取失败的cnf消息.
*   2012-02-14      linlan                  LM-Bug00001026      双卡双待：在tranfser状态下，需要支持背景卡的TC_BCCH接收 
*   2012-02-15      sunzhiqiang             LM-Bug00001625      双卡双待: 帧号调整时需将MMC事物删除
*   2012-02-16      wanghairong             LM-Bug00001679      双卡双待：gsm系统内切换失败回切时需要重新设置定时信息表中邻区测量状态
*   2012-02-24      wanghairong             LM-Bug00001727      双卡双待:gsm下搜索td时对g_st_gsm_tdd_cs_rx_cfg.b_cst_valid_cnt_down的状态判断降低错误等级  
*   2012-02-25      wanghairong             LM-Bug00001744      双卡双待：gsm_fcp_nocell函数中接收到非预期消息时直接丢弃  
*   2012-02-27      wanghairong             LM-Bug00001755      双卡双待：gsm系统内盲切换配置FCB/SB任务时需要添加新的事件类型
*   2012-02-28      fushilong               LM-Bug00001699      双卡双待：CCCH和PCH的优先级不一样，添加事务时，需要按照实际所属的事务进行添加 
*   2012-02-28      linlan                  LM-Bug00001771      双卡双待：在调整定时时，如果需要调整接收SB的位置，需要调整到规定的偏移范围内
*   2012-02-28      linlan                  LM-Bug00001751      双卡双待：转状态时，如果是系统间盲切换，则不对转状态的sim_id进行拦截 
*   2012-02-29      sunzhiqiang             LM-Enh00000574      双卡双待: T+G下不进行寻呼冲突检测
*   2012-03-02      fushilong               LM-Enh00000593      规避ZView3.4.0编译器导致的跑飞问题
*   2012-03-05      fushilong               LM-Bug00001801      双卡双待: GSM 无网络搜索被TD业务打断时，需要删除相关的任务和事件。
*   2012-03-08      ningyaojun              LM-Bug00001800      [DualCard] g_u16_gsm_fcp_3rd_state should be handled in gsm_fcp_state_changing().
*   2012-03-12      yuzhengfeng             PhyFA-Enh00001243   省电优化：RSSI测量长度改为32bit
*   2012-03-30      linlan                  PhyFA-Req00001232   [DualCard]LM.2.30.00 多卡待机相对定时处理方式修改
*   2012-04-12      wangjunli               PhyFA-Req00001026   [LTE] LTE 3.0版本开发,编译通过,消除pclint msg.
*   2012-03-26      wanghairong             LM-Bug00001913      双卡双待：gsm下stop rach但stop原因不是进入idle状态时，不需要退出互斥流
*   2012-04-10      sunzhiqiang             LM-Bug00001937      预读MIB打开的情况下，物理层处理有问题
*   2012-04-24      wangjunli               PhyFA Req00001254   [LTE] L1CC_GSM的UT中同模块的函数调用用开关L1CC_GSM_RTRT_UT_SWITCH打桩
*   2012-04-25      wangjunli               PhyFA-Bug00003311   [LTE] 无网络配置一直睡眠时，s_u32_latest_rtx_fn赋值错误
*   2012-04-24      linlan                  PhyFA-Enh00001237   L1CC-GSM: 收发任务配置的冲突检查细化 
*   2012-04-20      dujianzhong    L        M-Enh00000656       en the audio data received from ARM is speed mismatched,init enc BUFFER.
*   2012-05-02      caisiwen       P        hyFA-Bug00003336    LTE]L1CC_GSM:gsm_hlp_int_slave_gsm_meas修改
*   2012-05-03      linlan                  LM-Enh00000622      双卡双待：卡A重选时，卡B正在无网络搜索，在完成卡A的小区重选流程后，继续卡B的无网络搜索流程，而不是从头开始搜网 
*   2012-05-03      wanghairong             LM-Bug00002009      gsm下发生系统内重选/切换时，不论是否有td邻区的定时都需要进行相应的帧号转换
*   2012-05-08      caisiwen                PhyFA-Req00001253   [LTE] 消除0.60dev分支LC-LINT告警
*   2012-05-11      sunzhiqiang             LM-Bug00002010      双卡双待：在CS状态下，需检测是否存在无网络下的FB任务 
*   2012-05-09      wuxiaorong              PhyFA-Enh00001283   [lte3.0]mailbridge名称修改
*   2012-05-10      wanghairong             PhyFA-Enh00001279   gsm下搜索td优化
*   2012-05-24      wanghairong             LM-Bug00002075      双卡双待：gsm_fcp_frametimer_change中需要使用局部变量来保存比较的帧号
*   2012-05-24      wangjunli               PhyFA-Req00001276   [LTE3.0]给高层上报GSM读系统消息接口更新涉及LTE和GSM的变动
*   2012-05-24      wanghuan                LM-Bug00002083      配置search td接收任务时检查 配置表中频点与当前状态表中频点 是否一致，若不一致，本帧放弃配置
*   2012-05-30      wanghuan                LM-Enh00000738      inform USF_DETECTED state to HLS for stopping and restarting T3180.
*   2012-06-01      wangjunli               PhyFA-Bug00003430   [LTE3.0]L1CC_GSM:OAL_ASSERT使用考虑不周全导致错误拦截
*   2012-06-08      wangjunli               PhyFA-Bug00003465   [LTE3.0]L1CC_GSM:目标小区SI相对定时计算错误
*   2012-05-09      wanghuan                LM-Enh00000681      optimization for HT100. 
*   2012-06-11      sunzhiqiang             LM-Bug00002137      双卡双待:在TRANSFER下先配置高优先级的TC_BCCH
*   2012-06-28      linlan                  LM-Bug00002185      颈本〨CF协议测试】在TRANSFER状态下当TA无效时不能发送normal burst 
*   2012-06-28      linlan                  LM-Bug00002182      颈本〨CF射频测试】L1CC_GSM：EDGE SRB环回模式下，TBF立即生效时，处理存在问题。
*   2012-07-02      wuxiaorong              PhyFA-Req00001305   LTE3.0] GSM/TD系统间接收事件配置时进行异模式唤醒  
*   2012-07-05      sunzhiqiang             LM-Bug00002205      颈本〨CF协议测试】CS下新TA必须在下一个SACCH周期开始才能生效 
*   2012-07-07      guxiaobo                LM-Bug00002182      rework
*   2012-07-09      dujianzhong             LM-Bug00002207      双卡双待:GSM模式下没有SIM卡激活时,gsm_fcp_state_changing函数中会进ASSERT,另外gsm_fc_init所使用的卡号可能有问题
*   2012-07-10      linlan                  LM-Bug00002202      1810双通：修改定时维护关系后FCP处理测量控制消息过于依赖当前系统状态和测量使能关系，某些状态（CS等）无法处理测控消息
*   2012-07-13      sunzhiqiang             PhyFA-Bug00003577   RRTL配置AB,8PSK的NB有问题
*   2012-07-16      wangjunli               PhyFA-Bug00003574   [LTE3.0]:无网络搜索GSM未启动小区搜索
*   2012-07-16      wuxiaorong              PhyFA-Bug00003574   [LTE3.0]:无网络搜索GSM未启动小区搜索
*   2012-07-17      caisiwen                PhyFA-Bug00003572   [LTE3.0]lte作为主模式在idle下让GSM做测量时GSM的FCP报latest_rtx_fn_cal fail
*   2012-07-26      wangjunli               PhyFA-Bug00003606   [LTE3.0]gsm_fcp_transfer收到USF消息DYNAMIC模式添加发送任务顺序错误
*   2012-07-25      sunzhiqiang             LM-Bug00002248      1810 双通：AMT1进入后，PL报的消息中sim ID不对及打开发射无响应
*   2012-07-30      caisiwen                PhyFA-Bug00003614   [LTE3.0]LTE下背景读GSM的BCCH，GSM返回读不到BCCH
*   2012-08-06      wuxiaorong              PhyFA-Bug00003624   [LTE3.0]LTE CNNT态下测量GSM dump
*   2012-08-16      wuxiaorong              LTE-Bug00001172     [LTEV2R1]DD接收通道延迟设置不对导致发送延迟，发送buffer 放在为cache区发送数据没刷cache BER class2指标超标;buffer没有必要cache able的都改成un cache.
*   2012-08-17      wuxiaorong              LTE-Bug00001178     [LTEV2R1]GSM寻呼上报帧号错误.
*   2012-08-17      linlan                  LM-Bug00002282      双卡双待：transfer下，重启NC测量的请求在FCP_USF_ind之前收到，导致NC测量异常停止
*   2012-08-21      wanghuan                LM-Enh00000831      configuration for receiving si7\si8 on BCCH EXT when fta mode.
*   2012-09-03      dujianzhong             PhyFA-Enh00001343   Modify inputing variable type when use function OAL_PRINT(). 
*   2012-08-31      linlan                  LM-Bug00002304      1810双卡：给高层上报完无网络搜网结果后，直接将配置表设成无效 
*   2012-09-07      sunzhiqiang             PhyFA-Req00001361   1810V2.3物理层2G搜索3G策略优化_GSM_Phase2
*   2012-09-14      wuxiaorong              LTE-Bug00001262     [LTEV2R1]PS业务进IDLE时刚好有邻区FCB成功发到FCP处理的拦截去掉 .
*   2012-09-17      ningyaojun              LM-Bug00002329      Once P_RR_MPAL_STOP_MONITOR_UTRAN_BCCH_REQ is commanded by HLS, g_st_gsm_utran_bch_tb.u16_utran_bch_tb_flg should be cleared immediately by HLP instead of and ahead of FCP,
                                                                Because it's too late to handle it in gsm_fcp_frametimer_post_handle(), which may result in a error of downloading an unexpected event TSNTASK_RX_TDD_READ_BCH_1SLOT_DWPTS to DD.
*   2012-09-19      linlan                  LM-Bug00002347      1810双卡：删除帧任务表bitmap时帧号换算错误 
*   2012-09-19      wangjunli               PhyFA-Req00001374   [ltemv2r1]GSM支持pecker功能
*   2012-09-24      linlan                  LM-Bug00002355      1810双卡：在调整帧号重配无网络搜网状态前需判断是否收到了MMC的搜网请求
*   2012-09-22      wangjunli               LTE-Bug00001291     [LTEV2R1]睡眠专攻测试，GSM三模无网络搜睡眠60ms唤醒一次，间隔太短
*   2012-09-29      ningyaojun              LM-Bug00002350      modify u16_repeat_frms of FACCH_H_TX in C_ST_TSTASK_ATTRIBUTE_TBL[]                                                            
*   2012-10-15      sunzhiqiang             LM-Req00000500      add agc index method for 8208
*   2012-10-22      wangjunli               LTE-Bug00001328     [LTEV2R1]L1CC_GSM在开启了辅模式无网络搜网，一轮搜网完成后，自行退出无网络并清除该卡相应的激活信?
*   2012-10-24      wangjunli               PhyFA-Req00001398   [LTE]睡眠无限帧方案修改
*   2012-10-26      sunzhiqiang             PhyFA-Req00001391   1.4分支增加RRTLGSM下连收功能
*   2012-11-01      sunzhiqiang             LM-Bug00002422      物理层在预读MIB消息时不睡眠
*   2012-11-06      linlan                  LM-Bug00002430      1810双卡：在GSM_NOCELL状态下，没有解析完MODE_CHANGE消息时不能调睡眠 
*   2012-11-09      sunzhiqiang             LM-Bug00002449      1810]GSM AMR语音通话过程中，由于TSNTASK_SACCH_RX 中，u16_ch_mode未赋值，后续sub_level计算错误，导致终端掉话
*   2012-11-09      linlan                  LM-Bug00002440      【1810双卡】TD频点排序过程中转状态，接收RX_TDD_1FRAME事件被删除，搜网流程无法恢复
*   2012-11-27      linlan                  LM-Bug00002471      1810双卡（GGE)：PCCCH更新latest_fn有误导致无法配置FCB
*   2012-11-29      wangjunli               PhyFA-Req00001418   add FER reporting(merge L8132-Req00000262)
*   2012-11-26      wuxiaorong              PhyFA-Req00001316   [LTEM-V2R2] FDD版本物理层特性软件开发,修改睡眠控制.
*   2012-12-04      sunzhiqiang             PhyFA-Bug00003720   1810V2.3RRTL单时隙发送功率和数据有问题.
*   2012-12-10      wuxiaorong              LTE-Bug00001401     [LTEV2R1]lte l1cc和DD修改关于删除事件的接口.
*   2012-12-17      wangjunli               LTE-Bug00001418     綥TEM_V2R1】GSM PS业务过程收到is_starting_frame_valid为0的时隙重分配会出现内存越界,根据L1761-BUG00000369派生
*   2012-12-18      sunzhiqiang             LM-Bug00002510      1810双卡：切换失败时如果FB和SB表里有节点需将此节点删除并将定时信息里对应的state重置为NONE 
*   2012-12-18      sunzhiqiang             LM-Enh00000977      1810：非信令综测使用的固定数据源更改成调制谱对称的数据源
*   2012-12-19      wangjunli               LTE-Bug00001432     綥teM_V2.10.01整机\富士通射频】GSM辅模式测量配置表u32_reported_fn没有赋初值
*   2012-12-21      sunzhiqiang             LM-Bug00002525      1810：SID被FACCH抢占，没有再重发SID
*   2012-12-25      yuzhengfeng             PhyFA-Bug00003766   綥TE】GSM找gap时需要对offset进行转换
*   2012-12-19      ningyaojun              LM-Bug00002527      TD+GSM NOCELL: (1) L1CC GSM HLP had better configure an endless sleeping initiatively, after all of the current nocell sim-cards are cleared when handling MSG_MMC_GSM_STOP_CELLSEARCH_REQ;
*                                                                              (2) In spite that whether a mutual-exclusive-flow conflict is encountered or not, FCP should not reset the wake-up-fn as (UINT32)GSM_INVALID_FN to wakeup in GSM_BGS_SLEEP, in gsm_fcp_nocell_process().
*   2012-12-28      linlan                  LM-Bug00002544      【1810双卡】一卡FTA=1，另一卡FTA=0时，驻留相同服务小区时需要确保对于RSSI处理结果复用的正确性 
*   2013-01-14      wangjunli               PhyFA-Bug00003797   [LTEM-V2R2]L1CC_GSM调睡眠DD因时钟校准未生效，L1CC自行调唤醒规避
*   2013-01-16      ningyaojun              LM-Bug00002571      Type of variables and parameters in function gsm_calc_interval_from_fn2peiodpos() is corrected, from UINT16 to UINT32.
*   2013-01-21      wuxiaorong              LTE-Enh00000358     [LTEM-V2R2]GSM辅模式预读系统消息前进行(有测量定时）FCB搜网同步过程. 
*   2013-01-24      fushilong               LM-Bug00002594      1810双卡：消息MSG_GSM_CS_TDD_EVENT_COMPLETE_IND触发FCP立刻配置任务时，需要判断当前与卡相关的3G任务是否有效
*   2013-01-28      wuxiaorong              PhyFA-Enh00001430   [LTEM-V2R2]GSM辅模式测量修改gsm_hlp_slave_cfg_balist的原型并废除gsm_slave_meas_req_t. 
*   2013-01-31      wuxiaorong              PhyFA-Enh00001435   [LTEM_V2R2]从GSM重选到TD帧定时会偏移，利用GSM延迟睡眠进行规避，提高TD重同步成功率
*   2013-02-27      wangjunli               LTE-Bug00001647     【LTE 3.20.00】GSM在辅模式测量过程收到了测量周期更新，需要及时更新测量配置参数
*   2013-03-06      sunzhiqiang             LM-Bug00002651      1810双卡：G+G下一卡做业务，另一卡CCCH配置与TCH冲突时需同步更新另一张卡的帧号
*   2013-02-28      linlan                  LM-Bug00002645      [1810]modify l1 detect function in NOCELL state 
*   2013-03-08      sunzhiqiang             LM-Bug00002655      CS业务配置TCH信道时需更新另一张卡的帧号信息。
*   2013-03-12      sunzhiqiang             LM-Bug00002658      1810双卡：在GSM_CS状态下如果业务互斥则用帧中断更新帧号
*   2013-03-12      sunzhiqiang             LM-Bug00002656      1810双卡单通:在删除事件时，如果时隙为负数并且offset_in_ts不为0，需要对start_offset补一个时隙的长度 
*   2013-03-18      wangjunli               LTE-Bug00001719     【TDM】GSM睡眠控制标志应在唤醒中断中置FALSE
*   2013-03-19      yuzhengfeng             PhyFA Bug00003879   [LMV2]L1CC/ALGO需给所创建的消息SIM_ID/MODE_TYPE字段赋初值
*   2013-03-16      sunzhiqiang             LM-Enh00001099      1810双卡:CS业务下AGC调整策略需要优化，SACCH上AGC不使用跳频频点AGC更新并且TCH上AGC调整幅度改为SACCH_AGC加10
*   2013-03-20      sunzhiqiang             LM-Enh00001106      1810双卡：CS业务切换弱小区和切换失败回原小区时SACCH AGC可以继承切换前邻区测量的AGC
*   2013-04-03      wangjunli               LTE-Bug00001784     【LteM_V3.20.01】BSIC失败后需要清掉FCB标志
*   2013-04-02      sunzhiqiang             LM-Enh00001118      1810双卡：FB搜索次数3次改1次
*   2013-04-15      sunzhiqiang             LM-Bug00002721      1810双卡：切换失败后错误的认为帧号转圈导致SACCH发送没有配置使得网络radio_link超时下发channel release
*   2013-04-22      fushilong               LM-Enh00001128      1810双卡：EDGE下，对于IR buffer中可能存在的bsn相同的残留数据，进行清除操
*   2013-04-27      sunzhiqiang             PhyFA-Enh00001504   FDT异步AGC校准同步策略优化
*   2013-05-03      wangjunli               LTE-Bug00001873     删除事务时也需要根据u16_mode_timing来匹配(l1cc_mmc_del_affair_type_t )
*   2013-05-15      wangjunli               LTE-Bug00001934     【LTEM3.20.00】GSM辅模式测量收到NCELL_LIST更新消息时对该卡测量列表处理有误
*   2013-05-16      yancheng                LM-Bug00002757      1713双卡：G+G下，次卡DSC后背景搜网，BSIC和读BCCH期间配睡眠未能及时唤醒，导致主卡多次漏寻呼 
*   2013-05-17      yuzhengfeng             LM-Bug00002767      1810双卡: GSM增加一个时隙内出现3个任务的处理 
*   2013-05-20      sunzhiqiang             LM-Bug00002781      1810双卡：在PLMN搜索时更新帧号判断错误 
*   2013-05-20      sunzhiqiang             LM-Bug00002788      1810双卡：FCB成功时不管SB是否配置成功都应该更新睡眠帧号           
*   2013-05-27      wangjunli               LTE-Req00000341     綥TE_V3.20.02联调测试】1761PILOT支持动态开关TH固核电源
*   2013-05-28      wangjunli               LM-Bug00002795      304:一卡从IDLE转CS另一卡MSG_GSM_CS_TDD_EVENT_COMPLTE_IND消息不应该直接拦截
*   2013-06-03      wangjunli               PhyFA-Bug00003987   【LTE】GSM睡眠唤醒状态维护有问题
*   2013-06-05      sunzhiqiang             PhyFA-Bug00003983   SAIC分支下CIR功率值平滑BUFFER，初始化清零
*   2013-06-17      wangjunli               LTE-Bug00002069     【TDM】GSM下TH固核动态关电源方案需增加信号量保护
*   2013-08-29      wangjunli               LM-Enh00001287      1713+1209B：转TRANFER状态时按normal_ccch配置full_ccch接收需要完善避免漏配
*   2013-09-24      wangjunli               LTE-Bug00002422    【L1761MS】[TDM]【ATS挂测】TCH配置下收到残留的89tsFCB接收需要直接丢弃(20130922-02066)
*   2014-02-21      linlan                  Phyfa-Req00001575   V4空间规划修改及SVN与CC代码同步
*   2013-10-15      sunzhiqiang             LM-Bug00002982      一卡TD下CS业务，一卡GSM下IDLE这个场景下不做寻呼冲突检测
*   2013-09-03      sunzhiqiang             LM-Enh00001294      高优先级卡删除低优先级卡部分寻呼帧的场景下应进行寻呼冲突检测
*   2014-02-28      wangjunli               LM-Bug00003093      1813+304：信道切换删除FCB接收时清除FCB标志
*   2014-03-14      linlan                  LM-Enh00001202      GSM transfer下预读邻区系统消息
*   2014-03-17      wuxiaorong              PhyFA-Bug00004202   [TDM]GSM 周期事务预占没有判断是否要mmc
*   2014-03-19      linlan                  LM-Enh00001440      add PS_SIG affire flow process  
*   2014-03-24      xiongjiangjiang         LTE-Bug00002799     【TDM】【LTEM_3.20.10.R10.T01】LOOPBACK消息处理中混用next_fn和next_block_fn ，导致事件重复配置产生死机
*   2014-03-30      gaowu                   PhyFA-Bug00004297   [NGM]调用DD的接口传递帧号参数时super fn一定要初始化0 
*   2014-03-30      gaowu                   PhyFA-Bug00004299   [NGM]初始态不能进gap control 
*   2014-04-03      wuxiaorong              PhyFA-Enh00001777   [NGM] GSM 的gsm_check_single_card_active增强修改
*   2014-04-06      wuxiaorong              PhyFA-Req00001655   [NGM]GSM辅模式测量对480ms周期20ms长的DMO支持
*   2014-04-11      wangjunli               LTE-Enh00000814     【TDM】物理层上报FACCH/F的READY_IND消息帧号确保是BLOCK头
*   2014-04-14      wuxiaorong              PhyFA-Bug00004431   [NGM]L1CC_GSM在gsm_fcp_get_tsntask_latestfn内没有初始化st_tsn_info，导致死机
*   2014-04-17      zhengying               PhyFA-Enh00001823   [NGM]MSG_GSM_HLP_FCP_NEED_RESYNC_FOR_HO_REQ  这条消息原语在v4代码中没有用到，需删除 
*   2014-04-18      gaowu                   PhyFA-Bug00004461   [NGM]L1CC_GSM在gsm_fcp_set_ded_trans_nextfn_bsic_rssi_meas_task函数中st_next_fn_tail赋值过大，导致死机
*   2014-04-23      wangjunli               PhyFA-Enh00001698   GSM临界场景下，default分支输出调试打印，而不是死机
*   2014-04-28      sunzhiqiang             PhyFA-Enh00001647   调整RRBP和BCCH的优先级
*   2014-04-30      wangjunli               LTE-Enh00000846     【TDM】FTA测试提高PDCH TX发送优先级并尝试NC SI提前译码
*   2014-05-05      wuxiaorong              PhyFA-Enh00001877    [NGM]gsm_rtx_event_req局部结构体数组变量改成动态申请和释放
*   2014-05-06      gaowu                   PhyFA-Bug00004496    [NGM]GSM添加伪事件时，拦截条件不正确
*   2014-05-06      gaowu                   PhyFA-Enh00001876    [NGM]GSM配置superfn时，需要将值置为0 
*   2014-05-09      linlan                  PhyFA-Bug00004614    [NMG]transfer下需要根据USF结果清除u16_next_block_tx_flag标志
*   2014-05-09      wuxiaorong              PhyFA-Req00001737    [NGM]gsm_get_tsntask_by_active的最大最小时隙范围修改
*   2014-05-09      wuxiaorong              PhyFA-Enh00001883    [NGM]dd_event_req返回OAL_ FAILURE的下载事件来不及配置修改
*   2014-05-12      linlan                  PhyFA-Bug00004615    [NGM]gsm_fcp_check_tsntask中帧号操作错误 
*   2014-05-12      xiongjiangjiang         PhyFA-Enh00001894    [NGM]对2的幂次求余操作优化
*   2014-05-20      wuxiaorong              PhyFA-Bug00004781   [NGM]GSM的事务预占的超帧号未赋值0出现DD ASSERT: gsm superfn is not 0 
*   2014-05-19      zhengying               PhyFA-Bug00004763    [NGM]gsm_fcp_pre_add_mmc_affair的事务id未填写和u16_rtx_status填写错误 
*   2014-05-26      linlan                  PhyFA-Bug00004837   [NGM]多时隙上行发送APC配置错误 
*   2014-05-29      xiongjiangjiang         PhyFA-Bug00004838    [NGM]gsm_check_other_standby_info错误修改
*   2014-05-30      xiongjiangjiang         PhyFA-Req00001782   [NGM]补充代码中未加段名的函数段定义.
*   2014-06-05      wuxiaorong              PhyFA-Bug00004912   [NGM]GSM的idle睡眠使能事件下载过期修改. 
*   2014-06-06      zhengying               NGM-Bug00000089     [NGM]GSM预占下一个寻呼点的接收需要增加判断flag标志位是否有效 
*   2014-06-13      linlan                  PhyFA-Bug00004976   [NGM]调用DD接口dd_gsm_event_del入参使用错误 
*   2014-06-13      wuxiaorong              PhyFA-Bug00005006   NGM]GSM的FCB测量任务删除u16_running_bitmap/u16_start_bitmap的对应bit清0 
*   2014-06-18      wuxiaorong              PhyFA-Req00001811   [NGM]协议层调用消息创建接口能够把行号和文件号记录到内存控制块头中 .
*   2014-06-18      wuxiaorong              NGM-Bug00000109     [TDM]SDCCH下FCB测量存在问题
*   2014-06-18      linlan                  PhyFA-Bug00005064   [NGM]dedicate状态根据搜网配置表上报冲突指示 
*   2014-06-24      sunzhiqiang             NGM-Bug00000127     [NGM][GSM状态转换时需更新接收MMC时间，以免在下一个状态误判接收异常]
*   2014-06-27      linlan                  PHYFA-Bug00005065   [NGM]PS业务下前景搜网问题修改 rework
*   2014-07-03      linlan                  PHYFA-Bug00005189   [NGM]transfer下前景搜网在BSIC过程中SYNC_BCCH事件没有正确下载
*   2014-07-03      linlan                  PHYFA-Bug00005191   [NGM]transfer下推测PS SIG后又再次进入PS SIG的时候，需要删除前景搜网配置的事件 
*   2014-07-03      wuxiaorong              PhyFA-Bug00005194   [NGM] 异模式idle下的GSM辅模式测量FCB成功的u16_rxtask_type为GSM_GAP_MEAS
*   2014-07-08      gaowu                   PhyFA-Bug00005217   [NGM]l1cc_gsm发送withdraw消息需要判断是否为withdraw_req消息
*   2014-07-08      gaowu                   PhyFA-Bug00005230   [NGM]GSM连接态下，L1CC_GSM在分配出的空闲窗内，配置了RSSI测量
*   2014-07-15      gaowu                   PhyFA-Bug00005326   [NGM]T+G双待，TDS回收GAP时GSM未正确填写CNF消息.
*   2014-07-17      wuxiaorong              PhyFA-Bug00005346   [NGM]G+T下G在nocell，T待机时出现0x13f00
*   2014-07-18      linlan                  NGM-Bug00000195     [NGM]SDCCH转TCH删除FCB时要遍历meas_sync_task配置表
*   2014-07-21      wuxiaorong              NGM-Bug00000206     [TDM]GSM模式转走后的FCP_DEL_TASK_NORMAL拖尾任务删除需要判断待激活状态
*   2014-07-23      linlan                  NGM-Bug00000213     [NGM]一待业务下没有测量时，从业务转IDLE后需要清除另一待的同步配置表
*   2014-07-23      gaowu                   PhyFA-Req00001899   [NGM]TDS保护间隔变大导致分配给GSM辅模式的窗变小，GSM需要修订接收数据长度
*   2014-07-25      wuxiaorong              PhyFA-Bug00005426   [NGM]gsm辅模式状态下的sbsearch流程的任务配置需要变更.
*   2014-07-30      wuxiaorong              PhyFA-Bug00005464   [NGM]GSM的SB search当帧配当帧睡眠控制latest_rtx_frame_pos设置有问题 
*   2014-08-05      sunzhiqiang             NGM-Bug00000256     【TDM】转DEDICATED状态时需要将解交织BUFF刷CASHE 
*   2014-08-05      linlan                  PhyFA-Bug00005514   [NGM]CS状态下应该先判搜网测量状态再配置主辅模式测量
*   2014-08-05      wuxiaorong              PhyFA-Req00001925   [NGM]T业务下G测量在当前接收未完成前就需要帧中断下次接收任务 .
*   2014-08-12      xiongjiangjiang         PhyFA-Bug00005562   [NGM]GSM辅模式时需要增加在假的帧中断下用下一次的上报时间点来更新最近收发帧
*   2014-08-12      wuxiaorong              PhyFA-Enh00002131   [NGM]L1CC_GSM需要修改局部变量大的函数
*   2014-08-05      linlan                  NGM-Bug00000351     【NGM】【TDM】1643异常唤醒转业务状态后需要唤醒4210
*   2014-08-22      sunzhiqiang             NGM-Bug00000370     【NGM】【TDM】辅模式测量存在的情况下睡眠唤醒帧号更新应该在当前状态任务全部处理完后与辅模式测量上报点进行比较。 
*   2014-08-22      linlan                  NGM-Bug00000358     【TDM】GSM IDLE下测量FCB没有配置的时候不能清测量配置表的有效标置
*   2014-08-26      linlan                  NGM-Bug00000400     【NGM】系统间切换需要调整本地定时到网络定时
*   2014-09-01      wuxiaorong              PhyFA-Bug00005665   [NGM]GSM的RBSIC点唤醒后没有配任务出现空唤醒
*   2014-09-04      gaowu                   PhyFA-Bug00005626   [NGM]gsm辅模式在长GAP窗内配置FCB不能复用模式内的TSNTASK_NC_CONN_FCB_RX 
*   2014-09-11      gaowu                   NGM-Bug00000456     [NGM]GSM IDLE GAP回收时记录回收生效时间需要转换成MMC时间
*   2014-09-11      linlan                  NGM-Bug00000383     【NGM】【TDM】双待在TRANFER下NC_BCCH频点个数超过6个导致MMC事务池满 
*   2014-09-11      linlan                  NGM-Bug00000499     [NGM]GSM在异模式业务状态下更新了lastest_fn导致另一待前景搜网FCB配置不下去
*   2014-09-20      wuxiaorong              NGM-Bug00000585     [NGM]G辅模SB Search跨帧配置出现N帧配N+2帧导致后续N+1帧配N+2帧时把先前配置任务删除
*   2014-09-22      wuxiaorong              PhyFA-Req00001980   [NGM]功耗测试GSM的非空寻呼唤醒后的RSSI接收配置限定在第1和第2帧寻呼接收之间.
*   2014-09-22      gaowu                   NGM-Bug00000586     [NGM]l1cc_gsm需要将伪事件的配置按照非周期事件来配置
*   2014-09-25      wuxiaorong              NGM-Bug00000627     【NGM】【TDM】GSM的帧中断在GAP回收的CNF时间点前不配任务时需要更新stp_latest_rtx_frame_pos->u32_fn为下下帧保证不睡眠
*   2014-09-16      linlan                  NGM-Bug00000620     【NGM】降低TRANSFER_BCCH优先级不需要判断双待
*   2014-09-25      guxiaobo                NGM-Bug00000496    [TDM]T预读G SI时，申请到GAP后填写SI的bitmap有误，导致后续未配置SI接收 
*   2014-09-28      linlan                  NGM-Bug00000643     【NGM】【TDM】高层消息下来的时候更新latest_fn以便重新根据配置表配置睡眠唤醒时间 
*   2014-10-09      wuxiaorong              NGM-Bug00000677     [NGM] 优先T搜时在G下进NOCELL，G没有转状态到GSM_NOCELL
*   2014-10-10      linlan                  NGM-Enh00000122     【TDM】睡眠唤醒后启动4210时间的配置和判断挪到事件下载之前 
*   2014-10-20      gaowu                   NGM-Bug00000617     [NGM]【TDM】异模式回收GAP删除GAP内时隙任务时由于GAP配置表无效导致取不到事件长度
*   2014-10-22      guxiaobo                NGM-Bug00000798     【NGM】[tdm]T连接态测GSM，GSM配置RSSI的空闲窗查找有误
******************************************************************************************************************************************************/
#undef THIS_FILE_NAME_ID
#define THIS_FILE_NAME_ID L1CC_GSM_FCP_FILE_ID


/******************************************************************************
*  INCLUDE FILES
******************************************************************************/

#include "l1cc_gsm_common_all.h"
#include "l1cc_gsm_fcp.h"/* 必须在ps.h之前 */
#include "l1cc_gsm_hlp.h"
#include "l1cc_gsm_cellsearch.h"
#include "l1cc_gsm_dl_ccch.h"
#include "l1cc_gsm_ul_ccch.h"
#include "l1cc_gsm_bcch.h"
#include "l1cc_gsm_packet_idle.h"
#include "l1cc_gsm_packet_transfer.h"
#include "l1cc_gsm_packet_meas.h"
#include "l1cc_gsm_circuitswitch.h"
#include "l1cc_gsm_circuitswitch_ho.h"
#include "l1cc_gsm_meas.h"
#include "l1cc_gsm_timeinfo.h"
#include "l1cc_gsm_amt.h"
#include "l1cc_gsm_interrat_common.h"
#include "l1cc_gsm_eq_mc.h"
#include "l1cc_gsm_cc_dec_mc.h"
#include "l1cc_gsm_cc_enc_mc.h"
#include "l1cc_gsm_gap_mng_slave.h"
#include "l1cc_gsm_gap_mng_master.h"
#include "l1cc_gsm_fc.h"
#include "l1cc_gsm_common_func.h"
#include "l1cc_gsm_pecker.h"
#include "l1cc_gsm_sleep_ctrl.h"
#include "l1cc_gsm_heartbeat.h"
/******************************************************************************
** EXTERN VARIABLES DECLEARATION                                                                                                                                             
******************************************************************************/

/******************************************************************************                                                                                              
*  MACRO DEFINITION                                                                                                                                                          
******************************************************************************/

/* FIX PhyFA-Req00001525 BEGIN  2013-11-18 : wuxiaorong */
#define  NO_RTX_FLAG                   (0)
#define  SPECIAL_EVENT_FLAG            (1)
#define  NORMAL_EVENT_FLAG             (2)
#define  CONTINUE_RX_GSM_FLAG          (4)
#define  CONTINUE_TX_GSM_FLAG          (5)
#define  EVENT_INFO_TABLE_NUM          (10)
#define  MAX_PARSE_GSM_SLOT_NUM        (MAX_GSM_SLOT_NUM*2)
#define  GSM_NULL_CHANNEL_TYPE         (0xFF)        
/* FIX PhyFA-Req00001525 END  2013-11-18 : wuxiaorong */


                   
/******************************************************************************
*  TYPE DEFINITION
******************************************************************************/

typedef struct gsm_del_conflict_tstask_info_tag
{
    UINT16 u16_del_cnt;
    UINT16 u16_reserved;
    UINT16 u16_del_tstask[32];
    SINT16 s16_del_tstask_tsn[32];
    UINT32 u32_del_tstask_fn[32];
}gsm_del_conflict_tstask_info_t;



typedef struct gsm_rtx_overlap_gap_info_tag
{
    SINT32 s32_gsm_rx2gsm_rx_overlap_gap;/* unit: qbit*/
    SINT32 s32_gsm_rx2gsm_tx_overlap_gap;/* unit: qbit*/

    SINT32 s32_gsm_tx2gsm_rx_overlap_gap;/* unit: qbit*/
    SINT32 s32_gsm_tx2gsm_tx_overlap_gap;/* unit: qbit*/
    
    SINT32 s32_gsm_rtx2gsm_rtx_overlap_gap; /* max{s32_gsm_rx2gsm_rx_overlap_gap,s32_gsm_rx2gsm_tx_overlap_gap,s32_gsm_tx2gsm_rx_overlap_gap,s32_gsm_tx2gsm_tx_overlap_gap}*/

    #if defined(TDS_MODE_SWITCH)
    SINT32 s32_gsm_rx2tds_rx_overlap_gap;/* unit: qbit*/
    SINT32 s32_gsm_tx2tds_rx_overlap_gap;/* unit: qbit*/
    SINT32 s32_tds_rx2gsm_rx_overlap_gap;/* unit: qbit*/
    SINT32 s32_tds_rx2gsm_tx_overlap_gap;/* unit: qbit*/
    #endif
    #if defined(WCDMA_MODE_SWITCH)
    SINT32 s32_gsm_rx2wcdma_rx_overlap_gap;/* unit: qbit*/
    SINT32 s32_gsm_tx2wcdma_rx_overlap_gap;/* unit: qbit*/
    SINT32 s32_wcdma_rx2gsm_rx_overlap_gap;/* unit: qbit*/
    SINT32 s32_wcdma_rx2gsm_tx_overlap_gap;/* unit: qbit*/
    #endif
    
    #if defined(LTE_MODE_SWITCH)
    SINT32 s32_gsm_rx2lte_rx_overlap_gap;/* unit: qbit*/
    SINT32 s32_gsm_tx2lte_rx_overlap_gap;/* unit: qbit*/
    SINT32 s32_lte_rx2gsm_rx_overlap_gap;/* unit: qbit*/
    SINT32 s32_lte_rx2gsm_tx_overlap_gap;/* unit: qbit*/
    #endif
}gsm_rtx_overlap_gap_info_t;

typedef struct gsm_download_rtx_event_info_tag
{
    gsm_main_frametask_t *stp_next_fn_frame_task; 
    gsm_main_frametask_t *stp_next2_fn_frame_task; 
    UINT16_PTR            u16p_next_fn_bitmap;
    UINT16_PTR            u16p_next2_fn_bitmap;
}gsm_download_rtx_event_info_t;
typedef struct gsm_conflict_and_delete_param_save_tag
{
    gsm_del_conflict_tstask_info_t gsm_del_conflict_task_info;

    UINT32     u32_add_fn;
    UINT16     u16_add_tsntask;
    UINT16     u16_add_tsn;
    
    SINT16     s16_rtx_offset;    
    UINT16     u16_tmp_tsn;/*后面这几个变量暂时没用，可以把删除过程中变量备份一下*/
    UINT16     s16_occupied_tsn0;
    UINT16     s16_occupied_tsn1;   
}gsm_conflict_and_delete_param_save_t;



/******************************************************************************
* GLOBAL VAR DEFINITION
******************************************************************************/
/* FIX PhyFA-Enh00001647 BEGIN 2014-04-28 : sunzhiqiang */
/* FIX LTE-Enh00000846 BEGIN  2014-04-29: wangjunli */
/* FIX PhyFA-Bug00005626 BEGIN 2014-09-04 : gaowu */
/* FIX NGM-Bug00000454 BEGIN 2014-09-12 : gaowu */
/* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
/* FIX NGM-Bug00000496 BEGIN 2014-09-25 : guxiaobo */
/* modify TSNTASK_NULL_73 */
L1CC_GSM_DRAM_DATA_CONST_SECTION
CONST gsm_tstask_attribute_t C_ST_TSTASK_ATTRIBUTE_TBL[GSM_TSTASK_NUM]={
        /*u16_tstask*/                         /*u16_priority*/                        /*addtsntask cb*/                      /*get_tsn_info*/                    /* get_pos_info*/                   /*u16_event_type*/      /*u16_rxtask_type*/     /*u16_rtx_status*/   /*u16_repeat_frms*/    /*u16_rx_len_ind*/           /*get s32_rtx_len(qbit)*/    /*e_affair_id*/                                                                                                 
/*0 */  {TSNTASK_NULL                         ,GSM_TSNTASK_PRIOR_ZERO                 ,NULL_PTR                               ,NULL_PTR                           ,NULL_PTR                             ,(UINT16)0              ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },                                                                               
/*1 */  {TSNTASK_SCAN_RF                      ,GSM_TSNTASK_MEAS_RX_PRIOR              ,gsm_add_cellsearch_rssi_rx_tsntask_cb  ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_FG_CELLSEARCH_RSSI     },                                                                       
/*2 */  {TSNTASK_FCB                          ,GSM_TSNTASK_MEAS_RX_PRIOR              ,gsm_add_cellsearch_fb_rx_tsntask_cb    ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_SYNC_NSLOT      ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_FG_CELLSEARCH_FCCH_SCH },                                                                       
/*3 */  {TSNTASK_SB                           ,GSM_TSNTASK_MEAS_RX_PRIOR              ,gsm_add_cellsearch_sb_rx_tsntask_cb    ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_FG_CELLSEARCH_FCCH_SCH },                                                                       
/*4 */  {TSNTASK_BCCH_RX                      ,GSM_TSNTASK_BCCH_PRIOR                 ,gsm_add_bcch_rx_tsntask_cb             ,gsm_get_bcch_rx_tsn_info           ,gsm_get_bcch_rx_pos_info             ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_SC_BCCH           },                                                                       
/*5 */  {TSNTASK_PBCCH_RX                     ,GSM_TSNTASK_BCCH_PRIOR                 ,gsm_add_pbcch_rx_tsntask_cb            ,gsm_get_pbcch_rx_tsn_info          ,gsm_get_pbcch_rx_pos_info            ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },                                                                       
/*6 */  {TSNTASK_CCCH_RX                      ,GSM_TSNTASK_CCCH_PRIOR                 ,gsm_add_ccch_rx_tsntask_cb             ,gsm_get_ccch_rx_tsn_info           ,gsm_get_ccch_rx_pos_info             ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CCCH                   },                                                                      
/*7 */  {TSNTASK_CCCH_CS_RX                   ,GSM_TSNTASK_RACH_TX_PRIOR              ,gsm_add_ccch_rx_tsntask_cb             ,gsm_get_ccch_rx_tsn_info           ,gsm_get_ccch_rx_pos_info             ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_RACH_CCCH              },                                                                       
/*8 */  {TSNTASK_CCCH_PCH_RX                  ,GSM_TSNTASK_PCH_PRIOR                  ,gsm_add_ccch_rx_tsntask_cb             ,gsm_get_ccch_rx_tsn_info           ,gsm_get_ccch_rx_pos_info             ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_PCH                    },                                                                       
/*9 */  {TSNTASK_TRANSFER_CCCH_RX             ,GSM_TRANSFER_CCCH_RX_PRIOR             ,gsm_add_ccch_rx_tsntask_cb             ,gsm_get_ccch_rx_tsn_info           ,gsm_get_ccch_rx_pos_info             ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_PCH                    },                                                                       
/*10*/  {TSNTASK_PCCCH_RX                     ,GSM_TSNTASK_CCCH_PRIOR                 ,gsm_add_pccch_rx_tsntask_cb            ,gsm_get_pccch_rx_tsn_info          ,gsm_get_pccch_rx_pos_info            ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },                                                                       
/*11*/  {TSNTASK_NC0_BCCH_RX                  ,GSM_TSNTASK_NC0_RX_PRIOR               ,gsm_add_nc_bcch_rx_tsntask_cb          ,gsm_get_nc_bcch_rx_tsn_info        ,gsm_get_nc_bcch_rx_pos_info          ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_NC0_BCCH          },                                                                       
/*12*/  {TSNTASK_NC1_BCCH_RX                  ,GSM_TSNTASK_NC1_RX_PRIOR               ,gsm_add_nc_bcch_rx_tsntask_cb          ,gsm_get_nc_bcch_rx_tsn_info        ,gsm_get_nc_bcch_rx_pos_info          ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_NC1_BCCH         },                                                                       
/*13*/  {TSNTASK_NC2_BCCH_RX                  ,GSM_TSNTASK_NC2_RX_PRIOR               ,gsm_add_nc_bcch_rx_tsntask_cb          ,gsm_get_nc_bcch_rx_tsn_info        ,gsm_get_nc_bcch_rx_pos_info          ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_NC2_BCCH          },                                                                       
/*14*/  {TSNTASK_NC3_BCCH_RX                  ,GSM_TSNTASK_NC3_RX_PRIOR               ,gsm_add_nc_bcch_rx_tsntask_cb          ,gsm_get_nc_bcch_rx_tsn_info        ,gsm_get_nc_bcch_rx_pos_info          ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_NC3_BCCH          },                                                                       
/*15*/  {TSNTASK_NC4_BCCH_RX                  ,GSM_TSNTASK_NC4_RX_PRIOR               ,gsm_add_nc_bcch_rx_tsntask_cb          ,gsm_get_nc_bcch_rx_tsn_info        ,gsm_get_nc_bcch_rx_pos_info          ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_NC4_BCCH         },                                                                       
/*16*/  {TSNTASK_NC5_BCCH_RX                  ,GSM_TSNTASK_NC5_RX_PRIOR               ,gsm_add_nc_bcch_rx_tsntask_cb          ,gsm_get_nc_bcch_rx_tsn_info        ,gsm_get_nc_bcch_rx_pos_info          ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_NC5_BCCH           },                                                                       
/*17*/  {TSNTASK_NC6_BCCH_RX                  ,GSM_TSNTASK_NC6_RX_PRIOR               ,gsm_add_nc_bcch_rx_tsntask_cb          ,gsm_get_nc_bcch_rx_tsn_info        ,gsm_get_nc_bcch_rx_pos_info          ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },                                                                       
/*18*/  {TSNTASK_TC_BCCH_RX                   ,GSM_TSNTASK_TC_RX_PRIOR                ,gsm_add_tc_bcch_rx_tsntask_cb          ,gsm_get_tc_bcch_rx_tsn_info        ,gsm_get_tc_bcch_rx_pos_info          ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_TC_BCCH           },                                                                       
/*19*/  {TSNTASK_TC_SEARCH_BCCH_RX            ,GSM_TSNTASK_TC_RX_PRIOR                ,gsm_add_tc_search_bcch_rx_tsntask_cb   ,gsm_get_tc_search_bcch_rx_tsn_info ,gsm_get_tc_search_bcch_rx_pos_info   ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_TC_BCCH           },                                                                       
/*20*/  {TSNTASK_RACH_TX                      ,GSM_TSNTASK_RACH_TX_PRIOR              ,gsm_add_rach_tx_tsntask_cb             ,NULL_PTR                           ,NULL_PTR                             ,TX_GSM_AB              ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_RACH                   },                                                                       
/*21*/  {TSNTASK_PRACH_TX                     ,GSM_TSNTASK_RACH_TX_PRIOR              ,gsm_add_prach_tx_tsntask_cb            ,NULL_PTR                           ,NULL_PTR                             ,TX_GSM_AB              ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_RACH                   },                                                                       
/*22*/  {TSNTASK_SDCCH_RX                     ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_sdcch_rx_tsntask_cb            ,gsm_get_sdcch_rx_tsn_info          ,gsm_get_sdcch_rx_pos_info            ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*23*/  {TSNTASK_SDCCH_TX                     ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_sdcch_tx_tsntask_cb            ,gsm_get_sdcch_tx_tsn_info          ,gsm_get_sdcch_tx_pos_info            ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*24*/  {TSNTASK_SACCH_RX                     ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_sacch_rx_tsntask_cb            ,gsm_get_sacch_rx_tsn_info          ,gsm_get_sacch_rx_pos_info            ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*25*/  {TSNTASK_SACCH_H_RX                   ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,NULL_PTR                               ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*26*/  {TSNTASK_SACCH_TX                     ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_sacch_tx_tsntask_cb            ,gsm_get_sacch_tx_tsn_info          ,gsm_get_sacch_tx_pos_info            ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*27*/  {TSNTASK_SACCH_H_TX                   ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,NULL_PTR                               ,NULL_PTR                           ,NULL_PTR                             ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*28*/  {TSNTASK_TCH_RX                       ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_tch_rx_tsntask_cb              ,gsm_get_tch_rx_tsn_info            ,gsm_get_tch_rx_pos_info              ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*29*/  {TSNTASK_TCH_H_RX                     ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_tch_h_rx_tsntask_cb            ,gsm_get_tch_h_rx_tsn_info          ,gsm_get_tch_h_rx_pos_info            ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*30*/  {TSNTASK_TCH_F96_RX                   ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_tch_f96_rx_tsntask_cb          ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*31*/  {TSNTASK_TCH_F144_RX                  ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_tch_f144_rx_tsntask_cb         ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*32*/  {TSNTASK_TCH_TX                       ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_tch_tx_tsntask_cb              ,gsm_get_tch_tx_tsn_info            ,gsm_get_tch_tx_pos_info              ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*33*/  {TSNTASK_TCH_H_TX                     ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_tch_h_tx_tsntask_cb            ,gsm_get_tch_h_tx_tsn_info          ,gsm_get_tch_h_tx_pos_info            ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*34*/  {TSNTASK_TCH_F96_TX                   ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_tch_f96_tx_tsntask_cb          ,gsm_get_tch_f96_tx_tsn_info        ,gsm_get_tch_f96_tx_pos_info          ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*35*/  {TSNTASK_TCH_F144_TX                  ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_tch_f144_tx_tsntask_cb         ,gsm_get_tch_f144_tx_tsn_info       ,gsm_get_tch_f144_tx_pos_info         ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*36*/  {TSNTASK_FACCH_TX                     ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_facch_tx_tsntask_cb            ,gsm_get_facch_tx_tsn_info          ,gsm_get_facch_tx_pos_info            ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*37*/  {TSNTASK_FACCH_H_TX                   ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_facch_h_tx_tsntask_cb          ,gsm_get_facch_h_tx_tsn_info        ,gsm_get_facch_h_tx_pos_info          ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)8             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*38*/  {TSNTASK_HANDOVER_RX                  ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_ho_rx_tsntask_cb               ,gsm_get_ho_rx_tsn_info             ,gsm_get_ho_rx_pos_info               ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*39*/  {TSNTASK_HANDOVER_TX                  ,GSM_TSNTASK_DEDI_RTX_PRIOR             ,gsm_add_ho_tx_tsntask_cb               ,NULL_PTR                           ,NULL_PTR                             ,TX_GSM_AB              ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*40*/  {TSNTASK_PDTCH_TX                     ,GSM_TSNTASK_PDCH_TX_PRIOR              ,gsm_add_pdtch_tx_tsntask_cb            ,gsm_get_pdtch_tx_tsn_info          ,gsm_get_pdtch_tx_pos_info            ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_PS_CNNT                },                                                                       
/*41*/  {TSNTASK_PDTCH_RX                     ,GSM_TSNTASK_PDCH_RX_PRIOR              ,gsm_add_pdtch_rx_tsntask_cb            ,gsm_get_pdtch_rx_tsn_info          ,gsm_get_pdtch_rx_pos_info            ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_PS_CNNT                },                                                                       
/*42*/  {TSNTASK_SINGLEBLOCK_TX               ,GSM_TSNTASK_SINGLEBLOCK_TX_PRIOR       ,gsm_add_snglblk_tx_tsntask_cb          ,NULL_PTR                           ,NULL_PTR                             ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_PS_CNNT                },                                                                       
/*43*/  {TSNTASK_RRBP_TX0                     ,GSM_TSNTASK_RRBP_TX_PRIOR              ,gsm_add_rrbp_tx_tsntask_cb             ,NULL_PTR                           ,NULL_PTR                             ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_PS_CNNT                },                                                                       
/*44*/  {TSNTASK_PTCCH_TX                     ,GSM_TSNTASK_PTCCH_RTX_PRIOR            ,gsm_add_ptcch_tx_tsntask_cb            ,gsm_get_ptcch_tx_tsn_info          ,gsm_get_ptcch_tx_pos_info            ,TX_GSM_AB              ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_PS_CNNT                },                                                                       
/*45*/  {TSNTASK_PTCCH_RX                     ,GSM_TSNTASK_PTCCH_RTX_PRIOR            ,gsm_add_ptcch_rx_tsntask_cb            ,gsm_get_ptcch_rx_tsn_info          ,gsm_get_ptcch_rx_pos_info            ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_PS_CNNT                },                                                                       
/*46*/  {TSNTASK_MEAS_RSSI                    ,GSM_TSNTASK_MEAS_RX_PRIOR              ,NULL_PTR                               ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },                                                                       
/*47*/  {TSNTASK_CBCH_RX                      ,GSM_TSNTASK_CBCH_RX_PRIOR              ,gsm_add_cbch_tx_tsntask_cb             ,gsm_get_cbch_rx_tsn_info           ,NULL_PTR                             ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CBCH                   },                                                                       
/*48*/  {TSNTASK_BG_RSSI_RX                   ,GSM_TSNTASK_BG_RSSI_RX_PRIOR           ,gsm_add_cellsearch_rssi_rx_tsntask_cb  ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_BG_CELLSEARCH_RSSI     },                                                                       
/*49*/  {TSNTASK_BG_FCB_RX                    ,GSM_TSNTASK_BG_FCB_RX_PRIOR            ,gsm_add_cellsearch_fb_rx_tsntask_cb    ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_SYNC_NSLOT      ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_BG_CELLSEARCH_FCCH_SCH },                                                                       
/*50*/  {TSNTASK_BG_BSIC_RX                   ,GSM_TSNTASK_BG_BSIC_RX_PRIOR           ,gsm_add_cellsearch_sb_rx_tsntask_cb    ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_BG_CELLSEARCH_FCCH_SCH },                                                                       
/*51*/  {TSNTASK_BG_TC0_BCCH_RX               ,GSM_TSNTASK_BG_TC_BCCH_RX_PRIOR        ,gsm_add_bg_tc_bcch_rx_tsntask_cb       ,gsm_get_bg_tc_bcch_rx_tsn_info     ,gsm_get_bg_tc_bcch_rx_pos_info       ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_TC_BCCH           },
/*52*/  {TSNTASK_BG_TC1_BCCH_RX               ,GSM_TSNTASK_BG_TC_BCCH_RX_PRIOR        ,gsm_add_bg_tc_bcch_rx_tsntask_cb       ,gsm_get_bg_tc_bcch_rx_tsn_info     ,gsm_get_bg_tc_bcch_rx_pos_info       ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_TC_BCCH           },
/*53*/  {TSNTASK_BG_TC2_BCCH_RX               ,GSM_TSNTASK_BG_TC_BCCH_RX_PRIOR        ,gsm_add_bg_tc_bcch_rx_tsntask_cb       ,gsm_get_bg_tc_bcch_rx_tsn_info     ,gsm_get_bg_tc_bcch_rx_pos_info       ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_TC_BCCH           },
/*54*/  {TSNTASK_BG_TC3_BCCH_RX               ,GSM_TSNTASK_BG_TC_BCCH_RX_PRIOR        ,gsm_add_bg_tc_bcch_rx_tsntask_cb       ,gsm_get_bg_tc_bcch_rx_tsn_info     ,gsm_get_bg_tc_bcch_rx_pos_info       ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_TC_BCCH           },
/*55*/  {TSNTASK_BG_TC4_BCCH_RX               ,GSM_TSNTASK_BG_TC_BCCH_RX_PRIOR        ,gsm_add_bg_tc_bcch_rx_tsntask_cb       ,gsm_get_bg_tc_bcch_rx_tsn_info     ,gsm_get_bg_tc_bcch_rx_pos_info       ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_TC_BCCH           },
/*56*/  {TSNTASK_BG_TC5_BCCH_RX               ,GSM_TSNTASK_BG_TC_BCCH_RX_PRIOR        ,gsm_add_bg_tc_bcch_rx_tsntask_cb       ,gsm_get_bg_tc_bcch_rx_tsn_info     ,gsm_get_bg_tc_bcch_rx_pos_info       ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_TC_BCCH           },
/*57*/  {TSNTASK_SYNC_CELL0_BCCH_RX           ,GSM_TSNTASK_SYNC_BCCH_RX_PRIOR         ,gsm_add_sync_bcch_rx_tsntask_cb        ,gsm_get_sync_bcch_rx_tsn_info      ,gsm_get_sync_bcch_rx_pos_info        ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_SC_BCCH           },
/*58*/  {TSNTASK_SYNC_CELL1_BCCH_RX           ,GSM_TSNTASK_SYNC_BCCH_RX_PRIOR         ,gsm_add_sync_bcch_rx_tsntask_cb        ,gsm_get_sync_bcch_rx_tsn_info      ,gsm_get_sync_bcch_rx_pos_info        ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_SC_BCCH           },
/*59*/  {TSNTASK_SYNC_CELL2_BCCH_RX           ,GSM_TSNTASK_SYNC_BCCH_RX_PRIOR         ,gsm_add_sync_bcch_rx_tsntask_cb        ,gsm_get_sync_bcch_rx_tsn_info      ,gsm_get_sync_bcch_rx_pos_info        ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_SC_BCCH           },
/*60*/  {TSNTASK_SYNC_CELL3_BCCH_RX           ,GSM_TSNTASK_SYNC_BCCH_RX_PRIOR         ,gsm_add_sync_bcch_rx_tsntask_cb        ,gsm_get_sync_bcch_rx_tsn_info      ,gsm_get_sync_bcch_rx_pos_info        ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_SC_BCCH           },
/*61*/  {TSNTASK_SYNC_CELL4_BCCH_RX           ,GSM_TSNTASK_SYNC_BCCH_RX_PRIOR         ,gsm_add_sync_bcch_rx_tsntask_cb        ,gsm_get_sync_bcch_rx_tsn_info      ,gsm_get_sync_bcch_rx_pos_info        ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_SC_BCCH           },
/*62*/  {TSNTASK_SYNC_CELL5_BCCH_RX           ,GSM_TSNTASK_SYNC_BCCH_RX_PRIOR         ,gsm_add_sync_bcch_rx_tsntask_cb        ,gsm_get_sync_bcch_rx_tsn_info      ,gsm_get_sync_bcch_rx_pos_info        ,RX_GSM_NSLOT           ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_SC_BCCH           },
/*63*/  {TSNTASK_NC_INIT_BSIC_RX              ,GSM_TSNTASK_NC_INIT_BSIC_RX_PRIOR      ,gsm_add_nc_meas_sb_rx_tsntask_cb       ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_MEAS               ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_MEAS_SCH          },                                                                       
/*64*/  {TSNTASK_NC_RE_BSIC_RX                ,GSM_TSNTASK_NC_RE_BSIC_RX_PRIOR        ,gsm_add_nc_meas_sb_rx_tsntask_cb       ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_MEAS               ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_MEAS_SCH          },                                                                       
/*65*/  {TSNTASK_NC_IDLE_FCB_RX               ,GSM_TSNTASK_NC_IDLE_FCB_RX_PRIOR       ,gsm_add_nc_meas_idle_fb_rx_tsntask_cb  ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_SYNC_NSLOT      ,GSM_MEAS               ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_MEAS_FCCH         },                                                                       
/*66*/  {TSNTASK_NC_RSSI_RX                   ,GSM_TSNTASK_NC_RSSI_RX_PRIOR           ,gsm_add_nc_meas_rssi_rx_tsntask_cb     ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_MEAS               ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_MEAS_RSSI         },                                                                       
/*67*/  {TSNTASK_NC_CONN_FCB_RX               ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_nc_meas_conn_fb_rx_tsntask_cb  ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_MEAS               ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*68*/  {TSNTASK_GPRS_INTF_MEAS_RX            ,GSM_TSNTASK_GPRS_INTF_MEAS_RX_PRIOR    ,gsm_add_gprs_intf_meas_rx_tsntask_cb   ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_INTFMEAS           ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_RSSI         },                                                                       
/*69*/  {TSNTASK_NC_CONN_INIT_BSIC_RX         ,GSM_TSNTASK_NC_CONN_INIT_BSIC_RX_PRIOR ,gsm_add_nc_meas_sb_rx_tsntask_cb       ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_MEAS               ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*70*/  {TSNTASK_FAKEGAP_NC_RE_BSIC_RX        ,GSM_TSNTASK_NC_RE_BSIC_RX_PRIOR        ,gsm_add_nc_meas_sb_rx_tsntask_cb       ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_MEAS               ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_FAKEGAP_MEAS_SCH       },                                                                      
/*71*/  {TSNTASK_FAKEGAP_NC_RSSI_RX           ,GSM_TSNTASK_NC_RSSI_RX_PRIOR           ,gsm_add_nc_meas_rssi_rx_tsntask_cb     ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_MEAS               ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_FAKEGAP_MEAS_RSSI      },                                                                       
/*72*/  {TSNTASK_TRANSFER_NC_BCCH_RX          ,GSM_TSNTASK_TRANSFER_NC_BCCH_PRIOR     ,gsm_add_transfer_nc_bcch_tsntask_cb   ,gsm_get_transfer_nc_bcch_rx_tsn_info,gsm_get_transfer_nc_bcch_rx_pos_info ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_TRANSFER_NC_BCCH       },                                                                       
/*73*/  {TSNTASK_TRANSFER_BCCH_RX             ,GSM_TSNTASK_TRANSFER_BCCH_PRIOR        ,gsm_add_bcch_rx_tsntask_cb             ,gsm_get_bcch_rx_tsn_info           ,gsm_get_bcch_rx_pos_info             ,RX_GSM_NSLOT           ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_SC_BCCH           },                                                                       
/*74*/  {TSNTASK_HO_FCB                       ,GSM_TSNTASK_HO_FCB_BSIC_RX_PRIOR       ,gsm_add_cellsearch_fb_rx_tsntask_cb    ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_SYNC_NSLOT      ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*75*/  {TSNTASK_HO_SB                        ,GSM_TSNTASK_HO_FCB_BSIC_RX_PRIOR       ,gsm_add_cellsearch_sb_rx_tsntask_cb    ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_CELLSEARCH         ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CS_CNNT                },                                                                       
/*76*/  {TSNTASK_MANU_SB                      ,GSM_TSNTASK_MEAS_RX_PRIOR              ,gsm_add_amt_sb_rx_tsntask_cb           ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_AMT                ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },                                                                       
/*77*/  {TSNTASK_MANU_FCB                     ,GSM_TSNTASK_MANU_FCB_PRIOR             ,gsm_add_amt_fb_rx_tsntask_cb           ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_SYNC_NSLOT      ,GSM_AMT                ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },                                                                       
/*78*/  {TSNTASK_MANU_TX                      ,GSM_TSNTASK_MANU_TX_PRIOR              ,gsm_add_amt_tx_tsntask_cb              ,NULL_PTR                           ,NULL_PTR                             ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },                                                                       
/*79*/  {TSNTASK_MANU_RX_RSSI                 ,GSM_TSNTASK_MANU_RX_RSSI_PRIOR         ,gsm_add_amt_rssi_rx_tsntask_cb         ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_AMT                ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },                                                                       
/*80*/  {TSNTASK_NC_CONN_RRTL_FCB_RX          ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_nc_meas_conn_fb_rx_tsntask_cb  ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_AMT                ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },
/*81*/  {TSNTASK_PDTCH_FTA_TX                 ,GSM_TSNTASK_PDCH_FTA_TX_PRIOR          ,gsm_add_pdtch_tx_tsntask_cb            ,gsm_get_pdtch_tx_tsn_info          ,gsm_get_pdtch_tx_pos_info            ,TX_GSM_GMSK_NB         ,(UINT16)0              ,GSM_TX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_PS_CNNT                },
/*82*/  {TSNTASK_G_DED_12SLOT_NC_SYNC_RX      ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_nc_meas_conn_fb_rx_tsntask_cb  ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_MEAS               ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*83*/  {TSNTASK_L_CNNT_06MS_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*84*/  {TSNTASK_IRAT_CNNT_9SLOT_FCB_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_nc_meas_conn_fb_rx_tsntask_cb  ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                        
/*85*/  {TSNTASK_T_TS0_ONLY_NC_SYNC_RX        ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_tds_gap_rx_len_ind   ,gsm_get_tds_gap_rtx_len    ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*86*/  {TSNTASK_T_TS0_1SLOT_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_tds_gap_rx_len_ind   ,gsm_get_tds_gap_rtx_len    ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*87*/  {TSNTASK_T_TS0_2SLOT_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_tds_gap_rx_len_ind   ,gsm_get_tds_gap_rtx_len    ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*88*/  {TSNTASK_T_TSN_2SLOT_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_tds_gap_rx_len_ind   ,gsm_get_tds_gap_rtx_len    ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*89*/  {TSNTASK_T_TSN_3SLOT_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_tds_gap_rx_len_ind   ,gsm_get_tds_gap_rtx_len    ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                        
/*90*/  {TSNTASK_T_INTERVAL_1_NC_SYNC_RX      ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                        
/*91*/  {TSNTASK_T_INTERVAL_2_NC_SYNC_RX      ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                        
/*92*/  {TSNTASK_T_DMO_480_20MS_NC_SYNC_RX    ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_SYNC_NSLOT      ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                        
/*93*/  {TSNTASK_IRAT_CNNT_TC_BCCH_RX         ,GSM_TSNTASK_TC_RX_PRIOR                ,gsm_add_tc_bcch_rx_tsntask_cb           ,gsm_get_irat_cnnt_tc_bcch_rx_tsn_info,gsm_get_irat_cnnt_tc_bcch_rx_pos_info,RX_GSM_NSLOT         ,(UINT16)0              ,GSM_RX_DATA       ,(UINT16)4             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_IDLE_TC_BCCH           },                                                                       
/*94*/  {TSNTASK_NULL_94                      ,(UINT16)0                              ,NULL_PTR                               ,NULL_PTR                           ,NULL_PTR                             ,(UINT16)0              ,(UINT16)0              ,(UINT16)0         ,(UINT16)0             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },                                                                        
/*95*/  {TSNTASK_NULL_95                      ,(UINT16)0                              ,NULL_PTR                               ,NULL_PTR                           ,NULL_PTR                             ,(UINT16)0              ,(UINT16)0              ,(UINT16)0         ,(UINT16)0             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_INVALID                    },                                                                        
/*96*/  {TSNTASK_W_DCH_7SLOT_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*97*/  {TSNTASK_W_DCH_10SLOT_NC_SYNC_RX      ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*98*/  {TSNTASK_W_DCH_14SLOT_NC_SYNC_RX      ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*99*/  {TSNTASK_W_FACH_10MS_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_SYNC_NSLOT      ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*100*/ {TSNTASK_W_FACH_20MS_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_SYNC_NSLOT      ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*101*/ {TSNTASK_W_FACH_40MS_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_SYNC_NSLOT      ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*102*/ {TSNTASK_W_FACH_06MS_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*103*/ {TSNTASK_W_FACH_08MS_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NBIT            ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*104*/ {TSNTASK_W_FACH_12MS_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_SYNC_NSLOT      ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*105*/ {TSNTASK_W_FACH_30MS_NC_SYNC_RX       ,GSM_TSNTASK_NC_CONN_FCB_RX_PRIOR       ,gsm_add_irat_conn_nc_sync_rx_tsntask_cb,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_SYNC_NSLOT      ,GSM_IRAT_GAP_MEAS      ,GSM_RX_DATA       ,(UINT16)1             ,gsm_get_rx_tstask_len_ind    ,gsm_get_tstask_rtx_len     ,MMC_AFF_GSM_CNNT_MEAS_FCCH         },                                                                       
/*106*/ {TSNTASK_PSD_TDS_TS_RX                ,GSM_TSNTASK_PSD_IRAT_RX_BY_TS_PRIOR    ,gsm_add_psd_irat_ts_rx_tsntask_cb      ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NSLOT           ,GSM_PSD_TASK           ,TDS_RX_DATA       ,(UINT16)1             ,gsm_get_psd_task_len_ind     ,gsm_get_psd_task_rtx_len   ,MMC_AFF_INVALID                    },                                                                        
/*107*/ {TSNTASK_PSD_LTE_FN_RX                ,GSM_TSNTASK_PSD_IRAT_RX_BY_FN_PRIOR    ,gsm_add_psd_irat_fn_rx_tsntask_cb      ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NSLOT           ,GSM_PSD_TASK           ,LTETDD_RX_DATA    ,(UINT16)1             ,gsm_get_psd_task_len_ind     ,gsm_get_psd_task_rtx_len   ,MMC_AFF_INVALID                    },                                                                     
/*108*/ {TSNTASK_PSD_TDS_FN_RX                ,GSM_TSNTASK_PSD_IRAT_RX_BY_FN_PRIOR    ,gsm_add_psd_irat_fn_rx_tsntask_cb      ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NSLOT           ,GSM_PSD_TASK           ,TDS_RX_DATA       ,(UINT16)1             ,gsm_get_psd_task_len_ind     ,gsm_get_psd_task_rtx_len   ,MMC_AFF_INVALID                    },                                                                                          
/*109*/ {TSNTASK_PSD_WCDMA_FN_RX              ,GSM_TSNTASK_PSD_IRAT_RX_BY_FN_PRIOR    ,gsm_add_psd_irat_fn_rx_tsntask_cb      ,NULL_PTR                           ,NULL_PTR                             ,RX_GSM_NSLOT           ,GSM_PSD_TASK           ,WCDMA_RX_DATA     ,(UINT16)1             ,gsm_get_psd_task_len_ind     ,gsm_get_psd_task_rtx_len   ,MMC_AFF_INVALID                    },
}; 
/* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
/* FIX NGM-Bug00000454 END   2014-09-12 : gaowu */

/* NOTE:u16_rx_len_ind,按189bit切分最后一块必须大于5bit*/  
L1CC_GSM_DRAM_DATA_CONST_SECTION                                                                                                                                                                                                                                                                                                                                                    
CONST gsm_rtx_len_t  C_S32_TSTASK_RTX_LEN_TBL[GSM_TSTASK_NUM]={
/*u16_tstask*/                       /*u16_rx_len_ind*/     /*fillword*/       /*s32_rtx_len(qbit)*/                             

/*0  TSNTASK_NULL                    */   {(UINT16)0              ,0           ,(SINT32)0                                         },
/*1  TSNTASK_SCAN_RF                 */   {(UINT16)32             ,0           ,(SINT32)(32*4)                                    },
/*2  TSNTASK_FCB                     */   {(UINT16)89             ,0           ,(SINT32)(89*625)                                  },
/*3  TSNTASK_SB                      */   {(UINT16)170            ,0           ,(SINT32)(170*4)                                   },
/*4  TSNTASK_BCCH_RX                 */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*5  TSNTASK_PBCCH_RX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*6  TSNTASK_CCCH_RX                 */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*7  TSNTASK_CCCH_CS_RX              */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*8  TSNTASK_CCCH_PCH_RX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*9  TSNTASK_TRANSFER_CCCH_RX        */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*10 TSNTASK_PCCCH_RX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*11 TSNTASK_NC0_BCCH_RX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*12 TSNTASK_NC1_BCCH_RX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*13 TSNTASK_NC2_BCCH_RX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*14 TSNTASK_NC3_BCCH_RX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*15 TSNTASK_NC4_BCCH_RX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*16 TSNTASK_NC5_BCCH_RX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*17 TSNTASK_NC6_BCCH_RX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*18 TSNTASK_TC_BCCH_RX              */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*19 TSNTASK_TC_SEARCH_BCCH_RX       */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*20 TSNTASK_RACH_TX                 */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*21 TSNTASK_PRACH_TX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*22 TSNTASK_SDCCH_RX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*23 TSNTASK_SDCCH_TX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*24 TSNTASK_SACCH_RX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*25 TSNTASK_SACCH_H_RX              */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*26 TSNTASK_SACCH_TX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*27 TSNTASK_SACCH_H_TX              */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*28 TSNTASK_TCH_RX                  */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*29 TSNTASK_TCH_H_RX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*30 TSNTASK_TCH_F96_RX              */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*31 TSNTASK_TCH_F144_RX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*32 TSNTASK_TCH_TX                  */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*33 TSNTASK_TCH_H_TX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*34 TSNTASK_TCH_F96_TX              */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*35 TSNTASK_TCH_F144_TX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*36 TSNTASK_FACCH_TX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*37 TSNTASK_FACCH_H_TX              */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*38 TSNTASK_HANDOVER_RX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*39 TSNTASK_HANDOVER_TX             */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*40 TSNTASK_PDTCH_TX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*41 TSNTASK_PDTCH_RX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*42 TSNTASK_SINGLEBLOCK_TX          */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*43 TSNTASK_RRBP_TX0                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*44 TSNTASK_PTCCH_TX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*45 TSNTASK_PTCCH_RX                */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*46 TSNTASK_MEAS_RSSI               */   {(UINT16)32             ,0           ,(SINT32)(32*4)                                    },
/*47 TSNTASK_CBCH_RX                 */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*48 TSNTASK_BG_RSSI_RX              */   {(UINT16)32             ,0           ,(SINT32)(32*4)                                    },
/*49 TSNTASK_BG_FCB_RX               */   {(UINT16)89             ,0           ,(SINT32)(89*625)                                  },
/*50 TSNTASK_BG_BSIC_RX              */   {(UINT16)170            ,0           ,(SINT32)(170*4)                                   },
/*51 TSNTASK_BG_TC0_BCCH_RX          */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*52 TSNTASK_BG_TC1_BCCH_RX          */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*53 TSNTASK_BG_TC2_BCCH_RX          */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*54 TSNTASK_BG_TC3_BCCH_RX          */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*55 TSNTASK_BG_TC4_BCCH_RX          */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*56 TSNTASK_BG_TC5_BCCH_RX          */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },

/*57 TSNTASK_SYNC_CELL0_BCCH_RX      */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*58 TSNTASK_SYNC_CELL1_BCCH_RX      */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*59 TSNTASK_SYNC_CELL2_BCCH_RX      */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*60 TSNTASK_SYNC_CELL3_BCCH_RX      */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*61 TSNTASK_SYNC_CELL4_BCCH_RX      */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*62 TSNTASK_SYNC_CELL5_BCCH_RX      */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },

/*63 TSNTASK_NC_INIT_BSIC_RX         */   {(UINT16)170            ,0           ,(SINT32)(170*4)                                   },
/*64 TSNTASK_NC_RE_BSIC_RX           */   {(UINT16)170            ,0           ,(SINT32)(170*4)                                   },
/*65 TSNTASK_NC_IDLE_FCB_RX          */   {(UINT16)89             ,0           ,(SINT32)(89*625)                                  },
/*66 TSNTASK_NC_RSSI_RX              */   {(UINT16)32             ,0           ,(SINT32)(32*4)                                    },
/*67 TSNTASK_NC_CONN_FCB_RX          */   {(UINT16)((7*189)+77)   ,0           ,(SINT32)(GSM_QBITS_OF_7x189BIT)                   },
/*68 TSNTASK_GPRS_INTF_MEAS_RX       */   {(UINT16)32             ,0           ,(SINT32)(32*4)                                    },
/*69 TSNTASK_NC_CONN_INIT_BSIC_RX    */   {(UINT16)170            ,0           ,(SINT32)(170*4)                                   },
/*70 TSNTASK_FAKEGAP_NC_RE_BSIC_RX   */   {(UINT16)170            ,0           ,(SINT32)(170*4)                                   },
/*71 TSNTASK_FAKEGAP_NC_RSSI_RX      */   {(UINT16)32             ,0           ,(SINT32)(32*4)                                    },
/*72 TSNTASK_TRANSFER_NC_BCCH_RX     */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*73 TSNTASK_TRANSFER_BCCH_RX        */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*74 TSNTASK_HO_FCB                  */   {(UINT16)89             ,0           ,(SINT32)(89*625)                                  },
/*75 TSNTASK_HO_SB                   */   {(UINT16)170            ,0           ,(SINT32)(170*4)                                   },
/*76 TSNTASK_MANU_SB                 */   {(UINT16)170            ,0           ,(SINT32)(170*4)                                   },
/*77 TSNTASK_MANU_FCB                */   {(UINT16)89             ,0           ,(SINT32)(89*625)                                  },
/*78 TSNTASK_MANU_TX                 */   {(UINT16)0              ,0           ,(SINT32)(624)                                     },
/*79 TSNTASK_MANU_RX_RSSI            */   {(UINT16)32             ,0           ,(SINT32)(32*4)                                    },
/*80 TSNTASK_NC_CONN_RRTL_FCB_RX     */   {(UINT16)((7*189)+77)   ,0           ,(SINT32)(GSM_QBITS_OF_7x189BIT)                   },
/*81 TSNTASK_PDTCH_FTA_TX            */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*82 TSNTASK_G_DED_12SLOT_NC_SYNC_RX */   {(UINT16)((9*189)+11)   ,0           ,(SINT32)((UINT32)((189*9)+11)*(UINT32)4)          },
/*83 TSNTASK_L_CNNT_06MS_NC_SYNC_RX  */   {(UINT16)((7*189)+77)   ,0           ,(SINT32)((UINT32)((189*7)+77)*(UINT32)4)          },
/*84 TSNTASK_IRAT_CNNT_FCB_RX        */   {(UINT16)((7*189)+77)   ,0           ,(SINT32)(GSM_QBITS_OF_7x189BIT)                   },
/*85 TSNTASK_T_TS0_ONLY_NC_SYNC_RX   */   {(UINT16)189            ,0           ,(SINT32)(189*4)                                   },
/*86 TSNTASK_T_TS0_1SLOT_NC_SYNC_RX  */   {(UINT16)((1*189)+180)  ,0           ,(SINT32)(((189*1)+180)*4)                         },
/*87 TSNTASK_T_TS0_2SLOT_NC_SYNC_RX  */   {(UINT16)((2*189)+170)  ,0           ,(SINT32)(((189*2)+170)*4)                         },
/*88 TSNTASK_T_TSN_2SLOT_NC_SYNC_RX  */   {(UINT16)((1*189)+108)  ,0           ,(SINT32)(((189*1)+108)*4)                         },
/*89 TSNTASK_T_TSN_3SLOT_NC_SYNC_RX  */   {(UINT16)((2*189)+99)   ,0           ,(SINT32)(((189*2)+99)*4)                          },
/*90 TSNTASK_T_INTERVAL_1_NC_SYNC_RX */   {(UINT16)((7*189)+77)   ,0           ,(SINT32)(((189*7)+77)*4)                          },
/*91 TSNTASK_T_INTERVAL_2_NC_SYNC_RX */   {(UINT16)((7*189)+77)   ,0           ,(SINT32)(((189*7)+77)*4)                          },
/*92 TSNTASK_T_DMO_480_20MS_NC_SYNC_RX*/  {(UINT16)33             ,0           ,(SINT32)(33*625)                                  },
/*93 TSNTASK_IRAT_CNNT_TC_BCCH_RX    */   {(UINT16)0              ,0           ,(SINT32)624                                       },
/*94 TSNTASK_NULL_94                 */   {(UINT16)0              ,0           ,(SINT32)0                                         },
/*95 TSNTASK_NULL_95                 */   {(UINT16)0              ,0           ,(SINT32)0                                         },
/*96 TSNTASK_W_DCH_7SLOT_NC_SYNC_RX  */   {(UINT16)(6*189)        ,0           ,(SINT32)((189*6)*4)                               },
/*97 TSNTASK_W_DCH_10SLOT_NC_SYNC_RX */   {(UINT16)((8*189)+128)  ,0           ,(SINT32)(((189*8)+128)*4)                         },
/*98 TSNTASK_W_DCH_14SLOT_NC_SYNC_RX */   {(UINT16)((12*189)+94)  ,0           ,(SINT32)(((189*12)+94)*4)                         },
/*99 TSNTASK_W_FACH_10MS_NC_SYNC_RX  */   {(UINT16)16             ,0           ,(SINT32)(16*625)                                  },
/*100 TSNTASK_W_FACH_20MS_NC_SYNC_RX  */  {(UINT16)33             ,0           ,(SINT32)(33*625)                                  },
/*101 TSNTASK_W_FACH_40MS_NC_SYNC_RX  */  {(UINT16)68             ,0           ,(SINT32)(68*625)                                 },
/*102 TSNTASK_W_FACH_06MS_NC_SYNC_RX  */  {(UINT16)((7*189)+77)   ,0           ,(SINT32)(((189*7)+77)*4)                         },
/*103 TSNTASK_W_FACH_08MS_NC_SYNC_RX  */  {(UINT16)((10*189)+111) ,0           ,(SINT32)(((189*10)+111)*4)                       },
/*104 TSNTASK_W_FACH_12MS_NC_SYNC_RX  */  {(UINT16)19             ,0           ,(SINT32)(19*625)                                 },
/*105 TSNTASK_W_FACH_30MS_NC_SYNC_RX  */  {(UINT16)19             ,0           ,(SINT32)(19*625)                                 },
/*106 TSNTASK_PSD_TDS_TS_RX           */  {(UINT16)0              ,0           ,(SINT32)0                                        },
/*107 TSNTASK_PSD_LTE_FN_RX           */  {(UINT16)0              ,0           ,(SINT32)0                                        },
/*108 TSNTASK_PSD_TDS_FN_RX           */  {(UINT16)0              ,0           ,(SINT32)0                                        }, 
/*109 SNTASK_PSD_WCDMA_FN_RX          */  {(UINT16)0              ,0           ,(SINT32)0                                        }
}; 
/* FIX NGM-Bug00000496 END 2014-09-17 : guxiaobo */
/* FIX PhyFA-Bug00005626 END   2014-09-04 : gaowu */
/* FIX LTE-Enh00000846 END  2014-04-29: wangjunli */
/* FIX PhyFA-Enh00001647 END 2014-04-28 : sunzhiqiang */

/* Fix L2000-Bug00001070 BEGIN 2010-04-19 : guxiaobo */
L1CC_GSM_DRAM_DATA_CONST_SECTION
CONST gsm_main_frametask_t C_ST_GSM_EMPTY_MAIN_FRAMETASK[GSM_MAX_MAINFRAME_TASKS] = GSM_FRAMETASK_16_TABLE;
/* Fix L2000-Bug00001070 END 2010-04-19 : guxiaobo */

L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION
UINT32   g_u32_gsm_fn[MAX_ACTIVE_AMT] ={0};

L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION
UINT16   g_u16_gsm_state[MAX_ACTIVE_AMT] ={0};
L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION
UINT16   g_u16_gsm_fcp_new_state[MAX_ACTIVE_AMT] ={0};
/* only used by l1cc_gsm_fcp.c */
L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION
UINT16   g_u16_gsm_fcp_3rd_state[MAX_ACTIVE_AMT] ={0};


    
L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION
gsm_main_frametask_t       g_st_gsm_main_frametask[MAX_ACTIVE_AMT][GSM_MAX_MAINFRAME_TASKS] ={{{0}}};



L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION 
UINT32_PTR    g_u32p_gsm_agc_addr[MAX_ACTIVE_AMT][AGC_APC_BUF_LEN][MAX_GSM_SLOT_NUM + 1] ={{{NULL_PTR}}};   /* AGC value address */
L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION 
SINT16        g_s16_gsm_apc[MAX_ACTIVE_AMT][AGC_APC_BUF_LEN+1][MAX_GSM_SLOT_NUM] ={{{0}}};            /* GSM APC value */
/* FIX LM-Bug00001262    END   2011-11-14  : linlan*/



L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION
gsm_standby_info_t  g_st_gsm_standby_info[MAX_ACTIVE_AMT] ={{{0}}};

L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION
UINT16   g_u16_state_change_standby_id[MAX_ACTIVE_AMT] ={0};



L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION
gsm_rtx_overlap_gap_info_t    g_st_gsm_rtx_overlap_gap_info[MAX_ACTIVE_AMT] ={{0}};
L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION
gsm_frametimer_change_info_t  g_st_gsm_frametimer_change_info[MAX_ACTIVE_AMT] ={{0}};
L1CC_GSM_DRAM_ZERO_INIT_DATA_SECTION
gsm_conflict_and_delete_param_save_t g_st_conflict_and_delete_param_save[MAX_ACTIVE_AMT] = {{{0}}};



/******************************************************************************
*  LOCAL FUNCTION PROTOTYPE DECLARATION
******************************************************************************/


STATIC OAL_STATUS gsm_fcp_frametimer_pre_handle(UINT16_PTR u16p_msg_rx_cfg);
STATIC OAL_STATUS gsm_fcp_frametimer_post_handle(VOID);
STATIC BOOLEAN    gsm_fcp_state_changing(VOID);
STATIC OAL_STATUS gsm_fcp_state_change(CONST oal_msg_t * CONST stp_oal_msg);
STATIC OAL_STATUS gsm_fcp_frametimer_change(VOID);
STATIC VOID       gsm_set_fcp_new_state(IN CONST_UINT16 u16_fcp_new_state);
STATIC VOID       gsm_set_fcp_3rd_state(IN CONST_UINT16 u16_fcp_3rd_state);

/*  10个系统状态处理函数 */
/* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
STATIC OAL_STATUS gsm_fcp_init_state(CONST_UINT32 u32_next_fn,oal_msg_t CONST * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos);
STATIC OAL_STATUS gsm_fcp_cs(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos);
STATIC OAL_STATUS gsm_fcp_idle(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos);
STATIC OAL_STATUS gsm_fcp_cs_rach(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos);
STATIC OAL_STATUS gsm_fcp_ps_rach(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos);
STATIC OAL_STATUS gsm_fcp_nocell(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos);
STATIC OAL_STATUS gsm_fcp_transfer(UINT32 u32_next_fn,oal_msg_t *stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos);
STATIC OAL_STATUS gsm_fcp_handover(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos);
STATIC OAL_STATUS gsm_fcp_dedicated(CONST_UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos);
STATIC OAL_STATUS gsm_fcp_slave(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos);
/* FIX NGM-Bug00000499 END   2014-09-11 : linlan */


/*  帧任务表操作函数  */
STATIC OAL_STATUS gsm_fcp_compare_prior(UINT32 u32_left_fn,SINT16 s16_left_tsn,CONST_UINT16 u16_right_tstask, CONST_UINT16 u16_right_standby_id);
STATIC OAL_STATUS gsm_fcp_tsntask_joint(UINT16 u16_left_tstask,UINT16 u16_right_tstask);
STATIC SINT16     gsm_fcp_calc_tsn(CONST_SINT32 s32_event_offset);
STATIC OAL_STATUS gsm_fcp_check_rtx_conflict(UINT16 u16_tstask,IN CONST gsm_offset_in_frame_t* CONST stp_frame_pos,INOUT gsm_del_conflict_tstask_info_t* CONST stp_del_tsntask,UINT16 u16_standby_id);

STATIC OAL_STATUS gsm_fcp_del_bitmap(CONST_UINT32 u32_fn, CONST_SINT16 s16_deleted_tsn);
STATIC OAL_STATUS gsm_fcp_del_rtx_conflict(UINT32 u32_fn, UINT16 u16_tsn, UINT16 u16_tstask, UINT32 u32_deleted_fn,SINT16 s16_deleted_tsn);

STATIC OAL_STATUS gsm_fcp_get_tb_tsn_info(gsm_tsn_info_t* stp_tsn_info);
STATIC OAL_STATUS gsm_fcp_get_tb_pos_info(gsm_tb_pos_t *stp_tb_pos_info,IN CONST_UINT16 u16_tstask,IN CONST_UINT32 u32_fn,IN CONST_UINT16 u16_standby_id);
STATIC OAL_STATUS gsm_fcp_del_tsntask(UINT32 u32_fn, SINT16 s16_deleted_tsn, CONST_UINT16 u16_standby_id);
STATIC OAL_STATUS gsm_fcp_check_isarrive(gsm_tsn_info_t* stp_tsn_info,gsm_tb_pos_t* stp_tb_pos);
STATIC OAL_STATUS gsm_fcp_arrive_pos(CONST gsm_tb_pos_t * CONST stp_tb_pos_info,UINT32 u32_fn);
STATIC OAL_STATUS gsm_fcp_get_spec_tb_info(UINT16_PTR u16p_tsn_num,UINT16_PTR u16_tsn_list,SINT16_PTR s16p_rtx_offset_list,VOID_PTR *stp_rrbp_data_list,CONST_UINT16 u16_tstask,UINT32 u32_fn,CONST_UINT16 u16_data_state,UINT16 u16_standby_id);
STATIC VOID       gsm_fcp_del_tsntask_ind(CONST oal_msg_t * CONST stp_oal_msg);

/*  帧定时操作函数  */
STATIC OAL_STATUS gsm_fcp_frame_timing_change_req(IN CONST oal_msg_t* CONST stp_oal_msg);

/*  事务操作函数  */
STATIC OAL_STATUS gsm_fcp_affair_operate_req(oal_msg_t *stp_oal_msg);
STATIC VOID       gsm_fcp_pre_add_mmc_affair(IN CONST UINT16 u16_tstask,IN CONST UINT16 u16_standby_id,IN CONST UINT32 u32_fn);


/*  模式间射频保护间隔函数 */
STATIC VOID       gsm_init_rtx_overlap_gap_info(VOID);
STATIC gsm_rtx_overlap_gap_info_t* gsm_get_rtx_overlap_gap_info(VOID);

/*  帧任务解析和下载函数 */
STATIC OAL_STATUS  gsm_fcp_parse_rtx_event(IN CONST gsm_download_rtx_event_info_t* CONST stp_download_rtx_event_info,
                                           OUT l1cc2dd_event_info_t *stp_l1cc2dd_event_info);


/*  主、辅模式下一些封装的公共的帧任务表任务添加函数 */
STATIC OAL_STATUS gsm_add_master_standby_idle_tsntask(IN CONST_UINT32 u32_next_fn,IN CONST_UINT16 u16_standby_id,OUT  gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list);
STATIC OAL_STATUS gsm_add_master_standby_nc_bcch_tsntask(IN CONST_UINT32 u32_next_fn,IN CONST_UINT16 u16_standby_id,OUT  gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list);
STATIC OAL_STATUS gsm_add_master_standby_plmn_bcch_tsntask(IN CONST_UINT32 u32_next_fn,IN CONST_UINT16 u16_standby_id,OUT  gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list);
STATIC OAL_STATUS gsm_add_slave_standby_tsntask(IN CONST_UINT32 u32_next_fn,IN CONST_UINT16 u16_standby_id,OUT  gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list);
STATIC OAL_STATUS gsm_add_sync_cell_bcch_tsntask(IN CONST_UINT32 u32_next_fn,IN CONST_UINT16 u16_standby_id,OUT  gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list);
STATIC OAL_STATUS gsm_add_ccch_in_transfer_tsntask(IN CONST_UINT32 u32_next_fn,OUT gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list);
STATIC OAL_STATUS gsm_fcp_cellsearch_in_transfer(IN CONST_UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,IN CONST_UINT16 u16_transfer_standby_id, gsm_offset_in_frame_t* stp_latest_fn_pos);
STATIC OAL_STATUS gsm_add_bcch_in_transfer_tsntask(IN CONST_UINT32 u32_next_fn,OUT gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list);


/*  其它的函数 */
STATIC BOOLEAN    gsm_check_irat_rx_pseudo_task(IN CONST_UINT16 u16_tstask);
STATIC VOID_PTR   gsm_fcp_create_hls_msg(oal_msg_t **stpp_oal_msg,UINT32 u32_primive_id,CONST_UINT16 u16_hls_struct_len, UINT16 u16_standby_id);

/* FIX PhyFA-Bug00005191    BEGIN     2014-07-03  : linlan*/
STATIC OAL_STATUS gsm_del_cellsearch_fcb(VOID);
/* FIX PhyFA-Bug00005191    END       2014-07-03  : linlan*/
/* FIX NGM-Bug00000206 BEGIN 2014-07-21 : wuxiaorong */
STATIC VOID       gsm_del_mmc_affair(IN CONST_UINT16  u16_standby_id,IN CONST UINT16 u16_tstask,IN CONST UINT32 u32_del_fn,IN CONST UINT16 u16_del_tsn,IN CONST SINT16 s16_offset_in_ts);
/* FIX NGM-Bug00000206 END   2014-07-21 : wuxiaorong */
STATIC OAL_STATUS gsm_del_fcb_task_reset_cellsearch_tb(VOID);
STATIC BOOLEAN    gsm_check_whether_need_mmc_conflict_judge(IN CONST_UINT16 u16_active_id,IN CONST_UINT16 u16_standby_id);


STATIC BOOLEAN    gsm_check_other_standby_info(IN CONST_UINT16 u16_standby_id);
STATIC BOOLEAN    gsm_check_flexible_cfg_tsntask(IN CONST_UINT16 u16_tstask);
STATIC BOOLEAN    gsm_is_nc_bcch_tsntask(IN CONST_UINT16 u16_tstask);
/* FIX NGM-Bug00000585 BEGIN 2014-09-20 : wuxiaorong */
STATIC VOID       gsm_check_del_running_sync_nslot_task(IN  CONST_UINT32 u32_next_fn,IN  CONST_UINT16 u16_rtx_bitmap);
/* FIX NGM-Bug00000585 END   2014-09-20 : wuxiaorong */

/* FIX NGM-Bug00000383     BEGIN     2014-09-11  : linlan*/
STATIC BOOLEAN gsm_check_whether_nc_bcch_need_preempt(IN CONST l1cc_mmc_affair_type_t e_affair_id);
/* FIX NGM-Bug00000383     END       2014-09-11  : linlan*/
/***********************************************************************************************************************
*  GLOBAL FUNCTION DEFINITION
***********************************************************************************************************************/


/***********************************************************************************************************************
* FUNCTION:          gsm_get_gsm_fn
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return Parameters: <name1>        <description1>
* Chage Histroy
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-10-14      gaowu                   PhyFA-Req00001525   [NGM]GSM 模式V4代码入库
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
UINT32 gsm_get_gsm_fn(IN CONST_UINT16 u16_active_id)
{
    
    return g_u32_gsm_fn[u16_active_id];
}

/***********************************************************************************************************************
* FUNCTION:          gsm_set_gsm_fn
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
VOID gsm_set_gsm_fn(IN CONST_UINT16 u16_active_id,IN CONST_UINT32 u32_fn)
{
    
    g_u32_gsm_fn[u16_active_id] = u32_fn;
    return ;
}



/***********************************************************************************************************************
* FUNCTION:          gsm_get_main_frametask
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
INLINE gsm_main_frametask_t* gsm_get_main_frametask(IN CONST_UINT32 u32_fn, IN CONST_UINT16 u16_active_id)
{
    /* FIX Enh00001894   BEGIN     2014-05-12  : xiongjiangjiang */
    CONST_UINT32  u32_index = GSM_MOD_2POWER(UINT32, u32_fn, GSM_MAX_MAINFRAME_TASKS);
    /* FIX Enh00001894   END     2014-05-12  : xiongjiangjiang */

    
    return  (&g_st_gsm_main_frametask[u16_active_id][u32_index]);
}
L1CC_GSM_DRAM_CODE_SECTION
gsm_tstask_attribute_t* gsm_get_tstask_attribute(IN CONST_UINT16 u16_tstask)
{
   CONST_UINT16 u16_index = u16_tstask&(UINT16)0x7f;
        
   return (gsm_tstask_attribute_t*)&(C_ST_TSTASK_ATTRIBUTE_TBL[u16_index]);
}

L1CC_GSM_DRAM_CODE_SECTION
SINT32 gsm_get_tstask_rtx_len(IN CONST_UINT16 u16_tstask)
{
   CONST_UINT16 u16_index = u16_tstask&(UINT16)0x7f;
        
   return (C_S32_TSTASK_RTX_LEN_TBL[u16_index].s32_rtx_len);
}

L1CC_GSM_DRAM_CODE_SECTION
UINT16 gsm_get_rx_tstask_len_ind(IN CONST_UINT16 u16_tstask)
{
   CONST_UINT16 u16_index = u16_tstask&(UINT16)0x7f;
        
   return (C_S32_TSTASK_RTX_LEN_TBL[u16_index].u16_rx_len_ind);
} 

 
/*************************************************************************************
Function Name:        gsm_get_frame_task_rtx_bitmap
Input parameters:     gsm_main_frametask_t *stp_frame_task
Output parameters:    NULL
Return value:         UINT16
Global Variables:
Description:          gsm_get_frame_task_rtx_bitmap.
Remark:
Change history:
Author                date                               No.
wuxiaorong            2008/02/01                         create
*************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
UINT16 gsm_get_frame_task_rtx_bitmap(IN CONST gsm_main_frametask_t * CONST stp_frame_task)
{
    OAL_ASSERT((NULL_PTR != stp_frame_task), "gsm_get_frame_task_rtx_bitmap:stp_frame_task is null");
    return(UINT16)(stp_frame_task->u16_rx_bitmap|stp_frame_task->u16_tx_bitmap);
}







/***********************************************************************************************************************
* FUNCTION:          gsm_get_standby_info
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return Parameters: <name1>        <description1>
* Chage Histroy
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-09-10      gaowu                   PhyFA-Req00001525   [NGM]GSM 模式V4代码入库
* 2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
gsm_standby_info_t* gsm_get_standby_info(VOID)
{
    UINT16 u16_active_id = 0;

    u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    return (&g_st_gsm_standby_info[u16_active_id]);
}

/***********************************************************************************************************************
* FUNCTION:          gsm_cb_get_standby_info
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return Parameters: <name1>        <description1>
* Chage Histroy
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-09-10      gaowu                   PhyFA-Req00001525   [NGM]GSM 模式V4代码入库
* 2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
gsm_standby_info_t* gsm_cb_get_standby_info(IN CONST_UINT16 u16_active_id)
{
    
    return (&g_st_gsm_standby_info[u16_active_id]);
}

/***********************************************************************************************************************
* FUNCTION:          gsm_get_gsm_state
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return Parameters: <name1>        <description1>
* Chage Histroy
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-09-10      gaowu                   PhyFA-Req00001525   [NGM]GSM 模式V4代码入库
* 2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
INLINE UINT16 gsm_get_gsm_state()
{
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    return (g_u16_gsm_state[u16_active_id]);
}

/***********************************************************************************************************************
* FUNCTION:          gsm_cb_get_gsm_state
* DESCRIPTION:       <被CALLBACK函数调用>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return Parameters: <name1>        <description1>
* Chage Histroy
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-09-10      gaowu                   PhyFA-Req00001525   [NGM]GSM 模式V4代码入库
* 2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
UINT16 gsm_cb_get_gsm_state(IN CONST_UINT16 u16_active_id)
{
    
    return (g_u16_gsm_state[u16_active_id]);
}

/***********************************************************************************************************************
* FUNCTION:          gsm_get_gsm_state
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return Parameters: <name1>        <description1>
* Chage Histroy
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-10-24      hanhaitao               PhyFA-Req00001525   [NGM]GSM 模式V4代码入库
* 2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
INLINE VOID gsm_set_gsm_state(IN CONST_UINT16 u16_state)
{
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    g_u16_gsm_state[u16_active_id] = u16_state;
    return;
}

/***********************************************************************************************************************
* FUNCTION:          gsm_get_gsm_new_state
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return Parameters: <name1>        <description1>
* Chage Histroy
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-09-10      gaowu                   PhyFA-Req00001525   [NGM]GSM 模式V4代码入库
* 2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
UINT16 gsm_get_fcp_new_state(VOID)
{
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    return (g_u16_gsm_fcp_new_state[u16_active_id]);
}



/***********************************************************************************************************************
* FUNCTION:          gsm_cb_get_fcp_new_state
* DESCRIPTION:       <callback调用>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-11-14      wuxiaorong              PhyFA-Req00001525   【NGM】GSM 模式V4代码入库 
* 2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
UINT16 gsm_cb_get_fcp_new_state(IN CONST_UINT16 u16_active_id)
{
    
    return (g_u16_gsm_fcp_new_state[u16_active_id]);
}

/***********************************************************************************************************************
* FUNCTION:          gsm_get_fcp_3rd_state
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
UINT16 gsm_get_fcp_3rd_state(VOID)
{
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    return (g_u16_gsm_fcp_3rd_state[u16_active_id]);
}
/***********************************************************************************************************************
* FUNCTION:          gsm_set_fcp_new_state
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
STATIC VOID gsm_set_fcp_new_state(IN CONST_UINT16 u16_fcp_new_state)
{
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    g_u16_gsm_fcp_new_state[u16_active_id] = u16_fcp_new_state;
    return;
}


/***********************************************************************************************************************
* FUNCTION:          gsm_set_fcp_3rd_state
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/


L1CC_GSM_DRAM_CODE_SECTION
STATIC VOID gsm_set_fcp_3rd_state(IN CONST_UINT16 u16_fcp_3rd_state)
{
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    g_u16_gsm_fcp_3rd_state[u16_active_id] = u16_fcp_3rd_state;
    return;
}


    
/***********************************************************************************************************************
* FUNCTION:          gsm_set_rx_event_agc_value_addr
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
VOID gsm_set_rx_event_agc_value_addr(IN CONST_UINT32 u32_fn,IN CONST_UINT16 u16_tsn,CONST_UINT32_PTR_CONST u32p_agc_addr)
{
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    
    g_u32p_gsm_agc_addr[u16_active_id][u32_fn % (UINT32)AGC_APC_BUF_LEN][u16_tsn] = (UINT32_PTR)u32p_agc_addr;
    return;
}
/***********************************************************************************************************************
* FUNCTION:          gsm_set_tx_event_apc_value
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
* 2014-05-26      linlan                  PhyFA-Bug00004837   [NGM]多时隙上行发送APC配置错误 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
VOID gsm_set_tx_event_apc_value(IN CONST_UINT32 u32_fn,IN CONST_UINT16 u16_apc_tsn,IN CONST_SINT16 s16_apc_value)
{
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    
    /* FIX LM-Bug00000868 BEGIN 2011-09-28: linlan */
    /* FIX Enh00001894   BEGIN     2014-05-12  : xiongjiangjiang */
    g_s16_gsm_apc[u16_active_id][gsm_fn_operate(u32_fn + (UINT32)(u16_apc_tsn/(UINT16)MAX_GSM_SLOT_NUM)) % (UINT32)AGC_APC_BUF_LEN][GSM_MOD_2POWER(UINT16, u16_apc_tsn, MAX_GSM_SLOT_NUM)] =  s16_apc_value;
    /* FIX Enh00001894   END     2014-05-12  : xiongjiangjiang */
    /*DD 获取APC的时候，多时隙是按照取首时隙后，直接指针一直++,因此全局变量开的大小是[AGC_APC_BUF_LEN+1]，防止DD取越界,并且对于最后一个数组元素也要正确赋值，如下*/
    if(((gsm_fn_operate(u32_fn) % (UINT32)AGC_APC_BUF_LEN)==(UINT32)((UINT32)AGC_APC_BUF_LEN - (UINT32)1)) && (u16_apc_tsn >= (UINT16)MAX_GSM_SLOT_NUM))
    {
        g_s16_gsm_apc[u16_active_id][AGC_APC_BUF_LEN][GSM_MOD_2POWER(UINT16, u16_apc_tsn, MAX_GSM_SLOT_NUM)] =  s16_apc_value;
    }
    
    /* FIX LM-Bug00000868 END 2011-09-28: linlan */
    return;
}







L1CC_GSM_DRAM_CODE_SECTION
SINT32 gsm_get_compare_rtx_overlap_gap(IN CONST_UINT16 u16_left_rtx_status,IN CONST_UINT16 u16_right_rtx_status)
{
    SINT32 s32_rtx_overlap_gap= 0;
    CONST gsm_rtx_overlap_gap_info_t* stp_rtx_gap_info = NULL_PTR;

    stp_rtx_gap_info = gsm_get_rtx_overlap_gap_info();

    switch(u16_left_rtx_status)
    {
        case GSM_RX_DATA:
        {
            switch(u16_right_rtx_status)
            {
                case GSM_RX_DATA:
                    s32_rtx_overlap_gap = stp_rtx_gap_info->s32_gsm_rx2gsm_rx_overlap_gap;
                    break;
                case GSM_TX_DATA:
                     s32_rtx_overlap_gap = stp_rtx_gap_info->s32_gsm_rx2gsm_tx_overlap_gap;
                    break;
                #if defined(TDS_MODE_SWITCH)
                case TDS_RX_DATA:
                     s32_rtx_overlap_gap = stp_rtx_gap_info->s32_gsm_rx2tds_rx_overlap_gap;
                    break;
                #endif
                #if defined(WCDMA_MODE_SWITCH)
                case WCDMA_RX_DATA:
                    s32_rtx_overlap_gap = stp_rtx_gap_info->s32_gsm_rx2wcdma_rx_overlap_gap;
                    break;
                #endif
                #if defined(LTE_MODE_SWITCH)
                case LTEFDD_RX_DATA:
                case LTETDD_RX_DATA:
                    s32_rtx_overlap_gap = stp_rtx_gap_info->s32_gsm_rx2lte_rx_overlap_gap;
                   break;
                #endif
                default:
                 break;
            }

        }
        break;
        case GSM_TX_DATA:
        {

            switch(u16_right_rtx_status)
            {
                case GSM_RX_DATA:
                    s32_rtx_overlap_gap = stp_rtx_gap_info->s32_gsm_tx2gsm_rx_overlap_gap;
                    break;
                case GSM_TX_DATA:
                     s32_rtx_overlap_gap = stp_rtx_gap_info->s32_gsm_tx2gsm_tx_overlap_gap;
                    break;
                #if defined(TDS_MODE_SWITCH)
                case TDS_RX_DATA:
                     s32_rtx_overlap_gap = stp_rtx_gap_info->s32_gsm_tx2tds_rx_overlap_gap;
                    break;
                #endif
                #if defined(WCDMA_MODE_SWITCH)
                case WCDMA_RX_DATA:
                    s32_rtx_overlap_gap = stp_rtx_gap_info->s32_gsm_tx2wcdma_rx_overlap_gap;
                    break;
                #endif
                #if defined(LTE_MODE_SWITCH)
                case LTEFDD_RX_DATA:
                case LTETDD_RX_DATA:
                    s32_rtx_overlap_gap = stp_rtx_gap_info->s32_gsm_tx2lte_rx_overlap_gap;
                   break;
                #endif  
                default:
                 break;
            }
        }
        break;
        #if defined(TDS_MODE_SWITCH)
        case TDS_RX_DATA:
        {
            if(GSM_RX_DATA == u16_right_rtx_status)
            {
                s32_rtx_overlap_gap = stp_rtx_gap_info->s32_tds_rx2gsm_rx_overlap_gap;
            }
            else
            {
                s32_rtx_overlap_gap = stp_rtx_gap_info->s32_tds_rx2gsm_tx_overlap_gap;
            }
        }
        break;
        #endif
        #if defined(WCDMA_MODE_SWITCH)
        case WCDMA_RX_DATA:
        {
            if(GSM_RX_DATA == u16_right_rtx_status)
            {
                s32_rtx_overlap_gap = stp_rtx_gap_info->s32_wcdma_rx2gsm_rx_overlap_gap;
            }
            else
            {
                s32_rtx_overlap_gap = stp_rtx_gap_info->s32_wcdma_rx2gsm_tx_overlap_gap;
            }
        }
        break;
        #endif
        #if defined(LTE_MODE_SWITCH)
        case LTETDD_RX_DATA:
        case LTEFDD_RX_DATA:
        {
            if(GSM_RX_DATA == u16_right_rtx_status)
            {
                s32_rtx_overlap_gap = stp_rtx_gap_info->s32_lte_rx2gsm_rx_overlap_gap;
            }
            else
            {
                s32_rtx_overlap_gap = stp_rtx_gap_info->s32_lte_rx2gsm_tx_overlap_gap;
            }
        }
        
        break;
        #endif
        default:
             break;
    }

    return s32_rtx_overlap_gap;
}
    
L1CC_GSM_DRAM_CODE_SECTION
SINT32 gsm_get_gsm_rtx2rx_overlap_gap(VOID)
{
    SINT32 s32_rtx_overlap_gap= 0;
    CONST gsm_rtx_overlap_gap_info_t* stp_rtx_gap_info = NULL_PTR;

    stp_rtx_gap_info = gsm_get_rtx_overlap_gap_info();

    s32_rtx_overlap_gap = GSM_MAX(stp_rtx_gap_info->s32_gsm_rx2gsm_rx_overlap_gap,stp_rtx_gap_info->s32_gsm_tx2gsm_rx_overlap_gap);
    return s32_rtx_overlap_gap;
}
/***********************************************************************************************************************
* FUNCTION:          gsm_get_frametimer_change_info
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
gsm_frametimer_change_info_t*  gsm_get_frametimer_change_info(VOID)
{
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    
    return &(g_st_gsm_frametimer_change_info[u16_active_id]);
}


/******************************************************************************
*
* FUNCTION
*
*    gsm_get_tsntask
*
* DESCRIPTION
*
*    
*
* NOTE
*
*    NA
*
* PARAMETERS
*
*      
*
* RETURNS
*
*    NA
*
* CALLS
*
*    <function1>
*
* CALLED BY
*
*    <function1>
*
* VERSION
*
*    <date>        <author>       <CR_ID>                  <DESCRIPTION>
*    2010-09-28    guxiaoob      PHYFA-Req00000766         add note
*   2014-05-09      wuxiaorong              PhyFA-Req00001737   [NGM]gsm_get_tsntask_by_active的最大最小时隙范围修改
*2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
******************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
gsm_slot_rtx_t*gsm_get_tsntask_by_active(IN UINT32 u32_fn, IN SINT16 s16_tsn,IN UINT16 u16_active_id)
{
    SINT16 s16_max_tsn = 0;
    SINT16 s16_min_tsn = 0;

    UINT32 u32_temp_fn;
    gsm_main_frametask_t         *stp_gsm_main_frametask;
    /* FIX PhyFA-Req00001737 BEGIN 2014-05-09 : wuxiaorong */
    #if 0
    CONST gsm_rtx_overlap_gap_info_t *stp_rtx_gap_info;
    #endif
    

    #if 0
    stp_rtx_gap_info =&(g_st_gsm_rtx_overlap_gap_info[u16_active_id]);

   
    /*FIX LM-Req00000192  BEGIN  2011-03-03 :sunzhiqiang*/   
    s16_max_tsn = ((SINT16)7 + (SINT16)((SINT32)((SINT32)GSM_QBITS_OF_7x189BIT + stp_rtx_gap_info->s32_gsm_tx2gsm_rx_overlap_gap 
                   + stp_rtx_gap_info->s32_gsm_rx2gsm_tx_overlap_gap + (SINT32)1)/(SINT32)GSM_QBITS_PER_TS) + (SINT16)1 );
    /*FIX LM-Req00000192  END  2011-03-03 :sunzhiqiang*/
    #endif
    /* RX_GSM_NBIT事件最长为2 frames,再考虑提前一帧配置 */
    s16_max_tsn = ((SINT16)7+(SINT16)16+(SINT16)1);
    s16_min_tsn = -((SINT16)16+(SINT16)1);

    /*support handover and reselection function,gsm's frame number is already changed*/        
    if((s16_tsn < s16_min_tsn) || ((SINT16)s16_tsn > s16_max_tsn)) 
    {
        oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_TSN));
    }
    /* FIX PhyFA-Req00001737 END   2014-05-09 : wuxiaorong */
    else if( 0 > s16_tsn)
    {
        u32_fn = gsm_fn_operate((u32_fn + (UINT32)GSM_MAX_FN)- (UINT32)1);
        s16_tsn += (SINT16)8;

        if(s16_tsn < 0)
        {
            u32_fn = gsm_fn_operate((u32_fn + (UINT32)GSM_MAX_FN) - (UINT32)1);
            s16_tsn += (SINT16)8;
        }        
    }
    u32_temp_fn = (UINT32)((SINT32)u32_fn+((SINT32)s16_tsn/(SINT32)GSM_MAX_TIMESLOTS));
    stp_gsm_main_frametask = gsm_get_main_frametask(u32_temp_fn,u16_active_id);

    /* FIX Enh00001894   BEGIN     2014-05-12  : xiongjiangjiang */
    return(&stp_gsm_main_frametask->st_slot_rtx[GSM_MOD_2POWER(SINT32, s16_tsn, GSM_MAX_TIMESLOTS)]);
    /* FIX Enh00001894   END     2014-05-12  : xiongjiangjiang */
}

/******************************************************************************
*
* FUNCTION
*
*    gsm_get_tsntask
*
* DESCRIPTION
*
*
*
* NOTE
*
*    NA
*
* PARAMETERS
*
*
*
* RETURNS
*
*    NA
*
* CALLS
*
*    <function1>
*
* CALLED BY
*
*    <function1>
*
* VERSION
*
*    <date>        <author>       <CR_ID>                  <DESCRIPTION>
*    2010-09-28    guxiaoob      PHYFA-Req00000766         add note
*2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
******************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
gsm_slot_rtx_t* gsm_get_tsntask(IN UINT32 u32_fn, IN SINT16 s16_tsn)
{
    gsm_slot_rtx_t* stp_gsm_slot_rtx = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    

    stp_gsm_slot_rtx = gsm_get_tsntask_by_active(u32_fn,s16_tsn,u16_active_id);

    return stp_gsm_slot_rtx;
}

/************************************************************
** Function Name: gsm_fcp_main ()
** Description: the main function of the module FCP
** Input : NULL
** Output : NULL
** Return: OAL_SUCCESS or OAL_FAILURE
** Notes:
2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*   2014-08-05      wuxiaorong              PhyFA-Req00001925   [NGM]T业务下G测量在当前接收未完成前就需要帧中断下次接收任务 .
*   2014-09-20      wuxiaorong              PhyFA-Req00001980   [NGM]功耗测试GSM的非空寻呼唤醒后的RSSI接收配置限定在第1和第2帧寻呼接收之间.
*   2014-09-25      wuxiaorong              NGM-Bug00000627     【NGM】【TDM】GSM的帧中断在GAP回收的CNF时间点前不配任务时需要更新stp_latest_rtx_frame_pos->u32_fn为下下帧保证不睡眠
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
VOID gsm_fcp_main(VOID)
{
    CONST oal_primitive_id_t waitmsg_list[2] = {(UINT32)1,(UINT32)OAL_ANY_MSG_ID};
    UINT32 u32_next_fn;    
    oal_msg_t *stp_oal_msg;

    oal_primitive_id_t    msg_primitive_id;
    u_time_info_t   u_time_info;
    /* FIX PHYFA-Enh00000742   BEGIN  2010-06-22  : wanghuan */
    UINT16 u16_msg_rx_cfg = 0;
    /* FIX PHYFA-Enh00000742   END  2010-06-22  : wanghuan */
    CONST gsm_cell_reselect_req_t *stp_cell_resel_req = NULL_PTR;
    /* FIX PhyFA-Req00000825 BEGIN  2011-01-10 : wuxiaorong */           
    #ifdef PECKER_SWITCH 
    /* Fix PhyFA Req00000171 begin, guxiaobo, 20080826*/
    UINT16 u16_current_state;
    /* Fix PhyFA Req00000171 end, guxiaobo, 20080826*/
    #endif
    /* FIX PhyFA-Req00000825 END  2011-01-10 : wuxiaorong */ 
    UINT16  u16_gsm_advance_del_fn = GSM_FCP_ADVANCE_DEL_FN;    
    UINT32  u32_gsm_fn;
    UINT16  u16_gsm_state;
    UINT16  u16_gsm_fcp_new_state;
    UINT16  u16_gsm_fcp_3rd_state;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR;
    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
    gsm_latest_rtx_frame_pos_t * stp_gsm_latest_rtx_frame_pos = NULL_PTR;
    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */
    /* FIX PhyFA-Req00001980 BEGIN 2014-09-22 : wuxiaorong */
    CONST  msg_gsm_l1cc_fcp_del_tsntask_ind_t  *stp_del_tsntask_ind = NULL_PTR;
    /* FIX PhyFA-Req00001980 END   2014-09-22 : wuxiaorong */

    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
    stp_gsm_latest_rtx_frame_pos = gsm_get_latest_rtx_frame_pos();            
    stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */

    while(1)
    {
        stp_oal_msg = oal_msg_receive(waitmsg_list,(UINT16)OAL_IPC_WAIT_FOREVER);
        if(NULL_PTR != stp_oal_msg)
        {
            msg_primitive_id = OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg);
            
            switch(msg_primitive_id)
            {
                case MSG_GSM_COMMON_FCP_STATE_CHANGE_REQ:
                    gsm_fcp_state_change(stp_oal_msg);
                    u16_msg_rx_cfg = 0;
                    break;
                /* FIX PhyFA-Req00000089  BEGIN  2008-03-31  : huangjinfu */
                case MSG_GSM_L1CC_FCP_NOCELL_SLEEP_REQ:
                    /* FIX PhyFA-Req00000936 BEGIN  2011-06-02  : wuxiaorong */
                    u16_msg_rx_cfg = 0;
                    gsm_fcp_nocell_sleep_req(stp_oal_msg);
                    /* FIX PhyFA-Req00000936 END  2011-06-02  : wuxiaorong */
                    /* FIX PhyFA-Req00000089  END  2008-03-31  : huangjinfu */
                    break;
                case MSG_GSM_HLP_FCP_CELL_RESELECT_REQ:
                    stp_cell_resel_req = (gsm_cell_reselect_req_t *)OAL_GET_OAL_MSG_BODY(stp_oal_msg);
                    gsm_transform_all_cells_timeinfo(stp_cell_resel_req->u16_target_cell_idx,stp_cell_resel_req->u16_standby_id, GSM_RESELECT_TRANSFORM, g_u32_gsm_fn[u16_active_id]);
                    u16_msg_rx_cfg = 0;
                    break;
                case MSG_GSM_HLP_FCP_HO_FAIL_IND:
                    gsm_fcp_handover_failure(stp_oal_msg);
                    /* FIX PhyFA-Bug00000285  BEGIN  2008-03-31  : huangjinfu */
                    u16_msg_rx_cfg = 0;
                    /* FIX PhyFA-Bug00000285  END  2008-03-31  : huangjinfu */
                    break;
                case MSG_ISR_FCP_FRAMETIMER_IND:
                    /* FIX PhyFA-Enh00000089   BEGIN  2008-04-07  : huangjinfu */
                    gsm_fcp_frametimer_pre_handle(&u16_msg_rx_cfg);
                    /* FIX PhyFA-Enh00000089   END  2008-04-07  : huangjinfu */                   
                    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
                    stp_gsm_latest_rtx_frame_pos->b_not_cfg_sleep = OAL_FALSE;
                    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */
                    break;
                /* FIX NGM-Bug00000634    BEGIN  2014-09-25  : linlan */                    
                case MSG_GSM_HLP_FCP_MSG_WAKE_UP:
                    /*HLP收到高层消息，需要更新latest fn( = next fn + 1)，防止后续收到FCP_DEL配置睡眠*/
                    stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos.u32_fn = gsm_fn_operate(gsm_get_gsm_fn(u16_active_id) + (UINT32)2);
                    u16_msg_rx_cfg = 0;
                    break;
                /* FIX NGM-Bug00000634    END    2014-09-25  : linlan */                    
                case MSG_GSM_EQ_FCP_USF_DEC_IND:
                /* FIX PhyFA-Req00000105    BEGIN   2008-04-21  : ningyaojun*/
                case MSG_GSM_EQ_FCP_LOOPBACK_IND:
                /* FIX PhyFA-Req00000105    END     2008-04-21  : ningyaojun*/
                case MSG_GSM_MEAS_FCP_FCBSUCC_IND:
                /* Fix PhyFA Bug00000440 begin, guxiaobo, 20080911 */
                case MSG_GSM_DEC_FCP_USF_DEC_IND:
                /* Fix PhyFA Bug00000440 end, guxiaobo, 20080911 */
                /*FIX PHYFA-Req00000737  BEGIN  2010-06-07 :sunzhiqiang*/
                case MSG_GSM_MEAS_HLP_PT_FCB_CNF:
                /*FIX PHYFA-Req00000737  END  2010-06-07 :sunzhiqiang*/
                /* FIX PhyFA-Req00001925 BEGIN 2014-08-05 : wuxiaorong */
                #if 0
                /* FIX PhyFA-Bug00005426 BEGIN 2014-07-25 : wuxiaorong */
                case MSG_GSM_L1RESP_FCP_SBFAIL_IND:
                /* FIX PhyFA-Bug00005426 END   2014-07-25 : wuxiaorong */
                #endif
                /* FIX PhyFA-Req00001925 END   2014-08-05 : wuxiaorong */

                    dd_net_time_get(u16_active_id, (UINT16)GSM_TIMING, &u_time_info);
                    /* 如果分配的GAP回收点在收到FCBSUCC之后，就先不配置，单卡需要确保不冲突 */
                    if (OAL_TRUE == gsm_check_gap_withdraw_point_is_arrive(&u_time_info.st_fn_offset))
                    {
                        /* 进这个分支,目前消息进行丢弃处理 */
                        u16_msg_rx_cfg = 0;
                    }
                    else
                    {
                        u16_msg_rx_cfg = 1;
                    }
                    
                    break;
                /* FIX PhyFA-Enh00000250 BEGIN  2008-10-22  : wuxiaorong */
                /* FIX AM-Enh00000083    BEGIN  2010-07-12  : wuxiaorong */
                case MSG_GSM_COMMON_FCP_DEL_TSNTASK_IND:
                    u16_msg_rx_cfg = 0;
                    gsm_fcp_del_tsntask_ind(stp_oal_msg);
                    /* FIX NGM-Bug00000627 BEGIN 2014-09-25 : wuxiaorong */
                    u16_gsm_state = gsm_get_gsm_state();/* 这个不能往前提,因为中间可能会改变 */
                    if(GSM_IDLE == u16_gsm_state)
                    {
                        /* FIX PhyFA-Req00001980 BEGIN 2014-09-22 : wuxiaorong */
                        /* 寻呼接收提前译码，需要及时调用睡眠请求. */
                        stp_del_tsntask_ind = (msg_gsm_l1cc_fcp_del_tsntask_ind_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));
                        if((FCP_DEL_TASK_NORMAL == stp_del_tsntask_ind->u16_del_type)
                            &&(MPAL_PAG_CH == stp_del_tsntask_ind->u16_ch_type))
                        {

                            OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,(UINT16)GSM_MODE,"stp_del_tsntask_ind->u32_fn:0x%lx, b_not_cfg_sleep:%d,stp_latest_rtx_frame_pos->u32_fn:0x%lx",stp_del_tsntask_ind->u32_fn,stp_gsm_latest_rtx_frame_pos->b_not_cfg_sleep,stp_latest_rtx_frame_pos->u32_fn);

                            if(OAL_FALSE == stp_gsm_latest_rtx_frame_pos->b_not_cfg_sleep )
                            { 
                                OAL_ASSERT((stp_latest_rtx_frame_pos->u32_fn!= stp_del_tsntask_ind->u32_fn),"g_st_latest_frame_pos[u16_active_id] calculated fail!");
                                /* 根据s_u32_latest_rtx_fn[u16_active_id]和当前帧任务情况，看是否需要睡眠 */
                                gsm_fcp_sleep_cfg(u16_active_id,stp_del_tsntask_ind->u32_fn,stp_latest_rtx_frame_pos);
                            }
                        }     
                        /* FIX PhyFA-Req00001980 END   2014-09-22 : wuxiaorong */
                    }
                    /* FIX NGM-Bug00000627 END   2014-09-25 : wuxiaorong */
                    break;
                    /* FIX AM-Enh00000083   END  2010-07-12  : wuxiaorong */
                /* FIX PhyFA-Enh00000250 END  2008-10-22  : wuxiaorong */


                /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
                case MSG_GSM_COMMON_FCP_FRAME_TIMING_CHANGE_REQ:
                    u16_msg_rx_cfg = 0;
                    gsm_fcp_frame_timing_change_req(stp_oal_msg);
                    break;
                /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/

                /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
                case MSG_GSM_COMMON_FCP_AFFAIR_OPERATE_REQ:
                    u16_msg_rx_cfg = 0;
                    gsm_fcp_affair_operate_req(stp_oal_msg);
                    break;
                /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
                default:
                    oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNEXPECTED_LOCAL_MSG_RECEIVED));
                    break;
            }

            u32_gsm_fn = gsm_get_gsm_fn(u16_active_id);
            u16_gsm_state = gsm_get_gsm_state();/* 这个不能往前提,因为中间可能会改变 */
            u16_gsm_fcp_new_state = gsm_get_fcp_new_state();
            u16_gsm_fcp_3rd_state = gsm_get_fcp_3rd_state();
            /*这个next_fn的含义不是真实的next fn，而是配置任务的帧号，有可能存在当帧(物理实际帧号)配置当帧的场景*/
            u32_next_fn = gsm_get_frametimer_rtx_cfg_fn(u32_gsm_fn,stp_oal_msg);
            
            if((u16_gsm_state == u16_gsm_fcp_new_state)
                && ((UINT16)1 == u16_msg_rx_cfg)                       
                && ((UINT16)0xFF == u16_gsm_fcp_3rd_state))
            {
                gsm_fcp_set_frametask_header((UINT16)GSM_FCP_TSNTASK_ONLY_FN,u32_next_fn,(UINT16)0);

                /* FIX PhyFA-Req00000825 BEGIN  2011-01-10 : wuxiaorong */           
                #ifdef PECKER_SWITCH 
                /* Fix PhyFA Req00000171 begin, guxiaobo, 20080826*/
                u16_current_state = u16_gsm_state & (UINT16)0xf0;
                gsm_fcp_check_ded_ch_ind_state(u16_current_state);
                /* Fix PhyFA Req00000171 end, guxiaobo, 20080826*/
                #endif
                /* FIX PhyFA-Req00000825 END  2011-01-10 : wuxiaorong */ 
                /* FIX PhyFA-Enh00000259   BEGIN   2008-12-03  : ningyaojun*/
                u16_gsm_advance_del_fn = GSM_FCP_ADVANCE_DEL_FN;    
                /* FIX PhyFA-Enh00000259   END     2008-12-03  : ningyaojun*/ 
                   
                switch(u16_gsm_state)
                {
                    case GSM_INIT:
                        gsm_fcp_init_state(u32_next_fn,stp_oal_msg,stp_gsm_latest_rtx_frame_pos);
                       break;
                    /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
                    case GSM_SLAVE:
                        #ifdef L1CC_GSM_RTRT_UT_SWITCH
                        stub_gsm_fcp_slave(u32_next_fn,stp_oal_msg);
                        #else
                        gsm_fcp_slave(u32_next_fn,stp_oal_msg,stp_gsm_latest_rtx_frame_pos);
                        #endif
                        break;
                    /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/
                    case GSM_CS:
                        gsm_fcp_cs(u32_next_fn,stp_oal_msg,stp_gsm_latest_rtx_frame_pos);
                       break;
                    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
                    case GSM_IDLE:
                        /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
                        #ifdef L1CC_GSM_RTRT_UT_SWITCH
                        stub_gsm_fcp_idle(u32_next_fn,stp_oal_msg);
                        #else
                        gsm_fcp_idle(u32_next_fn,stp_oal_msg,stp_gsm_latest_rtx_frame_pos);
                        #endif
                        /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/
                       break;
                    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
                    case GSM_CS_RACH:
                        gsm_fcp_cs_rach(u32_next_fn,stp_oal_msg,stp_gsm_latest_rtx_frame_pos);
                       break;
                    case GSM_PS_RACH:
                        gsm_fcp_ps_rach(u32_next_fn,stp_oal_msg,stp_gsm_latest_rtx_frame_pos);
                       break;
                    case GSM_DEDICATED:
                        /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
                        /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/    
                        gsm_fcp_dedicated(u32_next_fn,stp_oal_msg,stp_gsm_latest_rtx_frame_pos);
                       break;
                    case GSM_HANDOVER:
                        gsm_fcp_handover(u32_next_fn,stp_oal_msg,stp_gsm_latest_rtx_frame_pos);
                       break;
                    case GSM_TRANSFER:
                        /* Fix LM-Enh00000211 BEGIN 2011-03-25: dujianzhong */
                        /* Fix LM-Enh00000211 END 2011-03-25: dujianzhong */
                        gsm_fcp_transfer(u32_next_fn,stp_oal_msg,stp_gsm_latest_rtx_frame_pos);
                        /* FIX PhyFA-Enh00000259   BEGIN   2008-11-08  : ningyaojun*/
                        u16_gsm_advance_del_fn = (GSM_FCP_ADVANCE_DEL_FN + 4);
                        /* FIX PhyFA-Enh00000259   END     2008-11-08  : ningyaojun*/
                       break;
                    /* FIX PhyFA-Req00000936 BEGIN  2011-06-02  : wuxiaorong */
                    case GSM_NOCELL:
                         gsm_fcp_nocell(u32_next_fn,stp_oal_msg,stp_gsm_latest_rtx_frame_pos);
                          break;
                    /* FIX PhyFA-Req00000936 END  2011-06-02  : wuxiaorong */
    
                    case GSM_MANU_TEST:
                        gsm_fcp_manu_test(u32_next_fn,stp_oal_msg);
                        /*FIX LM-Req00000395  BEGIN  2011-10-13  : sunzhiqiang*/ 
                        gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)GSM_MAX_FN + (UINT32)1));
                        /*FIX LM-Req00000395  END  2011-10-13  : sunzhiqiang*/ 
                        break;
                    /*FIX PHYFA-Req00000737  BEGIN  2010-06-07 :sunzhiqiang*/
                    case GSM_AMT_DEDICATED:
                        gsm_fcp_amt_dedicated(u32_next_fn,stp_oal_msg);
                        /*FIX LM-Req00000395  BEGIN  2011-10-13  : sunzhiqiang*/ 
                        gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)GSM_MAX_FN + (UINT32)1));                                               
                        /*FIX LM-Req00000395  END  2011-10-13  : sunzhiqiang*/ 
                        break;
                    /*FIX PHYFA-Req00000737  END  2010-06-07 :sunzhiqiang*/
                    default:
                        oal_error_handler(ERROR_CLASS_CRITICAL, GSM_FCP_ERR(UNEXPECTED_SYS_STATE));
                       break;
                }
                /* FIX LM-Bug00002282  BEGIN  2012-08-17 : linlan */ 
                if( MSG_ISR_FCP_FRAMETIMER_IND == msg_primitive_id )
                {                   
                    /* FIX PhyFA-Req00000936 BEGIN  2011-06-09  : wuxiaorong */
                    #ifdef L1CC_GSM_RTRT_UT_SWITCH
                    stub_gsm_fcp_frametimer_post_handle();
                    #else
                    gsm_fcp_frametimer_post_handle();
                    #endif
                    /* FIX PhyFA-Req00000936 END  2011-06-09  : wuxiaorong */ 
                    
                }
                /* FIX LM-Bug00002282  END    2012-08-17 : linlan */
                /* FIX PhyFA-Enh00000259   BEGIN   2008-11-08  : ningyaojun*/
                gsm_fcp_del_frametask(gsm_fn_operate((u32_next_fn + (UINT32)GSM_MAX_FN) - (UINT32)u16_gsm_advance_del_fn),(UINT16) UNSPECIFIED_STANDBY_ID);
                /* FIX PhyFA-Enh00000259   END     2008-11-08  : ningyaojun*/
                /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
                if(OAL_TRUE  == stp_gsm_latest_rtx_frame_pos->b_not_cfg_sleep )
                {
                    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
                }

                /* FIX PhyFA-Req00000985 BEGIN  2011-07-25 : wuxiaorong */   
                OAL_ASSERT((stp_latest_rtx_frame_pos->u32_fn!= u32_next_fn),"g_st_latest_frame_pos[u16_active_id] calculated fail!");

                /* 根据s_u32_latest_rtx_fn[u16_active_id]和当前帧任务情况，看是否需要睡眠 */
                /* FIX PhyFA-Req00001316 BEGIN  2012-11-26 : wuxiaorong */
                #ifdef L1CC_GSM_RTRT_UT_SWITCH
                stub_gsm_fcp_sleep_cfg(u32_next_fn,stp_latest_rtx_frame_pos,(SINT32)0);
                #else
                gsm_fcp_sleep_cfg(u16_active_id,u32_next_fn,stp_latest_rtx_frame_pos);
                #endif
                /* FIX PhyFA-Req00001316 END  2012-11-26 : wuxiaorong */
                /* FIX PhyFA-Req00000985 END  2011-07-25 : wuxiaorong */   
            }
            oal_msg_release(&stp_oal_msg);

            /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/ 

            #ifdef L1CC_GSM_RTRT_UT_SWITCH
            break;
            #endif
        }
        else
        {
            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
            #ifdef L1CC_GSM_RTRT_UT_SWITCH
            break;
            #endif
        }
    }
    
}/*end of gsm_fcp_main*/




L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
OAL_STATUS gsm_fcp_search_spare_windows(SINT16_PTR s16p_cfg_tsn,SINT16_PTR s16p_cfg_rtx_offset,
                                        UINT32 u32_fn,CONST_UINT16 u16_event_len,CONST_SINT16 s16_search_start_offset,CONST_SINT16 s16_search_end_offset)
{
    UINT32 u32_fcp_ret = OAL_FAILURE;
    SINT32 s32_event_start_offset;
    SINT32 s32_event_end_offset = (SINT32)0;
    SINT32 s32_win_start_offset;
    SINT32 s32_win_end_offset;
    CONST gsm_slot_rtx_t *stp_tmp_slot_rtx = NULL_PTR;
    CONST gsm_slot_rtx_t *stp_event_slot_rtx = NULL_PTR;
    SINT16 s16_win_start_tsn;
    SINT16 s16_win_end_tsn;
    SINT16 s16_tmp_tsn;
    SINT16 s16_event_tsn;
    SINT16 s16_tmp_rtx_offset;
    SINT16 j;
    UINT16 u16_conflict_flg;
    SINT16 s16_tmp_cfg_tsn = INVALID_EVENT_TSN;
    SINT16 s16_check_tsn;
    SINT32 s32_rtx_overlap_gap;
    CONST gsm_tstask_attribute_t* stp_tstask_attribute = NULL_PTR;
    /* FIX NGM-Bug00000798  BEGIN   2014-10-22  : guxiaobo */
    SINT32 s32_task_begin_offset; /*该值表示：检测有空闲窗情况下，配置任务的起始位置*/
    SINT32 s32_task_end_offset; /*该值表示：检测有空闲窗情况下，配置任务的结束位置*/
    /* FIX NGM-Bug00000798  END   2014-10-22  : guxiaobo */
   
    
    OAL_ASSERT(NULL_PTR!=s16p_cfg_tsn, "gsm_fcp_search_spare_windows null pointer");
    OAL_ASSERT(NULL_PTR!=s16p_cfg_rtx_offset, "gsm_fcp_search_spare_windows null pointer");

    s32_win_start_offset = (SINT32)s16_search_start_offset;
    s32_rtx_overlap_gap = gsm_get_gsm_rtx2rx_overlap_gap();

    for(;;)
    {
        /* FIX NGM-Bug00000798  BEGIN   2014-10-22  : guxiaobo */
        s32_task_begin_offset = s32_win_start_offset + s32_rtx_overlap_gap; 
        s32_task_end_offset = s32_task_begin_offset + (SINT32)u16_event_len; 
        if(s32_task_end_offset > (SINT32)s16_search_end_offset) /* 配置任务的结束位置已经超过了搜索窗尾，说明不用再进行后续判断了 */
        {
            break;
        }
        
        s32_win_end_offset =  s32_task_end_offset + s32_rtx_overlap_gap;
        /* FIX NGM-Bug00000798  END   2014-10-22  : guxiaobo */

        s16_win_start_tsn = gsm_fcp_calc_tsn(s32_win_start_offset);
        s16_win_end_tsn = gsm_fcp_calc_tsn(s32_win_end_offset);
        u16_conflict_flg = 0;
        for(s16_tmp_tsn = s16_win_start_tsn;s16_tmp_tsn <= s16_win_end_tsn;s16_tmp_tsn++)
        {
            stp_tmp_slot_rtx = gsm_get_tsntask(u32_fn,s16_tmp_tsn);

            for(j = 0; j < (UINT16)2; j++)
            {
                s16_event_tsn = stp_tmp_slot_rtx->s16_occupied_tsn[j];
                if(INVALID_EVENT_TSN == s16_event_tsn)
                {
                    break;
                }
                s16_event_tsn += (((s16_tmp_tsn + (SINT16)8) / (SINT16)8) - (SINT16)1) * (SINT16)8;
                stp_event_slot_rtx = gsm_get_tsntask(u32_fn,s16_event_tsn);

                stp_tstask_attribute = gsm_get_tstask_attribute(stp_event_slot_rtx->u16_tstask);
                s32_event_start_offset = ((SINT32)s16_event_tsn * (SINT32)GSM_QBITS_PER_TS )+ (SINT32)stp_event_slot_rtx->s16_rtx_offset;
                s32_event_end_offset = s32_event_start_offset + stp_tstask_attribute->fp_get_rtx_len(stp_event_slot_rtx->u16_tstask);

                if(  ((s32_win_start_offset < s32_event_start_offset)&&(s32_win_end_offset < s32_event_start_offset))
                   ||((s32_win_start_offset > s32_event_start_offset)&&(s32_win_start_offset > s32_event_end_offset)))
                {
                    ;
                }
                else
                {
                    u16_conflict_flg = 1;
                    break;
                }
            }
            if((UINT16)1 == u16_conflict_flg)
            {
                break;
            }
        }
        if(s16_tmp_tsn == (s16_win_end_tsn + 1))/*succ*/
        {
            /* FIX NGM-Bug00000798  BEGIN   2014-10-22  : guxiaobo */
            s16_tmp_cfg_tsn = (SINT16)((s32_task_begin_offset + (SINT32)12) / (SINT32)GSM_QBITS_PER_TS);
            /* FIX NGM-Bug00000798  END   2014-10-22  : guxiaobo */
            stp_tmp_slot_rtx = gsm_get_tsntask(u32_fn,s16_tmp_cfg_tsn);
            if(stp_tmp_slot_rtx->u16_tstask)
            {
                s32_win_start_offset = (SINT32)((((SINT32)s16_tmp_cfg_tsn + (SINT32)1) * (SINT32)GSM_QBITS_PER_TS) - s32_rtx_overlap_gap);
            }
            else
            {
                 
                 /*check s16_occupied_tsn */
                 /* FIX NGM-Bug00000798  BEGIN   2014-10-22  : guxiaobo */
                 s16_tmp_rtx_offset = (SINT16)(s32_task_begin_offset % (SINT32)GSM_QBITS_PER_TS);
                 /* FIX NGM-Bug00000798  END   2014-10-22  : guxiaobo */
                 if( (GSM_QBITS_PER_TS - 12) <= s16_tmp_rtx_offset )
                 {
                    s16_tmp_rtx_offset -= GSM_QBITS_PER_TS;
                 }
                 /*need to check */
                 if(s16_tmp_rtx_offset < (SINT16)0)
                 {
                     s16_check_tsn = s16_tmp_cfg_tsn - 1;
                     stp_tmp_slot_rtx = gsm_get_tsntask(u32_fn,(SINT16)s16_check_tsn);
                     /*this tsn is occupied , try next tsn */
                     if((stp_tmp_slot_rtx->s16_occupied_tsn[0]==s16_check_tsn)||(stp_tmp_slot_rtx->s16_occupied_tsn[1]==s16_check_tsn) )
                     {
                        s32_win_start_offset = ((SINT32)s16_tmp_cfg_tsn*(SINT32)GSM_QBITS_PER_TS) -s32_rtx_overlap_gap ;
                       
                     }
                     else
                     {   
                         *s16p_cfg_tsn = s16_tmp_cfg_tsn;
                         *s16p_cfg_rtx_offset = s16_tmp_rtx_offset;
                         u32_fcp_ret = OAL_SUCCESS;
                         break;
                     }
                 }   
                 else
                 {
                     *s16p_cfg_tsn = s16_tmp_cfg_tsn;
                     *s16p_cfg_rtx_offset = s16_tmp_rtx_offset;
                     u32_fcp_ret = OAL_SUCCESS;
                     break;
                 }
                
                
            }
        }
        else
        {
           
            if(s32_win_start_offset > s32_event_end_offset)
            {
                oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
            }
            s32_win_start_offset = s32_event_end_offset + (SINT32)1;
        }

    }
    return u32_fcp_ret;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC BOOLEAN gsm_check_flexible_cfg_tsntask(IN CONST_UINT16 u16_tstask)
{
    BOOLEAN b_flexible_cfg_tstask = OAL_FALSE;
    BOOLEAN b_nc_bcch_tsntask = OAL_FALSE;
    
    b_nc_bcch_tsntask = gsm_is_nc_bcch_tsntask(u16_tstask);
    
    if((TSNTASK_TRANSFER_CCCH_RX == u16_tstask)    
       ||(TSNTASK_TRANSFER_NC_BCCH_RX == u16_tstask)
       ||b_nc_bcch_tsntask)
    {
        b_flexible_cfg_tstask = OAL_TRUE;
    }
    return b_flexible_cfg_tstask;
}

L1CC_GSM_DRAM_CODE_SECTION
STATIC BOOLEAN gsm_is_nc_bcch_tsntask(IN CONST_UINT16 u16_tstask)
{
    BOOLEAN b_nc_bcch_tsntask = OAL_FALSE;

    if((TSNTASK_NC0_BCCH_RX == u16_tstask)
       ||(TSNTASK_NC1_BCCH_RX == u16_tstask)
       ||(TSNTASK_NC2_BCCH_RX == u16_tstask)
       ||(TSNTASK_NC3_BCCH_RX == u16_tstask)
       ||(TSNTASK_NC4_BCCH_RX == u16_tstask)
       ||(TSNTASK_NC5_BCCH_RX == u16_tstask))
    {
        b_nc_bcch_tsntask = OAL_TRUE;
    }
    return b_nc_bcch_tsntask;
}


/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_check_tsntask
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2014-08-12      wuxiaorong              PhyFA-Enh00002131    [NGM]L1CC_GSM需要修改局部变量大的函数
*
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
OAL_STATUS gsm_fcp_check_tsntask(IN CONST_UINT16 u16_tstask,IN CONST gsm_offset_in_frame_t* CONST stp_frame_pos, IN CONST_UINT16 u16_standby_id,INOUT UINT16_PTR_CONST u16p_conflict_flag)
{
    UINT16  u16_i;
    UINT32  u32_fcp_ret = OAL_SUCCESS;
    UINT16  u16_tmp_tstask;
    UINT32  u32_fn;
    UINT16  u16_tsn;
    SINT16  s16_rtx_offset;
    UINT16  u16_repeat_frms;
    gsm_del_conflict_tstask_info_t* stp_del_tsntask = NULL_PTR;
    gsm_offset_in_frame_t st_frame_pos;
    /* FIX PhyFA-Req00001525 BEGIN 2013-11-07 : zhengying */
    CONST gsm_conn_ch_tb_t *stp_gsm_conn_ch_tb = NULL_PTR;
    CONST gsm_channel_parameters_t *stp_gsm_params_ch = NULL_PTR;
    /* FIX PhyFA-Req00001525 END   2013-11-07 : zhengying */
    /* FIX LM-Enh00001307 BEGIN  2013-09-13: wangjunli */
    UINT16 u16_conflict_amt = 0;
    UINT16 u16_temp_index = 0;
    BOOLEAN b_flexible_cfg_tsntask = OAL_FALSE;
   
    OAL_ASSERT(NULL_PTR != u16p_conflict_flag,"gsm_fcp_check_tsntask u16p_conflict_flag NULL_PTR");
    /* FIX LM-Enh00001307 END  2013-09-13: wangjunli */


    OAL_ASSERT(NULL_PTR !=stp_frame_pos,"gsm_fcp_check_tsntask,stp_frame_pos =NULL_PTR");

    u32_fn = stp_frame_pos->u32_fn;
    u16_tsn = stp_frame_pos->u16_tsn;
    s16_rtx_offset = stp_frame_pos->s16_offset;


    /* FIX PhyFA-Req00001525 BEGIN 2013-11-07 : zhengying */
    stp_gsm_conn_ch_tb     = gsm_get_conn_ch_tb();
    stp_gsm_params_ch      = &stp_gsm_conn_ch_tb->st_channel_params;
    /* FIX PhyFA-Req00001525 END   2013-11-07 : zhengying */
    if((UINT16)16 < u16_tsn)
    {
        oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
        return OAL_FAILURE;
    }
    u32_fn = gsm_fn_operate(u32_fn + ((UINT32)u16_tsn/(UINT32)GSM_MAX_TIMESLOTS));
    /* FIX Enh00001894   BEGIN     2014-05-12  : xiongjiangjiang */
    u16_tsn = GSM_MOD_2POWER(UINT16, u16_tsn, GSM_MAX_TIMESLOTS);
    /* FIX Enh00001894   END     2014-05-12  : xiongjiangjiang */

    u16_tmp_tstask = u16_tstask & (UINT16)0x7f;

    /* 帧内位置赋值 */
    st_frame_pos.u32_fn = u32_fn;
    st_frame_pos.u16_tsn = u16_tsn;
    st_frame_pos.s16_offset = s16_rtx_offset;

    /* FIX PhyFA-Enh00002131 BEGIN 2014-08-12 : wuxiaorong */
    stp_del_tsntask = (gsm_del_conflict_tstask_info_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,
                                                                     (UINT32)OAL_GET_TYPE_SIZE_IN_WORD(gsm_del_conflict_tstask_info_t));          

    oal_mem_set(stp_del_tsntask,OAL_GET_TYPE_SIZE_IN_WORD(gsm_del_conflict_tstask_info_t),(UINT16)0);
    /* FIX PhyFA-Enh00002131 END   2014-08-12 : wuxiaorong */
    /* FIX LM-Enh00001307 BEGIN  2013-09-13: wangjunli */
    /*检查tstask是否是可灵活配置的时隙任务*/  
    b_flexible_cfg_tsntask =  gsm_check_flexible_cfg_tsntask(u16_tstask);
    if(b_flexible_cfg_tsntask)
    {        
        for(u16_i=(UINT16)0; u16_i<(UINT16)4; u16_i++)
        {
            st_frame_pos.u32_fn = gsm_fn_operate(u32_fn + (UINT32)u16_i);
            u32_fcp_ret = gsm_fcp_check_rtx_conflict(u16_tmp_tstask,&st_frame_pos,stp_del_tsntask,u16_standby_id);
            if(OAL_SUCCESS != u32_fcp_ret)
            {
                u16p_conflict_flag[u16_i] = (UINT16)1;
                u16_conflict_amt++;
            }
        }
        u32_fcp_ret = OAL_SUCCESS; /*尝试删除后判断是否能够配置*/
    }
    else
    /* FIX LM-Enh00001307 END  2013-09-13: wangjunli */   
    {
        u32_fcp_ret = gsm_fcp_check_rtx_conflict(u16_tmp_tstask,&st_frame_pos,stp_del_tsntask,u16_standby_id);
        
        /*FIX LM-Enh00001061  BEGIN   2013-01-30  : ningyaojun*/
        u16_repeat_frms = C_ST_TSTASK_ATTRIBUTE_TBL[u16_tmp_tstask].u16_repeat_frms;

        if((OAL_SUCCESS == u32_fcp_ret) && ((UINT16)4 == u16_repeat_frms))    
        {
            if(!(((TSNTASK_SACCH_RX == u16_tstask) || (TSNTASK_SACCH_TX == u16_tstask))
                 &&((UINT16)MPAL_SDCCH != stp_gsm_params_ch->u16_ch_type)))
            {
                /* 对于 4 帧交织的信道，系统内检查冲突只检查前2帧改为4帧都检查，
                   否则存在允许 BCCH/CCCH 部分 bursts 事务激活成功就配置事件的优化之后，就会出现系统内冲突检查不完整而发生事件冲突(0X1050A)异常，例如:
                   同一帧，先配置 PCH 前2帧因事务冲突没配事件，但成功配置了后2帧的事件，再配置相邻时隙的 NC BCCH，如果系统内只检查前2帧则看不到PCH，会认为不冲突而配置 4 帧接收，后两帧 NC BCCH 和 PCH 未检查，发生冲突。 */
                for(u16_i=(UINT16)1; u16_i<u16_repeat_frms; u16_i++)
                {
                    st_frame_pos.u32_fn = gsm_fn_operate(st_frame_pos.u32_fn + (UINT32)1);
                    u32_fcp_ret = gsm_fcp_check_rtx_conflict(u16_tmp_tstask,&st_frame_pos,stp_del_tsntask,u16_standby_id);
                    if(OAL_SUCCESS != u32_fcp_ret)
                    {
                        break;
                    }
                }
            }
        }
    }

    if(OAL_SUCCESS == u32_fcp_ret)    
    {        
        for(u16_i = 0; u16_i < stp_del_tsntask->u16_del_cnt; u16_i++)
        {

            u32_fcp_ret = gsm_fcp_del_rtx_conflict(u32_fn, u16_tsn,u16_tstask,stp_del_tsntask->u32_del_tstask_fn[u16_i],stp_del_tsntask->s16_del_tstask_tsn[u16_i]);
            if(OAL_SUCCESS != u32_fcp_ret)
            {
                /* FIX LM-Enh00001307 BEGIN  2013-09-13: wangjunli */
                /*FIX PhyFA-Enh00001624  BEGIN   2013-10-22  : wangjunli*/
                if(b_flexible_cfg_tsntask )
                /*FIX PhyFA-Enh00001624  END   2013-10-22  : wangjunli*/
                {
                    u16_temp_index = (UINT16)(((stp_del_tsntask->u32_del_tstask_fn[u16_i]+ (UINT32)GSM_MAX_FN) - u32_fn)%(UINT32)GSM_MAX_FN);
                    
                    OAL_ASSERT(u16_temp_index <(UINT16)4,"u16_temp_index err!");
                    if((UINT16)0 == u16p_conflict_flag[u16_temp_index])
                    {
                        u16p_conflict_flag[u16_temp_index] = 1;
                        u16_conflict_amt++;
                    }
                    u32_fcp_ret = OAL_SUCCESS; /*最后根据u16_conflict_amt判断是否能够配置*/
                }
                else
                {
                    break;
                }
                /* FIX LM-Enh00001307 END  2013-09-13: wangjunli */
            }
        }
    }

    /* FIX PhyFA-Enh00002131 BEGIN 2014-08-12 : wuxiaorong */
    oal_mem_free((CONST_VOID_PTR*)&stp_del_tsntask);
    /* FIX PhyFA-Enh00002131 END   2014-08-12 : wuxiaorong */
    /*FIX PhyFA-Enh00001624  BEGIN   2013-10-22  : wangjunli*/
    if( b_flexible_cfg_tsntask && ((UINT16)2 < u16_conflict_amt))
    {
        u32_fcp_ret = OAL_FAILURE;
    }
    /* FIX LM-Enh00001307 END  2013-09-13: wangjunli */
    return u32_fcp_ret;
}
/***********************************************************************************************************************
* FUNCTION:          gsm_get_rtx_overlap_gap_info
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
STATIC gsm_rtx_overlap_gap_info_t* gsm_get_rtx_overlap_gap_info(VOID)
{
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    
    return &(g_st_gsm_rtx_overlap_gap_info[u16_active_id]);
}


/***********************************************************************************************************************
*  LOCAL FUNCTION DEFINITION
***********************************************************************************************************************/
/***********************************************************************************************************************
* FUNCTION:          gsm_init_rtx_overlap_gap_info
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
STATIC VOID gsm_init_rtx_overlap_gap_info(VOID)
{
    gsm_rtx_overlap_gap_info_t* stp_rtx_gap_info = NULL_PTR;
    UINT16  u16_rfch_id;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    CONST_SINT32  s32_compensate_us = (SINT32)MMC_EVENT_GAP_COMPENSATE_VAL + GSM_INTERRAT_EVENT_GAP;/* us*/

    


    /* 这个不用RFID,只需要知道RFCH_ID  */
    u16_rfch_id = dd_rfch_id_get(u16_active_id,(UINT16)GSM_TIMING);
        
    stp_rtx_gap_info = gsm_get_rtx_overlap_gap_info();
    stp_rtx_gap_info->s32_gsm_rx2gsm_rx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_GSM_RX_GSM_RX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);
    stp_rtx_gap_info->s32_gsm_rx2gsm_tx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_GSM_RX_GSM_TX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);
    stp_rtx_gap_info->s32_gsm_tx2gsm_rx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_GSM_TX_GSM_RX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);
    stp_rtx_gap_info->s32_gsm_tx2gsm_tx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_GSM_TX_GSM_TX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);

    stp_rtx_gap_info->s32_gsm_rtx2gsm_rtx_overlap_gap = GSM_MAX(stp_rtx_gap_info->s32_gsm_rx2gsm_rx_overlap_gap,stp_rtx_gap_info->s32_gsm_rx2gsm_tx_overlap_gap);
    stp_rtx_gap_info->s32_gsm_rtx2gsm_rtx_overlap_gap = GSM_MAX(stp_rtx_gap_info->s32_gsm_rtx2gsm_rtx_overlap_gap,stp_rtx_gap_info->s32_gsm_tx2gsm_rx_overlap_gap);
    stp_rtx_gap_info->s32_gsm_rtx2gsm_rtx_overlap_gap = GSM_MAX(stp_rtx_gap_info->s32_gsm_rtx2gsm_rtx_overlap_gap,stp_rtx_gap_info->s32_gsm_rx2gsm_tx_overlap_gap);

    #if defined(TDS_MODE_SWITCH)
    stp_rtx_gap_info->s32_gsm_rx2tds_rx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_GSM_RX_TDS_RX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);
    stp_rtx_gap_info->s32_gsm_tx2tds_rx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_GSM_TX_TDS_RX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);
    stp_rtx_gap_info->s32_tds_rx2gsm_rx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_TDS_RX_GSM_RX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);
    stp_rtx_gap_info->s32_tds_rx2gsm_tx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_TDS_RX_GSM_TX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);
    #endif
    #if defined(WCDMA_MODE_SWITCH)
    stp_rtx_gap_info->s32_gsm_rx2wcdma_rx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_GSM_RX_WCDMA_RX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);
    stp_rtx_gap_info->s32_gsm_tx2wcdma_rx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_GSM_TX_WCDMA_RX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);
    stp_rtx_gap_info->s32_wcdma_rx2gsm_rx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_WCDMA_RX_GSM_RX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);
    stp_rtx_gap_info->s32_wcdma_rx2gsm_tx_overlap_gap = GSM_MMC_US2GSM_QBIT(DD_WCDMA_RX_GSM_TX_OVERLAP_GAP(u16_rfch_id) +s32_compensate_us);
    #endif
    
    #if defined(LTE_MODE_SWITCH)
    stp_rtx_gap_info->s32_gsm_rx2lte_rx_overlap_gap = GSM_MMC_US2GSM_QBIT(GSM_MAX(DD_GSM_RX_LTETDD_RX_OVERLAP_GAP(u16_rfch_id),DD_GSM_RX_LTEFDD_RX_OVERLAP_GAP(u16_rfch_id)) +s32_compensate_us);
    stp_rtx_gap_info->s32_gsm_tx2lte_rx_overlap_gap = GSM_MMC_US2GSM_QBIT(GSM_MAX(DD_GSM_TX_LTETDD_RX_OVERLAP_GAP(u16_rfch_id),DD_GSM_TX_LTEFDD_RX_OVERLAP_GAP(u16_rfch_id)) +s32_compensate_us);
    stp_rtx_gap_info->s32_lte_rx2gsm_rx_overlap_gap = GSM_MMC_US2GSM_QBIT(GSM_MAX(DD_LTEFDD_RX_GSM_RX_OVERLAP_GAP(u16_rfch_id),DD_LTETDD_RX_GSM_RX_OVERLAP_GAP(u16_rfch_id)) +s32_compensate_us);
    stp_rtx_gap_info->s32_lte_rx2gsm_tx_overlap_gap = GSM_MMC_US2GSM_QBIT(GSM_MAX(DD_LTEFDD_RX_GSM_TX_OVERLAP_GAP(u16_rfch_id),DD_LTEFDD_RX_GSM_TX_OVERLAP_GAP(u16_rfch_id)) +s32_compensate_us);
    #endif
    
    return;
}
/* FIX PhyFA-Enh00000120 BEGIN  2008-05-02  : huangjinfu */
L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
STATIC OAL_STATUS gsm_fcp_compare_prior(UINT32 u32_left_fn,SINT16 s16_left_tsn,CONST_UINT16 u16_right_tstask, CONST_UINT16 u16_right_standby_id)
{
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    UINT32 u32_fcp_ret = OAL_FAILURE;
    UINT16 u16_left_tstask;
    CONST gsm_slot_rtx_t *stp_left_slot_rtx = NULL_PTR;
    UINT16 u16_left_prio;
    UINT16 u16_left_standby_id;
    UINT16 u16_right_prio;    
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    
    stp_gsm_standby_info = gsm_get_standby_info();
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    stp_left_slot_rtx = gsm_get_tsntask(u32_left_fn,s16_left_tsn);
    //u16_left_tsntask = stp_left_slot_rtx->u16_tsntask;

    /*FIX PhyFA-Bug00002713  BEGIN   2011-08-01  : ningyaojun*/
    if(GSM_TSNTASK_EXPIRED == stp_left_slot_rtx->u16_expired)
    {
        return OAL_SUCCESS;
    }
    u16_left_tstask = stp_left_slot_rtx->u16_tstask;
    u16_left_standby_id = stp_left_slot_rtx->u16_standby_id;
    OAL_ASSERT(((u16_left_tstask&(UINT16)0x7f)<(UINT16)GSM_TSTASK_NUM), "gsm_fcp_compare_prior():u16_tstask invalid!\n");
    OAL_ASSERT((u16_left_standby_id<(UINT16)MAX_STANDBY_AMT), "gsm_fcp_compare_prior():u16_standby_id illegal!\n");
    /*FIX PhyFA-Bug00002713  END     2011-08-01  : ningyaojun*/

    /*FIX LM-Bug00001181   BEGIN   2011-11-03  : yuzhengfeng*/
    u16_left_prio   = C_ST_TSTASK_ATTRIBUTE_TBL[u16_left_tstask&((UINT16)0x7f)].u16_priority;    
    u16_right_prio  = C_ST_TSTASK_ATTRIBUTE_TBL[u16_right_tstask&((UINT16)0x7f)].u16_priority;
    /*FIX LM-Bug00001181   END   2011-11-03  : yuzhengfeng*/
    u16_left_prio   = (u16_left_prio*(UINT16)MAX_STANDBY_AMT)+stp_gsm_standby_info->u16_standby_priority[u16_left_standby_id];
    u16_right_prio  = (u16_right_prio*(UINT16)MAX_STANDBY_AMT)+stp_gsm_standby_info->u16_standby_priority[u16_right_standby_id];    
    if(((UINT16)0x80 == (u16_left_tstask & (UINT16)0x80)) && ((UINT16)0x80 == (u16_right_tstask & (UINT16)0x80)))
    {
        oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(TSNTASK_CONFLICT));
        return OAL_FAILURE;
    }

    /* FIX PhyFA-Bug00004545   BEGIN  2014-04-25: xiongjiangjiang */
    if((TSNTASK_NC0_BCCH_RX <= u16_left_tstask) && (TSNTASK_TC_BCCH_RX > u16_left_tstask  ) &&
        (((TSNTASK_NC0_BCCH_RX <= u16_right_tstask) && (TSNTASK_TC_BCCH_RX > u16_right_tstask )) ||
           (TSNTASK_NC_CONN_FCB_RX == u16_right_tstask)||(TSNTASK_NC_CONN_RRTL_FCB_RX == u16_right_tstask)))
    /* FIX PhyFA-Bug00004545   END  2014-04-25: xiongjiangjiang */
    {
        ;
    }
    else if(((u16_left_tstask & (UINT16)0x7f) == (u16_right_tstask & (UINT16)0x7f))
        && ((UINT16)0 == (u16_right_tstask & (UINT16)0x80)) && ((UINT16)0x80 == (u16_left_tstask & (UINT16)0x80)))
    {
        u32_fcp_ret = OAL_SUCCESS;
    }
    else if(u16_left_prio < u16_right_prio)
    {
        u32_fcp_ret = OAL_SUCCESS;
    }
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
    
    return u32_fcp_ret;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/

/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_tsntask_joint(UINT16 u16_left_tstask,UINT16 u16_right_tstask)
{
    UINT32 u32_fcp_ret = OAL_FAILURE;

    u16_left_tstask &= (UINT16)0x7f;
    u16_right_tstask &=(UINT16)0x7f;

    switch(u16_left_tstask)
    {
        case TSNTASK_PDTCH_RX:
            if(TSNTASK_PDTCH_RX == u16_right_tstask)
            {
                u32_fcp_ret = OAL_SUCCESS;
            }
            break;
        /* FIX LM-Req00000128  BEGIN   2011-01-14  : sunzhiqiang*/
        case TSNTASK_MANU_TX:
            if(TSNTASK_MANU_TX == u16_right_tstask)
            {
                u32_fcp_ret = OAL_SUCCESS;
            }
            break;
        /* FIX LM-Req00000128  END   2011-01-14  : sunzhiqiang*/
        /* FIX A2KPH-Bug00001743 BEGIN  2009-03-20  : huangjinfu */
        /* FIX LTE-Enh00000846 BEGIN  2014-04-29: wangjunli */
        case TSNTASK_RRBP_TX0:
        case TSNTASK_PDTCH_TX:
        case TSNTASK_PDTCH_FTA_TX:
        case TSNTASK_PTCCH_TX:
            if((TSNTASK_PDTCH_TX == u16_right_tstask) ||
              (TSNTASK_PDTCH_FTA_TX == u16_right_tstask)||
                (TSNTASK_RRBP_TX0 == u16_right_tstask) ||
                (TSNTASK_PTCCH_TX == u16_right_tstask)
               )
            {
                u32_fcp_ret = OAL_SUCCESS;
            }
            break;
        /* FIX LTE-Enh00000846 END  2014-04-29: wangjunli */
        /* FIX A2KPH-Bug00001743 END  2009-03-20 : huangjinfu */
        default:
            break;
    }
    return u32_fcp_ret;
}


L1CC_GSM_DRAM_CODE_SECTION
STATIC SINT16 gsm_fcp_calc_tsn(CONST_SINT32 s32_event_offset)
{
    
    /*FIX LM-Bug00002656  BEGIN   2013-03-12  : sunzhiqiang*/
    return (SINT16)(((s32_event_offset + ((SINT32)2*(SINT32)GSM_QBITS_PER_FN)) / (SINT32)GSM_QBITS_PER_TS) - (SINT32)16);
    /*FIX LM-Bug00002656  END   2013-03-12  : sunzhiqiang*/
    
}



L1CC_GSM_DRAM_CODE_SECTION
OAL_STATUS gsm_fcp_set_slot_rtxoffset(CONST gsm_slot_rtx_t * CONST stp_slot_rtx,IN CONST_SINT16 s16_rtx_offset,IN CONST_UINT32 u32_fn,IN CONST_UINT16 u16_tsn,UINT16 u16_tstask)
{
    SINT32 s32_event_start_offset;
    SINT32 s32_event_end_offset;
    SINT16 s16_tmp_tsn;
    UINT16 j;
    gsm_slot_rtx_t *stp_tmp_slot_rtx;
    UINT16 u16_event_type;
    UINT16 u16_tmp_tstask;
    CONST gsm_tstask_attribute_t*     stp_tstask_attribute = NULL_PTR;


    OAL_ASSERT(NULL_PTR!=stp_slot_rtx, "gsm_fcp_set_slot_rtxoffset null pointer");
    OAL_ASSERT(((u16_tstask&(UINT16)0x7f)<(UINT16)GSM_TSTASK_NUM),"gsm_fcp_set_slot_rtxoffset():u16_tstask invalid!");


    u16_tmp_tstask = u16_tstask;

    switch(u16_tmp_tstask)
    {
        case TSNTASK_FACCH_H_TX:
            u16_tstask = TSNTASK_FACCH_TX;
            break;
        /* FIX PhyFA-Req00000767  BEGIN  2010-09-28 : songzhiyuan */
        case TSNTASK_TCH_H_TX:
        case TSNTASK_TCH_F96_TX:
        case TSNTASK_TCH_F144_TX:
            u16_tstask = TSNTASK_TCH_TX;
            break;

        case TSNTASK_SACCH_H_TX:
            u16_tstask = TSNTASK_SACCH_TX;
            break;

        case TSNTASK_SACCH_H_RX:
            u16_tstask = TSNTASK_SACCH_RX;
            break;

        case TSNTASK_TCH_H_RX:
        case TSNTASK_TCH_F96_RX:
        case TSNTASK_TCH_F144_RX:
            u16_tstask = TSNTASK_TCH_RX;
            break;
        /* FIX PhyFA-Req00000767  END  2010-09-28 : songzhiyuan */
        default:
            break;
    }



    stp_tstask_attribute = gsm_get_tstask_attribute(u16_tstask);
    
    /* Fix PHYFA-Enh00000707 END 2010-05-27 : guxiaobo */
    u16_event_type = stp_tstask_attribute->u16_event_type;
    /*FIX PhyFA-Req00000936  BEGIN   2011-06-14  : ningyaojun*/
    if(GSM_TX_DATA != stp_tstask_attribute->u16_rtx_status)
    {
        if((-12 > s16_rtx_offset )||((GSM_QBITS_PER_TS - GSM_TS_DELTA) <= s16_rtx_offset  ))
        {
            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_RTX_OFFSET));
            return OAL_FAILURE;
        }
    }
    else
    {
        if(( -256 > s16_rtx_offset ) ||(0 < s16_rtx_offset ))
        {
            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_RTX_OFFSET));
            return OAL_FAILURE;
        }
        /* FIX LTE-Bug00001172  BEGIN 2012-08-16 : wuxiaorong */
        #if 0
        _cache_flush((UINT32)stp_slot_rtx->u16p_tx_addr, (UINT32)GSM_BYTE_SIZE_OF_CCBC_ENC_TXBUFF);
        #endif
        /* FIX LTE-Bug00001172  END 2012-08-16 : wuxiaorong */
        
    }
    /*FIX PhyFA-Req00000936  BEGIN   2011-06-14  : ningyaojun*/

    s32_event_start_offset = (SINT32)(((SINT32)u16_tsn * (SINT32)GSM_QBITS_PER_TS)  +(SINT32) s16_rtx_offset);  //???
    s32_event_end_offset = s32_event_start_offset + (SINT32)stp_tstask_attribute->fp_get_rtx_len(u16_tstask);
    for(s16_tmp_tsn = gsm_fcp_calc_tsn(s32_event_start_offset);
               s16_tmp_tsn <= gsm_fcp_calc_tsn(s32_event_end_offset); s16_tmp_tsn++)
    {
        stp_tmp_slot_rtx = gsm_get_tsntask(u32_fn,s16_tmp_tsn);
        for(j = (UINT16)0; j < (UINT16)2; j++)
        {
            if(INVALID_EVENT_TSN == stp_tmp_slot_rtx->s16_occupied_tsn[j])
            {
                if(((UINT16)0 == j )&& (INVALID_EVENT_TSN != stp_tmp_slot_rtx->s16_occupied_tsn[1]))
                {
                    oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(TSNTASK_CONFLICT));
                    return OAL_FAILURE;
                }
                if(0 > s16_tmp_tsn )
                {
                    stp_tmp_slot_rtx->s16_occupied_tsn[j] = (SINT16)u16_tsn + 8;
                }
                else if( 8 > s16_tmp_tsn)
                {
                    stp_tmp_slot_rtx->s16_occupied_tsn[j] = (SINT16)u16_tsn;
                }
                else if(16 > s16_tmp_tsn)
                {
                    stp_tmp_slot_rtx->s16_occupied_tsn[j] = (SINT16)u16_tsn - 8;
                }
                else if(24 > s16_tmp_tsn  )
                {
                    stp_tmp_slot_rtx->s16_occupied_tsn[j] = (SINT16)u16_tsn - 16;
                }
                else
                {
                    oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_TSN));
                    return OAL_FAILURE;
                }
                break;
            }
        }
        
        if(s16_tmp_tsn == (SINT16)u16_tsn)
        {
            stp_tmp_slot_rtx->s16_rtx_offset = s16_rtx_offset;
            stp_tmp_slot_rtx->u16_tstask = u16_tstask;
            /* FIX A2KPH-Bug00000543 BEGIN  2008-08-05 : wuxiaorong */
            stp_tmp_slot_rtx->u16_expired = GSM_TSNTASK_NOT_EXPIRED;
            /* FIX A2KPH-Bug00000543 END  2008-08-05 : wuxiaorong */
            if((UINT16)0 == (u16_tstask & (UINT16)0x80))/*if is not pseudo task*/
            {
                stp_tmp_slot_rtx->u16_event_type= u16_event_type;
            }
        }
    }
    return OAL_SUCCESS;
}

/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_check_rtx_conflict
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*   2014-06-13      linlan                  PhyFA-Bug00004976   [NGM]调用DD接口dd_gsm_event_del入参使用错误 
***********************************************************************************************************************/

/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/

/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_check_rtx_conflict(UINT16 u16_tstask,IN CONST gsm_offset_in_frame_t* CONST stp_frame_pos,INOUT gsm_del_conflict_tstask_info_t* CONST stp_del_tsntask,UINT16 u16_standby_id)
{
    UINT16 u16_del_tstask;
    SINT32 s32_event_start_offset;
    SINT32 s32_event_end_offset;
    SINT32 s32_win_start_offset;
    SINT32 s32_win_end_offset;
    gsm_slot_rtx_t *stp_tmp_slot_rtx = NULL_PTR;
    CONST gsm_slot_rtx_t *stp_event_slot_rtx = NULL_PTR;
    SINT16 s16_win_start_tsn;
    SINT16 s16_win_end_tsn;
    SINT16 s16_tmp_tsn;
    SINT16 s16_event_tsn;
    UINT32 u32_fcp_ret = OAL_SUCCESS;
    UINT16 j;
    UINT16 u16_tmp_del_cnt;
    UINT16 k;
    /* FIX LM-Bug00001262 BEGIN 2011-11-13: linlan */ 
    UINT32 u32_del_event_fn;
    UINT16 u16_del_event_tsn;
    /* FIX LM-Bug00001262 END 2011-11-13: linlan */ 
    /* FIX PhyFA-Req00000423 BEGIN 2009-09-24: wuxiaorong */
    SINT32 s32_temp_offset;    
    /* FIX AM-Bug00000109 BEGIN 2010-06-28: wuxiaorong */
    UINT16 u16_compare_tstask = 0;
    /* FIX AM-Bug00000109 END   2010-06-28: wuxiaorong */
    /* FIX PhyFA-Enh00001237 BEGIN  2012-04-30  : linlan */
    UINT16 u16_compare_rtx_status = 0;
    UINT16 u16_tsk_rtx_status = 0;
    SINT32 s32_trx_offset_qbit = (SINT32)0;
    SINT32 s32_tsk_offset_begin = (SINT32)0;
    SINT32 s32_tsk_offset_end = (SINT32)0;
    BOOLEAN b_event_not_conflict = OAL_FALSE;

    /* FIX LTE-Bug00001401  BEGIN  2012-12-10 : wuxiaorong */
    fn_offset_info_t  st_net_fn_offset_del = {(UINT16)0,(UINT16)0,(UINT32)0,(SINT32)0};
    /* FIX LTE-Bug00001401  END  2012-12-10 : wuxiaorong */

    UINT32 u32_fn;
    UINT16 u16_tsn;
    SINT16 s16_rtx_offset;
    CONST gsm_tstask_attribute_t* stp_tstask_attribute   = NULL_PTR;
    CONST gsm_tstask_attribute_t* stp_compare_tstask_attribute   = NULL_PTR;
    CONST gsm_rtx_overlap_gap_info_t* stp_rtx_gap_info = NULL_PTR;
    gsm_conflict_and_delete_param_save_t* stp_conflict_delete_pavam = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    
    stp_rtx_gap_info =&(g_st_gsm_rtx_overlap_gap_info[u16_active_id]);
    stp_conflict_delete_pavam = &(g_st_conflict_and_delete_param_save[u16_active_id]);


    OAL_ASSERT(NULL_PTR!=stp_del_tsntask, "gsm_fcp_check_rtx_conflict null pointer");
    OAL_ASSERT(NULL_PTR!=stp_frame_pos, "gsm_fcp_check_rtx_conflict.stp_frame_pos null pointer");

    u32_fn  = stp_frame_pos->u32_fn;
    u16_tsn = stp_frame_pos->u16_tsn;
    s16_rtx_offset = stp_frame_pos->s16_offset;
    /*FIX PHYFA-Req00001291   BEGIN   2012-06-11  : linlan*/
    stp_conflict_delete_pavam->u16_add_tsntask = u16_tstask;
    stp_conflict_delete_pavam->u32_add_fn = u32_fn;
    stp_conflict_delete_pavam->s16_rtx_offset = s16_rtx_offset;
    stp_conflict_delete_pavam->u16_add_tsn = u16_tsn;
    /*FIX PHYFA-Req00001291   END     2012-06-11  : u16_tsn*/

    stp_tstask_attribute = gsm_get_tstask_attribute(u16_tstask);

    
    s32_temp_offset = (SINT32)(((SINT32)u16_tsn * (SINT32)GSM_QBITS_PER_TS) + (SINT32)s16_rtx_offset);
    s32_tsk_offset_begin = s32_temp_offset;
    s32_tsk_offset_end = s32_tsk_offset_begin + stp_tstask_attribute->fp_get_rtx_len(u16_tstask);
    /* FIX PhyFA-Enh00001237 END    2012-04-30  : linlan */


    /* FIX AM-Bug00000109 BEGIN 2010-06-28: wuxiaorong */
    /* FIX AM-Bug00000310 BEGIN 2010-11-09: wuxiaorong */
    /*FIX PhyFA-Req00001305  BEGIN   2012-07-02  : wuxiaorong*/
    //u16_rtx_status = stp_tstask_attribute->u16_rtx_status;

    
    s32_win_start_offset = s32_temp_offset - stp_rtx_gap_info->s32_gsm_rtx2gsm_rtx_overlap_gap;//GSM_RX_GSM_OFFSET_QBIT_BEFORE_RTX;
    s32_win_end_offset   = ((s32_temp_offset + stp_tstask_attribute->fp_get_rtx_len(u16_tstask)) + (SINT32)stp_rtx_gap_info->s32_gsm_rtx2gsm_rtx_overlap_gap/*GSM_RX_GSM_OFFSET_QBIT_AFTER_RTX*/);

    /* FIX AM-Bug00000109 END   2010-06-28: wuxiaorong */
    /* FIX PhyFA-Req00000423 END 2009-09-24: wuxiaorong */

    s16_win_start_tsn = gsm_fcp_calc_tsn(s32_win_start_offset);
    s16_win_end_tsn = gsm_fcp_calc_tsn(s32_win_end_offset);

    stp_tmp_slot_rtx = gsm_get_tsntask(u32_fn, (SINT16)((SINT16)u16_tsn - (SINT16)1));
    /*FIX PhyFA-Req00001045  BEGIN   2011-09-09  : ningyaojun*/
    /* FIX LM-Bug00000474 BEGIN 2011-04-13: wanghuan */ 
    /* FIX LM-Bug00001262 BEGIN 2011-11-13: linlan */ 
    if(  (OAL_SUCCESS == gsm_fcp_tsntask_joint(u16_tstask,stp_tmp_slot_rtx->u16_tstask))/*tsntasks can just be jointed when they have the same rtx_offset*/
       /*&&(GSM_TSNTASK_NOT_DOWNLOADED == stp_tmp_slot_rtx->u16_downloaded_ind)*/
       &&(s16_rtx_offset == stp_tmp_slot_rtx->s16_rtx_offset )
      )
    /* FIX LM-Bug00000474 END 2011-04-13: wanghuan */ 
    /*FIX PhyFA-Req00001045  END     2011-09-09  : ningyaojun*/ 
    {
        if((UINT16)GSM_TSNTASK_DOWNLOADED == stp_tmp_slot_rtx->u16_downloaded_ind)
        {
            if(u16_tsn >= (UINT16)1)
            {
                u32_del_event_fn = u32_fn;
                u16_del_event_tsn = (UINT16)((UINT16)u16_tsn - (UINT16)1);
            }
            else
            {
                u32_del_event_fn = gsm_fn_operate((u32_fn + (UINT32)GSM_MAX_FN) -(UINT32)1);
                u16_del_event_tsn = (UINT16)(((UINT16)u16_tsn + (UINT16)MAX_GSM_SLOT_NUM) - (UINT16)1);
            }

            /* FIX LTE-Bug00001401  BEGIN  2012-12-10 : wuxiaorong */    
            if((OAL_STATUS)OAL_SUCCESS == dd_gsm_event_del(u16_active_id,(UINT16)TX_GSM_NSLOT, u32_del_event_fn, u16_del_event_tsn,&st_net_fn_offset_del))
            /* FIX LTE-Bug00001401  END  2012-12-10 : wuxiaorong */
            {
                stp_tmp_slot_rtx->u16_downloaded_ind = GSM_TSNTASK_NOT_DOWNLOADED;
            } 
        }
        if((UINT16)GSM_TSNTASK_NOT_DOWNLOADED == stp_tmp_slot_rtx->u16_downloaded_ind)
        {
            /*FIX PhyFA-Bug00002713  BEGIN   2011-07-21  : ningyaojun*/        
            if(((UINT16)TSNTASK_PDTCH_RX == u16_tstask)&&((UINT16)0 == u16_tsn))
            {
                /*cross-frame TSNTASK_PDTCH_RX is not allowed, L2000-Bug00000199*/  
            }
            else
            {
                s16_win_start_tsn = (SINT16)u16_tsn;
            }
        }
        /*FIX PhyFA-Bug00002713  END     2011-07-21  : ningyaojun*/       
        /* FIX LM-Bug00001262 END 2011-11-13: linlan */ 
    }

    stp_tmp_slot_rtx = gsm_get_tsntask(u32_fn, (SINT16)((SINT16)u16_tsn + (SINT16)1));
    /*FIX PhyFA-Req00001045  BEGIN   2011-09-09  : ningyaojun*/
    /* FIX LM-Bug00000474 BEGIN 2011-04-13: wanghuan */ 
    /* FIX LM-Bug00001262 END 2011-11-13: linlan */ 
    if(  (OAL_SUCCESS == gsm_fcp_tsntask_joint(u16_tstask,stp_tmp_slot_rtx->u16_tstask))
       /*&&(GSM_TSNTASK_NOT_DOWNLOADED == stp_tmp_slot_rtx->u16_downloaded_ind)*/
       &&(s16_rtx_offset == stp_tmp_slot_rtx->s16_rtx_offset)
      )
    /* FIX LM-Bug00000474 END 2011-04-13: wanghuan */ 
    /*FIX PhyFA-Req00001045  END     2011-09-09  : ningyaojun*/ 
    {

        if((UINT16)GSM_TSNTASK_DOWNLOADED == stp_tmp_slot_rtx->u16_downloaded_ind)
        {
            if(u16_tsn >= (UINT16)7)
            {
                u32_del_event_fn = gsm_fn_operate(u32_fn + (UINT32)1);
                /* FIX Enh00001894   BEGIN     2014-05-12  : xiongjiangjiang */
                u16_del_event_tsn = GSM_MOD_2POWER(UINT16, (u16_tsn + 1), MAX_GSM_SLOT_NUM);
                /* FIX Enh00001894   END     2014-05-12  : xiongjiangjiang */
            }
            else
            {
                u32_del_event_fn = u32_fn;
                u16_del_event_tsn = (UINT16)((UINT16)u16_tsn + (UINT16)1);
            }
            
            /* FIX LTE-Bug00001401  BEGIN  2012-12-10 : wuxiaorong */
            if((OAL_STATUS)OAL_SUCCESS == dd_gsm_event_del(u16_active_id,(UINT16)TX_GSM_NSLOT, u32_del_event_fn, u16_del_event_tsn,&st_net_fn_offset_del))
            /* FIX LTE-Bug00001401  END  2012-12-10 : wuxiaorong */
            {
                stp_tmp_slot_rtx->u16_downloaded_ind = GSM_TSNTASK_NOT_DOWNLOADED;
            }
        }
        if((UINT16)GSM_TSNTASK_NOT_DOWNLOADED == stp_tmp_slot_rtx->u16_downloaded_ind)
        {
            s16_win_end_tsn = (SINT16)u16_tsn - 1;
        }
        /* FIX LM-Bug00001262 END 2011-11-13: linlan */ 
    }

    u16_tmp_del_cnt = stp_del_tsntask->u16_del_cnt;
    for(s16_tmp_tsn = s16_win_start_tsn;s16_tmp_tsn <= s16_win_end_tsn;s16_tmp_tsn++)
    {
        stp_tmp_slot_rtx = gsm_get_tsntask(u32_fn,s16_tmp_tsn);

        for(j = 0; j < (UINT16)2; j++)
        {
            /* FIX PhyFA-Enh00001237 BEGIN  2012-04-30  : linlan */
            b_event_not_conflict = OAL_FALSE;
            /* FIX PhyFA-Enh00001237 END    2012-04-30  : linlan */
            s16_event_tsn = stp_tmp_slot_rtx->s16_occupied_tsn[j];
            if(INVALID_EVENT_TSN == s16_event_tsn)
            {
                break;
            }
            s16_event_tsn += (((s16_tmp_tsn + 8) / 8 )- 1) * 8;
            stp_event_slot_rtx = gsm_get_tsntask(u32_fn,s16_event_tsn);
            /* FIX AM-Bug00000109 BEGIN 2010-06-28: wuxiaorong */
            u16_compare_tstask = stp_event_slot_rtx->u16_tstask & (UINT16)0x7f;
            /* FIX AM-Bug00000109 END   2010-06-28: wuxiaorong */

            stp_compare_tstask_attribute = gsm_get_tstask_attribute(u16_compare_tstask);
           
            //u16_event_prior = stp_event_slot_rtx->u16_ts_prio;
            s32_event_start_offset = (SINT32)(((SINT32)s16_event_tsn *(SINT32) GSM_QBITS_PER_TS )+ (SINT32)stp_event_slot_rtx->s16_rtx_offset);
            s32_event_end_offset = (SINT32)(s32_event_start_offset + stp_compare_tstask_attribute->fp_get_rtx_len(u16_compare_tstask));
            /* FIX PhyFA-Enh00001237 BEGIN  2012-04-30  : linlan */
            u16_compare_rtx_status = stp_compare_tstask_attribute->u16_rtx_status;
            u16_tsk_rtx_status = stp_tstask_attribute->u16_rtx_status;
            
            if ( s32_tsk_offset_begin > s32_event_end_offset)
            {
                s32_trx_offset_qbit = gsm_get_compare_rtx_overlap_gap(u16_compare_rtx_status,u16_tsk_rtx_status);
                //这个linlan review一下
                //s32_trx_offset_qbit = g_s32_gsm_gsm_overlap_gap_array[u16_compare_rtx_status - (UINT16)GSM_RX_GSM_DATA][u16_tsk_rtx_status - (UINT16)GSM_RX_GSM_DATA];
                if ((s32_event_end_offset + s32_trx_offset_qbit) < s32_tsk_offset_begin)
                {
                    b_event_not_conflict = OAL_TRUE;
                }
            }
            else if (s32_tsk_offset_end < s32_event_start_offset)
            {
                s32_trx_offset_qbit = gsm_get_compare_rtx_overlap_gap(u16_tsk_rtx_status,u16_compare_rtx_status);
                //这个linlan review一下
                //s32_trx_offset_qbit = g_s32_gsm_gsm_overlap_gap_array[u16_tsk_rtx_status - (UINT16)GSM_RX_GSM_DATA][u16_compare_rtx_status - (UINT16)GSM_RX_GSM_DATA];
                if ((s32_event_start_offset - s32_trx_offset_qbit) > s32_tsk_offset_end)
                {
                    b_event_not_conflict = OAL_TRUE;
                }
            }
            if (b_event_not_conflict)
            /* FIX PhyFA-Enh00001237 END  2012-04-30  : linlan */   
            {
                ;
            }
            else
            {
                /* FIX PhyFA-Enh00000120 BEGIN  2008-05-02  : huangjinfu */
                if(OAL_SUCCESS == gsm_fcp_compare_prior(u32_fn,s16_event_tsn,u16_tstask,u16_standby_id))
                {
                    /* FIX A2KPH-Bug00003516  BEGIN   2010-04-27  : ningyaojun*/   
                    if(s16_event_tsn < -16)
                    {
                        /*FIX PhyFA-Enh00001343 BEGIN 2012-09-03: dujianzhong */
                        OAL_PRINT(u16_standby_id,
                                  (UINT16)GSM_MODE,
                                  "nextfn=0x%lx,tsn=0x%lx,task=0x%lx; EVENT is too late to be deleted:fn=0x%lx,tsn=%ld,task=0x%lx\r\n",
                                  u32_fn,
                                  (UINT32)u16_tsn,
                                  (UINT32)u16_tstask,
                                  u32_fn,
                                  (SINT32)s16_event_tsn,     
                                  (UINT32)stp_event_slot_rtx->u16_tstask);   
                        /*FIX PhyFA-Enh00001343 END   2012-09-03: dujianzhong */
                        u32_fcp_ret = OAL_FAILURE;
                        break;
                    }
                    /* FIX A2KPH-Bug00003516  END     2010-04-27  : ningyaojun*/
                    else
                    {
                        /* FIX PhyFA-Bug00000520 BEGIN  2008-06-16  : huangjinfu */
                        /* FIX A2KPH-Bug00000913 BEGIN  2008-09-24: wuxiaorong */
                        u16_del_tstask = stp_event_slot_rtx->u16_tstask;
                        for(k = 0; k < u16_tmp_del_cnt; k++)
                        {
                            if((s16_event_tsn == stp_del_tsntask->s16_del_tstask_tsn[k]) &&
                               (u16_del_tstask == stp_del_tsntask->u16_del_tstask[k]) &&
                               /* FIX LM-Enh00000503  BEGIN  2011-12-26: linlan */
                               (u32_fn == stp_del_tsntask->u32_del_tstask_fn[k]))
                               /* FIX LM-Enh00000503  END  2011-12-26: linlan */
                            {
                                break;
                            }
                        }
                        if(k == u16_tmp_del_cnt)
                        {
                            stp_del_tsntask->u32_del_tstask_fn[u16_tmp_del_cnt] = u32_fn;
                            stp_del_tsntask->s16_del_tstask_tsn[u16_tmp_del_cnt] = s16_event_tsn;
                            stp_del_tsntask->u16_del_tstask[u16_tmp_del_cnt] = u16_del_tstask;
                            u16_tmp_del_cnt++;
                        }
                        /* FIX A2KPH-Bug00000913 END  2008-09-24: wuxiaorong */
                        /* FIX PhyFA-Bug00000520 END  2008-06-16  : huangjinfu */
                    }
                }
                else
                {
                    u32_fcp_ret = OAL_FAILURE;
                    break;
                }
            }
        }
        /* FIX PhyFA-Bug00000520 BEGIN  2008-06-16  : huangjinfu */
        if(OAL_FAILURE == u32_fcp_ret)
        {
           break;
        }
    }

    stp_del_tsntask->u16_del_cnt = u16_tmp_del_cnt;

    /* FIX PhyFA-Bug00000520 END  2008-06-16  : huangjinfu */

    /* FIX A2KPH-Bug00000913 BEGIN  2008-09-30: wuxiaorong */
    if(OAL_SUCCESS == u32_fcp_ret)
    {
         /* FIX A2KPH-Bug00001828 BEGIN  2009-03-25: wuxiaorong */
        if(OAL_FAILURE == gsm_fcp_compare_prior(u32_fn, (SINT16)u16_tsn,u16_tstask, u16_standby_id))
        {
            return OAL_FAILURE;
        }
        /* FIX A2KPH-Bug00001828 END  2009-03-25: wuxiaorong */
        stp_event_slot_rtx = gsm_get_tsntask(u32_fn, (SINT16)u16_tsn);
        u16_del_tstask  = stp_event_slot_rtx->u16_tstask;
        k = stp_del_tsntask->u16_del_cnt;
        if(((UINT16)GSM_TSNTASK_TSN_INVALID != u16_del_tstask) && (TSNTASK_NULL != u16_del_tstask))
        {
           for(j = 0; j < k; j++)
            {
                if(((u16_del_tstask == stp_del_tsntask->u16_del_tstask[j])
                    &&(u32_fn  == stp_del_tsntask->u32_del_tstask_fn[j])) 
                    &&((SINT16)u16_tsn == stp_del_tsntask->s16_del_tstask_tsn[j]))
                {
                      break;
                }
            }
            if(j == k)
            {
                stp_del_tsntask->u32_del_tstask_fn[k] = u32_fn;
                stp_del_tsntask->s16_del_tstask_tsn[k] = (SINT16)u16_tsn;
                stp_del_tsntask->u16_del_tstask[k] = u16_del_tstask;
               (stp_del_tsntask->u16_del_cnt)++;
            }
        }
    }
    /* FIX A2KPH-Bug00000913 END  2008-09-30: wuxiaorong */
    /*FIX PHYFA-Req00001291   BEGIN   2012-06-11  : linlan*/
    if ((UINT16)0 != stp_del_tsntask->u16_del_cnt)
    {
        oal_mem_copy(&(stp_conflict_delete_pavam->gsm_del_conflict_task_info), stp_del_tsntask, (UINT16)OAL_GET_TYPE_SIZE_IN_WORD(gsm_del_conflict_tstask_info_t));
    }   
    /*FIX PHYFA-Req00001291   END     2012-06-11  : linlan*/
    return u32_fcp_ret;
}
/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_del_bitmap
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/

/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_del_bitmap(CONST_UINT32 u32_fn, CONST_SINT16 s16_deleted_tsn)
{
    gsm_main_frametask_t *stp_main_frametask;
    UINT16 u16_tmp_tsn;
    UINT16 u16_bitmap;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    /*FIX LM-Bug00002347   BEGIN   2012-09-19  : linlan*/
    stp_main_frametask = gsm_get_main_frametask(gsm_fn_operate((UINT32)((u32_fn + (UINT32)(UINT32)GSM_MAX_FN) + ((UINT32)(((SINT32)s16_deleted_tsn + (SINT32)16)/(SINT32)8)-(UINT32)2))),u16_active_id);
    /* FIX PhyFA-Bug00000479 BEGIN  2008-05-31  : huangjinfu */
    u16_tmp_tsn = (UINT16)((UINT16)(s16_deleted_tsn + (SINT16)16) % (UINT16)8);
    /*FIX LM-Bug00002347   END     2012-09-19  : linlan*/
    u16_bitmap = (UINT16)(1 << u16_tmp_tsn);
    /* FIX PhyFA-Bug00000479 END    2008-05-31  : huangjinfu */

    /* FIX PhyFA-Bug00000520 BEGIN  2008-06-21  : huangjinfu */
    if(stp_main_frametask->u16_rx_bitmap & u16_bitmap)
    {
        stp_main_frametask->u16_rx_bitmap &=  (~u16_bitmap);
    }
    else if(stp_main_frametask->u16_tx_bitmap & u16_bitmap)
    /* FIX PhyFA-Bug00000520 END  2008-06-21  : huangjinfu */
    {
        stp_main_frametask->u16_tx_bitmap &=  (~u16_bitmap);
    }
    else
    {
       oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_BITMAP));
       return OAL_FAILURE;
    }

    return OAL_SUCCESS;
}


/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_del_rtx_conflict
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*   2014-06-13      linlan                  PhyFA-Bug00004976   [NGM]调用DD接口dd_gsm_event_del入参使用错误 
***********************************************************************************************************************/

/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
/* FIX A2KPH-Bug00003516  BEGIN   2010-04-27  : ningyaojun*/
L1CC_GSM_DRAM_CODE_SECTION
/*u16_tstask and u16_deleted_tstask(u16_tstask&0x7f) are useless here!*/
STATIC OAL_STATUS gsm_fcp_del_rtx_conflict(UINT32 u32_fn, UINT16 u16_tsn, UINT16 u16_tstask, /*CONST_UINT16 u16_deleted_tstask,*/UINT32 u32_deleted_fn,SINT16 s16_deleted_tsn)
{
    UINT16 u16_burst_start_index;
    UINT16 u16_burst_end_index;
    UINT16 u16_burst_index;
    CONST gsm_slot_rtx_t *stp_del_slot_rtx = NULL_PTR;
    UINT16 u16_del_tstask;
    /* FIX A2KPH-Bug00000543    BEGIN   2008-07-22  : wuxiaorong */
    UINT32 u32_temp_fn;
    /* FIX A2KPH-Bug00000543    END     2008-07-22  : wuxiaorong */

    /* FIX A2KPH-Bug00003516  BEGIN   2010-03-06  : ningyaojun*/
    UINT32  u32_event_fn;
    SINT16   s16_event_tsn;
    //gsm_main_frametask_t    *stp_frame_task = NULL_PTR;
    UINT16       u16_event_del_flag = 0; /*0:nothing deleted, 1:event deleted, 2:tsntask deleted*/
    OAL_STATUS  u32_fcp_ret = OAL_SUCCESS;      
    /* FIX A2KPH-Bug00003516  END     2010-03-06  : ningyaojun*/            
    /* FIX LM-Bug00001262 BEGIN 2011-11-30: linlan */
    UINT16 u16_del_event_type;
    /* FIX LM-Bug00001262 EDN 2011-11-30: linlan */


    /*FIX LM-Bug00002350  BEGIN   2012-09-29  : ningyaojun*/
    UINT16  u16_repeat_frms = 0;
    /*FIX LM-Bug00002350  END     2012-09-29  : ningyaojun*/
    /* FIX LTE-Bug00001401  BEGIN  2012-12-10 : wuxiaorong */
    fn_offset_info_t  st_net_fn_offset_del = {(UINT16)0,(UINT16)0,(UINT32)0,(SINT32)0};
    /* FIX LTE-Bug00001401  END  2012-12-10 : wuxiaorong */
    //CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    

    
    stp_del_slot_rtx = gsm_get_tsntask(u32_deleted_fn,s16_deleted_tsn);
    u16_del_tstask = stp_del_slot_rtx->u16_tstask;
    /*FIX LM-Bug00002350  BEGIN   2012-09-29  : ningyaojun*/
    u16_repeat_frms = C_ST_TSTASK_ATTRIBUTE_TBL[u16_del_tstask].u16_repeat_frms;
    /*FIX LM-Bug00002350  END     2012-09-29  : ningyaojun*/
    
    /* FIX PhyFA-Bug00002849  BEGIN  2011-08-23  : yuzhengfeng */
    #if 0
    OAL_ASSERT((u16_deleted_tstask==u16_del_tstask),"gsm_fcp_del_rtx_conflict():u16_deleted_tstask illegal!");
    #endif
    /* FIX PhyFA-Bug00002849  END  2011-08-23  : yuzhengfeng */
    /* FIX PhyFA-Bug00000425 BEGIN  2008-05-09  : huangjinfu */
    if((UINT16)0x80 == (u16_del_tstask & (UINT16)0x80))
    {
        /* FIX LTE-Enh00000846 BEGIN  2014-04-29: wangjunli */
        if((TSNTASK_PDTCH_TX == (u16_del_tstask & (UINT16)0x7f))||(TSNTASK_PDTCH_FTA_TX == (u16_del_tstask & (UINT16)0x7f)))
        /* FIX LTE-Enh00000846 END  2014-04-29: wangjunli */
        {
            u16_burst_start_index = stp_del_slot_rtx->u16_burst_index;
            u16_burst_end_index = 2;
        }
        else
        {
            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(TSNTASK_CONFLICT));
            return OAL_FAILURE;
        }
    }
    //else if(s_u16_gsm_tsntask_4burst_flg[u16_del_tstask])
    /* FIX PhyFA-Bug00000425 END  2008-05-09  : huangjinfu */
    /*FIX LM-Bug00002350  BEGIN   2012-09-29  : ningyaojun*/
    else if(u16_repeat_frms == (UINT16)4)  /*BCCH/CCCH/TCH_F/PDTCH..._RX/TX*/
    {
        u16_burst_start_index = stp_del_slot_rtx->u16_burst_index;
        u16_burst_end_index = (UINT16)((((UINT32)u16_burst_start_index/(UINT32)4) + (UINT32)1)*(UINT32)4) ;       
    }
    else if(u16_repeat_frms == (UINT16)8) /*TSNTASK_FACCH_H_TX*/
    {
        u16_burst_start_index = stp_del_slot_rtx->u16_burst_index;
        u16_burst_end_index = u16_repeat_frms;
    }
    /*FIX LM-Bug00002350  END     2012-09-29  : ningyaojun*/     
    else
    {
        u16_burst_start_index = 0;
        u16_burst_end_index = 1;
    }
    
    for(u16_burst_index = u16_burst_start_index; u16_burst_index < u16_burst_end_index; u16_burst_index++)
    {
        /* FIX A2KPH-Bug00000543    BEGIN   2008-07-22  : wuxiaorong */
        u32_temp_fn = gsm_fn_operate((u32_deleted_fn + (UINT32)u16_burst_index) - (UINT32)u16_burst_start_index);
        stp_del_slot_rtx = gsm_get_tsntask(u32_temp_fn, s16_deleted_tsn);
        /*FIX PhyFA-Req00001305  BEGIN   2012-07-02  : wuxiaorong*/
        if((UINT16)GSM_TSNTASK_EXPIRED != stp_del_slot_rtx->u16_expired)
        /*FIX PhyFA-Req00001305  END   2012-07-02  : wuxiaorong*/
        {
            u16_event_del_flag =0;
            u32_event_fn = u32_temp_fn;
            s16_event_tsn = s16_deleted_tsn;
            /*Note: here u32_temp_fn may be larger than u32_fn, i.e. u32_temp_fn>(g_u32_gsm_fn[u16_active_id]+1), for u16_burst_index increases!*/
            if((UINT16)0x00 == (stp_del_slot_rtx->u16_tstask & (UINT16)0x80))                /*the slot is not a pseudo task*/              
            {
                if(s16_event_tsn<-16)
                {
                    oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_TSN));
                }
                else if(s16_event_tsn<-8)
                {
                    u32_event_fn = gsm_fn_operate((u32_event_fn + (UINT32)GSM_MAX_FN) - (UINT32)2);
                    s16_event_tsn += 16;
                }
                else if(s16_event_tsn<0)
                {
                    u32_event_fn = gsm_fn_operate((u32_event_fn + (UINT32)GSM_MAX_FN) - (UINT32)1);
                    s16_event_tsn += 8;
                }
                else if(s16_event_tsn>=8)
                {
                    /*there is no need to judge event deleting...*/
                }

                //stp_frame_task = gsm_get_main_frametask(u32_event_fn);  
                
                /* FIX LM-Bug00000379    BEGIN  2011-02-11  : wanghuan */
                if((UINT16)GSM_TSNTASK_DOWNLOADED == stp_del_slot_rtx->u16_downloaded_ind) /*the slot has been downloaded*/ 
                {
                    /*if(OAL_SUCCESS != oal_rtx_task_del(stp_frame_task, NULL_PTR, s16_event_tsn, 1))*/
                    /* FIX LM-Bug00000729  BEGIN  2011-08-29  : yuzhengfeng */
                    /* FIX LM-Bug00001262 BEGIN 2011-11-30: linlan */ /*fix again. dd can't recognise TX_GSM_GMSK_NB and TX_GSM_8PSK_NB*/
                    if(((UINT16)TX_GSM_GMSK_NB == stp_del_slot_rtx->u16_event_type)||((UINT16)TX_GSM_8PSK_NB == stp_del_slot_rtx->u16_event_type))
                    {
                        u16_del_event_type = (UINT16)TX_GSM_NSLOT;
                    }
                    else
                    {
                        u16_del_event_type = stp_del_slot_rtx->u16_event_type;
                    }
                    /* FIX LTE-Bug00001401  BEGIN  2012-12-10 : wuxiaorong */
                    if(OAL_SUCCESS != dd_gsm_event_del(OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID,u16_del_event_type,u32_event_fn,(UINT16)s16_event_tsn,&st_net_fn_offset_del))
                    /* FIX LTE-Bug00001401  END  2012-12-10 : wuxiaorong */
                    {                   
                        u16_event_del_flag = 0;
                        /* FIX LM-Bug00001262 END 2011-11-30: linlan */
                        oal_error_handler(ERROR_CLASS_MINOR, GSM_FCP_ERR(RTX_EVENT_DELETE_FAILED));
                        //u16_event_del_flag = 0;
                        u32_fcp_ret = OAL_FAILURE;  
                        //break;
                    }
                    /* FIX LM-Bug00000729  END  2011-08-29  : yuzhengfeng */
                    else
                    {                     
                        u16_event_del_flag = 1;
                        u32_fcp_ret = OAL_SUCCESS;                          
                    }
                }
                else    /*there is no need to delete event, because the slot hasn't been downloaded to OAL*/
                {
                    u16_event_del_flag = 2;  
                    u32_fcp_ret = OAL_SUCCESS;  
                }
                /* FIX LM-Bug00000379    END  2011-02-11  : wanghuan */
            }
            /* FIX AM-Bug00000023   BEGIN  2010-05-01  : wuxiaorong */
            else
            {
                u16_event_del_flag = 4;  /* delete a pseudo task*/    
                u32_fcp_ret = OAL_SUCCESS;  
            }
             /* AM-Bug00000023   END  2010-05-01  : wuxiaorong */
            
            /* to display which event/tsntask has been deleted: */ 
            if(u16_event_del_flag != (UINT16)0)    
            {
                if(u16_event_del_flag == (UINT16)1) /*1: a downloaded tsntask(i.e. event) has been deleted*/
                {
                    OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,
                              (UINT16)GSM_MODE,
                              "gsm_fcp_del_rtx_conflict():nextfn=0x%lx,tsn=%ld,task=0x%lx; EVENT deleted fn=0x%lx,tsn=%ld,task=0x%lx,FILE_ID: %ld, LINE_NO: %ld\r\n",
                              u32_fn,
                              (UINT32)u16_tsn,
                              (UINT32)u16_tstask,
                              u32_event_fn,
                              (SINT32)s16_event_tsn,     
                              (UINT32)stp_del_slot_rtx->u16_tstask,
                              (UINT32)THIS_FILE_NAME_ID, 
                              (UINT32)__LINE__); 
                }
                else /*2,3: a tsntask is deleted that has NOT been downloaded*/
                {
                    /* FIX AM-Bug00000023   BEGIN  2010-05-01  : wuxiaorong */
                    if(u16_event_del_flag == (UINT16)4) /* delete a pseudo task*/   
                    {
                         /* FIX LTE-Enh00000846 BEGIN  2014-04-29: wangjunli */
                         if((u16_tstask != TSNTASK_PDTCH_TX)&&(u16_tstask != TSNTASK_PDTCH_FTA_TX))
                         /* FIX LTE-Enh00000846 END  2014-04-29: wangjunli */
                         {
                              /*FIX PhyFA-Enh00001343 BEGIN 2012-09-03: dujianzhong */
                              OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,
                                        (UINT16)GSM_MODE,
                                        "nextfn=0x%lx,tsn=0x%lx,task=x%lx; TASK deleted fn=0x%lx,tsn=0x%lx,task=0x%lx\r\n",
                                        u32_fn,
                                        (UINT32)u16_tsn,
                                        (UINT32)u16_tstask,
                                        u32_temp_fn,
                                        (SINT32)s16_deleted_tsn,     
                                        (UINT32)stp_del_slot_rtx->u16_tstask); 
                              /*FIX PhyFA-Enh00001343 END   2012-09-03: dujianzhong */
                         }
                    }
                    /* AM-Bug00000023   END  2010-05-01  : wuxiaorong */
                }
                gsm_fcp_del_tsntask(u32_temp_fn,s16_deleted_tsn,(UINT16)UNSPECIFIED_STANDBY_ID);
            }
            else
            {
                break;
            }
        }
         /* FIX A2KPH-Bug00000543    END     2008-07-22  : wuxiaorong */
    }
    return u32_fcp_ret;
}
/* FIX PhyFA-Enh00000105  END  2008-04-19  : huangjinfu */
/* FIX A2KPH-Bug00003516  END     2010-04-27  : ningyaojun*/
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/



/******************************************************************************
*  FUNCTION DEFINITION
******************************************************************************/

/************************************************************
** Function Name: gsm_fcp_get_latestfn ()
** Description:
** Input :
      u32p_fn_array:   the array of the frame number
      u16_count:  the size of the u32p_fn_array

** Output : NULL

** Return:
      u32_latest_fn: the latest frame number

** Notes:
2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*************************************************************/
/* FIX PhyFA-Req00000089  BEGIN  2008-03-31  : huangjinfu */
L1CC_GSM_DRAM_CODE_SECTION
UINT32 gsm_fcp_get_latestfn(INOUT CONST_UINT32_PTR_CONST u32p_fn_array,IN CONST_UINT16 u16_count)
{
    UINT16 i;
    /* FIX PHYFA-Enh00000742   BEGIN    2010-06-22  : wanghuan */
    UINT32 u32_latest_fn = (UINT32)0;
    /* FIX PHYFA-Enh00000742   END    2010-06-22  : wanghuan */
    UINT16  u16_latest_index = INVALID_BYTE;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    
    OAL_ASSERT(NULL_PTR!=u32p_fn_array, "gsm_fcp_get_latestfn null pointer");
    

    if((UINT16)0 == u16_count)
    {
        /* FIX PhyFA-Bug00002799 BEGIN  2011-08-13 : wuxiaorong */ 
        /* FIX LM-Bug00000923 BEGIN  2011-10-08 : wanghairong */ 
        return gsm_fn_operate(g_u32_gsm_fn[u16_active_id] + (UINT32)14); 
        /* FIX LM-Bug00000923 END  2011-10-08 : wanghairong */ 
        /* FIX PhyFA-Bug00002799 BEGIN  2011-08-13 : wuxiaorong */
    }
    
    for(i=0; i<u16_count; i++)
    {
        OAL_ASSERT(((UINT32)GSM_INVALID_FN != u32p_fn_array[i]), "gsm_fcp_get_latestfn():u32p_fn_array[] error!\n");
        if(INVALID_BYTE == u16_latest_index)
        {
            u16_latest_index = i;
            u32_latest_fn = u32p_fn_array[i];
        }
        else if(gsm_fn_compare(u32p_fn_array[i],u32_latest_fn))
        {
            u32_latest_fn = u32p_fn_array[i];
            u16_latest_index = i;
        }
        else
        {
            ;/*u32_latest_fn should not be changed*/
        }
    }
    return u32_latest_fn;
}
/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_get_latest_frame_pos
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
VOID gsm_fcp_get_latest_frame_pos(IN CONST gsm_tstask_latest_frame_pos_t* CONST stp_all_latest_frame_pos,OUT gsm_offset_in_frame_t* stp_latest_frame_pos)
{

    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    
    OAL_ASSERT(NULL_PTR != stp_all_latest_frame_pos, "gsm_fcp_get_latest_frame_pos null pointer");
    OAL_ASSERT(NULL_PTR != stp_latest_frame_pos, "gsm_fcp_get_latest_frame_pos null pointer");
        
    

    if((UINT16)0 == stp_all_latest_frame_pos->u16_latest_pos_num)
    {
        /* FIX PhyFA-Bug00002799 BEGIN  2011-08-13 : wuxiaorong */ 
        /* FIX LM-Bug00000923 BEGIN  2011-10-08 : wanghairong */ 
        stp_latest_frame_pos->s16_offset = 0;
        stp_latest_frame_pos->u16_tsn = 0;
        stp_latest_frame_pos->u32_fn = gsm_fn_operate(g_u32_gsm_fn[u16_active_id] + (UINT32)14); 
        return;
        /* FIX LM-Bug00000923 END  2011-10-08 : wanghairong */ 
        /* FIX PhyFA-Bug00002799 BEGIN  2011-08-13 : wuxiaorong */
    }

    gsm_get_latest_pos(&(stp_all_latest_frame_pos->st_latest_frame_pos[0]),stp_all_latest_frame_pos->u16_latest_pos_num,stp_latest_frame_pos);
    return ;
}
/* FIX PhyFA-Req00000089  END  2008-03-31  : huangjinfu */







/************************************************************
** Function Name: gsm_fcp_get_tsntask_latestfn ()
** Description: get the latest frame number from the table
** Input :
      u16_tstask:   the task of the the timeslot
      u32_fn:  the frame number

** Output : NULL

** Return:
      u32_latest_fn: the latest frame number

** Notes:
*   2014-04-14      wuxiaorong              PhyFA-Bug00004431   [NGM]L1CC_GSM在gsm_fcp_get_tsntask_latestfn内没有初始化st_tsn_info，导致死机
*************************************************************/
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
L1CC_GSM_DRAM_CODE_SECTION
OAL_STATUS gsm_fcp_get_tsntask_latestfn(IN CONST_UINT16 u16_tstask,IN CONST_UINT32 u32_cur_fn,IN CONST_UINT16 u16_standby_id,OUT gsm_offset_in_frame_t* CONST stp_latest_frame_pos)
{
    UINT32 u32_lastest_fn;    
    UINT16 u16_table_flag = GSM_TABLE_INVALID;
    UINT16 u16_period = INVALID_WORD;//0;
    UINT16 u16_fn_in_period = 0;
    UINT32 u32_next_fn;
    gsm_tstask_latest_fn_info_t st_latest_fn_info;
    gsm_tsn_info_t st_tsn_info;

    OAL_ASSERT(stp_latest_frame_pos != NULL_PTR, "");
    st_latest_fn_info.u16_table_flag = GSM_TABLE_INVALID;

    switch(u16_tstask)
    {
        case TSNTASK_NULL:
            break;
        case TSNTASK_SCAN_RF:
            break;
        case TSNTASK_FCB:
        /* FIX LM-Bug00001755   BEGIN  2012-02-29 : wanghairong */
        case TSNTASK_HO_FCB:
        /* FIX LM-Bug00001755   END  2012-02-29 : wanghairong */
            break;
        case TSNTASK_SB:
        /* FIX LM-Bug00001755   BEGIN  2012-02-29 : wanghairong */
        case TSNTASK_HO_SB:
        /* FIX LM-Bug00001755   END  2012-02-29 : wanghairong */
        /*FIX PhyFA-Enh00001108  BEGIN  2011-09-09 :sunzhiqiang*/
        case TSNTASK_MANU_SB:
        /*FIX PhyFA-Enh00001108  END  2011-09-09 :sunzhiqiang*/
            break;
        case TSNTASK_BCCH_RX:
        /* FIX PhyFA-Enh00001647 BEGIN 2014-04-28 : sunzhiqiang */
        case TSNTASK_TRANSFER_BCCH_RX:
        /* FIX PhyFA-Enh00001647 END 2014-04-28 : sunzhiqiang */
            gsm_get_bcch_rx_latest_fn(u16_standby_id,&st_latest_fn_info);
            break;
            
        /* FIX PhyFA-Req00000134 BEGIN  2008-06-25 : wuxiaorong */
        /* wuxiaorong 20080227 add for cbs begin*/
        case TSNTASK_CBCH_RX:
        /*FIX PhyFA-Bug00002704  BEGIN   2011-08-10  : ningyaojun*/
           gsm_get_cbch_rx_latest_fn(u16_standby_id,&st_latest_fn_info); 
            break;
        /* wuxiaorong 20080227 add for cbs end*/
        /* FIX PhyFA-Req00000134 END  2008-06-25 : wuxiaorong */

        case TSNTASK_PBCCH_RX:
            gsm_get_pbcch_rx_latest_fn(u16_standby_id,&st_latest_fn_info); 
            break;
        case TSNTASK_CCCH_RX:
        /*  FIX LM-Req00000393  BEGIN  2011-11-17 :fushilong  */    
        case TSNTASK_CCCH_CS_RX:
        case TSNTASK_CCCH_PCH_RX:    
        /*  FIX LM-Req00000393  END 2011-11-17 :fushilong    */       
        /* FIX AM-Bug00000151 BEGIN  2010-07-16 : wanghuan */
        case TSNTASK_TRANSFER_CCCH_RX:
        /* FIX AM-Bug00000151 END    2010-07-16 : wanghuan */
            gsm_get_ccch_rx_latest_fn(u16_standby_id,&st_latest_fn_info);
            break;
        case TSNTASK_PCCCH_RX:
            gsm_get_pccch_rx_latest_fn(u16_standby_id,&st_latest_fn_info);       
            break;
        case TSNTASK_NC0_BCCH_RX:
        case TSNTASK_NC1_BCCH_RX:
        case TSNTASK_NC2_BCCH_RX:
        case TSNTASK_NC3_BCCH_RX:
        case TSNTASK_NC4_BCCH_RX:
        case TSNTASK_NC5_BCCH_RX:
        case TSNTASK_NC6_BCCH_RX:
            gsm_get_nc_bcch_rx_latest_fn(u16_standby_id,u16_tstask,&st_latest_fn_info);
            break;
        /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
        /* FIX AM-Bug00000229 BEGIN  2010-09-26  : gaochunsheng */
        case TSNTASK_BG_TC0_BCCH_RX:
        case TSNTASK_BG_TC1_BCCH_RX:
        case TSNTASK_BG_TC2_BCCH_RX:
        case TSNTASK_BG_TC3_BCCH_RX:
        case TSNTASK_BG_TC4_BCCH_RX:
        case TSNTASK_BG_TC5_BCCH_RX:
            gsm_get_bg_tc_bcch_rx_latest_fn(u16_standby_id, u16_tstask,&st_latest_fn_info);
            break;
        /* FIX AM-Bug00000229 END  2010-09-26  : gaochunsheng */
        case TSNTASK_SYNC_CELL0_BCCH_RX:
        case TSNTASK_SYNC_CELL1_BCCH_RX:
        case TSNTASK_SYNC_CELL2_BCCH_RX:
        case TSNTASK_SYNC_CELL3_BCCH_RX:
        case TSNTASK_SYNC_CELL4_BCCH_RX:
        case TSNTASK_SYNC_CELL5_BCCH_RX:
            gsm_get_sync_bcch_rx_latest_fn(u16_standby_id, u16_tstask,&st_latest_fn_info);
            break;
        case TSNTASK_TC_BCCH_RX:
            gsm_get_tc_bcch_rx_latest_fn(u16_standby_id,&st_latest_fn_info);
            break;
        case TSNTASK_TC_SEARCH_BCCH_RX:
            gsm_get_tc_search_bcch_rx_latest_fn(u16_standby_id, &st_latest_fn_info);

            break;          
           
        /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/
        case TSNTASK_TRANSFER_NC_BCCH_RX:    
            gsm_get_transfer_nc_bcch_rx_latest_fn(u16_standby_id, &st_latest_fn_info);
            break;
        case TSNTASK_RACH_TX:
            break;
        case TSNTASK_PRACH_TX:
            break;
        case TSNTASK_SDCCH_RX:
            gsm_get_sdcch_rx_latest_fn(&st_latest_fn_info);
            break;
        case TSNTASK_SDCCH_TX:
            gsm_get_sdcch_tx_latest_fn(&st_latest_fn_info);
            break;
        case TSNTASK_SACCH_RX:
            gsm_get_sacch_rx_latest_fn(&st_latest_fn_info);
            break;
        case TSNTASK_SACCH_TX:
            gsm_get_sacch_tx_latest_fn(&st_latest_fn_info);
            break;
        case TSNTASK_TCH_RX:
            break;
        case TSNTASK_TCH_TX:
            break;
        case TSNTASK_FACCH_TX:
            break;
        case TSNTASK_HANDOVER_RX:
            break;
        case TSNTASK_HANDOVER_TX:
            break;
        case TSNTASK_PDTCH_TX:
        /* FIX LTE-Enh00000846 BEGIN  2014-04-29: wangjunli */
        case TSNTASK_PDTCH_FTA_TX:
        /* FIX LTE-Enh00000846 END  2014-04-29: wangjunli */
            break;
        case TSNTASK_PDTCH_RX:
            break;
        case TSNTASK_PTCCH_RX:
            break;
        case TSNTASK_RRBP_TX0:
            break;
        case TSNTASK_PTCCH_TX:
            break;
        case TSNTASK_SINGLEBLOCK_TX:
            break;
        case TSNTASK_MEAS_RSSI:
            break;
        default:
            break;
     }
    u16_table_flag = st_latest_fn_info.u16_table_flag;

    if((UINT16)GSM_TABLE_VALID == u16_table_flag)
    {
        u16_period = st_latest_fn_info.u16_period;
        u16_fn_in_period = st_latest_fn_info.u16_fn_in_period;
  
        /* FIX PhyFA-Bug00002704  BEGIN   2011-08-10  : ningyaojun*/
        u32_lastest_fn = gsm_fn_operate(((u32_cur_fn/(UINT32)u16_period)*(UINT32)u16_period) + (UINT32)u16_fn_in_period);

        /* FIX LM-Bug00000707   BEGIN 2011-8-22: wuxiaorong */ 
        u32_next_fn = gsm_fn_operate(u32_cur_fn + (UINT32)1);
        if(gsm_fn_compare(u32_lastest_fn,gsm_fn_operate(u32_next_fn + (UINT32)1)))/* if u32_lastest_fn<=u32_next_fn,...u32_lastest_fn must be bigger than u32_next_fn*/
        {
           
            u32_lastest_fn = gsm_fn_operate(u32_lastest_fn + (UINT32)u16_period);
                        
            if(u32_lastest_fn == u32_next_fn )/* if u32_lastest_fn==u32_next_fn,...*/
            {
                u32_lastest_fn = gsm_fn_operate(u32_lastest_fn + (UINT32)u16_period);
            }
        }
        /* FIX LM-Bug00000707   END 2011-8-22: wuxiaorong */
        /* FIX PhyFA-Bug00002704  END     2011-08-10  : ningyaojun*/
        /* FIX PhyFA-Bug00004431 BEGIN 2014-04-14 : wuxiaorong */
        st_tsn_info.u16_tstask = u16_tstask;
        st_tsn_info.u16_standby_id = u16_standby_id;
        st_tsn_info.u32_fn = u32_cur_fn;
        /* FIX PhyFA-Bug00004431 END   2014-04-14 : wuxiaorong */
        gsm_fcp_get_tb_tsn_info(&st_tsn_info);
        stp_latest_frame_pos->u32_fn = u32_lastest_fn;
        stp_latest_frame_pos->u16_tsn = st_tsn_info.u16_start_tsn;
        stp_latest_frame_pos->s16_offset = st_tsn_info.s16_rtx_offset;
        
     }
     else
     {
        stp_latest_frame_pos->u32_fn = (UINT32)GSM_INVALID_FN;
     }
     return OAL_SUCCESS;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/




L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
STATIC OAL_STATUS gsm_fcp_get_tb_tsn_info(gsm_tsn_info_t* stp_tsn_info)
{
    OAL_STATUS u32_ret = OAL_FAILURE;
    CONST gsm_tstask_attribute_t*     stp_tstask_attribute = NULL_PTR;

    OAL_ASSERT(NULL_PTR!=stp_tsn_info, "gsm_fcp_get_tb_tsn_info stp_tsn_info=null");

    
    stp_tstask_attribute = &C_ST_TSTASK_ATTRIBUTE_TBL[stp_tsn_info->u16_tstask];

    if(NULL_PTR != stp_tstask_attribute->fp_get_tsn_info)
    {
        u32_ret = (*stp_tstask_attribute->fp_get_tsn_info)(stp_tsn_info);
    }

    return u32_ret;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/





/************************************************************
** Function Name: gsm_fcp_get_tb_pos_info ()
** Description: get position infomation from the table
** Input :
      u16_tstask:   the task of the the timeslot

** Output :
     stp_tb_pos_info: the position infomation of the table
     u16p_tb_pos:     the position array of the table
     s16_rtx_offset: the timeslot's TA value or RX offset value

** Return:
      u32_latest_fn: the latest frame number

** Notes:
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_get_tb_pos_info(gsm_tb_pos_t *stp_tb_pos_info,IN CONST_UINT16 u16_tstask,IN CONST_UINT32 u32_fn,IN CONST_UINT16 u16_standby_id)
{
    OAL_STATUS u32_ret = OAL_FAILURE;
    CONST gsm_tstask_attribute_t*     stp_tstask_attribute = NULL_PTR;
    gsm_pos_info_t st_pos_info;

    
    OAL_ASSERT(NULL_PTR != stp_tb_pos_info, "gsm_fcp_get_tb_pos_info stp_tb_pos_info=null");

    stp_tstask_attribute = &C_ST_TSTASK_ATTRIBUTE_TBL[u16_tstask];

    st_pos_info.stp_tb_pos_info = stp_tb_pos_info;
    st_pos_info.u16_standby_id = u16_standby_id;
    st_pos_info.u16_tstask = u16_tstask;
    st_pos_info.u32_fn = u32_fn;
    if(NULL_PTR != stp_tstask_attribute->fp_get_pos_info)
    {
        u32_ret = (*stp_tstask_attribute->fp_get_pos_info)(&st_pos_info);
    }

    return u32_ret;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/

/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_set_frametask_header
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
OAL_STATUS gsm_fcp_set_frametask_header(IN CONST_UINT16 u16_rtx_flg,IN CONST_UINT32 u32_fn,IN CONST_UINT16 u16_tsn)
{
    gsm_main_frametask_t *stp_main_frametask;
    UINT32 u32_tmp_fn;
    UINT16 u16_tmp_tsn;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    

    u32_tmp_fn = gsm_fn_operate((u32_fn + ((UINT32)u16_tsn /(UINT32) GSM_MAX_TSN_TASKS)));
    /* FIX Enh00001894   BEGIN     2014-05-12  : xiongjiangjiang */
    u16_tmp_tsn = GSM_MOD_2POWER(UINT16, u16_tsn, GSM_MAX_TSN_TASKS);
    /* FIX Enh00001894   END     2014-05-12  : xiongjiangjiang */
    stp_main_frametask =gsm_get_main_frametask(u32_tmp_fn,u16_active_id);
    stp_main_frametask->u32_fn = u32_tmp_fn;

    switch(u16_rtx_flg)
    {
        case GSM_FCP_TSNTASK_RX:
            stp_main_frametask->u16_rx_bitmap |= (UINT16)(1<< u16_tmp_tsn);
            break;
        case GSM_FCP_TSNTASK_TX:
            stp_main_frametask->u16_tx_bitmap |= (UINT16)(1<<u16_tmp_tsn);
            break;
        case GSM_FCP_TSNTASK_ONLY_FN:
            break;
        default:
            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
            break;
    }

    return OAL_SUCCESS;
}

/************************************************************
** Function Name: gsm_fcp_del_tsntask ()
** Description: get position infomation from the table
** Input :
      u32_fn : the frame number
      u16_tsn : the timeslot deleted

** Output : NULL

** Return:
      u32_latest_fn: the latest frame number

** Notes:
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
STATIC OAL_STATUS gsm_fcp_del_tsntask(UINT32 u32_fn, SINT16  s16_deleted_tsn, CONST_UINT16 u16_standby_id)
{
    gsm_slot_rtx_t *stp_del_slot_rtx;
    SINT32 s32_event_start_offset;
    SINT32 s32_event_end_offset;
    SINT16  s16_tmp_tsn;
    SINT16 s16_event_tsn;
    UINT16 j;
    CONST gsm_tstask_attribute_t* stp_tstask_attribute = NULL_PTR;

    stp_del_slot_rtx = gsm_get_tsntask(u32_fn,s16_deleted_tsn);
    /* FIX PhyFA-Bug00002849  BEGIN  2011-08-23  : yuzhengfeng */
    if(GSM_TSNTASK_EXPIRED == stp_del_slot_rtx->u16_expired)
    {
        return OAL_FAILURE;
    }
    /* FIX PhyFA-Bug00002849  END  2011-08-23  : yuzhengfeng */

    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    if(UNSPECIFIED_STANDBY_ID != u16_standby_id)
    {
        if(u16_standby_id != stp_del_slot_rtx->u16_standby_id)
        {
            return OAL_FAILURE;
        }
    }


    stp_tstask_attribute = gsm_get_tstask_attribute(stp_del_slot_rtx->u16_tstask);
 
    s32_event_start_offset = (SINT32)(((SINT32)s16_deleted_tsn * (SINT32)GSM_QBITS_PER_TS)  + (SINT32)stp_del_slot_rtx->s16_rtx_offset); 

    /* FIX NGM-Bug00000617 BEGIN 2014-10-20 : gaowu */
    /* 由于删除任务的时候，会把此5个任务获取长度的任务表有效标志位清除，导致无法获取长度，因此需要从帧任务表中获取 */
    if ((TSNTASK_T_TS0_ONLY_NC_SYNC_RX <= stp_del_slot_rtx->u16_tstask)
         && (stp_del_slot_rtx->u16_tstask <= TSNTASK_T_TSN_3SLOT_NC_SYNC_RX))
    {
        /* 从帧任务表中获取长度，并转换成QBIT长度 */
        s32_event_end_offset = s32_event_start_offset + (SINT32)(stp_del_slot_rtx->u16_rx_len_ind * GSM_QBITS_PER_BIT);
    }
    else
    {
        s32_event_end_offset =(SINT32)( s32_event_start_offset + stp_tstask_attribute->fp_get_rtx_len(stp_del_slot_rtx->u16_tstask));
    }
    /* FIX NGM-Bug00000617 END   2014-10-20 : gaowu */

    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/


    for(s16_tmp_tsn = gsm_fcp_calc_tsn(s32_event_start_offset);
               s16_tmp_tsn <= gsm_fcp_calc_tsn(s32_event_end_offset); s16_tmp_tsn++)
    {
        stp_del_slot_rtx = gsm_get_tsntask(u32_fn,s16_tmp_tsn);

        /* FIX AM-Enh00000083   BEGIN  2010-07-12  : wuxiaorong */
        if( (s16_tmp_tsn == s16_deleted_tsn) 
           &&((INVALID_EVENT_TSN == stp_del_slot_rtx->s16_occupied_tsn[0]) || ((UINT16)0 == stp_del_slot_rtx->u16_tstask))
           &&(GSM_TSNTASK_NOT_EXPIRED == stp_del_slot_rtx->u16_expired))
        /* FIX AM-Enh00000083   END  2010-07-12  : wuxiaorong */
        {
            /* FIX LM-Bug00000796 BEGIN  2011-09-16  : yuzhengfeng */
            /* FIX LM-Enh00000405 BEGIN  2011-10-08  : wanghairong */
            if((UINT16)RX_GSM_SYNC_NSLOT == stp_del_slot_rtx->u16_event_type)
            /* FIX LM-Enh00000405 END  2011-10-08  : wanghairong */
            {

                gsm_fcp_del_bitmap(u32_fn, s16_tmp_tsn);
                
                /* FIX PhyFA-Req00001525 BEGIN  2013-11-18 : wuxiaorong */
                /* 修改了帧任务表，下面需要确认 */
                #if 0
                oal_mem_set(stp_del_slot_rtx,
                            (UINT16)OAL_MEM_U16_TYPE,
                            (UINT16)(((UINT16)sizeof(gsm_slot_rtx_t)/(UINT16)sizeof(UINT16)) - (UINT16)4),
                            (UINT32)0);   
                #endif
                oal_mem_set(stp_del_slot_rtx,
                        (UINT16)(((UINT16)sizeof(gsm_slot_rtx_t)/(UINT16)sizeof(UINT16))),
                        (UINT16)0);   
                /* FIX PhyFA-Req00001525 END  2013-11-18 : wuxiaorong */
                stp_del_slot_rtx->s16_occupied_tsn[0] = INVALID_EVENT_TSN;
                stp_del_slot_rtx->s16_occupied_tsn[1] = INVALID_EVENT_TSN; 
                /* FIX LM-Enh00000405 BEGIN  2011-10-08  : wanghairong */
                OAL_PRINT(u16_standby_id,(UINT16)GSM_MODE,"TSNTASK_NC_IDLE_FCB_RX DELETE or RX_TDD_5FRAME_128CHIP DELETE ");
                /* FIX LM-Enh00000405 END  2011-10-08  : wanghairong */
                break;
            }
            
            /* FIX LM-Bug00000796 END  2011-09-16  : yuzhengfeng */
        }

        if(s16_deleted_tsn == s16_tmp_tsn)
        {
            /* FIX PhyFA-Bug00000520 BEGIN  2008-06-21  : huangjinfu */
            if((UINT16)0x80 != (stp_del_slot_rtx->u16_tstask & (UINT16)0x80))
            {
                gsm_fcp_del_bitmap(u32_fn, s16_tmp_tsn);
            }
            /* FIX PhyFA-Req00000215   BEGIN   2008-12-07  : ningyaojun*/
            /* Fix PhyFA-Bug00001015 BEGIN 2009-03-10 : guxiaobo */
             /* FIX PhyFA-Req00001525 BEGIN  2013-11-18 : wuxiaorong */
            /* 修改了帧任务表，下面需要确认 */
            /*注意下这个不能报帧任务表直接mem set,因为下面for循环中还要使用s16_occupied_tsn*/
            oal_mem_set(stp_del_slot_rtx,
                    (UINT16)(((UINT16)sizeof(gsm_slot_rtx_t)/(UINT16)sizeof(UINT16)) - (UINT16)4),
                    (UINT16)0);   
             /* FIX PhyFA-Req00001525 END  2013-11-18 : wuxiaorong */
            /* Fix PhyFA-Bug00001015 END 2009-03-10 : guxiaobo */
            /* FIX PhyFA-Req00000215   END     2008-12-07  : ningyaojun*/
            stp_del_slot_rtx->u16_spare_qbits = GSM_QBITS_PER_TS;
            /*gsm_fcp_del_bitmap(u32_fn,s16_tmp_tsn);*/
        }

        for(j = (UINT16)0; j < (UINT16)2; j++)
        {
            s16_event_tsn = stp_del_slot_rtx->s16_occupied_tsn[j] + ((((s16_tmp_tsn +(SINT16) 8) /(SINT16) 8) - (SINT16)1) *(SINT16) 8);
            if(s16_deleted_tsn == s16_event_tsn)
            {
                stp_del_slot_rtx->s16_occupied_tsn[j] = INVALID_EVENT_TSN;
                if((UINT16)0 == j)
                {
                    stp_del_slot_rtx->s16_occupied_tsn[0] = stp_del_slot_rtx->s16_occupied_tsn[1];
                    stp_del_slot_rtx->s16_occupied_tsn[1] = INVALID_EVENT_TSN;
                }
                break;
            }
        }
        /* FIX AM-Enh00000083   BEGIN  2010-07-12  : wuxiaorong */
        if(((UINT16)2 == j)&&(GSM_TSNTASK_NOT_EXPIRED == stp_del_slot_rtx->u16_expired))
        /* FIX AM-Enh00000083   END  2010-07-12  : wuxiaorong */
        {
            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(TSNTASK_CONFLICT));
        }
    }

    return OAL_SUCCESS;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/

/* FIX NGM-Bug00000383     BEGIN     2014-09-11  : linlan*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC BOOLEAN gsm_check_whether_nc_bcch_need_preempt(IN CONST l1cc_mmc_affair_type_t e_affair_id)
{
    
    BOOLEAN b_mmc_need_preempt = OAL_TRUE;
    UINT16 i;
    UINT16 u16_nc_tbl_cout = 0;
    CONST gsm_nc_bcch_tb_t * stp_gsm_nc_bcch_tb;
    
    for(i = 0; i < MAX_STANDBY_AMT ; i++)
    {
        stp_gsm_nc_bcch_tb = gsm_get_nc_bcch_tbl_cfg(i);
        /*本通下，nc_bcch有效的配置表个数*/
        if(GSM_TABLE_VALID == stp_gsm_nc_bcch_tb->u16_nc_bcch_tb_flg)
        {
            u16_nc_tbl_cout++;
        }
        
    }
    /*本通内存在两个以上的NC配置表有效时，仅NC0_BCCH和NC1_BCCH进行预占*/
    if (u16_nc_tbl_cout >= (UINT16)2)
    {
        if((MMC_AFF_GSM_IDLE_NC1_BCCH  < e_affair_id) && (e_affair_id <= MMC_AFF_GSM_IDLE_NC5_BCCH) )
        {
            b_mmc_need_preempt = OAL_FALSE;
        }           
    }    
    return b_mmc_need_preempt;    
}
/* FIX NGM-Bug00000383     END       2014-09-11  : linlan*/


/* FIX PhyFA-Req00001525 BEGIN  2013-11-18 : wuxiaorong */
L1CC_GSM_DRAM_CODE_SECTION
STATIC BOOLEAN gsm_check_whether_need_mmc_conflict_judge(IN CONST_UINT16 u16_active_id,IN CONST_UINT16 u16_standby_id)
{
    BOOLEAN b_mmc_conflict_judge = OAL_FALSE;
    CONST gsm_standby_info_t  *stp_simcard_info = NULL_PTR;
    UINT16  u16_active_flag;
    UINT16  u16_gsm_state;

    /* 如果是单卡激活，那么需要进一步判断GSM的卡激活状态 */
    if(OAL_TRUE == gsm_check_single_card_active(u16_active_id,u16_standby_id))
    {
        stp_simcard_info = gsm_get_standby_info();
        u16_gsm_state = gsm_get_gsm_state();
        OAL_ASSERT((UINT16)1 == stp_simcard_info->u16_active_standby_num,"gsm_check_whether_need_mmc_conflict_judge,active_sim_num err");
        u16_active_flag = stp_simcard_info->u16_active_flag[u16_standby_id];
       
        /* 检查是否是GSM主模式卡激活 */  
        if((UINT16)GSM_MASTER_STANDBY_ACTIVE == u16_active_flag)/* GSM卡主模式激活 */
        {
            if(GSM_IDLE == u16_gsm_state)
            {
                b_mmc_conflict_judge = OAL_TRUE;
            }
            else
            {
                /* 检查GSM主模式有没有分配给辅模式需要MMC冲突判决的GAP */  
                if(gsm_check_alloc_irat_gap_mmc_judge(u16_standby_id))/* 如果有 */
                {
                    b_mmc_conflict_judge = OAL_TRUE;
                }
            }
        }
        else/* GSM卡辅模式激活 */
        {
            OAL_ASSERT((UINT16)GSM_SLAVE_STANDBY_ACTIVE == u16_active_flag,"gsm_check_whether_need_mmc_conflict_judge,active_flag err");


            /* 判断 GSM 接收的模式间GAP是否需要mmc冲突判决 */
            if(gsm_check_slave_recv_gap_mmc_judge(u16_standby_id))
            {
                b_mmc_conflict_judge = OAL_TRUE;
            }  
        }
    }
    else
    {
       /* 如果是多卡激活，那么一定需要MMC的事务冲突判决 */
       b_mmc_conflict_judge = OAL_TRUE;
    }
   

    return b_mmc_conflict_judge;
}
/* FIX PhyFA-Req00001525 END  2013-11-18 : wuxiaorong */

/************************************************************
** Function Name: gsm_fcp_add_tsntask ()
** Description: add the timelost task into the main frame task table
** Input :
      u16_tstask:    the timelost task
      u32_fn:  the frame number
      u16_tsn:   timeslot number

** Output : NULL

** Return:  OAL_SUCCESS or OAL_FAILURE
** Notes:
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*   2014-05-20      wuxiaorong              PhyFA-Bug00004781   [NGM]GSM的事务预占的超帧号未赋值0出现DD ASSERT: gsm superfn is not 0 
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
OAL_STATUS gsm_fcp_add_tsntask(IN CONST_UINT16 u16_tstask,IN CONST gsm_offset_in_frame_t* CONST stp_frame_pos,IN CONST_UINT16 u16_arfcn_index,IN CONST_UINT16 u16_meas_index,IN CONST_UINT16 u16_standby_id,IN CONST_UINT16_PTR_CONST u16p_conflict_flag)
{
    OAL_STATUS u32_ret = OAL_FAILURE;
    BOOLEAN b_mmc_conflict_judge;
    UINT32  u32_fn;
    UINT16  u16_tsn;
    SINT16  s16_rtx_offset;
    UINT16  u16_i;
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    UINT16 u16_rtx_status;
    SINT32 s32_rtx_len;         /*Qbits*/
    UINT16 u16_repeat_frms;
    BOOLEAN b_nc_bcch_tsntask = OAL_FALSE;
    l1cc_mmc_affair_type_t  e_affair_id; 

    CONST gsm_tstask_attribute_t*     stp_tstask_attribute = NULL_PTR;
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
    gsm_tsntask_info_t  st_add_info;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    l1cc_mmc_raw_affair_info_t  st_raw_affair_info;      
    l1cc_mmc_act_result_t       st_act_result;    

    /*FIX LM-Enh00001061  BEGIN   2013-01-30  : ningyaojun*/
    UINT16 u16_affair_activate_fail_amt = (UINT16)0;
    BOOLEAN b_affair_activate_result[4] = {OAL_TRUE, OAL_TRUE, OAL_TRUE, OAL_TRUE};
    /*FIX LM-Enh00001061  END     2013-01-30  : ningyaojun*/

    OAL_ASSERT(NULL_PTR != stp_frame_pos,"gsm_fcp_add_tsntask,stp_frame_pos null");
    /* FIX LM-Enh00001307 BEGIN  2013-09-13: wangjunli */
    OAL_ASSERT( NULL_PTR != u16p_conflict_flag,"gsm_fcp_add_tsntask u16p_conflict_flag NULL_PTR");
    /* FIX LM-Enh00001307 END  2013-09-13: wangjunli */
    
    u32_fn = stp_frame_pos->u32_fn;
    u16_tsn = stp_frame_pos->u16_tsn;
    s16_rtx_offset = stp_frame_pos->s16_offset;
    
    st_add_info.stp_frame_pos = (gsm_offset_in_frame_t*)stp_frame_pos;
    st_add_info.u16_arfcn_index = u16_arfcn_index;
    st_add_info.u16_meas_index =u16_meas_index;
    st_add_info.u16_standby_id = u16_standby_id;
    st_add_info.u16_tstask = u16_tstask;
    st_add_info.u16p_conflict_flag = (UINT16_PTR)u16p_conflict_flag;
    /* FIX PhyFA-Bug00004496 BEGIN 2014-05-06 : gaowu */
    /* FIX PhyFA-Bug00002849  BEGIN  2011-08-23  : yuzhengfeng */
    OAL_ASSERT((u16_tsn <= (UINT16)18),"gsm_fcp_add_tsntask():u16_tsn illegal!");       
    /* FIX PhyFA-Bug00002849  END  2011-08-23  : yuzhengfeng */
    /* FIX PhyFA-Bug00004496 END   2014-05-06 : gaowu */
    OAL_ASSERT((u16_tstask<(UINT16)GSM_TSTASK_NUM),"gsm_fcp_add_tsntask():u16_tstask illegal!");

    stp_tstask_attribute = gsm_get_tstask_attribute(u16_tstask);    
    //u16_event_type  = stp_tstask_attribute->u16_event_type;
    u16_rtx_status  = stp_tstask_attribute->u16_rtx_status;   
    /* FIX PhyFA-Enh00001243 BEGIN 2012-03-12: yuzhengfeng */
    s32_rtx_len     = stp_tstask_attribute->fp_get_rtx_len(u16_tstask);
    /* FIX PhyFA-Enh00001243 END 2012-03-12: yuzhengfeng */
    /* FIX LM-Bug00001755   BEGIN  2012-02-27 : wanghairong */
    e_affair_id     = stp_tstask_attribute->e_affair_id;    
    /* FIX LM-Bug00001755   END  2012-02-27 : wanghairong */
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/

    b_mmc_conflict_judge = gsm_check_whether_need_mmc_conflict_judge(u16_active_id,u16_standby_id);
    /* 添加事件，需要模式间的MMC事务判决 */

    if(OAL_TRUE == b_mmc_conflict_judge)
    {
        /*激活事务*/
        if(MMC_AFF_INVALID != e_affair_id)
        {   
            st_raw_affair_info.e_affair_id          = e_affair_id;
            st_raw_affair_info.u16_standby_id       = u16_standby_id;
            st_raw_affair_info.u16_mode_timing      = GSM_TIMING;
            //st_raw_affair_info.u16_logch_id         = LOGCH_0;
            st_raw_affair_info.u16_active_id        = u16_active_id;
            st_raw_affair_info.b_fuzzy_match        = OAL_TRUE;
            st_raw_affair_info.u16_rtx_status       = u16_rtx_status;  /*GSM_RX_GSM_DATA, GSM_TX_GSM_DATA, GSM_RX_TDD_DATA*/
            st_raw_affair_info.u16_freq             = INVALID_WORD;    
            st_raw_affair_info.st_time_len.u16_superfn          = 0; /*superfn必须为0*/
            st_raw_affair_info.st_time_len.u32_sub_fn           = u32_fn; 
            st_raw_affair_info.st_time_len.s32_start_offset     = (SINT32)(((SINT32)u16_tsn*(SINT32)GSM_QBITS_PER_TS)+(SINT32)s16_rtx_offset); //???if u16_tsn>7
            st_raw_affair_info.st_time_len.u32_duration         = (UINT32)s32_rtx_len;   
            st_raw_affair_info.u32_test_info_start_tsn  = (UINT32)u16_tsn; 
            u16_repeat_frms = stp_tstask_attribute->u16_repeat_frms;

            for(u16_i = 0; u16_i<u16_repeat_frms; u16_i++)
            {
                st_raw_affair_info.st_time_len.u32_sub_fn  = gsm_fn_operate((UINT32)(u32_fn + (UINT32)u16_i));     
                l1cc_mmc_activate_affair(&st_raw_affair_info,&st_act_result); 

                /*FIX LM-Enh00001061  BEGIN   2013-01-30  : ningyaojun*/
                /* 允许 MPAL_BCCH/MPAL_CCCH 信道一个 TTI 的部分 bursts 能激活事务成功也能配置事件，用以减少事务冲突时的寻呼和系统消息损失 */
                if(MMC_SUCCESS != st_act_result.e_act_result)
                {
                    /* MPAL_BCCH/MPAL_CCCH 信道会尝试提前译码或部分 bursts 译码，允许 BCCH/CCCH 部分bursts能事务激活成功就配置事件。对应关系如下:
                    //u16_tstask              //u16_rxtask_type   //e_affair_id           
                    {TSNTASK_BCCH_RX          ,0                 ,MMC_AFF_GSM_IDLE_SC_BCCH } //MPAL_BCCH
                    {TSNTASK_CCCH_RX          ,0                 ,MMC_AFF_GSM_CCCH         } //MPAL_CCCH
                    {TSNTASK_NC0_BCCH_RX      ,0                 ,MMC_AFF_GSM_IDLE_NC_BCCH } //MPAL_BCCH
                    {TSNTASK_NC1_BCCH_RX      ,0                 ,MMC_AFF_GSM_IDLE_NC_BCCH } //MPAL_BCCH
                    {TSNTASK_NC2_BCCH_RX      ,0                 ,MMC_AFF_GSM_IDLE_NC_BCCH } //MPAL_BCCH
                    {TSNTASK_NC3_BCCH_RX      ,0                 ,MMC_AFF_GSM_IDLE_NC_BCCH } //MPAL_BCCH
                    {TSNTASK_NC4_BCCH_RX      ,0                 ,MMC_AFF_GSM_IDLE_NC_BCCH } //MPAL_BCCH
                    {TSNTASK_NC5_BCCH_RX      ,0                 ,MMC_AFF_GSM_IDLE_NC_BCCH } //MPAL_BCCH
                    {TSNTASK_NC6_BCCH_RX      ,0                 ,MMC_AFF_GSM_IDLE_NC_BCCH } //MPAL_BCCH
                    {TSNTASK_TC_BCCH_RX       ,0                 ,MMC_AFF_GSM_IDLE_TC_BCCH } //MPAL_BCCH
                    {TSNTASK_BG_TC_BCCH_RX    ,GSM_CELLSEARCH    ,MMC_AFF_GSM_IDLE_TC_BCCH } //MPAL_BCCH //不会提前译码，搜网事件
                    {TSNTASK_TRANSFER_CCCH_RX ,0                 ,MMC_AFF_GSM_PCH          } //MPAL_CCCH
                    {TSNTASK_CCCH_CS_RX       ,0                 ,MMC_AFF_GSM_RACH_CCCH    } //MPAL_CCCH //不会提前译码，RACH状态
                    {TSNTASK_CCCH_PCH_RX      ,0                 ,MMC_AFF_GSM_PCH          } //MPAL_CCCH
                    */
                    b_nc_bcch_tsntask = gsm_is_nc_bcch_tsntask(u16_tstask);
                    if(  (u16_tstask == TSNTASK_CCCH_RX)
                       ||(u16_tstask == TSNTASK_TRANSFER_CCCH_RX)
                       ||(u16_tstask == TSNTASK_CCCH_PCH_RX)               
                       ||(u16_tstask == TSNTASK_BCCH_RX)
                       /* FIX PhyFA-Enh00001647 BEGIN 2014-04-28 : sunzhiqiang */
                       ||(u16_tstask == TSNTASK_TRANSFER_BCCH_RX)
                       /* FIX PhyFA-Enh00001647 END 2014-04-28 : sunzhiqiang */
                       ||(b_nc_bcch_tsntask)
                       ||(u16_tstask == TSNTASK_TC_BCCH_RX)  
                       ||(u16_tstask == TSNTASK_TRANSFER_NC_BCCH_RX)
                       /* Fix LM-Bug00002963 BEGIN   2013-09-16 : linlan*/
                       ||(u16_tstask == TSNTASK_TC_SEARCH_BCCH_RX)
                       /* Fix LM-Bug00002963 END     2013-09-16 : linlan*/
                      )
                    {
                        u16_affair_activate_fail_amt++;
                        b_affair_activate_result[u16_i] = OAL_FALSE;
                
                        /* 4bursts 对应的事务激活成功个数必须大于等于 2， 否则仍然按照失败处理 */
                        if(u16_affair_activate_fail_amt > (UINT16)2)
                        {
                            return OAL_FAILURE;   
                        }
                    }
                    else
                    {
                        return OAL_FAILURE;
                    }
                }
                /*FIX LM-Enh00001061  END     2013-01-30  : ningyaojun*/
            }
        }
    }
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
    oal_mem_copy(&(st_add_info.b_affair_activate_result[0]),b_affair_activate_result,(UINT16)(sizeof(b_affair_activate_result)/sizeof(UINT16)));
    
    if(NULL_PTR != stp_tstask_attribute->fp_add_tsntask)
    {
        u32_ret = (*stp_tstask_attribute->fp_add_tsntask)(&st_add_info);
    }
    return u32_ret;
}

/************************************************************
** Function Name: gsm_fcp_del_frametask ()
** Description: delete certain main frame task table
** Input : UINT32 u32_fn

** Output : OAL_STATUS

** Return:  OAL_SUCCESS or OAL_FAILURE
** Notes:
2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
OAL_STATUS gsm_fcp_del_frametask(IN CONST_UINT32 u32_fn, IN CONST_UINT16 u16_standby_id)
{
    SINT16 i = 0;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    

    if(UNSPECIFIED_STANDBY_ID == u16_standby_id)
    {
        /* Fix L2000-Bug00001070 BEGIN 2010-04-19 : guxiaobo */
        oal_mem_copy(gsm_get_main_frametask(u32_fn,u16_active_id),
                     &C_ST_GSM_EMPTY_MAIN_FRAMETASK[0],
                     (UINT16)(sizeof(gsm_main_frametask_t)/sizeof(UINT16)));
        /* Fix L2000-Bug00001070 END 2010-04-19 : guxiaobo */
    }
    else
    {   
        OAL_ASSERT((u16_standby_id<(UINT16)MAX_STANDBY_AMT), "gsm_fcp_del_frametask(): Illegal u16_standby_id!\n");
        for(i=(SINT16)0; i<(SINT16)8; i++)
        {
            gsm_fcp_del_tsntask(u32_fn, (SINT16)i, u16_standby_id);
        }
    }    

    return OAL_SUCCESS;
}

/************************************************************
** Function Name: gsm_fcp_del_all_frametask ()
** Description: delete the main frame task table and the slaveant frame task table
** Input : NULL

** Output : NULL

** Return:  OAL_SUCCESS or OAL_FAILURE
** Notes:
2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
OAL_STATUS gsm_fcp_del_all_frametask(UINT16 u16_standby_id)
{
    SINT16  i = 0;    
    UINT32  u32_fn;
    /* Fix LM-Bug00001240 BEGIN 2011-11-10 : sunzhiqiang */
    UINT32  u32_fn_start;
    /* Fix LM-Bug00001240 END 2011-11-10 : sunzhiqiang */
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;

    
    
    
    if(UNSPECIFIED_STANDBY_ID == u16_standby_id)
    {
        /* Fix L2000-Bug00001070 BEGIN 2010-04-19 : guxiaobo */
        oal_mem_copy(&(g_st_gsm_main_frametask[u16_active_id][0]), 
                     &C_ST_GSM_EMPTY_MAIN_FRAMETASK[0], 
                     (UINT16)(((UINT16)GSM_MAX_MAINFRAME_TASKS*(UINT16)sizeof(gsm_main_frametask_t))/(UINT16)sizeof(UINT16)));
        /* Fix L2000-Bug00001070 END 2010-04-19 : guxiaobo */
    }
    else
    {
        /* Fix LM-Bug00001240 BEGIN 2011-11-10 : sunzhiqiang */
        u32_fn_start = g_u32_gsm_fn[u16_active_id];
        for(i=0; i<GSM_MAX_MAINFRAME_TASKS; i++)
        {
            u32_fn = gsm_fn_operate(u32_fn_start + (UINT32)(SINT32)i);
            gsm_fcp_del_frametask(u32_fn, u16_standby_id);
        }
        /* Fix LM-Bug00001240 END 2011-11-10 : sunzhiqiang */
    }
    return OAL_SUCCESS;
}



L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-06-08  : ningyaojun*/
STATIC OAL_STATUS gsm_fcp_arrive_pos(CONST gsm_tb_pos_t * CONST stp_tb_pos_info,UINT32 u32_fn)
{
    UINT16 u16_tmp_next_pos_index;
    /*FIX LM-Bug00002571  BEGIN   2013-01-16  : ningyaojun*/   
    UINT32 u32_interval_fn[32];
    /*FIX LM-Bug00002571  END     2013-01-16  : ningyaojun*/   
    UINT16 u16_i;
    UINT32 u32_fcp_ret=OAL_FAILURE;

    OAL_ASSERT(NULL_PTR!=stp_tb_pos_info, "gsm_fcp_arrive_pos null pointer");

    u16_tmp_next_pos_index = *(stp_tb_pos_info->u16p_next_pos_index);
    if(GSM_FCP_POS_INDEX_INVALID == u16_tmp_next_pos_index)
    {
        /*FIX LM-Bug00002571  BEGIN   2013-01-16  : ningyaojun*/
        u32_interval_fn[0] = gsm_calc_interval_from_fn2peiodpos((UINT32)stp_tb_pos_info->u16p_pos_list[0],u32_fn,(UINT32)stp_tb_pos_info->u16_period);
        for(u16_i=1; u16_i<stp_tb_pos_info->u16_pos_size; u16_i++)
        {
            u32_interval_fn[u16_i] = gsm_calc_interval_from_fn2peiodpos((UINT32)stp_tb_pos_info->u16p_pos_list[u16_i],u32_fn,(UINT32)stp_tb_pos_info->u16_period);
             if(u32_interval_fn[u16_i] < u32_interval_fn[u16_i-(UINT16)1])
             {
                u16_tmp_next_pos_index = u16_i;
                break;
             }
        }
        /*FIX LM-Bug00002571  END     2013-01-16  : ningyaojun*/

        if(GSM_FCP_POS_INDEX_INVALID == u16_tmp_next_pos_index)
        {
            u16_tmp_next_pos_index = 0;
        }
        *stp_tb_pos_info->u16p_next_pos_index = u16_tmp_next_pos_index;
    }
    if(stp_tb_pos_info->u16p_pos_list[u16_tmp_next_pos_index] == (UINT16)(u32_fn % (UINT32)stp_tb_pos_info->u16_period))
    {
        u32_fcp_ret=OAL_SUCCESS;
    }
    return u32_fcp_ret;
}
/*FIX PhyFA-Req00000936  END     2011-06-08  : ningyaojun*/



/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
/******************************************************************************
*
* FUNCTION
*
*    gsm_fcp_check_add_tsntask
*
* DESCRIPTION
*
*    check and add tsntask success or not
*
* NOTE
*
*    NA
*
* PARAMETERS
*
*   u16_tstask:                  
*   u32_fn:     
*   u16_tsn:    
*   s16_rtx_offset:        
*   u16_standby_id:    
*
*
* RETURNS
*
*    OAL_FAILURE:              check tsntask success but add tsntask fail;check tsntask except for pch fail
*    OAL_SUCCESS:              check tsntask success and add tsntask success
*    GSM_INTRA_CONFLICT:       check pch tsntask fail
*
* VERSION
*
*    <date>          <author>       <description>
*    2012-03-06      sunzhiqiang    check and add tsntask success or not
*
******************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
OAL_STATUS gsm_fcp_check_add_tsntask(IN CONST_UINT16 u16_tstask,IN CONST gsm_offset_in_frame_t* CONST stp_frame_pos,IN CONST_UINT16 u16_standby_id)
{
    UINT32 u32_fcp_ret = OAL_FAILURE;
    /* FIX LM-Enh00001307 BEGIN  2013-09-13: wangjunli */
    UINT16 u16_conflict_flag[4] = {(UINT16)0};
    
    u32_fcp_ret = gsm_fcp_check_tsntask(u16_tstask,stp_frame_pos,u16_standby_id,&u16_conflict_flag[0]);

    if(OAL_SUCCESS == u32_fcp_ret)
    {
        u32_fcp_ret = gsm_fcp_add_tsntask(u16_tstask,stp_frame_pos,(UINT16)0,(UINT16)0,u16_standby_id,&u16_conflict_flag[0]);
    }
    /* FIX LM-Enh00001307 END  2013-09-13: wangjunli */
    else
    {
        /* FIX LM-Enh00001145     BEGIN     2013-06-29: sunzhiqiang */
        if(((UINT16)TSNTASK_CCCH_PCH_RX == u16_tstask) || ((UINT16)TSNTASK_TRANSFER_CCCH_RX == u16_tstask))
        /* FIX LM-Enh00001145     BEGIN     2013-06-29: sunzhiqiang */
        {
            u32_fcp_ret = GSM_INTRA_CONFLICT;
        }
    }
    return u32_fcp_ret;
}



L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
STATIC OAL_STATUS gsm_fcp_check_isarrive(gsm_tsn_info_t* stp_tsn_info,gsm_tb_pos_t* stp_tb_pos)
{
    UINT32 u32_tb_flg;
    UINT32 u32_fcp_ret=OAL_FAILURE;
    UINT16 u16_tstask;
    UINT32 u32_fn;
    UINT16 u16_standby_id;
    CONST gsm_monitor_cbch_tb_t*  stp_cbch_tbl = NULL_PTR;

    OAL_ASSERT(NULL_PTR != stp_tb_pos,"stp_tb_pos null");
    OAL_ASSERT(NULL_PTR != stp_tsn_info,"stp_tsn_info null");

    u32_tb_flg = gsm_fcp_get_tb_tsn_info(stp_tsn_info);
    if(OAL_FAILURE == u32_tb_flg)
    {
        return OAL_FAILURE;
    }

    
    u32_fn     = stp_tsn_info->u32_fn;
    u16_tstask = stp_tsn_info->u16_tstask;
    u16_standby_id = stp_tsn_info->u16_standby_id;
    
    /*FIX PhyFA-Req00000936  BEGIN   2011-06-08  : ningyaojun*/
    gsm_fcp_get_tb_pos_info(stp_tb_pos,u16_tstask,u32_fn,u16_standby_id);
    u32_fcp_ret = gsm_fcp_arrive_pos(stp_tb_pos,u32_fn);
    /*FIX PhyFA-Req00000936  END     2011-06-08  : ningyaojun*/


    stp_cbch_tbl = gsm_get_cbch_tbl_cfg(u16_standby_id);
    /* FIX PhyFA-Req00000134 BEGIN  2008-06-25 : wuxiaorong */
    if(((UINT16)1 == stp_cbch_tbl->u16_cbch_tb_flg)
        &&((UINT16)1 == stp_cbch_tbl->u16_cbch_bitmap_tb_flg))
    {
        if((OAL_SUCCESS == u32_fcp_ret)&&(TSNTASK_CBCH_RX == u16_tstask))
        {
            if(OAL_FALSE == gsm_cbs_check_fn_in_bitmap(u32_fn,u16_standby_id))
            {
               return OAL_FAILURE;
            }
        }
    }

    /* FIX PhyFA-Req00000134 END  2008-06-25 : wuxiaorong */
    return u32_fcp_ret;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/


/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_add_frametask
* DESCRIPTION:       < 周期性事件的时隙任务添加和事务预占。非周期的测量/搜网任务都不使用该函数. >
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2014-03-17      wuxiaorong              PhyFA-Bug00004202   [TDM]GSM 周期事务预占没有判断是否要mmc
*
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
OAL_STATUS gsm_fcp_add_frametask(IN CONST_UINT16 u16_tstask,IN CONST_UINT32 u32_fn, IN CONST_UINT16 u16_standby_id)
{
    gsm_tb_pos_t st_tb_pos_info = {0, 0, NULL_PTR, NULL_PTR};

    UINT32 u32_fcp_ret=OAL_FAILURE;

    UINT16  u16_tmp_tsn;
    //UINT16  u16_start_tsn;
    //UINT16  u16_end_tsn;    

    /*FIX PhyFA-Req00000936  BEGIN   2011-06-08  : ningyaojun*/ 
    UINT16 i = 0;
    CONST gsm_tstask_attribute_t*     stp_tstask_attribute = NULL_PTR;
    l1cc_mmc_affair_type_t e_affair_id;
   
    //mmc_affair_result_type_t e_add_results[MMC_MAX_CHAN_AMT];
    UINT32 u32_next_pos_fn = (UINT32)GSM_INVALID_FN;
    UINT16 u16_repeat_frms;
    /*FIX PhyFA-Req00000936  END     2011-06-08  : ningyaojun*/

    /*FIX PhyFA-Bug00002773  BEGIN   2011-08-05  : ningyaojun*/ 
    UINT16 u16_current_pos_index = GSM_FCP_POS_INDEX_INVALID;
    UINT16 u16_next_pos_index    = GSM_FCP_POS_INDEX_INVALID;
    UINT32 u32_next_pos_distance = (UINT32)GSM_INVALID_FN;
    /*FIX PhyFA-Bug00002773  END     2011-08-05  : ningyaojun*/ 

    /*FIX LM-Bug00001699  BEGIN 2012-02-28 :fushilong*/
    UINT32 u32_scell_current_fn = (UINT32)0;
    CONST gsm_ccch_tb_t  *stp_gsm_ccch_tb = NULL_PTR;
    UINT16         u16_paging_period = 102;
    UINT16         u16_paging_offset = 0;   
    /*FIX LM-Bug00001699  END 2012-02-28 :fushilong*/

    gsm_tsn_info_t st_tsn_info;
    gsm_offset_in_frame_t st_frame_pos;
    UINT16 u16_gsm_state;
    gsm_ccch_tb_t*  stp_ccch_tbl = NULL_PTR;
    gsm_sc_bcch_tb_t *stp_gsm_sc_bcch_tb = NULL_PTR;
    l1cc_mmc_raw_affair_info_t*  stp_first_raw_affair_info; 
    l1cc_mmc_raw_affair_info_t*  stp_other_raw_affair_info;    
    l1cc_rat2mmc_preempt_affair_t* stp_preempt_affair;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    BOOLEAN    b_mmc_conflict_judge = OAL_FALSE;    
    /* FIX NGM-Bug00000383     BEGIN     2014-09-11  : linlan*/
    BOOLEAN    b_mmc_need_preempt = OAL_TRUE;    
    /* FIX NGM-Bug00000383     END       2014-09-11  : linlan*/

    st_tsn_info.u16_tstask = u16_tstask;
    st_tsn_info.u32_fn     = u32_fn;
    st_tsn_info.u16_standby_id = u16_standby_id;

    stp_ccch_tbl = gsm_get_ccch_tbl_cfg(u16_standby_id);
    
    u32_fcp_ret = gsm_fcp_check_isarrive(&st_tsn_info,&st_tb_pos_info);

    if(OAL_SUCCESS == u32_fcp_ret)
    {
        for(u16_tmp_tsn = st_tsn_info.u16_start_tsn;u16_tmp_tsn <= st_tsn_info.u16_end_tsn;u16_tmp_tsn++)
        {
             if((UINT16)GSM_FCP_TSN_VALID == st_tsn_info.u16_tsn_valid_list[u16_tmp_tsn])
             { 
                /*FIX LM-Bug00001204  BEGIN  2011-11-10 :linlan*/ /*cannot get the return result here, u32_fcp_ret just stand for frametask setting period*/
                /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
                /*u32_fcp_ret = gsm_fcp_check_add_tsntask(u16_tstask,u32_fn,u16_tmp_tsn,s16_rtx_offset,u16_standby_id);*/
                /* FIX LM-Enh00000435  BEGIN  2012-02-01  : sunzhiqiang */

                st_frame_pos.s16_offset = st_tsn_info.s16_rtx_offset;
                st_frame_pos.u16_tsn = u16_tmp_tsn;
                st_frame_pos.u32_fn = u32_fn;
                u32_fcp_ret = gsm_fcp_check_add_tsntask(u16_tstask,&st_frame_pos,u16_standby_id);
                if((UINT16)TSNTASK_CCCH_PCH_RX == u16_tstask)
                {
                    stp_ccch_tbl->u32_arrive_ret = OAL_SUCCESS;
                }
                /* FIX LM-Enh00000435  END  2012-02-01  : sunzhiqiang */

                u16_gsm_state = gsm_get_gsm_state();
                /* FIX LM-Bug00002651  BEGIN  2013-03-06  : sunzhiqiang */
                if(GSM_DEDICATED == u16_gsm_state)
                {
                    /* 连接态下另一张卡在IDLE下应该配置BCCH，NC BCCH或CCCH */
                    if((UINT16)TSNTASK_BCCH_RX == u16_tstask)
                    {
                        stp_gsm_sc_bcch_tb = gsm_get_sc_bcch_tbl_cfg(u16_standby_id);
                        stp_gsm_sc_bcch_tb->u32_arrive_ret = OAL_SUCCESS;                        
                    }
                    else if((UINT16)TSNTASK_CCCH_RX == u16_tstask)
                    {
                        stp_ccch_tbl->u32_arrive_ret = OAL_SUCCESS;
                    }
                }
                /* FIX LM-Bug00002651  END  2013-03-06  : sunzhiqiang */
                /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
                /*FIX LM-Bug00001204  BEGIN  2011-11-10 :linlan*/ 
             }
        }

        /*FIX PhyFA-Bug00002773  BEGIN   2011-08-05  : ningyaojun*/ 
        u16_current_pos_index   = *st_tb_pos_info.u16p_next_pos_index;
        u16_next_pos_index      = (*st_tb_pos_info.u16p_next_pos_index +(UINT16)1) % st_tb_pos_info.u16_pos_size;
        *st_tb_pos_info.u16p_next_pos_index = u16_next_pos_index;


        b_mmc_conflict_judge = gsm_check_whether_need_mmc_conflict_judge(u16_active_id,u16_standby_id);
        /* 添加事件，需要模式间的MMC事务判决 */
        if(OAL_TRUE == b_mmc_conflict_judge)
        {
        
            OAL_ASSERT((u16_tstask<(UINT16)GSM_TSTASK_NUM),"gsm_fcp_add_tsntask():u16_tstask illegal!");
            stp_tstask_attribute = gsm_get_tstask_attribute(u16_tstask);
            e_affair_id = stp_tstask_attribute->e_affair_id;
            
            /* FIX NGM-Bug00000383     BEGIN     2014-09-11  : linlan*/
            b_mmc_need_preempt = gsm_check_whether_nc_bcch_need_preempt(e_affair_id);
            if(OAL_FALSE  == b_mmc_need_preempt )
            {
                return u32_fcp_ret;
            }
            /* FIX NGM-Bug00000383     END       2014-09-11  : linlan*/

            /*FIX LM-Bug00001145  BEGIN  2011-10-27 :fushilong*/
            if(st_tb_pos_info.u16_pos_size != (UINT16)1)
            {
                u32_next_pos_distance = (UINT32)((((UINT32)st_tb_pos_info.u16p_pos_list[u16_next_pos_index] + (UINT32)st_tb_pos_info.u16_period) - (UINT32)st_tb_pos_info.u16p_pos_list[u16_current_pos_index]) % (UINT32)st_tb_pos_info.u16_period);  
            }
            else
            {
                u32_next_pos_distance = (UINT32)st_tb_pos_info.u16_period ;
            }
            /*FIX LM-Bug00001145  END 2011-10-27 :fushilong*/

            
            /*FIX LM-Bug00001212  BEGIN 2011-11-10 :linlan*/
            /*FIX LM-Bug00001415  BEGIN 2011-12-08 :sunzhiqiang*/
            if((UINT32)(u32_fn + u32_next_pos_distance) >= (UINT32)(UINT32)GSM_MAX_FN)
            /*FIX LM-Bug00001415  END 2011-12-08 :sunzhiqiang*/
            {
                u32_next_pos_fn = (UINT32)st_tb_pos_info.u16p_pos_list[u16_next_pos_index];
            }
            else
            {
                u32_next_pos_fn = gsm_fn_operate(u32_fn + u32_next_pos_distance);
            }
            /*FIX LM-Bug00001212  END 2011-11-10 :linlan*/
            OAL_ASSERT(((u32_next_pos_fn%(UINT32)st_tb_pos_info.u16_period) == (UINT32)st_tb_pos_info.u16p_pos_list[u16_next_pos_index]),"gsm_fcp_add_frametask():u32_next_pos_fn Illegal!\n");
            /*FIX PhyFA-Bug00002773  END     2011-08-05  : ningyaojun*/ 



            /*FIX LM-Bug00001699  BEGIN 2012-02-28 :fushilong*/
            if(((UINT16)TSNTASK_CCCH_RX == u16_tstask) || ((UINT16)TSNTASK_CCCH_PCH_RX == u16_tstask)|| ((UINT16)TSNTASK_TRANSFER_CCCH_RX == u16_tstask))
            {
                /*  如果下一次为寻呼则更改事务类型为寻呼的事务类型 */
                stp_gsm_ccch_tb = gsm_get_ccch_tbl_cfg(u16_standby_id);
                if((UINT16)1 == stp_gsm_ccch_tb->u16_ccch_tb_flg)
                {
                    u32_scell_current_fn = gsm_get_scell_current_fn(u32_next_pos_fn,(UINT16)0,(SINT16)0,u16_standby_id);
                    u16_paging_period = stp_gsm_ccch_tb->u16_paging_period;
                    u16_paging_offset = stp_gsm_ccch_tb->u16_paging_offset;
                    if((((UINT16)(gsm_fn_operate(u32_scell_current_fn + (UINT32)1)%(UINT32)u16_paging_period)) == u16_paging_offset)
                        ||((UINT16)(u32_scell_current_fn%(UINT32)u16_paging_period) == u16_paging_offset))
                    {
                        e_affair_id = C_ST_TSTASK_ATTRIBUTE_TBL[TSNTASK_CCCH_PCH_RX].e_affair_id;
                    }
                    else
                    {
                        e_affair_id = C_ST_TSTASK_ATTRIBUTE_TBL[TSNTASK_CCCH_RX].e_affair_id;
                    }
                }

            }
            /*FIX LM-Bug00001699  END 2012-02-28 :fushilong*/
            if(MMC_AFF_INVALID != e_affair_id) 
            {

                /* 申请内存 */
                stp_preempt_affair = (l1cc_rat2mmc_preempt_affair_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,
                                                                                    (UINT16)OAL_MEM_DRAM_LEVEL,
                                                                                    (UINT32)OAL_GET_TYPE_SIZE_IN_WORD(l1cc_rat2mmc_preempt_affair_t));     
                oal_mem_set(stp_preempt_affair,OAL_GET_TYPE_SIZE_IN_WORD(l1cc_rat2mmc_preempt_affair_t),(UINT16)0);
                stp_first_raw_affair_info = &(stp_preempt_affair->st_raw_affair[0]);
                stp_first_raw_affair_info->e_affair_id          = e_affair_id;
                stp_first_raw_affair_info->u16_standby_id           = u16_standby_id;
                stp_first_raw_affair_info->u16_active_id        = u16_active_id;
                //stp_first_raw_affair_info->u16_logch_id         = LOGCH_0;
                stp_first_raw_affair_info->b_fuzzy_match        = OAL_TRUE;
                stp_first_raw_affair_info->u16_mode_timing      = GSM_TIMING;
                stp_first_raw_affair_info->u16_rtx_status       = stp_tstask_attribute->u16_rtx_status;  /*GSM_RX_GSM_DATA, GSM_TX_GSM_DATA, GSM_RX_TDD_DATA*/
                stp_first_raw_affair_info->u16_freq             = INVALID_WORD; 
                /* FIX PhyFA-Enh00001876  BEGIN 2014-05-06 : gaowu */
                stp_first_raw_affair_info->st_time_len.u16_superfn = 0;  /*superfn必须为0*/
                /* FIX PhyFA-Enh00001876  END   2014-05-06 : gaowu */
                stp_first_raw_affair_info->st_time_len.u32_sub_fn         = u32_next_pos_fn; 
                stp_first_raw_affair_info->st_time_len.u32_duration         = (UINT32)stp_tstask_attribute->fp_get_rtx_len(u16_tstask);
      
                for(u16_tmp_tsn = st_tsn_info.u16_start_tsn; u16_tmp_tsn <= st_tsn_info.u16_end_tsn; u16_tmp_tsn++)
                {
                    stp_preempt_affair->u16_affair_cnt = 0;
                    if((UINT16)GSM_FCP_TSN_VALID == st_tsn_info.u16_tsn_valid_list[u16_tmp_tsn])
                    {    
                        stp_first_raw_affair_info->st_time_len.u16_superfn        = 0; /*superfn必须为0*/
                        stp_first_raw_affair_info->st_time_len.u32_sub_fn         = u32_next_pos_fn; 
                        stp_first_raw_affair_info->st_time_len.s32_start_offset   = (SINT32)(((SINT32)u16_tmp_tsn*(SINT32)GSM_QBITS_PER_TS)+(SINT32)st_tsn_info.s16_rtx_offset); //if u16_tsn>7
                        stp_first_raw_affair_info->u32_test_info_start_tsn  = (UINT32)u16_tmp_tsn;
                        stp_preempt_affair->u16_affair_cnt = 1;
                        u16_repeat_frms = stp_tstask_attribute->u16_repeat_frms;

                        for(i=1; i<u16_repeat_frms; i++)
                        {
                            stp_other_raw_affair_info = &(stp_preempt_affair->st_raw_affair[i]);
                            oal_mem_copy(stp_other_raw_affair_info,stp_first_raw_affair_info,(UINT16)OAL_GET_TYPE_SIZE_IN_WORD(l1cc_mmc_raw_affair_info_t));  
                            stp_other_raw_affair_info->st_time_len.u32_sub_fn  = gsm_fn_operate((UINT32)((SINT32)u32_next_pos_fn + (SINT32)i));                
                            stp_preempt_affair->u16_affair_cnt++;
                        }
                        /* 给MMC 发送预占消息 */
                        gsm_l1cc_send_msg(stp_preempt_affair,(UINT16)L1CC_MMC_TASK_ID,(UINT32)MSG_RAT2MMC_PREEMPT_AFFAIR_REQ, 
                                          (UINT16)sizeof(l1cc_rat2mmc_preempt_affair_t),u16_standby_id);
                    }
                }
                /* 释放内存 */
                oal_mem_free((CONST_VOID_PTR*)&stp_preempt_affair);  
            }
        }
    }
    return u32_fcp_ret;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/


/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_get_spec_tb_info
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-11-04      zhengying               PhyFA-Req00001525   [NGM]V4 GSM代码入库
*
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
STATIC OAL_STATUS gsm_fcp_get_spec_tb_info(UINT16_PTR u16p_tsn_num,UINT16_PTR u16_tsn_list,SINT16_PTR s16p_rtx_offset_list,VOID_PTR *stp_rrbp_data_list,CONST_UINT16 u16_tstask,UINT32 u32_fn,CONST_UINT16 u16_data_state,UINT16 u16_standby_id)
{
    OAL_STATUS fcp_ret = OAL_FAILURE;
    rrbp_data_t * stp_rrbp_data;
    gprs_fcp_rrbp_encbuff_info_st *stp_rrbp_encbuff;
    UINT16 u16_i;
    UINT16 u16_j;
    CONST gsm_ccch_tb_t*  stp_ccch_tbl = NULL_PTR;
    CONST gprs_pccch_tb_t* stp_gprs_pccch_tlb = NULL_PTR;
    CONST gsm_rach_tb_t * stp_gsm_rach_tb = NULL_PTR;
    CONST gprs_prach_tb_t *stp_gprs_prach_tb = NULL_PTR;
    gprs_pdtch_tb_t *  stp_gprs_pdtch_tb = NULL_PTR;
    CONST gsm_conn_ho_tb_t *stp_gsm_conn_ho_tb = NULL_PTR;
    CONST gsm_conn_ho_params_t *stp_gsm_conn_ho_params = NULL_PTR;
    CONST gsm_channel_parameters_t *stp_gsm_channel_params = NULL_PTR;
    CONST gsm_traffic_schedu_t *stp_gsm_traffic_schedu = NULL_PTR;
    OAL_ASSERT(NULL_PTR!=u16p_tsn_num, "gsm_fcp_get_spec_tb_info null pointer");
    OAL_ASSERT(NULL_PTR!=u16_tsn_list, "gsm_fcp_get_spec_tb_info null pointer");
    OAL_ASSERT(NULL_PTR!=s16p_rtx_offset_list, "gsm_fcp_get_spec_tb_info null pointer");
    OAL_ASSERT(NULL_PTR!=stp_rrbp_data_list, "gsm_fcp_get_spec_tb_info null pointer");

    

    stp_ccch_tbl = gsm_get_ccch_tbl_cfg(u16_standby_id);
    
    *u16p_tsn_num = 0;
    stp_gsm_rach_tb = gsm_get_rach_tab();
    stp_gprs_prach_tb = gsm_get_prach_tb();
    stp_gprs_pdtch_tb = gsm_get_gprs_cfg_tbl();
    
    switch(u16_tstask)
    {
        case TSNTASK_RACH_TX:
            if((UINT16)GSM_TABLE_VALID == stp_gsm_rach_tb->u16_rach_tb_flg)
            {
                if(u16_data_state == stp_gsm_rach_tb->u16_data_state)
                {
                    if(u32_fn == stp_gsm_rach_tb->u32_next_rach_fn)
                    {
                        *u16p_tsn_num = 1;
                         u16_tsn_list[0] = stp_ccch_tbl->u16_ccch_tn + (UINT16)3;
                         s16p_rtx_offset_list[0] = 0;
                         fcp_ret = OAL_SUCCESS;
                    }
                    else if(gsm_fn_compare(stp_gsm_rach_tb->u32_next_rach_fn,u32_fn))
                    {
                        /* FIX A2KPH-Enh00000876  BEGIN   2009-01-12  : ningyaojun*/
                        gsm_fcp_handle_hls_res((UINT16)GSM_FCP_RACH_CNF,stp_gsm_rach_tb->u32_next_rach_fn,OAL_FAILURE,u16_standby_id);                                
                        oal_error_handler(ERROR_CLASS_MINOR, GSM_FCP_ERR(RACH_TX_OVERDUE_FN));    
                        fcp_ret = OAL_FAILURE;        
                        /* FIX A2KPH-Enh00000876  END     2007-01-12  : ningyaojun*/                    
                    }
                }
            }
            break;
        case TSNTASK_PRACH_TX:
            if((UINT16)GSM_TABLE_VALID == stp_gprs_prach_tb->u16_prach_tb_flg)
            {
                if(u16_data_state == stp_gprs_prach_tb->u16_prach_data_state)
                {
                    

                    stp_gprs_pccch_tlb = gsm_get_gprs_pccch_tbl(u16_standby_id);
                    /* Fix PhyFA Bug00000440 begin, guxiaobo, 20080911 */
                    /* Fix A2KPH Bug00001019 begin, guxiaobo, 20081021*/
                    if( ((UINT16)MPAL_PAGING_REORG == stp_gprs_pccch_tlb->u16_page_mode )
                       || ((UINT16)MPAL_FULL_PCCCH_BLKS == stp_gprs_pccch_tlb->u16_page_mode))
                    {
                       /* if it is DATA_UNENCODED, do encode task immediately without checking fn's arrival.
                          Because prach enc fn is irrelevant with prach tx.*/
                        if((UINT16)GSM_DATA_UNENCODED == u16_data_state )
                        {
                             *u16p_tsn_num = 1;
                             u16_tsn_list[0] = stp_gprs_pccch_tlb->u16_pccch_tn + (UINT16)3;
                             s16p_rtx_offset_list[0] = 0;
                             fcp_ret = OAL_SUCCESS;
                        }
                        else if(((UINT16)GSM_DATA_ENCODED == u16_data_state  )&&(u32_fn == stp_gprs_prach_tb->u32_prach_fn))
                        {
                            *u16p_tsn_num = 1;
                             u16_tsn_list[0] = stp_gprs_pccch_tlb->u16_pccch_tn + (UINT16)3;
                             s16p_rtx_offset_list[0] = 0;
                             fcp_ret = OAL_SUCCESS;
                        }
                        else if(gsm_fn_compare(stp_gprs_prach_tb->u32_prach_fn,u32_fn))
                        {
                            /* FIX A2KPH-Enh00000876  BEGIN   2009-01-12  : ningyaojun*/
                            gsm_fcp_handle_hls_res((UINT16)GSM_FCP_RACH_CNF,stp_gsm_rach_tb->u32_next_rach_fn,OAL_FAILURE,u16_standby_id);                                
                            oal_error_handler(ERROR_CLASS_MINOR, GSM_FCP_ERR(RACH_TX_OVERDUE_FN));    
                            fcp_ret = OAL_FAILURE;            
                            /* FIX A2KPH-Enh00000876  END     2007-01-12  : ningyaojun*/        
                        }

                    }
                    else
                    {
                        if(u32_fn == stp_gprs_prach_tb->u32_prach_fn)
                        {
                            *u16p_tsn_num = 1;
                             u16_tsn_list[0] = stp_gprs_pccch_tlb->u16_pccch_tn + (UINT16)3;
                             s16p_rtx_offset_list[0] = 0;
                             fcp_ret = OAL_SUCCESS;
                        }
                        else if(gsm_fn_compare(stp_gprs_prach_tb->u32_prach_fn,u32_fn))
                        {
                            /* FIX A2KPH-Enh00000876  BEGIN   2009-01-12  : ningyaojun*/
                            gsm_fcp_handle_hls_res((UINT16)GSM_FCP_RACH_CNF,stp_gsm_rach_tb->u32_next_rach_fn,OAL_FAILURE,u16_standby_id);                                
                            oal_error_handler(ERROR_CLASS_MINOR, GSM_FCP_ERR(RACH_TX_OVERDUE_FN));    
                            fcp_ret = OAL_FAILURE;        
                            /* FIX A2KPH-Enh00000876  END     2007-01-12  : ningyaojun*/        
                        }

                    }
                    /* Fix PhyFA Bug00000440 end, guxiaobo, 20080911 */
                    /* Fix A2KPH Bug00001019 end, guxiaobo, 20081021*/
                }
            }
            break;
        case TSNTASK_SINGLEBLOCK_TX:
            if((UINT16)GSM_TABLE_VALID == stp_gprs_pdtch_tb->u16_pdtch_tb_flg)
            {
                stp_rrbp_data = &(stp_gprs_pdtch_tb->st_rrbp_data[0]);
                if(u16_data_state == stp_rrbp_data->u16_rrbp_data_state)
                {
                    if(u32_fn == stp_rrbp_data->u32_rrbp_send_fn)
                    {
                         *u16p_tsn_num = 1;
                         u16_tsn_list[0]  = (UINT16)(stp_rrbp_data->u16_rrbp_tn_slot + (UINT16)3);
                         /* FIX LM-Bug00002185    BEGIN   2012-06-28  : linlan*/   
                         if ((stp_rrbp_data->u16_rrbp_burst_type >= (UINT16)4) && ((UINT16)INVALID_BYTE != stp_gprs_pdtch_tb->u16_pdtch_ta))
                         {
                             s16p_rtx_offset_list[0] = -(SINT16)stp_gprs_pdtch_tb->u16_pdtch_ta;
                             stp_rrbp_data_list[0] = stp_rrbp_data;
                             fcp_ret = OAL_SUCCESS;
                         }
                         else if((UINT16)4 > (UINT16)(stp_rrbp_data->u16_rrbp_burst_type))
                         {
                             s16p_rtx_offset_list[0] = 0;
                             stp_rrbp_data_list[0] = stp_rrbp_data;
                             fcp_ret = OAL_SUCCESS;   
                         }
                         else
                         {
                            fcp_ret = OAL_FAILURE;
                         }
                         /* FIX LM-Bug00002185    END     2012-06-28  : linlan*/
    
                    }
                    else if(gsm_fn_compare(stp_rrbp_data->u32_rrbp_send_fn,u32_fn))
                    {
                        /*FIX LM-Bug00000987  BEGIN  2011-10-13 :fushilong*/
                        stp_rrbp_data->u16_rrbp_data_state = GSM_DATA_INVALID;                     
                        oal_error_handler(ERROR_CLASS_MINOR, GSM_FCP_ERR(INVALID_PARAM));
                        /*FIX LM-Bug00000987  END 2011-10-13 :fushilong*/
                    }
                }
            }
            break;
            /*support handover function*/
        case TSNTASK_HANDOVER_TX:
            stp_gsm_conn_ho_tb     = gsm_get_conn_ho_tb();
            stp_gsm_conn_ho_params = &stp_gsm_conn_ho_tb->st_conn_ho_params;
            stp_gsm_channel_params = &stp_gsm_conn_ho_tb->st_channel_params;
            stp_gsm_traffic_schedu = &stp_gsm_conn_ho_tb->st_traffic_schedu;
            if((UINT16)GSM_TABLE_VALID == stp_gsm_conn_ho_tb->u16_conn_ho_tb_flg)
            {
                if(u16_data_state == stp_gsm_conn_ho_params->u16_data_state)
                {
                    {
                        *u16p_tsn_num = 1;
                         switch(stp_gsm_channel_params->u16_ch_type)
                         {
                             case MPAL_SDCCH:
                                u16_tsn_list[0] = stp_gsm_traffic_schedu->u16_sdcch_rx_tn + (UINT16)3;
                                break;
                             case MPAL_TCH_F:
                                u16_tsn_list[0] = stp_gsm_traffic_schedu->u16_tch_rx_tn + (UINT16)3;
                                break;
                                /* FIX A2KPH-Bug00000509 BEGIN    2008-07-14: fushilong */
                                case MPAL_TCH_H_0:
                                case MPAL_TCH_H_1:
                                u16_tsn_list[0] = stp_gsm_traffic_schedu->u16_tch_rx_tn + (UINT16)3;
                                break;
                                /* FIX A2KPH-Bug00000509 END    2008-07-14: fushilong */
                             default:
                                 oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNKNOWN_CHAN_TYPE));
                                 break;
                         }

                         s16p_rtx_offset_list[0] = 0;
                         fcp_ret = OAL_SUCCESS;
                    }
                }
            }
            break;
        case TSNTASK_RRBP_TX0:
            /* FIX PhyFA-Bug00000436    BEGIN   2008-07-02  : ningyaojun*/      
            if(  ((UINT16)GSM_TABLE_VALID == stp_gprs_pdtch_tb->u16_pdtch_tb_flg)
               ||((UINT16)1 == stp_gprs_pdtch_tb->u16_rrbp_exist_without_tbf_flag)
              )
            /* FIX PhyFA-Bug00000436    END     2008-07-02  : ningyaojun*/
            {
                switch(u16_data_state)
                {
                    case GSM_DATA_UNENCODED:
                        u16_j = 0;
                        for(u16_i = 0; u16_i < (UINT16)RRBP_DATA_BUFF_NUM; u16_i++)
                        {
                            stp_rrbp_data = &stp_gprs_pdtch_tb->st_rrbp_data[u16_i];
                            if(u16_data_state == stp_rrbp_data->u16_rrbp_data_state)
                            {
                                if(u32_fn == stp_rrbp_data->u32_rrbp_send_fn)
                                {
                                    /* FIX PhyFA-Inc00000009   2007-12-26  : huangjinfu begin*/
                                    u16_tsn_list[u16_j]  = (UINT16)(stp_rrbp_data->u16_rrbp_tn_slot + (UINT16)3);
                                    /* FIX PhyFA-Bug00000572   BEGIN  2008-07-05  : huangjinfu */
                                    /* FIX LM-Bug00002185    BEGIN   2012-06-28  : linlan*/ 
                                    stp_rrbp_data_list[u16_j] = stp_rrbp_data;
                                    if((UINT16)4 > stp_rrbp_data->u16_rrbp_burst_type)
                                    {
                                        s16p_rtx_offset_list[u16_j] = 0;
                                        u16_j++;
                                    }
                                    else if((UINT16)INVALID_BYTE != stp_gprs_pdtch_tb->u16_pdtch_ta)
                                    {
                                        s16p_rtx_offset_list[u16_j] = -(SINT16)stp_gprs_pdtch_tb->u16_pdtch_ta;
                                        u16_j++;
                                    }
                                    else
                                    {
                                        continue;
                                    }
                                    /* FIX PhyFA-Bug00000572   END  2008-07-05  : huangjinfu */
                                    /* FIX PhyFA-Inc00000009   2007-12-26  : huangjinfu end*/
                                }
                                /* Fix A2KPH-Bug00001128  BEGIN 2008-11-08*/
                                else if( gsm_fn_operate(stp_rrbp_data->u32_rrbp_send_fn+(UINT32)1) == u32_fn)
                                {
                                    /*FIX PhyFA-Enh00001343 BEGIN 2012-09-03: dujianzhong */
                                    OAL_PRINT(u16_standby_id,(UINT16)GSM_MODE,"rrbp_send_fn = %ld, fn = %ld, rrbp encode has been delayed!", stp_rrbp_data->u32_rrbp_send_fn, u32_fn);
                                    /*FIX PhyFA-Enh00001343 END   2012-09-03: dujianzhong */
                                }
                                else if(gsm_fn_compare((UINT32)(stp_rrbp_data->u32_rrbp_send_fn+(UINT32)1), u32_fn))
                                {
                                    /*FIX PhyFA-Enh00001343 BEGIN 2012-09-03: dujianzhong */
                                    OAL_PRINT(u16_standby_id,(UINT16)GSM_MODE,"rrbp_send_fn = %ld, fn = %ld, rrbp tx has been canceled!", stp_rrbp_data->u32_rrbp_send_fn, u32_fn);
                                    /*FIX PhyFA-Enh00001343 END   2012-09-03: dujianzhong */
                                    stp_rrbp_data->u16_rrbp_data_state = GSM_DATA_INVALID;
                                }
                                /* Fix A2KPH-Bug00001128  END 2008-11-08*/
                            }
                        }
                        if ((UINT16)0 ==  u16_j )
                        {
                            fcp_ret = OAL_FAILURE; 
                        }
                        else
                        {                           
                            fcp_ret = OAL_SUCCESS; 
                        }
                        /* FIX LM-Bug00002185    END   2012-06-28  : linlan*/ 
                        (*u16p_tsn_num) = u16_j;
                        break;
                    case GSM_DATA_ENCODED:
                        u16_j = 0;
                        for(u16_i = 0; u16_i < (UINT16)RRBP_DATA_BUFF_NUM; u16_i++)
                        {
                            stp_rrbp_encbuff = gsm_get_rrbp_encbuff_info(u16_i);
                            if(u16_data_state == stp_rrbp_encbuff->u16_data_state)
                            {
                                if(u32_fn == stp_rrbp_encbuff->u32_rrbp_fn)
                                {
                                    u16_tsn_list[u16_j]  = (UINT16)(stp_rrbp_encbuff->u16_tsn);
                                    /* FIX PhyFA-Bug00000572   BEGIN  2008-07-05  : huangjinfu */
                                    if( (UINT16)4 > stp_rrbp_encbuff->u16_burst_type )
                                    {
                                        s16p_rtx_offset_list[u16_j] = 0;
                                    }
                                    else
                                    {
                                        s16p_rtx_offset_list[u16_j] = -(SINT16)stp_gprs_pdtch_tb->u16_pdtch_ta;
                                    }
                                    /* FIX PhyFA-Bug00000572   END  2008-07-05  : huangjinfu */
                                    u16_j++;
                                    fcp_ret = OAL_SUCCESS;
                                }
                                else if(gsm_fn_compare(stp_rrbp_encbuff->u32_rrbp_fn,u32_fn))
                                {
                                    /* FIX PhyFA-Enh00000089   BEGIN  2008-04-07  : huangjinfu */
                                    /*because of neighbour bcch rx*/
                                    stp_rrbp_encbuff->u16_data_state = GSM_DATA_SENT;
                                    fcp_ret = OAL_FAILURE;
                                    //oal_error_handler(OAL_RECOVERABLE_ERROR, L1CC_GSM_FCP_MODULE);
                                    /* FIX PhyFA-Enh00000089   END    2008-04-07  : huangjinfu */
                                }
                            }
                        }
                        (*u16p_tsn_num) = u16_j;
                        break;
                    default:
                        oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
                        break;
                }

            }
             break;
        default:
            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
            break;
        }
       return fcp_ret;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/

L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
OAL_STATUS gsm_fcp_add_special_frametask(UINT16 u16_tstask,UINT32 u32_fn, UINT16 u16_standby_id)
{
    SINT16  s16_rtx_offset_list[4];
    UINT16  u16_tsn_list[4];
    UINT16  u16_tsn_num;
    UINT16  u16_i;
    VOID_PTR vp_para[4];
    UINT32 u32_fcp_ret=OAL_FAILURE;
    gsm_offset_in_frame_t st_frame_pos;


    u32_fcp_ret = gsm_fcp_get_spec_tb_info(&u16_tsn_num,u16_tsn_list,s16_rtx_offset_list,vp_para,u16_tstask,u32_fn,(UINT16)GSM_DATA_ENCODED,u16_standby_id);

    if(OAL_SUCCESS== u32_fcp_ret)
    {
        for(u16_i=0;u16_i<u16_tsn_num;u16_i++)
        {
            st_frame_pos.s16_offset = s16_rtx_offset_list[u16_i];
            st_frame_pos.u16_tsn = u16_tsn_list[u16_i];
            st_frame_pos.u32_fn = u32_fn;
            u32_fcp_ret = gsm_fcp_check_add_tsntask(u16_tstask,&st_frame_pos,u16_standby_id);
        }
    }
    return u32_fcp_ret;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/








/************************************************************
** Function Name: gsm_fcp_send_enc_msg ()
** Description: send the msg MSG_GSM_FCP_ENC_DATA_REQ to GSM_CC_ENC_TASK
** Input :
      u16_tstask: the task of the the timeslot
      u32_fn:  the frame number

** Output : NULL

** Return:  OAL_SUCCESS or OAL_FAILURE
** Notes:
2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
OAL_STATUS gsm_fcp_enc_data(IN CONST_UINT16 u16_tstask,IN CONST_UINT32 u32_fn, IN CONST_UINT16 u16_standby_id)
{

    UINT32 u32_tb_flg;
    gsm_tb_pos_t st_tb_pos_info = {0, 0, NULL_PTR, NULL_PTR};


    UINT16  u16_tmp_tsn;
    //UINT16  u16_start_tsn;
    //UINT16  u16_end_tsn;
    //UINT16  u16_tsn_valid_list[11];
    UINT32 u32_sent_fn;
    gsm_tsn_info_t st_tsn_info;
    OAL_STATUS u32_fcp_ret=OAL_FAILURE;
    UINT16_PTR u16p_gsm_dummy_sent_blks = NULL_PTR;

    gsm_enc_frame_info_t st_enc_frame_info;
   // CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    CONST gsm_conn_ch_tb_t *stp_gsm_conn_ch_tb = NULL_PTR;
    
    stp_gsm_conn_ch_tb = gsm_get_conn_ch_tb();
    
    u32_sent_fn = gsm_fn_operate(u32_fn+(UINT32)GSM_DATA_ENC_ADVANCE);

    st_tsn_info.u16_tstask = u16_tstask;
    st_tsn_info.u32_fn     = u32_sent_fn;
    st_tsn_info.u16_standby_id = u16_standby_id;
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    u32_tb_flg = gsm_fcp_get_tb_tsn_info(&st_tsn_info);
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/


    if(OAL_FAILURE == u32_tb_flg)
    {
        return u32_fcp_ret;
    }

    /*FIX PhyFA-Req00000936  BEGIN   2011-06-08  : ningyaojun*/
    /* FIX LM-Bug00002185    BEGIN   2012-06-28  : linlan*/    
    u32_fcp_ret = gsm_fcp_get_tb_pos_info(&st_tb_pos_info,u16_tstask,u32_sent_fn, u16_standby_id);
    if(OAL_FAILURE == u32_fcp_ret )
    {
        return u32_fcp_ret;
    }
    u32_fcp_ret = OAL_FAILURE;
    /* FIX LM-Bug00002185    END     2012-06-28  : linlan*/
    if(TSNTASK_SACCH_TX == u16_tstask)
    {
        if(st_tb_pos_info.u16p_pos_list[0]==(UINT16)(u32_sent_fn%(UINT32)st_tb_pos_info.u16_period))
        {
            u32_fcp_ret = OAL_SUCCESS;
        }
    }
    /*FIX PhyFA-Req00000936  END     2011-06-08  : ningyaojun*/
    
    /* FIX PhyFA-Req00000124 BEGIN  2008-05-19  : fushilong */
    else if(TSNTASK_FACCH_H_TX == u16_tstask)
    {
        /*  the table may be g_st_gsm_conn_ho_tb?  */
        if (((UINT16)(u32_sent_fn%(UINT32)26) ==stp_gsm_conn_ch_tb->st_traffic_schedu.u16_facch_tx_pos[0]) ||
            ((UINT16)(u32_sent_fn%(UINT32)26) ==stp_gsm_conn_ch_tb->st_traffic_schedu.u16_facch_tx_pos[1]) ||
            ((UINT16)(u32_sent_fn%(UINT32)26) ==stp_gsm_conn_ch_tb->st_traffic_schedu.u16_facch_tx_pos[2]))
        {
            u32_fcp_ret = OAL_SUCCESS;
        }
    }
    /* FIX PhyFA-Req00000124 END  2008-05-19  : fushilong */
    else
    {
        /*FIX PhyFA-Req00000936  BEGIN   2011-06-08  : ningyaojun*/
        u32_fcp_ret = gsm_fcp_arrive_pos(&st_tb_pos_info,u32_sent_fn);
        /*FIX PhyFA-Req00000936  END     2011-06-08  : ningyaojun*/    
    }
    if(OAL_SUCCESS == u32_fcp_ret)
    {
        u16p_gsm_dummy_sent_blks = gsm_get_dummy_sent_blks();

        *u16p_gsm_dummy_sent_blks = 0;

        st_enc_frame_info.u16_standby_id = u16_standby_id;
        st_enc_frame_info.u16_start_tsn = st_tsn_info.u16_start_tsn;
        st_enc_frame_info.u16_end_tsn =st_tsn_info.u16_end_tsn;
        st_enc_frame_info.u16_tmp_tsn = st_tsn_info.u16_start_tsn;
        st_enc_frame_info.u16_tstask = u16_tstask;
        st_enc_frame_info.u32_fn = u32_fn;
        
        for(u16_tmp_tsn = st_tsn_info.u16_start_tsn;u16_tmp_tsn <= st_tsn_info.u16_end_tsn;u16_tmp_tsn++)
        {
             if((UINT16)GSM_FCP_TSN_VALID == st_tsn_info.u16_tsn_valid_list[u16_tmp_tsn])
             {
                st_enc_frame_info.u16_tmp_tsn = u16_tmp_tsn;
                /*gsm_fcp_trans_rxfn_to_txfn(&u32_fn,&u16_tsn,u32_fn,u16_tmp_tsn);*/
                switch(u16_tstask)
                {
                    case TSNTASK_SDCCH_TX:
                        gsm_sdcch_tx_enc_data(&st_enc_frame_info);
                        break;
                    case TSNTASK_SACCH_TX:
                        gsm_sacch_tx_enc_data(&st_enc_frame_info);
                         break;
                    case TSNTASK_FACCH_TX:

                        gsm_facch_tx_enc_data(&st_enc_frame_info);
                         break;
                    case TSNTASK_FACCH_H_TX:
                        gsm_facch_h_tx_enc_data(&st_enc_frame_info);
                         break;
                    case TSNTASK_TCH_TX:
                        gsm_tch_tx_enc_data(&st_enc_frame_info);
                         break;
                    case TSNTASK_TCH_H_TX:
                        gsm_tch_h_tx_enc_data(&st_enc_frame_info);
                         break;

                    case TSNTASK_TCH_F96_TX:
                    case TSNTASK_TCH_F144_TX:
                        gsm_tch_f96_f144_tx_enc_data(&st_enc_frame_info);
                        break;
                    case TSNTASK_PTCCH_TX:  
                        gsm_ptcch_tx_enc_data(&st_enc_frame_info);
                        break;
                    case TSNTASK_PDTCH_TX:
                    /* FIX LTE-Enh00000846 BEGIN  2014-04-29: wangjunli */
                    case TSNTASK_PDTCH_FTA_TX:
                    /* FIX LTE-Enh00000846 END  2014-04-29: wangjunli */
                        gsm_pdtch_tx_enc_data(&st_enc_frame_info);
                        break;
                    default:
                        oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
                        break;
                 }
             }
        }
    }
    return u32_fcp_ret;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/

/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_enc_special_data
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-11-04      zhengying               PhyFA-Req00001525   [NGM]V4 GSM代码入库
*2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
OAL_STATUS gsm_fcp_enc_special_data(UINT32_PTR u32p_msg_num,UINT16 u16_tstask,CONST_UINT32 u32_fn,UINT16 u16_standby_id)
{

    UINT32 u32_fcp_ret=OAL_FAILURE;
    UINT32 u32_sent_fn;
    UINT16 u16_tsc;
    /* FIX PhyFA-Req00000215   BEGIN   2008-12-03  : ningyaojun*/
    UINT16_PTR u16p_enc_data_addr;
    /* FIX PhyFA-Req00000215   END     2008-12-03  : ningyaojun*/
    freq_params_struct *stp_freq_params=NULL_PTR;
    gprs_pdtch_para_t *stp_pdtch_prarms=NULL_PTR;
    rrbp_data_t  *stp_rrbp_data_list[4];
    SINT16 s16_rtx_offset_list[4];
    UINT16  u16_tsn_list[4];
    UINT16 u16_tsn_num;
    UINT16 u16_i;
    
    rrbp_data_t  *stp_rrbp_data;
    UINT16 u16_rrbp_enc_tsn;
    UINT16  u16_relative_tsn;
    /* FIX LM-Bug00001262    BEGIN   2011-11-14  : linlan*/
    UINT16 u16_bc_buffer_index;
    /* FIX LM-Bug00001262    END   2011-11-14  : linlan*/
    /* FIX PhyFA-Req00001525 BEGIN 2013-09-29 : wuxiaorong */
    msg_gsm_fcp_enc_data_req_t *stp_enc_req = NULL_PTR;
    /* FIX PhyFA-Req00001525 END   2013-09-29 : wuxiaorong */
    CONST gsm_ccch_tb_t*  stp_ccch_tbl = NULL_PTR;
    gprs_pccch_tb_t* stp_gprs_pccch_tlb = NULL_PTR;
    gsm_rach_tb_t *stp_gsm_rach_tb = NULL_PTR;
    gprs_prach_tb_t *stp_gprs_prach_tb = NULL_PTR;
    /* FIX PhyFA-Req00001525 BEGIN 2013-11-05 : zhengying */
    CONST gsm_scell_info_t *stp_gsm_scell_info = NULL_PTR;
    CONST gprs_pdtch_tb_t  *stp_gprs_pdtch_tb = NULL_PTR;
    UINT16_PTR  u16p_rrbp_enc_tsn_occupied = NULL_PTR;
    gprs_fcp_rrbp_encbuff_info_st   *stp_gprs_rrbp_encbuff_info = NULL_PTR;
    gsm_conn_ho_tb_t *stp_gsm_conn_ho_tb = NULL_PTR;
    gsm_conn_ho_params_t *stp_gsm_conn_ho_params = NULL_PTR;
    gsm_channel_parameters_t *stp_gsm_channel_params = NULL_PTR;
    /* FIX PhyFA-Req00001525 END   2013-11-05 : zhengying */
    //CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    
    
    OAL_ASSERT(NULL_PTR != u32p_msg_num, "gsm_fcp_enc_special_data null pointer");

    u32_sent_fn = gsm_fn_operate(u32_fn + GSM_DATA_ENC_ADVANCE);
    stp_gprs_pdtch_tb = gsm_get_gprs_cfg_tbl();
    stp_ccch_tbl = gsm_get_ccch_tbl_cfg(u16_standby_id);
    u32_fcp_ret = gsm_fcp_get_spec_tb_info(&u16_tsn_num,u16_tsn_list,s16_rtx_offset_list,(VOID_PTR *)stp_rrbp_data_list,
                                           u16_tstask,u32_sent_fn,(UINT16)GSM_DATA_UNENCODED,u16_standby_id);

    stp_gsm_rach_tb = gsm_get_rach_tab();
    stp_gprs_prach_tb = gsm_get_prach_tb();
    stp_gsm_scell_info = gsm_get_gsm_scell_info(u16_standby_id);
    if(OAL_SUCCESS== u32_fcp_ret)
    {
        u32_fcp_ret = OAL_SUCCESS;
        switch(u16_tstask)
        {
            case TSNTASK_RACH_TX:
                /* FIX PhyFA-Req00001525 BEGIN 2013-09-29 : wuxiaorong */
                stp_enc_req = (msg_gsm_fcp_enc_data_req_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,(UINT32) OAL_GET_TYPE_SIZE_IN_WORD(msg_gsm_fcp_enc_data_req_t));     
                stp_enc_req->u16p_InData = (UINT16_PTR)&stp_gsm_rach_tb->u16_msg;
                stp_enc_req->u32_tx_fn = u32_sent_fn;
                stp_enc_req->u32_channeltype = (UINT32)MPAL_RACH;
                stp_enc_req->u32_channel_mode = (UINT32)MPAL_SIGNAL_CODE;
                stp_enc_req->u32_burst_type = (UINT32)(stp_gsm_rach_tb->u16_burst_type);
                stp_enc_req->u16_tsqnum = (stp_gsm_scell_info->u16_bcch_bsic);
                stp_enc_req->u16_tsn = (stp_ccch_tbl->u16_ccch_tn + (UINT16)3);

                stp_enc_req->u16_burstindex = (UINT16)0;
                stp_enc_req->u16_edge_hd_type = INVALID_BYTE;
                stp_enc_req->u16_cps = INVALID_BYTE;
                stp_enc_req->u16_standby_id = u16_standby_id;
                GSM_SEND_EC_REQ(stp_enc_req,u16_standby_id);
                oal_mem_free((CONST_VOID_PTR*)&stp_enc_req);   
                /* FIX PhyFA-Req00001525 END   2013-09-29 : wuxiaorong */
                stp_gsm_rach_tb->u16_data_state = GSM_DATA_ENCODED;
                
                break;
            case TSNTASK_PRACH_TX:
                
                stp_gprs_pccch_tlb = gsm_get_gprs_pccch_tbl(u16_standby_id);
                
                stp_freq_params = &(stp_gprs_pccch_tlb->st_freq_params);
                /* FIX PhyFA-Req00001525 BEGIN 2013-09-29 : wuxiaorong */
                stp_enc_req = (msg_gsm_fcp_enc_data_req_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,(UINT32) OAL_GET_TYPE_SIZE_IN_WORD(msg_gsm_fcp_enc_data_req_t));     
                stp_enc_req->u16p_InData = (UINT16_PTR)&stp_gprs_prach_tb->u16_prach_msg;
                stp_enc_req->u32_tx_fn = u32_sent_fn;   
                stp_enc_req->u32_channeltype = (UINT32)MPAL_PRACH;
                stp_enc_req->u32_channel_mode = (UINT32)MPAL_SIGNAL_CODE;
                stp_enc_req->u32_burst_type = (UINT32)(stp_gprs_prach_tb->u16_access_burst_type);
                stp_enc_req->u16_tsqnum = (stp_gsm_scell_info->u16_bcch_bsic);
                stp_enc_req->u16_tsn = (stp_gprs_pccch_tlb->u16_pccch_tn + (UINT16)3);
                stp_enc_req->u16_burstindex = (UINT16)0;
                stp_enc_req->u16_edge_hd_type = INVALID_BYTE;
                stp_enc_req->u16_cps = INVALID_BYTE;
                stp_enc_req->u16_standby_id = u16_standby_id;
                GSM_SEND_EC_REQ(stp_enc_req,u16_standby_id);
                oal_mem_free((CONST_VOID_PTR*)&stp_enc_req);   
                /* FIX PhyFA-Req00001525 END   2013-09-29 : wuxiaorong */
                

                /* Fix PhyFA Bug00000440 begin, guxiaobo, 20080911 */
                /* Fix A2KPH Bug00001019 begin, guxiaobo, 20081021*/
                if( ((UINT16)MPAL_PAGING_REORG == stp_gprs_pccch_tlb->u16_page_mode )
                    || ((UINT16)MPAL_FULL_PCCCH_BLKS == stp_gprs_pccch_tlb->u16_page_mode))
                {
                     /* only used in USF FREE based PRACH TX */
                    stp_gprs_prach_tb->u32_prach_enc_fn = u32_sent_fn;
                }
                else
                {
                    stp_gprs_prach_tb->u32_prach_enc_fn = (UINT32)GSM_MAX_FN; //invalid
                }
                /* Fix A2KPH Bug00001019 end, guxiaobo, 20081021*/
                /* Fix PhyFA Bug00000440 end, guxiaobo, 20080911 */

                stp_gprs_prach_tb->u16_prach_data_state= GSM_DATA_ENCODED;
                break;
            /*support handover function*/
            case TSNTASK_HANDOVER_TX:
                /* FIX PhyFA-Req00000215   BEGIN   2008-12-03  : ningyaojun*/
                stp_gsm_conn_ho_tb     = gsm_get_conn_ho_tb();
                stp_gsm_conn_ho_params = &stp_gsm_conn_ho_tb->st_conn_ho_params;
                stp_gsm_channel_params = &stp_gsm_conn_ho_tb->st_channel_params;
                
                u16p_enc_data_addr = (UINT16_PTR)&stp_gsm_conn_ho_params->u16_reference;                
                stp_freq_params  = &stp_gsm_channel_params->st_freq_after_time;
                /* FIX PhyFA-Req00001525 BEGIN 2013-09-29 : wuxiaorong */
                stp_enc_req = (msg_gsm_fcp_enc_data_req_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,(UINT32) OAL_GET_TYPE_SIZE_IN_WORD(msg_gsm_fcp_enc_data_req_t));     
                stp_enc_req->u16p_InData = u16p_enc_data_addr;
                stp_enc_req->u32_tx_fn = 0;
                stp_enc_req->u32_channeltype = (UINT32)MPAL_RACH;
                stp_enc_req->u32_channel_mode = (UINT32)MPAL_SIGNAL_CODE;
                stp_enc_req->u32_burst_type = (UINT32)ONE_ACCESS_BURST_OF_TYPE_8_BITS;
                stp_enc_req->u16_tsqnum = stp_gsm_conn_ho_params->u16_bsic;
                stp_enc_req->u16_tsn = 0;

                stp_enc_req->u16_burstindex = (UINT16)0;
                stp_enc_req->u16_edge_hd_type = INVALID_BYTE;
                stp_enc_req->u16_cps = INVALID_BYTE;
                stp_enc_req->u16_standby_id = u16_standby_id;
                GSM_SEND_EC_REQ(stp_enc_req,u16_standby_id);
                oal_mem_free((CONST_VOID_PTR*)&stp_enc_req);   
                /* FIX PhyFA-Req00001525 END   2013-09-29 : wuxiaorong */
                /* FIX PhyFA-Req00000215   END     2008-12-03  : ningyaojun*/             
                stp_gsm_conn_ho_params->u16_data_state = GSM_DATA_ENCODED;
                break;
            case TSNTASK_SINGLEBLOCK_TX:
                /* FIX PhyFA-Req00001111 BEGIN  2011-11-09 : wanghuan */ 
                gsm_get_singleblock_freqpara(&stp_freq_params,u32_sent_fn,u16_standby_id);
                /* FIX PhyFA-Req00001111 END  2011-11-09 : wanghuan */ 

                stp_rrbp_data = stp_rrbp_data_list[0];
                /* FIX PhyFA-Req00000215   BEGIN   2008-12-07  : ningyaojun*/
                u16p_enc_data_addr = (UINT16_PTR)((VOID_PTR)&stp_rrbp_data->u16_rrbp_data[0]);
                /* FIX PhyFA-Req00000215   END     2008-12-07  : ningyaojun*/
                stp_rrbp_data->u16_rrbp_data_state= GSM_DATA_ENCODED;

                /* FIX PhyFA-Req00000215   BEGIN   2008-12-07  : ningyaojun*/
                /*added by huangjinfu 20080827 at hangzhou*/
                u16_tsc = gprs_enc_update_tsc(stp_rrbp_data->u16_rrbp_burst_type,stp_freq_params->tsc,u16_standby_id);
                /* FIX PhyFA-Req00001525 BEGIN 2013-09-29 : wuxiaorong */
                stp_enc_req = (msg_gsm_fcp_enc_data_req_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,(UINT32) OAL_GET_TYPE_SIZE_IN_WORD(msg_gsm_fcp_enc_data_req_t));     
                stp_enc_req->u16p_InData = u16p_enc_data_addr;
                stp_enc_req->u32_tx_fn = u32_sent_fn;
                stp_enc_req->u32_channeltype = (UINT32)MPAL_PDTCH;
                stp_enc_req->u32_channel_mode = (UINT32)MPAL_SIGNAL_CODE;
                stp_enc_req->u32_burst_type = (UINT32)stp_rrbp_data->u16_rrbp_burst_type;
                stp_enc_req->u16_tsqnum = u16_tsc;
                stp_enc_req->u16_tsn = 0;
                stp_enc_req->u16_burstindex = (UINT16)0;
                stp_enc_req->u16_edge_hd_type = INVALID_BYTE;
                stp_enc_req->u16_cps = INVALID_BYTE;
                stp_enc_req->u16_standby_id = u16_standby_id;
                GSM_SEND_EC_REQ(stp_enc_req,u16_standby_id);
                oal_mem_free((CONST_VOID_PTR*)&stp_enc_req);   
                /* FIX PhyFA-Req00001525 END   2013-09-29 : wuxiaorong */
                /* FIX PhyFA-Req00000215   END     2008-12-07  : ningyaojun*/
                (*u32p_msg_num)++;
                break;
            case TSNTASK_RRBP_TX0:
                /* FIX PhyFA-Req00001525 BEGIN 2013-09-29 : wuxiaorong */
                 stp_enc_req = (msg_gsm_fcp_enc_data_req_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,(UINT32) OAL_GET_TYPE_SIZE_IN_WORD(msg_gsm_fcp_enc_data_req_t));     
                /* FIX PhyFA-Req00001525 END   2013-09-29 : wuxiaorong */
                for(u16_i=0; u16_i <u16_tsn_num; u16_i++)
                {
                    stp_rrbp_data = stp_rrbp_data_list[u16_i];

                    /* FIX PhyFA-Bug00000436    BEGIN   2008-07-28  : ningyaojun*/

                    if(OAL_SUCCESS== gsm_get_pdchpara_from_gprstb(&stp_pdtch_prarms, u32_sent_fn))
                    {
                        u16_tsc = stp_pdtch_prarms->st_freq_params.tsc;
                        /* FIX LM-Bug00001262    BEGIN   2011-11-14  : linlan*/
                        u16_bc_buffer_index = stp_rrbp_data->u16_rrbp_tn_slot - stp_pdtch_prarms->u16_rtx_start_tsn;
                        /* FIX LM-Bug00001262    END   2011-11-14  : linlan*/
                    }
                    else
                    {
                        if((UINT16)1 == stp_gprs_pdtch_tb->u16_rrbp_exist_without_tbf_flag)
                        {
                            u16_tsc = stp_gsm_scell_info->u16_bcch_bsic;
                            /* FIX LM-Bug00001262    BEGIN   2011-11-14  : linlan*/
                            u16_bc_buffer_index = (stp_rrbp_data->u16_rrbp_tn_slot + (UINT16)3) % (UINT16)4;
                            /* FIX LM-Bug00001262    END   2011-11-14  : linlan*/
                        }
                        else
                        {
                            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNKNOWN_SITUATION));
                            /* FIX PHYFA-Enh00000742   BEGIN    2010-06-22  : wanghuan */
                            return OAL_FAILURE;
                            /* FIX PHYFA-Enh00000742   END    2010-06-22  : wanghuan */
                        }
                    }
                    /* FIX PhyFA-Req00000215   END     2008-12-07  : ningyaojun*/
                    u16p_enc_data_addr = (UINT16_PTR)((VOID_PTR)&stp_rrbp_data->u16_rrbp_data[0]);
                    /* FIX PhyFA-Req00000215   END     2008-12-07  : ningyaojun*/
                    stp_rrbp_data->u16_rrbp_data_state= GSM_DATA_ENCODED;
                    u16_rrbp_enc_tsn = u16_tsn_list[u16_i];
                    /*NOTE: u16_tsn_list[]  = stp_rrbp_data->u16_rrbp_tn_slot + 3;*/
                    u16_relative_tsn = u16_rrbp_enc_tsn%(UINT16)4;
                    stp_gprs_rrbp_encbuff_info = gsm_get_rrbp_encbuff_info(u16_relative_tsn);
                    
                    /* FIX PhyFA-Bug00000436    END      2008-07-28  : ningyaojun*/
                    stp_gprs_rrbp_encbuff_info->u16_data_state = GSM_DATA_ENCODED;
                    stp_gprs_rrbp_encbuff_info->u16_tsn = u16_rrbp_enc_tsn;
                    stp_gprs_rrbp_encbuff_info->u32_rrbp_fn = u32_sent_fn;
                    stp_gprs_rrbp_encbuff_info->u16_burst_type = stp_rrbp_data->u16_rrbp_burst_type;
                    /* FIX PhyFA-Enh00000105  BEGIN  2008-04-19  : huangjinfu */
                    u16_tsc = gprs_enc_update_tsc(stp_rrbp_data->u16_rrbp_burst_type,u16_tsc,u16_standby_id);
                    /* FIX PhyFA-Enh00000105  END  2008-04-19  : huangjinfu */

                    /* FIX LM-Bug00001262    BEGIN   2011-11-14  : linlan*/
                    /* FIX PhyFA-Req00001525 BEGIN 2013-09-29 : wuxiaorong */
                    stp_enc_req->u16p_InData = (UINT16_PTR)u16p_enc_data_addr;
                    stp_enc_req->u32_tx_fn = u32_sent_fn;
                    
                    stp_enc_req->u32_channeltype = (UINT32)MPAL_PDTCH;
                    stp_enc_req->u32_channel_mode = (UINT32)MPAL_SIGNAL_CODE;
                    stp_enc_req->u32_burst_type = (UINT32)stp_rrbp_data->u16_rrbp_burst_type;
                    stp_enc_req->u16_tsqnum = u16_tsc;
                    stp_enc_req->u16_tsn = u16_bc_buffer_index;

                    stp_enc_req->u16_burstindex = (UINT16)0;
                    stp_enc_req->u16_edge_hd_type = INVALID_BYTE;
                    stp_enc_req->u16_cps = INVALID_BYTE;
                    stp_enc_req->u16_standby_id = u16_standby_id;
                    GSM_SEND_EC_REQ(stp_enc_req,u16_standby_id);
                    /* FIX PhyFA-Req00001525 END   2013-09-29 : wuxiaorong */
                    /* FIX LM-Bug00001262    END   2011-11-14  : linlan*/
                    (*u32p_msg_num)++;
                    u16p_rrbp_enc_tsn_occupied = gsm_get_rrbp_enc_tsn_occupied(u16_rrbp_enc_tsn);
                    *u16p_rrbp_enc_tsn_occupied = GPRS_RRBP_TSN_OCCUPIED;
                }
                /* FIX PhyFA-Req00001525 BEGIN 2013-09-29 : wuxiaorong */
                oal_mem_free((CONST_VOID_PTR*)&stp_enc_req);   
                /* FIX PhyFA-Req00001525 END   2013-09-29 : wuxiaorong */
                break;
            default:
                oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNEXPECTED_TSNTASK));
                break;
         }
     }

    return u32_fcp_ret;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/

/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_handle_hls_res
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-11-04      zhengying               PhyFA-Req00001525   [NGM]V4 GSM代码入库
*
***********************************************************************************************************************/
    

L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
OAL_STATUS gsm_fcp_handle_hls_res(CONST_UINT16 u16_msgtype,UINT32 u32_fn,CONST_UINT32 u32_para, UINT16 u16_standby_id)
{
    OAL_STATUS fcp_ret = OAL_FAILURE;

    /* FIX PHYFA-Enh00000742   BEGIN    2010-06-22  : wanghuan */
    oal_msg_t *stp_oal_msg = NULL_PTR;
    /* FIX PHYFA-Enh00000742   END    2010-06-22  : wanghuan */
    /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/
    p_gphy_rr_grr_data_ready_ind_t *stp_grr_ready_ind;
    p_gphy_rr_ready_to_send_t *stp_ready_to_send_ind;/*SINGLE BLOCK*/   
    hls_ccl1_lapdm_ready_ind_struct *stp_lapdm_ready_ind;
    /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/      
    /* FIX PhyFA-Req00000111 BEGIN  2008-05-19  : fushilong */
    /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/
    hls_ccl1_csd_ready_ind_struct   *stp_csd_ready_ind;/* fushilong @20080418 for tch_f96 etc. */
    /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/
    /* FIX PhyFA-Req00000111 END  2008-05-19  : fushilong */
    /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/
    p_gphy_rr_rach_conf_t *stp_rch_cnf;
    p_gphy_rr_connect_ind_t * stp_connect_ind_msg;
    /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/    
    UINT32 u32_ready_fn;
    /* FIX PhyFA-Bug00000213  BEGIN  2008-03-07  : huangjinfu */
    /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/
    p_gphy_rr_connect_fail_ind_t *stp_connect_fail_ind;
    p_gphy_rr_handover_fail_ind_t *stp_handover_fail_ind;
    /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/    
    /* FIX PhyFA-Bug00000213  END  2008-03-07  : huangjinfu */

    /* FIX A2KPH-Bug00000400 BEGIN  2008-06-20  : huangjinfu */
    gprs_pdtch_para_t *stp_pdtch_prarms;
    CONST gsm_rach_tb_t *stp_gsm_rach_tb = NULL_PTR;
    CONST gprs_prach_tb_t *stp_gprs_prach_tb = NULL_PTR;
    /* FIX PhyFA-Req00001525 BEGIN 2013-11-05 : zhengying */
    CONST gsm_scell_info_t *stp_gsm_scell_info = NULL_PTR;
    /* FIX PhyFA-Req00001525 END   2013-11-05 : zhengying */
    /* FIX PhyFA-Req00001525 BEGIN 2013-11-07 : zhengying */
    CONST gsm_conn_ch_tb_t *stp_gsm_conn_ch_tb = NULL_PTR;
    CONST gsm_channel_parameters_t *stp_gsm_params_ch = NULL_PTR;
    CONST gsm_traffic_schedu_t *stp_gsm_traffic_schedu = NULL_PTR;
    UINT16_PTR  u16p_ind_counter = NULL_PTR;
    
    stp_gsm_conn_ch_tb     = gsm_get_conn_ch_tb();
    stp_gsm_params_ch      = &stp_gsm_conn_ch_tb->st_channel_params;
    stp_gsm_traffic_schedu = &stp_gsm_conn_ch_tb->st_traffic_schedu;
    /* FIX PhyFA-Req00001525 END   2013-11-07 : zhengying */
    u32_ready_fn = gsm_fn_operate(u32_fn + (UINT32)6);
    stp_gsm_scell_info = gsm_get_gsm_scell_info(u16_standby_id);
    
    switch(u16_msgtype)
    {
        case GSM_FCP_TRANS_READY_IND:
            /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/
            stp_grr_ready_ind = gsm_fcp_create_hls_msg(&stp_oal_msg,P_GPHY_RR_GRR_DATA_READY_IND,(UINT16)sizeof(p_gphy_rr_grr_data_ready_ind_t),u16_standby_id);
            /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/
            stp_grr_ready_ind->ref_count = 1;
            stp_grr_ready_ind->fn = gsm_fcp_get_next_block_fn(u32_fn);//gsm_fn_operate(u32_fn + 4);
            stp_grr_ready_ind->blks_txed = (UINT16)(u32_para &(UINT32)0x000000ff);
            /* FIX A2KPH-Bug00000400 BEGIN  2008-06-20  : huangjinfu */
            /*stp_grr_ready_ind->blks_req= (UINT16)((u32_para & 0x0000ff00) >> 8);*/
            gsm_get_pdchpara_from_gprstb(&stp_pdtch_prarms,u32_ready_fn);
            if(stp_pdtch_prarms)
            {
                //OAL_MIN(stp_pdtch_prarms->u16_ul_ts_counter,(UINT16)((u32_para&0x0000ff00)>>8),stp_grr_ready_ind->blks_req);
                stp_grr_ready_ind->blks_req = (((stp_pdtch_prarms->u16_ul_ts_counter) < ((UINT16)((u32_para&(UINT32)0x0000ff00)>>8))) ? (stp_pdtch_prarms->u16_ul_ts_counter) : ((UINT16)((u32_para&(UINT32)0x0000ff00)>>8)))&(UINT16)0xff;  
            }
            else
            {
                stp_grr_ready_ind->blks_req= 0;
            }
            /* FIX A2KPH-Bug00000400 END  2008-06-20  : huangjinfu */
            /* Fix A2KPH Bug00000367 end, guxiaobo, 20081013*/
            /* Fix A2KPH Bug00001028 end, guxiaobo, 20081028*/
 
            stp_grr_ready_ind->alloc_exhaus= 0;
            /*  FIX LM-Enh00000738    BEGIN 2012-05-30: wanghuan*/
            stp_grr_ready_ind->usf_detected = (UINT16)((u32_para & (UINT32)0x00ff0000)>>16);
            /*  FIX LM-Enh00000738    END   2012-05-30: wanghuan*/
            fcp_ret = OAL_SUCCESS;
            break;
        case GSM_FCP_SINGLEBLOCK_READY_IND:
            /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/
            stp_ready_to_send_ind = gsm_fcp_create_hls_msg(&stp_oal_msg,P_GPHY_RR_READY_TO_SEND_IND,(UINT16)sizeof(p_gphy_rr_ready_to_send_t),u16_standby_id);
            /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/
            stp_ready_to_send_ind->ref_count = 1;
            /* FIX PhyFA-Bug00000665   BEGIN   2008-08-06  : ningyaojun*/         
            stp_ready_to_send_ind->blk_txed =  (UINT16)(u32_para&(UINT32)0xff);
            stp_ready_to_send_ind->no_blks_txed = (UINT16)(u32_para&(UINT32)0xff);
            /* FIX PhyFA-Bug00000665  END      2008-08-06  : ningyaojun*/            
            stp_ready_to_send_ind->alloc_exhausted = 0;
            stp_ready_to_send_ind->tdma_frame_number = u32_ready_fn;
            fcp_ret = OAL_SUCCESS;
            break;
        case GSM_FCP_DEDI_READY_IND:
            /* FIX PhyFA-Req00000111 BEGIN  2008-05-19  : fushilong */
            if((UINT32)MPAL_TCH_F == u32_para)
            {
                /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/
                stp_csd_ready_ind = gsm_fcp_create_hls_msg(&stp_oal_msg,P_GPHY_RR_CSD_READY_IND,(UINT16)sizeof(hls_ccl1_csd_ready_ind_struct),u16_standby_id);
                /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/
                stp_csd_ready_ind->ref_count = 1;
                stp_csd_ready_ind->ch_type = MPAL_TCH_F;
                /* FIX PhyFA-Req00000767  BEGIN  2010-09-28 : songzhiyuan */
                stp_csd_ready_ind->ch_mode = (UINT16)(stp_gsm_params_ch->u16_ch_mode&(UINT16)0xff);
                /* FIX PhyFA-Req00000767  END  2010-09-28 : songzhiyuan */
                stp_csd_ready_ind->ul_slot_map = 1;
                u16p_ind_counter = gsm_get_ready_ind_counter();
                *u16p_ind_counter =( *u16p_ind_counter + (UINT16)1)%(UINT16)1024;
                if ((UINT16)42 == *u16p_ind_counter )
                {
                    *u16p_ind_counter = *u16p_ind_counter;
                }

                stp_csd_ready_ind->tn = (UINT16)(stp_gsm_traffic_schedu->u16_tch_tx_tn&(UINT16)0xff);
                stp_csd_ready_ind->fn = u32_ready_fn;
            }
            else
            {
                /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/            
                stp_lapdm_ready_ind = gsm_fcp_create_hls_msg(&stp_oal_msg,P_GPHY_RR_LAPDM_READY_IND,(UINT16)sizeof(hls_ccl1_lapdm_ready_ind_struct),u16_standby_id);
                /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/
                stp_lapdm_ready_ind->ref_count = 1;
                stp_lapdm_ready_ind->ch_type = (UINT16)(u32_para&(UINT32)0xff);
                switch(u32_para)
                {
                    case MPAL_FACCH:
                        stp_lapdm_ready_ind->tn = (UINT16)(stp_gsm_traffic_schedu->u16_tch_tx_tn&(UINT16)0xff);
                        break;
                    case MPAL_SDCCH:
                        stp_lapdm_ready_ind->tn = (UINT16)(stp_gsm_traffic_schedu->u16_sdcch_tx_tn&(UINT16)0xff);
                        break;
                    case MPAL_SACCH:
                        stp_lapdm_ready_ind->tn = (UINT16)(stp_gsm_traffic_schedu->u16_sacch_tx_tn&(UINT16)0xff);
                        break;
                    default:
                        oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNKNOWN_CHAN_TYPE));
                        break;
                }

                stp_lapdm_ready_ind->fn = u32_ready_fn;
            }
            /* FIX PhyFA-Req00000111 END  2008-05-19  : fushilong */
            fcp_ret = OAL_SUCCESS;
            break;
        case GSM_FCP_RACH_CNF:
            stp_gsm_rach_tb = gsm_get_rach_tab();            
            /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/
            stp_rch_cnf = gsm_fcp_create_hls_msg(&stp_oal_msg,P_GPHY_RMPC_RACH_CONF,(UINT16)sizeof(p_gphy_rr_rach_conf_t),u16_standby_id);
            /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/
            stp_rch_cnf->ref_count = 1;
            stp_rch_cnf->identity = stp_gsm_rach_tb->u32_trans_id;
            stp_rch_cnf->msg = stp_gsm_rach_tb->u16_msg;
            stp_rch_cnf->tdma_frame_number = u32_fn;
            if(OAL_SUCCESS == u32_para)
            {
                stp_rch_cnf->status = 1;
            }
            else
            {
                stp_rch_cnf->status = 0;
            }

            fcp_ret = OAL_SUCCESS;
            break;
        case GSM_FCP_PRACH_CNF:
            stp_gprs_prach_tb = gsm_get_prach_tb();
            /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/          
            stp_rch_cnf = gsm_fcp_create_hls_msg(&stp_oal_msg,P_GPHY_RMPC_RACH_CONF,(UINT16)sizeof(p_gphy_rr_rach_conf_t),u16_standby_id);
            /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/

            stp_rch_cnf->ref_count = 1;
            stp_rch_cnf->identity = stp_gprs_prach_tb->u32_primitive_identity;
            stp_rch_cnf->msg = stp_gprs_prach_tb->u16_prach_msg;    
            stp_rch_cnf->tdma_frame_number = u32_fn;
            if(OAL_SUCCESS == u32_para)
            {
                stp_rch_cnf->status = 1;
            }
            else
            {
                stp_rch_cnf->status = 0;
            }

            fcp_ret = OAL_SUCCESS;
            break;
         case GSM_FCP_RR_CONN_IND:
            /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/
            stp_connect_ind_msg = gsm_fcp_create_hls_msg(&stp_oal_msg,P_GPHY_RR_CONNECT_IND,(UINT16)sizeof(p_gphy_rr_connect_ind_t),u16_standby_id);
            /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/
            stp_connect_ind_msg->ref_count = 1;
            stp_connect_ind_msg->status = 1;
            stp_connect_ind_msg->identity = u32_para;
            stp_connect_ind_msg->ts_frame_no.tdma_frame_number= u32_fn;
            stp_connect_ind_msg->ts_frame_no.time_slot = 0;
            /* FIX LM-Bug00000551 BEGIN  2011-5-18  : wanghuan */
            /*report the simple c_value caculated by the latest BCCH RSSI to HLS*/
            stp_connect_ind_msg->filloctet2_1[0] = map_signal_level_to_an_rxlev(stp_gsm_scell_info->s16_serv_bcch_rssi_latest);
            /* FIX LM-Bug00000551 END  2011-5-18  : wanghuan */
            fcp_ret = OAL_SUCCESS;
            break;
        /* FIX PhyFA-Bug00000213  BEGIN  2008-03-07  : huangjinfu */
         case GSM_FCP_CONN_FAIL:
            /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/            
            stp_connect_fail_ind = gsm_fcp_create_hls_msg(&stp_oal_msg,P_GPHY_RR_CONN_FAIL_IND,(UINT16)sizeof(p_gphy_rr_connect_fail_ind_t),u16_standby_id);
            /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/
            stp_connect_fail_ind->ref_count = 1;
            stp_connect_fail_ind->arfcn = (UINT16)u32_para;
            stp_connect_fail_ind->ch_type = MPAL_BCCH;
            stp_connect_fail_ind->fail_ind = 0;
            fcp_ret = OAL_SUCCESS;
            break;
         case GSM_FCP_HANDOVER_FAIL:
            /* FIX PhyFA-Req00000215   BEGIN   2008-12-17  : ningyaojun*/            
            stp_handover_fail_ind = gsm_fcp_create_hls_msg(&stp_oal_msg,P_GPHY_RR_HANDOVER_FAIL_IND,(UINT16)sizeof(p_gphy_rr_handover_fail_ind_t),u16_standby_id);
            /* FIX PhyFA-Req00000215   END     2008-12-17  : ningyaojun*/
            stp_handover_fail_ind->ref_count = 1;
            stp_handover_fail_ind->cause = INVALID_HO_FAIL_TYPE;
            fcp_ret = OAL_SUCCESS;
            break;
        /* FIX PhyFA-Bug00000213  END  2008-03-07  : huangjinfu */
        /* FIX LM-Bug00000775  BEGIN  2011-09-16  : wanghuan */
        case GSM_FCP_HANDOVER_CNF:
             gsm_fcp_create_hls_msg(&stp_oal_msg,(UINT32)P_GPHY_RR_HANDOVER_CNF,(UINT16)0,u16_standby_id);/*(21446)*/
             fcp_ret = OAL_SUCCESS;
             break;
        /* FIX LM-Bug00000775  END    2011-09-16  : wanghuan */   
        /* FIX LM-Enh00000435  BEGIN  2012-02-01  : sunzhiqiang */
        case GSM_FCP_PCH_CONFLICT_IND:
             gsm_fcp_create_hls_msg(&stp_oal_msg,(UINT32)P_GPHY_RR_CONFLICT_IND,(UINT16)0,u16_standby_id);/*(79863)*/
             fcp_ret = OAL_SUCCESS;
            break;
        /* FIX LM-Enh00000435  END  2012-02-01  : sunzhiqiang */
         default:
            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
            break;
    }

    if(OAL_SUCCESS == fcp_ret)
    {
         oal_msg_send(stp_oal_msg,(UINT16)L1CC_GSM_L1RESP_TASK_ID);/* 发送给hls的消息，先发给l1resp,然后转发给高层*/
    }

    /* FIX PhyFA-Req00000510   BEGIN     2010-02-26  : kehaidong*/
    /* FIX PhyFA-Req00000510   END     2010-02-26  : kehaidong*/
    return fcp_ret;
}


   


/* FIX PhyFA-Req00001525 BEGIN  2013-11-18 : wuxiaorong */
/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_parse_rtx_event
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return Parameters: <name1>        <description1>
* Chage Histroy
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2014-05-06      gaowu                   PhyFA-Enh00001876   [NGM]GSM配置superfn时，需要将值置为0 
*2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS  gsm_fcp_parse_rtx_event(IN CONST gsm_download_rtx_event_info_t* CONST stp_download_rtx_event_info,
                                           OUT l1cc2dd_event_info_t *stp_l1cc2dd_event_info)
{
    CONST gsm_main_frametask_t *stp_current_parse_frame_task;  /* the frame task parsed now */
    l1cc2dd_event_info_t        *stp_l1cc2dd_event_info_prev;               /* event info table generate preview */
    CONST l1cc2dd_event_info_t  *stp_event_table_start_addr;
    UINT32                     u32_event_sfn;
    SINT32                     s32_i;                                                               /* Loop counter */
    UINT16                     u16_slot_no;                                                    /* the slot no parsed now */
    CONST gsm_slot_rtx_t       *stp_slot_rtx =  NULL_PTR;
    l1cc2dd_gsm_event_t         *stp_gsm_event_info =  NULL_PTR;
    
    UINT16                     u16_slot_event_flag = NO_RTX_FLAG;
    UINT16                     u16_event_type;
    UINT16_PTR                 u16p_current_parse_bitmap;                     /* u16_ts_bitmap parsed now */
    UINT32                     u32_fc_ping_pang_index;
    SINT16                     s16_rtx_curr_offset;
    SINT16                     s16_rtx_old_offset = 0;
    UINT16                     u16_ts_bitmap_temp[2];
    BOOLEAN                    b_valid_event_flag = OAL_FALSE;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    
    OAL_ASSERT(NULL_PTR != stp_download_rtx_event_info, "The pointer is null(gsm_fcp_parse_rtx_event)!");
    OAL_ASSERT(NULL_PTR != stp_l1cc2dd_event_info, "The pointer is null(gsm_fcp_parse_rtx_event)!");

    stp_l1cc2dd_event_info_prev = stp_l1cc2dd_event_info;
    stp_event_table_start_addr = stp_l1cc2dd_event_info;
    u16_ts_bitmap_temp[0] = *(stp_download_rtx_event_info->u16p_next_fn_bitmap);
    u16_ts_bitmap_temp[1] = *(stp_download_rtx_event_info->u16p_next2_fn_bitmap);

    stp_gsm_event_info =  &(stp_l1cc2dd_event_info->u_event_info.st_gsm_info);
    stp_current_parse_frame_task = (gsm_main_frametask_t *)stp_download_rtx_event_info->stp_next_fn_frame_task; /* from the first frame */
    u16p_current_parse_bitmap = &u16_ts_bitmap_temp[0];
    for (s32_i = (SINT32)0; s32_i < (SINT32)MAX_PARSE_GSM_SLOT_NUM; s32_i++)
    {
        if (((UINT16)0 == u16_ts_bitmap_temp[0]) && ((UINT16)0 == u16_ts_bitmap_temp[1]))
        {
            break;
        }

        if (s32_i < (SINT32)MAX_GSM_SLOT_NUM)   /* parse the first faram task */
        {
            u16_slot_no = (UINT16)s32_i;
        }
        else  /* parse the second faram task */
        {
            stp_current_parse_frame_task = (gsm_main_frametask_t *)stp_download_rtx_event_info->stp_next2_fn_frame_task;
            u16_slot_no = (UINT16)s32_i - (UINT16)MAX_GSM_SLOT_NUM;
            u16p_current_parse_bitmap = &u16_ts_bitmap_temp[1];
        }

        if ((UINT16)0 != ((*u16p_current_parse_bitmap >> u16_slot_no) & (UINT16)0x1))  /* If this slot need parse */
        {
            *u16p_current_parse_bitmap &= ~(UINT16)((UINT16)1 << u16_slot_no);
            stp_slot_rtx = &(stp_current_parse_frame_task->st_slot_rtx[u16_slot_no]);
            u32_event_sfn = stp_current_parse_frame_task->u32_fn;
            u16_event_type = (UINT16) stp_slot_rtx->u16_event_type;
            switch (u16_event_type)
            {
                case RX_GSM_NSLOT:
                {
                    s16_rtx_curr_offset = stp_slot_rtx->s16_rtx_offset;
                    /* IF not continue rx event,
                       Or  continue rx event but  (uarfcn ||offset) has changed.
                       When uarfcn or offset changed, it should be a new event.
                    */
                    if (((UINT16)CONTINUE_RX_GSM_FLAG != u16_slot_event_flag)
                        ||((stp_gsm_event_info->u16_uarfcn != stp_slot_rtx->u16_arfcn) ||(s16_rtx_curr_offset != s16_rtx_old_offset)))
                    {
                        u16_slot_event_flag = CONTINUE_RX_GSM_FLAG;
                        stp_l1cc2dd_event_info->u16_event_type = u16_event_type;
                        stp_l1cc2dd_event_info->u16_rtx_status = GSM_RX_DATA;
                        stp_l1cc2dd_event_info->u16_active_id = u16_active_id;
                        stp_l1cc2dd_event_info->u16_standby_id = stp_slot_rtx->u16_standby_id;
                        stp_l1cc2dd_event_info->u16_mode_timing = GSM_TIMING;
                        
                        stp_gsm_event_info = &(stp_l1cc2dd_event_info->u_event_info.st_gsm_info);
                        stp_gsm_event_info->u16_rxtask_type       = stp_slot_rtx->u16_rxtask_type;
                        stp_gsm_event_info->u16_uarfcn            = stp_slot_rtx->u16_arfcn;
                     
                        //stp_gsm_event_info->u16_standby_id            = stp_slot_rtx->u16_standby_id;
                        /* FIX PhyFA-Req00001425 BEGIN  2012-12-28 : Yangliangliang */
                        stp_gsm_event_info->u16_ch_type           = (UINT16)stp_slot_rtx->u16_ch_type;
                        /* FIX PhyFA-Req00001425 END  2012-12-28 : Yangliangliang */
                        stp_gsm_event_info->u16_start_tsn         = u16_slot_no;
                        stp_gsm_event_info->st_fn_offset_info.u16_superfn = 0;/*superfn必须为0*/
                        stp_gsm_event_info->st_fn_offset_info.u32_subfn = u32_event_sfn;
                        /* FIX Enh00001894   BEGIN     2014-05-12  : xiongjiangjiang */
                        stp_gsm_event_info->st_fn_offset_info.s32_offset = ((SINT32)GSM_QBITS_PER_TS * GSM_MOD_2POWER(SINT32, u16_slot_no, MAX_GSM_SLOT_NUM)) + (SINT32)s16_rtx_curr_offset;  /**/
                        /* FIX Enh00001894   END     2014-05-12  : xiongjiangjiang */
                        s16_rtx_old_offset = s16_rtx_curr_offset;

                        u32_fc_ping_pang_index = u32_event_sfn % (UINT32)AGC_APC_BUF_LEN;
                        stp_gsm_event_info->vp_fc_para = (VOID_PTR)&(g_u32p_gsm_agc_addr[u16_active_id][u32_fc_ping_pang_index][u16_slot_no]);      /*point to agc address*/
                        stp_gsm_event_info->u16_ts_num = 1;
                        stp_gsm_event_info->u16_fc_num = 1;                                /*fc no in multi-slot event*/
                    }
                    else
                    {
                        stp_l1cc2dd_event_info = stp_l1cc2dd_event_info_prev;  /* continue event, use the event table preview */
                        stp_gsm_event_info->u16_ts_num++;    /*slot num in a event*/
                        stp_gsm_event_info->u16_fc_num++;    /* fc num */
                    }
                    break;
                }

                case RX_GSM_SYNC_NSLOT:
                case RX_GSM_NBIT:
                {
                    stp_l1cc2dd_event_info->u16_event_type = u16_event_type;
                    stp_l1cc2dd_event_info->u16_rtx_status = GSM_RX_DATA;
                    stp_l1cc2dd_event_info->u16_active_id = u16_active_id;
                    stp_l1cc2dd_event_info->u16_standby_id = stp_slot_rtx->u16_standby_id;
                    stp_l1cc2dd_event_info->u16_mode_timing = GSM_TIMING;
                    stp_gsm_event_info = &(stp_l1cc2dd_event_info->u_event_info.st_gsm_info);
                    stp_gsm_event_info->u16_rxtask_type       = stp_slot_rtx->u16_rxtask_type;
                    stp_gsm_event_info->u16_start_tsn         = u16_slot_no;
                    
                    /* FIX PhyFA-Req00001425 BEGIN  2012-12-28 : Yangliangliang */
                    stp_gsm_event_info->u16_ch_type           = (UINT16)stp_slot_rtx->u16_ch_type;
                    /* FIX PhyFA-Req00001425 END  2012-12-28 : Yangliangliang */
                    stp_gsm_event_info->u16_uarfcn            = stp_slot_rtx->u16_arfcn;
                    stp_gsm_event_info->st_fn_offset_info.s32_offset = ((SINT32)GSM_QBITS_PER_TS * (SINT32)u16_slot_no) + (SINT32)stp_slot_rtx->s16_rtx_offset;  /**/
                    stp_gsm_event_info->st_fn_offset_info.u16_superfn = 0;/*superfn必须为0*/
                    stp_gsm_event_info->st_fn_offset_info.u32_subfn  = stp_current_parse_frame_task->u32_fn;
                    u32_fc_ping_pang_index = stp_gsm_event_info->st_fn_offset_info.u32_subfn % (UINT32)AGC_APC_BUF_LEN;
                    stp_gsm_event_info->vp_fc_para            = (VOID_PTR)&(g_u32p_gsm_agc_addr[u16_active_id][u32_fc_ping_pang_index][u16_slot_no]);     /*agc address*/

                    if ((UINT16)RX_GSM_NBIT == u16_event_type)
                    {
                        u16_slot_event_flag = NORMAL_EVENT_FLAG;
                        stp_gsm_event_info->u16_para_len = stp_slot_rtx->u16_rx_len_ind;
                    }
                    else//???
                    {
                        u16_slot_event_flag = SPECIAL_EVENT_FLAG;
                        stp_gsm_event_info->u16_para_len = stp_slot_rtx->u16_rx_len_ind;
                    }
                    stp_gsm_event_info->u16_ts_num = 1;                             /*slot no in a event*/
                    stp_gsm_event_info->u16_fc_num = 1;                               /*fc no*/
                    break;
                }

                case TX_GSM_AB:
                case TX_GSM_GMSK_NB:
                case TX_GSM_8PSK_NB:
                {
                    if ((UINT16)CONTINUE_TX_GSM_FLAG != u16_slot_event_flag)
                    {
                        u16_slot_event_flag = CONTINUE_TX_GSM_FLAG;
                        stp_l1cc2dd_event_info->u16_event_type = TX_GSM_NSLOT;
                        stp_l1cc2dd_event_info->u16_rtx_status = GSM_TX_DATA;
                        stp_l1cc2dd_event_info->u16_active_id = u16_active_id;
                        stp_l1cc2dd_event_info->u16_standby_id = stp_slot_rtx->u16_standby_id;
                        stp_l1cc2dd_event_info->u16_mode_timing = GSM_TIMING;
                        stp_gsm_event_info = &(stp_l1cc2dd_event_info->u_event_info.st_gsm_info);
                        stp_gsm_event_info->u16_rxtask_type       = stp_slot_rtx->u16_rxtask_type;   
                      
                        /* FIX PhyFA-Req00001425 BEGIN  2012-12-28 : Yangliangliang */
                        stp_gsm_event_info->u16_ch_type           = (UINT16)stp_slot_rtx->u16_ch_type;
                        /* FIX PhyFA-Req00001425 END  2012-12-28 : Yangliangliang */
                        stp_gsm_event_info->u16_start_tsn         = u16_slot_no;
                        stp_gsm_event_info->u16_uarfcn            = stp_slot_rtx->u16_arfcn;
                        stp_gsm_event_info->st_fn_offset_info.u16_superfn = 0;/*superfn必须为0*/
                        stp_gsm_event_info->st_fn_offset_info.u32_subfn  = u32_event_sfn;
                        /* FIX Enh00001894   BEGIN     2014-05-12  : xiongjiangjiang */
                        stp_gsm_event_info->st_fn_offset_info.s32_offset = ((SINT32)GSM_QBITS_PER_TS * GSM_MOD_2POWER(SINT32, u16_slot_no, MAX_GSM_SLOT_NUM)) + (SINT32)stp_slot_rtx->s16_rtx_offset;  /**/
                        /* FIX Enh00001894   END     2014-05-12  : xiongjiangjiang */
                        /* FIX PhyFA-Req00001004  BEGIN  2011-08-08 : dongli */
                        stp_gsm_event_info->u16_ab_pos            = (UINT16)0;
                        stp_gsm_event_info->u16_modulation_bitmap = (UINT16)0;
                        u32_fc_ping_pang_index = u32_event_sfn % (UINT32)AGC_APC_BUF_LEN;
                        stp_gsm_event_info->vp_fc_para = &(g_s16_gsm_apc[u16_active_id][u32_fc_ping_pang_index][u16_slot_no]);   /*apc address*/
                        /* FIX PhyFA-Req00001004  END  2011-08-08 : dongli */
                        stp_gsm_event_info->u16p_gsm_data_txbuf = stp_slot_rtx->u16p_tx_addr;  /*this address point to the head of tx buffer include zero bit*/
                        stp_gsm_event_info->u16_ts_num = 1;
                        stp_gsm_event_info->u16_fc_num = 1;                                /*fc no in multi-slot event*/
                    }
                    else
                    {
                        stp_l1cc2dd_event_info = stp_l1cc2dd_event_info_prev;  /* continue event, use the event table preview */
                        stp_gsm_event_info->u16_ts_num++;    /*slot num in a event*/
                        stp_gsm_event_info->u16_fc_num++;    /* fc num */
                    }

                    if ((UINT16)TX_GSM_AB == u16_event_type)
                    {
                        stp_gsm_event_info->u16_ab_pos = stp_gsm_event_info->u16_ts_num;
                    }
                    else if ((UINT16)TX_GSM_8PSK_NB == u16_event_type)
                    {
                        stp_gsm_event_info->u16_modulation_bitmap = stp_gsm_event_info->u16_modulation_bitmap | ((UINT16)MODULATE_8PSK << (stp_gsm_event_info->u16_ts_num - (UINT16)1));
                    }
                    break;
                }


                default:
                {
                    u16_slot_event_flag = NO_RTX_FLAG;
                    break;
                }
            } /* END SWITCH*/

            if ((UINT16)NO_RTX_FLAG != u16_slot_event_flag)
            {
                b_valid_event_flag = OAL_TRUE;
                stp_l1cc2dd_event_info_prev = stp_l1cc2dd_event_info++;
                stp_l1cc2dd_event_info_prev->stp_event_next = stp_l1cc2dd_event_info;
            }
        }
        else
        {
            u16_slot_event_flag = NO_RTX_FLAG;
        } /* END IF  (u16_current_parse_bitmap >> u16_slot_no) & 0x1)*/
    } /* END FOR */

    if (OAL_TRUE == b_valid_event_flag)
    {
        /* check if the event table is over write */
        if (stp_l1cc2dd_event_info > (stp_event_table_start_addr + (UINT32)EVENT_INFO_TABLE_NUM))
        {
            oal_error_handler((UINT16)OAL_FATAL_ERROR, OAL_HWIF_EVENT_INFO_TABLE_NUM_OVERFLOW);
            return OAL_HWIF_EVENT_INFO_TABLE_NUM_OVERFLOW;
        }

        stp_l1cc2dd_event_info_prev->stp_event_next = NULL_PTR;   /* the end of event table list */
        return OAL_SUCCESS;
    }
    else
    {
        return OAL_FAILURE;
    }  
}

/***********************************************************************************************************************
* FUNCTION:          gsm_rtx_event_req
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-05      wuxiaorong              PhyFA-Enh00001877   [NGM]gsm_rtx_event_req局部结构体数组变量改成动态申请和释放
*   2014-05-09      wuxiaorong              PhyFA-Enh00001883    [NGM]dd_event_req返回OAL_ FAILURE的下载事件来不及配置修改
*   2014-09-22      gaowu                   NGM-Bug00000586     [NGM]l1cc_gsm需要将伪事件的配置按照非周期事件来配置
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
OAL_STATUS gsm_rtx_event_req(IN CONST_VOID_PTR_CONST vp_event_info0,
                             IN CONST_VOID_PTR_CONST vp_event_info1,
                             IN CONST_UINT16_PTR_CONST u16p_bitmap0, IN CONST_UINT16_PTR_CONST u16p_bitmap1)
{
    OAL_STATUS  oal_status_ret = OAL_FAILURE;

    
    l1cc2dd_event_info_t *stp_l1cc2dd_event_info = NULL_PTR;
    event_req_status_t    st_event_req_status;
    gsm_download_rtx_event_info_t st_download_rtx_event_info;
    /* FIX PhyFA-Bug00004592 BEGIN 2014-05-12 : wuxiaorong */
    UINT16  i;
    UINT16  u16_del_standby_id;
    UINT16  u16_del_start_tsn;
    UINT32  u32_del_subfn;
    OAL_STATUS  u32_ret = OAL_FAILURE;
    /* FIX PhyFA-Bug00004592 END   2014-05-12 : wuxiaorong */

    st_download_rtx_event_info.stp_next_fn_frame_task = (gsm_main_frametask_t *)vp_event_info0;
    st_download_rtx_event_info.stp_next2_fn_frame_task= (gsm_main_frametask_t *)vp_event_info1;
    st_download_rtx_event_info.u16p_next_fn_bitmap = (UINT16_PTR)u16p_bitmap0;
    st_download_rtx_event_info.u16p_next2_fn_bitmap = (UINT16_PTR)u16p_bitmap1; 


    stp_l1cc2dd_event_info = (l1cc2dd_event_info_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL, (UINT32)((UINT16)EVENT_INFO_TABLE_NUM*OAL_GET_TYPE_SIZE_IN_WORD(l1cc2dd_event_info_t)));     

     
    oal_status_ret = gsm_fcp_parse_rtx_event(&st_download_rtx_event_info, stp_l1cc2dd_event_info);
    if (OAL_SUCCESS == oal_status_ret)
    {

        oal_status_ret = dd_event_req(stp_l1cc2dd_event_info,&st_event_req_status);
        /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
        u32_ret = oal_tpc_filter_msg(MSG_GSM_FCP_PSD_FRAMETASK_INFO,(UINT16)UNSPECIFIED_STANDBY_ID,(UINT16)GSM_MODE);

        if (OAL_SUCCESS == u32_ret)
        {
            oal_tpc_trace_data(MSG_GSM_FCP_PSD_FRAMETASK_INFO,
                                (UINT16_PTR)vp_event_info0,
                                (UINT16)OAL_GET_TYPE_SIZE_IN_WORD(gsm_main_frametask_t),
                                gsm_get_gsm_fn(OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID),
                                (UINT16)UNSPECIFIED_STANDBY_ID,
                                (UINT16)GSM_MODE);
        }
        /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
        if (OAL_FAILURE == oal_status_ret)/*事件下载过期，可以导出来看 */
        {
            /* FIX PhyFA-Enh00001883 END  2014-05-08 : wuxiaorong */
            /* 初期必须ASSERT下来分析 */
            OAL_ASSERT(st_event_req_status.u16_late_event_cnt == 0,"fcp event download late!");
            for(i =0;i< st_event_req_status.u16_late_event_cnt;i++)
            {
                u16_del_standby_id = st_event_req_status.stp_late_event_info[i]->u16_standby_id;
                u16_del_start_tsn = st_event_req_status.stp_late_event_info[i]->u_event_info.st_gsm_info.u16_start_tsn;
                u32_del_subfn  =  st_event_req_status.stp_late_event_info[i]->u_event_info.st_gsm_info.st_fn_offset_info.u32_subfn;
                gsm_fcp_del_tsntask(u32_del_subfn,(SINT16)u16_del_start_tsn, u16_del_standby_id);
            }
            /* FIX PhyFA-Enh00001883 END  2014-05-08 : wuxiaorong */
        }
    }
    /* 释放申请的内存 */
    oal_mem_free((CONST_VOID_PTR *)&stp_l1cc2dd_event_info);

    return oal_status_ret;
}
/* FIX PhyFA-Req00001525 END  2013-11-18 : wuxiaorong */



/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
L1CC_GSM_DRAM_CODE_SECTION
OAL_STATUS gsm_fcp_get_undownloaded_slot_range(UINT32 u32_fn, CONST_UINT16 u16_end_slot, UINT16_PTR u16p_bitmap0, UINT16_PTR u16p_bitmap1)
{
    UINT16 u16_i;
    /*FIX PhyFA-Req00001045  BEGIN   2011-09-09  : ningyaojun*/
    gsm_main_frametask_t*   stp_gsm_main_frametask_tmp0 = NULL_PTR;
    CONST gsm_main_frametask_t*   stp_gsm_main_frametask_tmp1 = NULL_PTR;
    /*FIX PhyFA-Req00001045  END     2011-09-09  : ningyaojun*/ 
    gsm_slot_rtx_t*         stp_gsm_slot_rtx        = NULL_PTR;
    UINT16                  u16_previous_tstask     = TSNTASK_NULL;
    UINT16                  u16_previous_slot       = GSM_TSNTASK_TSN_INVALID;
    /*FIX PhyFA-Req00001045  BEGIN   2011-09-09  : ningyaojun*/
    UINT16                  u16_rtx_bit             = 0x0000;
    /*FIX PhyFA-Req00001045  END     2011-09-09  : ningyaojun*/ 
    UINT16                  u16_bitmap0             = 0x0000;
    UINT16                  u16_bitmap1             = 0x0000;
    OAL_STATUS              u32_ret= OAL_FAILURE;
    /*FIX PhyFA-Req00001305  BEGIN   2012-07-02  : wuxiaorong*/
    #if defined(_LC1761_) && !defined(L2000_L1CC_CLOSE_SLEEP)
    UINT16 u16_rtx_status = 0;
    #endif
    /*FIX PhyFA-Req00001305  END   2012-07-02  : wuxiaorong*/
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    OAL_ASSERT(NULL_PTR!=u16p_bitmap0, "gsm_fcp_get_undownloaded_slot_range null pointer");
    OAL_ASSERT(NULL_PTR!=u16p_bitmap1, "gsm_fcp_get_undownloaded_slot_range null pointer");
    for(u16_i=0; u16_i<=u16_end_slot; u16_i++)
    {
        stp_gsm_slot_rtx = gsm_get_tsntask(u32_fn, (SINT16)u16_i);
        if(GSM_TSNTASK_NOT_EXPIRED == stp_gsm_slot_rtx->u16_expired)
        {
            /*FIX PhyFA-Bug00002713  BEGIN   2011-07-21  : ningyaojun*/
            OAL_ASSERT((TSNTASK_NULL != stp_gsm_slot_rtx->u16_tstask),"gsm_fcp_get_undownloaded_slot_range():u16_tstask is TSNTASK_NULL!\n");                
            /*FIX PhyFA-Bug00002713  END     2011-07-21  : ningyaojun*/
            
            if((OAL_FALSE == gsm_check_irat_rx_pseudo_task(stp_gsm_slot_rtx->u16_tstask))
                && ((UINT16)GSM_TSNTASK_NOT_DOWNLOADED == stp_gsm_slot_rtx->u16_downloaded_ind))
            {
                if(u16_i <= (UINT16)7)
                {
                    /*FIX PhyFA-Req00001045  BEGIN   2011-09-09  : ningyaojun*/
                    stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_fn,u16_active_id);

                    u16_rtx_bit = ((UINT16)1<<u16_i);
                    if(u16_rtx_bit & (gsm_get_frame_task_rtx_bitmap(stp_gsm_main_frametask_tmp0))) /*the psudo task doesn't set the rx_bitmap nor tx_bitmpa*/
                    {
                        u16_bitmap0 |=  u16_rtx_bit;                      
                        u16_previous_slot = u16_i;
                        u16_previous_tstask = stp_gsm_slot_rtx->u16_tstask;  
                        /*important: u16_downloaded_ind must be set in slot! */
                        stp_gsm_slot_rtx->u16_downloaded_ind = GSM_TSNTASK_DOWNLOADED;

                        /*important: u16_downloaded_ind must be set in frame! */
                        stp_gsm_main_frametask_tmp0->u16_downloaded_ind = GSM_TSNTASK_DOWNLOADED;

                    }
                    /*FIX PhyFA-Req00001045  END     2011-09-09  : ningyaojun*/ 
                }
                else/*(u16_i >= 8, i.e. u32_fn+1)*/
                {
                    /*judge whether there are events in u32_fn+1[ts0~ts3] that shall can be combined into one NSLOTS event with u32_fn[ts7]*/
                    if((OAL_SUCCESS == gsm_fcp_tsntask_joint(u16_previous_tstask,stp_gsm_slot_rtx->u16_tstask))
                       &&(u16_previous_slot != (UINT16)GSM_TSNTASK_TSN_INVALID)
                       &&((u16_previous_slot+(UINT16)1) == u16_i)
                      )
                    {               
                        /*FIX PhyFA-Req00001045  BEGIN   2011-09-09  : ningyaojun*/
                        stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_fn+(UINT32)1),u16_active_id);

                        u16_rtx_bit = (UINT16)((UINT16)1<<(u16_i-(UINT16)8));
                        if(u16_rtx_bit & (gsm_get_frame_task_rtx_bitmap(stp_gsm_main_frametask_tmp1)))
                        {
                            u16_bitmap1 |=  u16_rtx_bit;        
                            u16_previous_slot = u16_i;
                            u16_previous_tstask = stp_gsm_slot_rtx->u16_tstask;
                            /*important: u16_downloaded_ind must be changed here! */
                            stp_gsm_slot_rtx->u16_downloaded_ind = GSM_TSNTASK_DOWNLOADED; 

                        }
                        /*FIX PhyFA-Req00001045  END     2011-09-09  : ningyaojun*/ 
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }

    if((UINT16)0x0000 != u16_bitmap0)
    {
        *u16p_bitmap0 = u16_bitmap0;
        *u16p_bitmap1 = u16_bitmap1;        
        u32_ret= OAL_SUCCESS;
    }
    return u32_ret;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/


/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_dedicated
* DESCRIPTION:       the handler of the state GSM_DEDICATED of the module FCP
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  stp_tsn_prios[10]: priority info of  [tsn7|tsn0|tsn1|tsn2|tsn3|tsn4|tsn5|tsn6|tsn7|tsn0]
      u32_next_fn:  the next frame number
      stp_oal_msg:        message received from other module
* Output Parameters: NULL
* Return value:       OAL_SUCCESS or OAL_FAILURE
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2013-11-05      zhengying               PhyFA-Req00001525   [NGM]V4 GSM代码入库
*   2014-05-14      wuxiaorong              PhyFA-Bug00004726   [NGM]GSM的SDCCH切换TCH失败回切SDCCH后上行编码交织buffer的index错误
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*   2014-06-18      linlan                  PhyFA-Bug00005064   [NGM]dedicate状态根据搜网配置表上报冲突指示 
*   2014-08-07      wuxiaorong              PhyFA-Bug00005535   [NGM]GSM邻区fcb search在业务下启动了但未完成转状态idle时需要清对应的u16_start_bitmap
*   2014-09-22      gaowu                   NGM-Bug00000586     [NGM]l1cc_gsm需要将伪事件的配置按照非周期事件来配置
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
STATIC OAL_STATUS gsm_fcp_dedicated(CONST_UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos)
{
    UINT16 u16_ready_ch_type;
    UINT16 u16_curr_ch_type;
    /* FIX PHYFA-Enh00000742   BEGIN    2010-06-22  : wanghuan */
    UINT16 u16_curr_ch_mode = MPAL_INVALID_CHANNEL_MODE;
    /* FIX PHYFA-Enh00000742   END    2010-06-22  : wanghuan */
    CONST_UINT32 u32_curr_fn = gsm_fn_operate((u32_next_fn+(UINT32)GSM_MAX_FN)-(UINT32)1);

    gsm_tb_pos_t st_tb_pos_info = {0, 0, NULL_PTR, NULL_PTR};

    gsm_channel_parameters_t   *stp_channel_params;
    /* FIX LM-Bug00002651  BEGIN  2013-03-06  : sunzhiqiang */
    OAL_STATUS oal_status_conflict_ret = OAL_SUCCESS;
    /* FIX LM-Bug00002651  END  2013-03-06  : sunzhiqiang */
    /*FIX LM-Req00000372  BEGIN   2011-09-23  : linlan*/ 
    UINT16 u16_sim_var;
    UINT16 k;
    /*FIX LM-Req00000372  END   2011-09-23  : linlan*/
    /* FIX PhyFA-Bug00000642 BEGIN 2008-08-05 : ChenLei */
    CONST time_info_struct *stp_time_info = NULL_PTR;
    /* FIX PhyFA-Bug00000642 END 2008-08-05 : ChenLei */
    
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    UINT16  u16_standby_id = UNSPECIFIED_STANDBY_ID;        
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp0 = NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp1 = NULL_PTR;    
    UINT16  u16_ts_bitmap[2] = {0,0}; 
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/ 
    mmc_time_info_t st_mmc_time_info;
    gsm_ccch_tb_t* stp_temp_ccch_tlb = NULL_PTR;
    gsm_nc_meas_sync_ctrl_t*  stp_nc_meas_sync_ctrl = NULL_PTR;
    gsm_nc_balist_t*  stp_nc_balist =NULL_PTR;  
    gsm_sc_bcch_tb_t *stp_gsm_sc_bcch_tb = NULL_PTR;
    /* FIX PhyFA-Req00001525 BEGIN 2013-11-07 : zhengying */
    CONST gsm_scell_info_t *stp_gsm_scell_info = NULL_PTR;
    gsm_conn_ho_tb_t *stp_gsm_conn_ho_tb = NULL_PTR;
    CONST gsm_conn_ho_params_t *stp_gsm_conn_ho_params = NULL_PTR; 
    gsm_conn_ch_tb_t *stp_gsm_conn_ch_tb = NULL_PTR;
    gsm_channel_parameters_t *stp_gsm_params_ch = NULL_PTR;
    CONST gsm_traffic_schedu_t *stp_gsm_traffic_schedu = NULL_PTR;
    time_info_struct *stp_time_info_ho = NULL_PTR;
    /* FIX PhyFA-Req00001525 END   2013-11-07 : zhengying */
    CONST gsm_nc_bcch_tb_t    *stp_gsm_nc_bcch_tbl = NULL_PTR;
    #ifdef PECKER_SWITCH 
    CONST gsm_ded_ch_report_ind_tb_t* stp_gsm_ded_ch_report_ind = NULL_PTR;
    #endif
    gsm_tsn_info_t st_tsn_info;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    UINT16 u16_nc_num;
    UINT16 m;
    CONST_UINT16 u16_tstask_nc_rx_tb[GSM_CSIDLE_MAX_TSNTASK_NC_NUM] =
    {
      TSNTASK_NC0_BCCH_RX,
      TSNTASK_NC1_BCCH_RX,
      TSNTASK_NC2_BCCH_RX,
      TSNTASK_NC3_BCCH_RX,
      TSNTASK_NC4_BCCH_RX,
      TSNTASK_NC5_BCCH_RX,
      TSNTASK_NC6_BCCH_RX
    };

    /* FIX LM-Enh00001334 BEGIN  2013-10-21  : wangjunli */
    UINT32 u32_ready_fn = (UINT32)GSM_INVALID_FN;
    /* FIX LM-Enh00001334 END  2013-10-21  : wangjunli */  
    /* FIX LTE-Enh00000814 BEGIN  2014-04-11  : wangjunli */
    UINT32 u32_report_fn = (UINT32)GSM_INVALID_FN;
    /* FIX LTE-Enh00000814 END  2014-04-11  : wangjunli */
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR;
    CONST gsm_normal_search_ctrl_t  *stp_normal_search;
    CONST gsm_cellsearch_t  *stp_cellsearch_tbl;
    CONST gsm_cellsearch_ctrl_t * stp_cellsearch_ctrl  = NULL_PTR;
    UINT16 u16_cellsearch_standby_id = 0;
    
    OAL_ASSERT(NULL_PTR!=stp_oal_msg, "gsm_fcp_dedicated null pointer");
    
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    /*Other simcards are not considered now, the function will be added later*/
    /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
    /* FIX PhyFA-Req00001525  BEGIN 2013-11-08 : zhengying */
    stp_gsm_conn_ch_tb = gsm_get_conn_ch_tb();
    u16_standby_id = stp_gsm_conn_ch_tb->u16_standby_id;  
    /* FIX PhyFA-Req00001525  END   2013-11-08 : zhengying */
    OAL_ASSERT((UNSPECIFIED_STANDBY_ID != u16_standby_id),"gsm_fcp_dedicated():g_st_gsm_conn_ch_tb.u16_standby_id is UNSPECIFIED_STANDBY_ID!");    
    OAL_ASSERT((NULL_PTR!= stp_gsm_latest_rtx_frame_pos),"gsm_fcp_dedicated():stp_gsm_latest_rtx_frame_pos is NULL_PTR!");    
    /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/  
    /* FIX PhyFA-Bug00002938    BEGIN 2011-09-09: wuxiaorong */ 
    
    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
    stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */

    /* FIX PhyFA-Bug00002938    END 2011-09-09: wuxiaorong */
    stp_nc_meas_sync_ctrl = gsm_get_nc_meas_sync_ctrl();
    stp_nc_balist = gsm_get_nc_balist(u16_standby_id);
        stp_gsm_standby_info = gsm_get_standby_info();
    /*support handover function*/
    stp_gsm_scell_info = gsm_get_gsm_scell_info(u16_standby_id);
    /* FIX PhyFA-Req00001525 BEGIN 2013-11-07 : zhengying */
    stp_gsm_conn_ho_tb     = gsm_get_conn_ho_tb();
    stp_gsm_conn_ho_params = &stp_gsm_conn_ho_tb->st_conn_ho_params;
    stp_gsm_conn_ch_tb     = gsm_get_conn_ch_tb();
    stp_gsm_params_ch      = &stp_gsm_conn_ch_tb->st_channel_params;
    stp_gsm_traffic_schedu = &stp_gsm_conn_ch_tb->st_traffic_schedu;
    /* FIX PhyFA-Req00001525 END   2013-11-07 : zhengying */
    if(stp_gsm_conn_ho_tb->u16_conn_ho_tb_flg)
    {
        stp_channel_params = &stp_gsm_conn_ho_tb->st_channel_params;

        /* FIX PhyFA-Bug00000642 BEGIN 2008-08-05 : ChenLei */
        /* FIX PHYFA-Bug00001302 BEGIN  2009-06-19: sunzhiqiang */
        /*FIX PhyFA-Bug00000674 BEGIN 2008-08-15 : ChenLei  */
        if( ((gsm_fn_compare((gsm_fn_operate((stp_channel_params->u32_starting_time+(UINT32)GSM_MAX_FN)-(UINT32)5)),u32_curr_fn))
            &&((UINT16)1 == stp_channel_params->u16_startingtime_valid))
        /*FIX PhyFA-Bug00000674 END 2008-08-15 : ChenLei  */
        /* FIX PHYFA-Bug00001302 END  2009-06-19: sunzhiqiang */
           || ((UINT16)1 == stp_channel_params->u16_beforetime_valid)
           ||((UINT16)0 == stp_channel_params->u16_startingtime_valid)
        )
        {
            /* FIX LM-Bug00000634 BEGIN 2011-07-18: wanghuan */ 
            if((UINT16)0 != stp_nc_meas_sync_ctrl->u16_running_bitmap)
            { 
                dd_event_stop_by_standby(u16_active_id,((UINT16)((UINT16)0x0001<<u16_standby_id)),(UINT16)GSM_TIMING,&st_mmc_time_info);
                /* FIX PhyFA-Bug00005535 BEGIN 2014-08-12 : wuxiaorong */
                stp_nc_meas_sync_ctrl->u16_running_bitmap = 0;/* bit0清0 */
                stp_nc_meas_sync_ctrl->u16_start_bitmap = 0;/* bit0清0 */
                /* FIX PhyFA-Bug00005535 END   2014-08-12 : wuxiaorong */
                
            }
            /* FIX LM-Bug00000634 END 2011-07-18: wanghuan */ 
            /* FIX PhyFA-Bug00000798 BEGIN 2008-10-13: ChenLei */
            if((UINT16)GSM_HANDOVER_SCELL_INDEX == stp_gsm_conn_ho_params->u16_ho_bcch_idx)
            {
                ;
            }
            else
            {
                /* FIX PhyFA-Bug00000753 BEGIN 2008-09-26: ChenLei */
                stp_time_info = gsm_get_ncell_timeinfo(stp_gsm_conn_ho_params->u16_ho_bcch_idx,u16_standby_id);
                /* FIX PhyFA-Bug00000753 END 2008-09-26: ChenLei */
                OAL_ASSERT((NULL_PTR != stp_time_info),"gsm_fcp_dedicated():Invalid timeinfo of u16_ho_bcch_idx!\n");    
                
                /* FIX PhyFA-Bug00000802 BEGIN 2008-11-07: ChenLei */
                /* FIX PHYFA-Bug00000815 BEGIN 2008-10-31 :ChenLei */
                /* FIX LM-Bug00000900 BEGIN  2011-09-29: sunzhiqiang */
                /* FIX PhyFA-Req00001525  BEGIN 2013-11-08 : zhengying */
                stp_time_info_ho                   = gsm_get_handover_cell_timeinfo();
                stp_time_info_ho->u32_serv_fn      = stp_time_info->u32_serv_fn;
                /* FIX LM-Bug00000900 END  2011-09-29: sunzhiqiang */
                stp_time_info_ho->u16_tsn          = stp_time_info->u16_tsn;
                stp_time_info_ho->s16_offset_in_ts = stp_time_info->s16_offset_in_ts;
                stp_time_info_ho->s32_delta_fn     = stp_time_info->s32_delta_fn;
                /* FIX PhyFA-Req00001525  END   2013-11-08 : zhengying */
                /* FIX PHYFA-Bug00000815 END 2008-10-31 :ChenLei */
                /* FIX PhyFA-Bug00000802 END 2008-11-07: ChenLei */

                gsm_fcp_handover_target_cell((UINT16)stp_gsm_conn_ho_params->u16_ho_bcch_idx,u16_standby_id);

                if((UINT16)1 == stp_channel_params->u16_startingtime_valid)
                {
                    stp_channel_params->u32_starting_time = gsm_fn_operate((UINT32)((SINT32)(UINT32)GSM_MAX_FN + (SINT32)stp_channel_params->u32_starting_time + stp_time_info->s32_delta_fn));
                }                
            }
            /* FIX PhyFA-Bug00000798 END 2008-10-13: ChenLei */
            /* FIX LM-Enh00001106 2013-03-20 BEGIN:sunzhiqiang */
            stp_nc_balist->s16_target_agc = stp_nc_balist->s16_nc_agc_list[stp_gsm_conn_ho_params->u16_ho_bcch_idx];
            stp_nc_balist->s16_oldcell_agc = stp_gsm_scell_info->s16_dedicated_reliable_agc;
            /* FIX LM-Enh00001106 2013-03-20 END:sunzhiqiang */

            /* FIX PhyFA-Bug00000682 BEGIN 2008-08-21 : ChenLei */
            stp_gsm_conn_ch_tb->u16_ta_value_old = stp_gsm_params_ch->u16_ta_value;
            /* FIX PhyFA-Bug00000682 END 2008-08-21 : ChenLei */
            /* FIX PhyFA-Bug00003983 BEGIN 2013-06-05 : sunzhiqiang */
            /* FIX PhyFA-Bug00004726 BEGIN 2014-05-14 : wuxiaorong */
            stp_gsm_conn_ch_tb->u16_ct_buffer_clear_flg  = 1; 
            stp_gsm_conn_ch_tb->u16_ilbuffer_clear_flg = 1;
            #if 0
            /* FIX PhyFA-Req00001525  BEGIN 2013-11-07 : zhengying */
            u16p_ded_clearbuff_flag  = gsm_get_ded_clearbuff_flag();
            *u16p_ded_clearbuff_flag = 1;
            /* FIX PhyFA-Req00001525  END   2013-11-07 : zhengying */
            #endif
            /* FIX PhyFA-Bug00004726 END   2014-05-14 : wuxiaorong */
            /* FIX PhyFA-Bug00003983 END 2013-06-05 : sunzhiqiang */
            gsm_send_state_change_req((UINT32)GSM_HANDOVER,u16_standby_id);
            /* FIX PhyFA-Bug00000213  END  2008-03-07  : huangjinfu */
            return OAL_FAILURE;
            /* FIX A2KPH-Bug00000412   END  2008-07-05: fushilong end*/
        }
    }

    /* FIX PhyFA-Bug00000832 BEGIN 2008-11-24 : ChenLei */
    if(((gsm_fn_compare(stp_gsm_conn_ch_tb->u32_connect_starting_time,u32_curr_fn))||(stp_gsm_conn_ch_tb->u32_connect_starting_time == u32_curr_fn))
      &&(OAL_TRUE == stp_gsm_conn_ch_tb->b_gsm_need_send_conind)
      &&(stp_gsm_conn_ch_tb->u32_connect_starting_time != (UINT32)GSM_INVALID_FN))
    {
        stp_gsm_conn_ch_tb->b_gsm_need_send_conind = OAL_FALSE;
        stp_gsm_conn_ch_tb->u32_connect_starting_time = (UINT32)GSM_INVALID_FN;

        gsm_hlp_send_dd_ch_conn_ind(u16_standby_id);
        /* FIX LM-Enh00001334 BEGIN  2013-10-21  : wangjunli */
        if(MPAL_TCH_F == stp_gsm_conn_ch_tb->st_channel_params.u16_ch_type)
        {
            u32_ready_fn = gsm_get_next_facch_tx_fn(gsm_fn_operate((u32_next_fn + (UINT32)GSM_MAX_FN) - (UINT32)1));
            if((UINT32)GSM_INVALID_FN != u32_ready_fn)/*距离下一个块2帧之内就不发了*/
            {
                gsm_fcp_handle_hls_res((UINT16)GSM_FCP_DEDI_READY_IND,gsm_fn_operate((u32_ready_fn + (UINT32)GSM_MAX_FN )- (UINT32)6) ,(UINT32)MPAL_FACCH,u16_standby_id);
            }
        }
        /* FIX LM-Enh00001334 END  2013-10-21  : wangjunli */
    }
    /* FIX PhyFA-Bug00000832 END 2008-11-24 : ChenLei */

    /* FIX A2KPH-Bug00001119 BEGIN 2008-11-19 : ChenLei */ 
    if(((gsm_fn_compare(stp_gsm_conn_ch_tb->u32_reconfig_starting_time,u32_curr_fn))||(stp_gsm_conn_ch_tb->u32_reconfig_starting_time == u32_curr_fn))
        &&(OAL_TRUE == stp_gsm_conn_ch_tb->b_gsm_need_send_reccnf))
    {
        stp_gsm_conn_ch_tb->b_gsm_need_send_reccnf = OAL_FALSE;
        stp_gsm_conn_ch_tb->u32_reconfig_starting_time = (UINT32)GSM_INVALID_FN;

        gsm_hlp_dd_ch_recfg_cnf(OAL_TRUE,(UINT16)1,u16_standby_id);
    }
    /* FIX A2KPH-Bug00001119 END 2008-11-19 : ChenLei */ 

    if((UINT16)GSM_TABLE_INVALID == stp_gsm_conn_ch_tb->u16_conn_ch_tb_flg)
    {
        return OAL_FAILURE;
    }

    stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();
    stp_normal_search = &(stp_cellsearch_ctrl->st_normal_search);
    if((UINT16)GSM_TABLE_VALID == stp_cellsearch_ctrl->u16_valid_flag)
    {
        if(((UINT16)GSM_TABLE_VALID == stp_normal_search->u16_valid_flag)&&(stp_normal_search->u16_search_standby_num>(UINT16)0))
        {
            u16_cellsearch_standby_id = stp_normal_search->u16_search_standby_id[0];
            stp_cellsearch_tbl = gsm_get_cellsearch_tbl_by_standby(u16_cellsearch_standby_id);
            gsm_check_cellsearch_affair_with_affairflow(u16_cellsearch_standby_id, stp_cellsearch_tbl);
        }
    }
    /*FIX LM-Bug00002205 BEGIN  2012-07-05  : sunzhiqiang*/
    /* Update TA value */
    stp_gsm_params_ch->u16_ta_value = gsm_fc_cs_ta_ctrl(u32_next_fn);
    /*FIX LM-Bug00002205 BEGIN  2012-07-05  : sunzhiqiang*/
    u16_curr_ch_type = stp_gsm_params_ch->u16_ch_type;

    switch(OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg))
    {
        case MSG_ISR_FCP_FRAMETIMER_IND:
            switch(u16_curr_ch_type)
            {
                case MPAL_SDCCH:

                    /* 伪事件优先配置，防止分配给辅模式的GAP被占 */
                    /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
                    gsm_check_and_add_psd_irat_task(u16_standby_id, u32_next_fn, GSM_PSD_FN_TASK);
                    /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
                    gsm_fcp_add_frametask(TSNTASK_SDCCH_RX,u32_next_fn,u16_standby_id);
                    gsm_fcp_add_frametask(TSNTASK_SACCH_RX,u32_next_fn,u16_standby_id);
                    gsm_fcp_add_frametask(TSNTASK_SDCCH_TX,u32_next_fn,u16_standby_id);
                    gsm_fcp_add_frametask(TSNTASK_SACCH_TX,u32_next_fn,u16_standby_id);
                   break;
                case MPAL_TCH_F:
                    u16_curr_ch_mode= stp_gsm_params_ch->u16_ch_mode;

                    /* 伪事件优先配置，防止分配给辅模式的GAP被占 */
                    /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
                    gsm_check_and_add_psd_irat_task(u16_standby_id, u32_next_fn, GSM_PSD_FN_TASK);
                    gsm_check_and_add_psd_irat_task(u16_standby_id, u32_next_fn, GSM_PSD_TS_TASK);
                    /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
                    /*for SACCH*/
                    gsm_fcp_add_frametask(TSNTASK_TCH_RX,u32_next_fn,u16_standby_id);
                    gsm_fcp_add_frametask(TSNTASK_SACCH_RX,u32_next_fn,u16_standby_id);
                    switch(u16_curr_ch_mode)
                    {
                        case MPAL_SIGNAL_CODE:
                            gsm_fcp_add_frametask(TSNTASK_FACCH_TX,u32_next_fn,u16_standby_id);
                            break;
                        case MPAL_TCH_FULL_SPEECH:
                        case MPAL_TCH_EFR_SPEECH:
                        /*FIX PhyFA-Req00000270  BEGIN  2009-03-20 :fushilong*/
                        case MPAL_TCHS_CODE_12_2:
                        case MPAL_TCHS_CODE_10_2:
                        case MPAL_TCHS_CODE_7_95:
                        case MPAL_TCHS_CODE_7_4 :
                        case MPAL_TCHS_CODE_6_7 :
                        case MPAL_TCHS_CODE_5_9 :
                        case MPAL_TCHS_CODE_5_15:
                        case MPAL_TCHS_CODE_4_75:
                        /*FIX PhyFA-Req00000270  END 2009-03-20 :fushilong*/
                            gsm_fcp_add_frametask(TSNTASK_TCH_TX,u32_next_fn,u16_standby_id);
                            break;
                        /* FIX PhyFA-Req00000111 BEGIN  2008-05-19  : fushilong */
                        /* FIX PhyFA-Req00000767  BEGIN  2010-09-28 : songzhiyuan */
                        case MPAL_TCH_CODE_4_8:
                        case MPAL_TCH_CODE_9_6:
                            gsm_fcp_add_frametask(TSNTASK_TCH_F96_TX,u32_next_fn,u16_standby_id);
                            break;
                        case MPAL_TCH_CODE_14_4:
                            gsm_fcp_add_frametask(TSNTASK_TCH_F144_TX,u32_next_fn,u16_standby_id);
                            break;
                        /* FIX PhyFA-Req00000767  BEGIN  2010-09-28 : songzhiyuan */
                        /* FIX PhyFA-Req00000111 END  2008-05-19  : fushilong */
                        default:
                            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNKNOWN_CHAN_TYPE));
                            break;
                    }

                    gsm_fcp_add_frametask(TSNTASK_SACCH_TX,u32_next_fn,u16_standby_id);
                    break;
                    /* FIX PhyFA-Req00000124 BEGIN  2008-05-19  : fushilong */
                case MPAL_TCH_H_0:
                case MPAL_TCH_H_1:
                    u16_curr_ch_mode= stp_gsm_params_ch->u16_ch_mode;

                    /* 伪事件优先配置，防止分配给辅模式的GAP被占 */
                    /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
                    gsm_check_and_add_psd_irat_task(u16_standby_id, u32_next_fn, GSM_PSD_FN_TASK);
                    /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
                    /*for SACCH*/
                    gsm_fcp_add_frametask(TSNTASK_TCH_H_RX,u32_next_fn,u16_standby_id);
                    gsm_fcp_add_frametask(TSNTASK_SACCH_RX,u32_next_fn,u16_standby_id);
                    switch(u16_curr_ch_mode)
                    {
                        case MPAL_SIGNAL_CODE:
                            gsm_fcp_add_frametask(TSNTASK_FACCH_H_TX,u32_next_fn,u16_standby_id);
                            break;
                        case MPAL_TCH_HALF_SPEECH:
                        /*FIX PhyFA-Req00000270  BEGIN  2009-03-20 :fushilong*/
                        case MPAL_TCHS_CODE_7_95:
                        case MPAL_TCHS_CODE_7_4 :
                        case MPAL_TCHS_CODE_6_7 :
                        case MPAL_TCHS_CODE_5_9 :
                        case MPAL_TCHS_CODE_5_15:
                        case MPAL_TCHS_CODE_4_75:
                        /*FIX PhyFA-Req00000270  END 2009-03-20 :fushilong*/
                            gsm_fcp_add_frametask(TSNTASK_TCH_H_TX,u32_next_fn,u16_standby_id);
                            break;
                         default:
                            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNEXPECTED_CHAN_TYPE));
                            break;
                    }

                    gsm_fcp_add_frametask(TSNTASK_SACCH_TX,u32_next_fn,u16_standby_id);
                    break;
                    /* FIX PhyFA-Req00000124 END  2008-05-19  : fushilong */
                default:
                    oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNEXPECTED_CHAN_TYPE));
                    break;
            }
            /*FIX LM-Req00000372  BEGIN   2011-09-23  : linlan*/
            /*FIX LM-Bug00000911  BEGIN   2011-10-12  : linlan*//*modify complete*/
            /* FIX PhyFA-Req00001525 BEGIN 2013-12-17 : gaowu */
            if(((UINT16)0 == stp_nc_meas_sync_ctrl->u16_valid_bitmap)
               ||((UINT16)0 == stp_nc_meas_sync_ctrl->u16_running_bitmap))
            {
                for(k = (UINT16)0; k < (UINT16)MAX_STANDBY_AMT; k++)
                {
                    if((UINT16)GSM_MASTER_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[k])
                    {
                        u16_sim_var = stp_gsm_standby_info->u16_standby_id[k];
                        /* 判断是否为待机卡的BCCH，NC BCCH和PCH */
                        if (u16_sim_var != u16_standby_id)
                        {
                            stp_gsm_sc_bcch_tb  = gsm_get_sc_bcch_tbl_cfg(u16_sim_var);
                            stp_temp_ccch_tlb   = gsm_get_ccch_tbl_cfg(u16_sim_var);
                            stp_gsm_nc_bcch_tbl = gsm_get_nc_bcch_tbl_cfg(u16_sim_var);
                            /* FIX LM-Bug00002651  BEGIN  2013-03-06  : sunzhiqiang */
                            /* add bcch rx */
                            if((UINT16)1 == stp_gsm_sc_bcch_tb->u16_sc_bcch_tb_flg)
                            {
                                oal_status_conflict_ret = gsm_fcp_add_frametask((UINT16)TSNTASK_BCCH_RX,u32_next_fn,u16_sim_var);
                                /* 如果到达BCCH配置点但是配置失败，说明是冲突导致，需要更新非业务卡的帧号信息 */
                                if((OAL_SUCCESS != oal_status_conflict_ret)&&(OAL_SUCCESS == stp_gsm_sc_bcch_tb->u32_arrive_ret))
                                {
                                    stp_gsm_sc_bcch_tb->u32_arrive_ret = OAL_FAILURE;
                                }
                            }
                            /* add ccch rx */
                            if((UINT16)1 == stp_temp_ccch_tlb->u16_ccch_tb_flg)
                            {
                                oal_status_conflict_ret = gsm_fcp_add_frametask((UINT16)TSNTASK_CCCH_RX,u32_next_fn,u16_sim_var);
                                /* 如果到达CCCH配置点但是配置失败，说明是冲突导致，需要更新非业务卡的帧号信息 */
                                if((OAL_SUCCESS != oal_status_conflict_ret)&&(OAL_SUCCESS == stp_temp_ccch_tlb->u32_arrive_ret))
                                {
                                    stp_temp_ccch_tlb->u32_arrive_ret= OAL_FAILURE;
                                }
                            }
                            /* FIX LM-Bug00002651  END  2013-03-06  : sunzhiqiang */
                            /* add nc bcch rx */
                            if ((UINT16)1 == stp_gsm_nc_bcch_tbl->u16_nc_bcch_tb_flg)
                            {
                                u16_nc_num = stp_gsm_nc_bcch_tbl->u16_avail_nc_num;
                                
                                for (m = (UINT16)0; m < u16_nc_num; m++)
                                {
                                    gsm_fcp_add_frametask(u16_tstask_nc_rx_tb[m],u32_next_fn,u16_sim_var);
                                }
                            }
                        }
                    }
                }
            }
            /* FIX PhyFA-Req00001525 END   2013-12-17 : gaowu */
            /*FIX LM-Bug00000911  END   2011-10-12  : linlan*/
            /*FIX LM-Req00000372  END   2011-09-23  : linlan*/
            /* FIX PhyFA-Req00000825 BEGIN  2011-01-10 : wuxiaorong */
            /* Fix PhyFA Req00000171 begin, guxiaobo, 20080826*/
            #ifdef PECKER_SWITCH 
            stp_gsm_ded_ch_report_ind = gsm_get_pecker_ded_ch_report_ind();
            if((UINT16)0 != stp_gsm_ded_ch_report_ind->u16_report_ctrl)
            {
                gsm_fcp_ded_ch_report_ind(u32_curr_fn, GSM_DEDICATED, u16_standby_id);

    
            }
            #endif
            /* Fix PhyFA Req00000171 end, guxiaobo, 20080826*/
            /* FIX PhyFA-Req00000825 END  2011-01-10 : wuxiaorong */
            /* FIX LM-Bug00000870    BEGIN 2011-9-26: wuxiaorong */ 
            gsm_fcp_set_meas_task(u32_next_fn, stp_latest_rtx_frame_pos);
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            /* FIX LM-Bug00000870    END 2011-9-26: wuxiaorong */ 

            /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
            stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
            stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn+(UINT32)1),u16_active_id);
            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)10, (UINT16_PTR)&u16_ts_bitmap[0], (UINT16_PTR)&u16_ts_bitmap[1]))
            {
                /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                  stp_gsm_main_frametask_tmp1,
                                  &u16_ts_bitmap[0], &u16_ts_bitmap[1]);   
                
                
            }
            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
            
            u16_ready_ch_type = GSM_NULL_CHANNEL_TYPE;
            switch(u16_curr_ch_type)
            {
                case MPAL_SDCCH:
                    gsm_fcp_enc_data(TSNTASK_SDCCH_TX,u32_curr_fn,u16_standby_id);
                    
                    st_tsn_info.u16_tstask = TSNTASK_SDCCH_TX;
                    st_tsn_info.u32_fn     = gsm_fn_operate(u32_curr_fn+GSM_DATA_READYIND_ADVANCE);
                    st_tsn_info.u16_standby_id = u16_standby_id;

                    if(OAL_SUCCESS == gsm_fcp_check_isarrive(&st_tsn_info,&st_tb_pos_info))
                    {
                        u16_ready_ch_type = MPAL_SDCCH;
                    }
                   break;
                case MPAL_TCH_F:
                    if(OAL_SUCCESS == gsm_fcp_enc_data(TSNTASK_FACCH_TX,u32_curr_fn, u16_standby_id))
                    {
                        /* FIX PhyFA-Req00000767  BEGIN  2010-09-28 : songzhiyuan */
                        if(((UINT16)MPAL_TCH_CODE_9_6 != u16_curr_ch_mode) && ((UINT16)MPAL_TCH_CODE_14_4 != u16_curr_ch_mode))    
                        {
                            u16_ready_ch_type = MPAL_FACCH;               
                            /* FIX LTE-Enh00000814 BEGIN  2014-04-11  : wangjunli */
                            if((UINT32)12 == (gsm_fn_operate(u32_curr_fn+(UINT32)6) %(UINT32)13))
                            {
                                u32_report_fn = gsm_fn_operate(u32_curr_fn+(UINT32)1);
                            }
                            else
                            {
                                u32_report_fn = u32_curr_fn;
                            }
                            gsm_fcp_handle_hls_res((UINT16)GSM_FCP_DEDI_READY_IND,u32_report_fn ,(UINT32)u16_ready_ch_type,u16_standby_id);
                            /* FIX LTE-Enh00000814 END  2014-04-11  : wangjunli */
                        }
                        /* FIX PhyFA-Req00000767  END  2010-09-28 : songzhiyuan */

                    }

                    switch(u16_curr_ch_mode)
                    {
                        case MPAL_SIGNAL_CODE:
                            break;
                        case MPAL_TCH_FULL_SPEECH:
                        case MPAL_TCH_EFR_SPEECH:
                        /*FIX PhyFA-Req00000270  BEGIN  2009-03-20 :fushilong*/
                        case MPAL_TCHS_CODE_12_2:
                        case MPAL_TCHS_CODE_10_2:
                        case MPAL_TCHS_CODE_7_95:
                        case MPAL_TCHS_CODE_7_4 :
                        case MPAL_TCHS_CODE_6_7 :
                        case MPAL_TCHS_CODE_5_9 :
                        case MPAL_TCHS_CODE_5_15:
                        case MPAL_TCHS_CODE_4_75:                                
                        /*FIX PhyFA-Req00000270  END 2009-03-20 :fushilong*/
                            gsm_fcp_enc_data(TSNTASK_TCH_TX,u32_curr_fn,u16_standby_id);
                            break;
                        /* FIX PhyFA-Req00000767  BEGIN  2010-09-28 : songzhiyuan */
                        case MPAL_TCH_CODE_4_8:
                        case MPAL_TCH_CODE_9_6:
                            if (OAL_SUCCESS == gsm_fcp_enc_data(TSNTASK_TCH_F96_TX,u32_curr_fn,u16_standby_id))
                            {
                                u16_ready_ch_type = MPAL_TCH_F;
                                gsm_fcp_handle_hls_res((UINT16)GSM_FCP_DEDI_READY_IND,u32_curr_fn ,(UINT32)u16_ready_ch_type,u16_standby_id);
                            }
                            break;
                        case MPAL_TCH_CODE_14_4:
                            if (OAL_SUCCESS == gsm_fcp_enc_data(TSNTASK_TCH_F144_TX,u32_curr_fn,u16_standby_id))
                            {
                                u16_ready_ch_type = MPAL_TCH_F;
                                gsm_fcp_handle_hls_res((UINT16)GSM_FCP_DEDI_READY_IND,u32_curr_fn ,(UINT32)u16_ready_ch_type,u16_standby_id);
                            }
                            break;
                         /* FIX PhyFA-Req00000767  END  2010-09-28 : songzhiyuan */
                         default:
                            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNEXPECTED_CHAN_TYPE));
                            break;
                    }

                   break;
                    /* FIX PhyFA-Req00000124 BEGIN  2008-05-19  : fushilong */
                    case MPAL_TCH_H_0:
                    case MPAL_TCH_H_1:
                    /* FIX PhyFA-Bug00000655 BEGIN  2008-08-14  : fushilong */
                    /* FIX PhyFA-Req00000124 BEGIN  2008-06-17  : fushilong */
                    gsm_fcp_enc_data(TSNTASK_FACCH_H_TX,gsm_fn_operate(u32_curr_fn + (UINT32)1),u16_standby_id);
                    /* FIX PhyFA-Req00000124 END  2008-06-17  : fushilong */
                    {
                        u16_ready_ch_type = MPAL_FACCH;
                        /*  fushilong@20080408 begin */
                        /* may be g_st_gsm_conn_ho_tb? */
                        /* FIX PhyFA-Req00000124 BEGIN  2008-06-11  : fushilong */
                        /*notify hls to prepare data in advance of 4 frame   */
                        if((((u32_curr_fn+(UINT32)4) %(UINT32)26)==(UINT32)(stp_gsm_conn_ch_tb->st_traffic_schedu.u16_facch_tx_pos[0]))||
                            (((u32_curr_fn+(UINT32)4) %(UINT32)26)==(UINT32)(stp_gsm_conn_ch_tb->st_traffic_schedu.u16_facch_tx_pos[1]))||
                            (((u32_curr_fn+(UINT32)4) %(UINT32)26)==(UINT32)(stp_gsm_conn_ch_tb->st_traffic_schedu.u16_facch_tx_pos[2])))
                            {
                               /* FIX PhyFA-Enh00000154 BEGIN  2008-06-16  : huangjinfu */
                                gsm_fcp_handle_hls_res((UINT16)GSM_FCP_DEDI_READY_IND,gsm_fn_operate((u32_curr_fn + (UINT32)GSM_MAX_FN )- (UINT32)2),(UINT32)u16_ready_ch_type,u16_standby_id);
                               /* FIX PhyFA-Enh00000154 END  2008-06-16  : huangjinfu */
                            }
                        /* FIX PhyFA-Req00000124 END  2008-06-11  : fushilong */
                        /*  fushilong@20080408 end */

                        /* FIX PhyFA-Bug00000655 END  2008-08-14  : fushilong */
                    }

                    switch(u16_curr_ch_mode)
                    {
                        case MPAL_SIGNAL_CODE:
                            break;
                        case MPAL_TCH_HALF_SPEECH:
                            /* FIX PhyFA-Bug00000655 BEGIN  2008-08-14  : fushilong */
                        /*FIX PhyFA-Req00000270  BEGIN  2009-03-20 :fushilong*/
                        case MPAL_TCHS_CODE_7_95:
                        case MPAL_TCHS_CODE_7_4 :
                        case MPAL_TCHS_CODE_6_7 :
                        case MPAL_TCHS_CODE_5_9 :
                        case MPAL_TCHS_CODE_5_15:
                        case MPAL_TCHS_CODE_4_75:                                
                        /*FIX PhyFA-Req00000270  END 2009-03-20 :fushilong*/
                            gsm_fcp_enc_data(TSNTASK_TCH_H_TX,gsm_fn_operate(u32_curr_fn + (UINT32)1),u16_standby_id);
                            /* FIX PhyFA-Bug00000655 END  2008-08-14  : fushilong */
                            break;
                         default:
                            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNEXPECTED_CHAN_TYPE));
                            break;
                    }

                   break;
                /* FIX PhyFA-Req00000124 END  2008-05-19  : fushilong */
                default:
                    oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNEXPECTED_CHAN_TYPE));
                   break;
            }

            gsm_fcp_enc_data(TSNTASK_SACCH_TX,u32_curr_fn,u16_standby_id);

            st_tsn_info.u16_tstask = TSNTASK_SACCH_TX;
            st_tsn_info.u32_fn     = gsm_fn_operate(u32_curr_fn+GSM_DATA_READYIND_ADVANCE);
            st_tsn_info.u16_standby_id = u16_standby_id;
                    
            if(OAL_SUCCESS == gsm_fcp_check_isarrive(&st_tsn_info,&st_tb_pos_info))
            {
                u16_ready_ch_type = MPAL_SACCH;
            }
            /* FIX PhyFA-Bug00000365   BEGIN  2008-04-19  : fushilong */    
            if((UINT16)MPAL_SDCCH == u16_ready_ch_type )
            {
                gsm_fcp_handle_hls_res((UINT16)GSM_FCP_DEDI_READY_IND,u32_curr_fn,(UINT32)u16_ready_ch_type,u16_standby_id);
            }
            else if( (UINT16)MPAL_SACCH == u16_ready_ch_type)
            {

                if( (gsm_fn_operate(u32_curr_fn+GSM_DATA_READYIND_ADVANCE)
                   %(UINT32) stp_gsm_traffic_schedu->u16_sacch_period)==
                   (UINT32)stp_gsm_traffic_schedu->u16_sacch_tx_pos[0])
                {
                    gsm_fcp_handle_hls_res((UINT16)GSM_FCP_DEDI_READY_IND,u32_curr_fn,(UINT32)u16_ready_ch_type,u16_standby_id);
                }

            }
            break;
        default:
            /* FIX PhyFA-Enh00001698 BEGIN  2014-04-23  : wangjunli*/
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            OAL_PRINT((UINT16)OAL_GET_OAL_MSG_STANDBY_ID(stp_oal_msg),(UINT16)GSM_MODE,"gsm_fcp_dedicated receive unexpected msg:%ld",OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg));
            /* FIX PhyFA-Enh00001698 END  2014-04-23  : wangjunli*/
            break;
    }

    return OAL_SUCCESS;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/




/*support handover function*/
/************************************************************
** Function Name: gsm_fcp_handover ()
** Description: the handler of the state GSM_HANDOVER of the module FCP
** Input :
      stp_tsn_prios[10]: priority info of  [tsn7|tsn0|tsn1|tsn2|tsn3|tsn4|tsn5|tsn6|tsn7|tsn0]
      u32_next_fn:  the next frame number
      stp_oal_msg:        message received from other module

** Output : NULL

** Return:  OAL_SUCCESS or OAL_FAILURE
** Notes:
2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_handover(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos)
{
    UINT16   u16_handover_type;
    UINT32   u32_enc_blks;
    CONST_UINT32  u32_curr_fn = gsm_fn_operate((u32_next_fn+(UINT32)GSM_MAX_FN)-(UINT32)1);
    UINT16  u16_channel_type = 0;
    /*FIX LM-Bug00001481  BEGIN   2011-12-27  : sunzhiqiang*/
    OAL_STATUS u32_fcp_ret = OAL_FAILURE;
    /*FIX LM-Bug00001481  END   2011-12-27  : sunzhiqiang*/
    /*FIX PhyFA-Req00000936  BEGIN   2011-07-12  : ningyaojun*/
    msg_gsm_fcp_l1resp_sync_handover_ind_t st_sync_handover_ind;
    /*FIX PhyFA-Req00000936  END     2011-07-12  : ningyaojun*/
    /*FIX PhyFA-Bug00002820  BEGIN   2011-08-15  : linlan*/
    UINT32 u32_calculated_sb_fn = (UINT32)0;
    /*FIX PhyFA-Bug00002820  END   2011-08-15  : linlan*/
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    CONST gsm_cellsearch_t    *stp_gsm_cellsearch     = NULL_PTR;
    CONST gsm_arfcn_cellsearch_t  *stp_gsm_arfcn_cellsearch_tb     = NULL_PTR;
   
    UINT16              u16_gsm_bg_state;
    /* Fix LM-Bug00000750 BEGIN 2011-09-14 : wanghuan*/
    /* FIX PhyFA-Enh00001154 BEGIN 2011-12-03: dujianzhong */
    SINT16 s16_ta_value_new;
    /* FIX PhyFA-Enh00001154 END   2011-12-03: dujianzhong */
    /* Fix LM-Bug00000750 END   2011-09-14 : wanghuan*/
    
    /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wuxiaorong */
    CONST msg_gsm_meas_l1cc_fcb_result_ind_t *stp_fcbsucc_ind =  NULL_PTR;
    /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */
    UINT16  u16_standby_id = UNSPECIFIED_STANDBY_ID;    
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp0 = NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp1 = NULL_PTR;    
    UINT16  u16_ts_bitmap[2] = {0,0}; 
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/ 
    gsm_offset_in_frame_t st_frame_pos;
    gsm_nc_balist_t*  stp_nc_balist =NULL_PTR;  
    /* FIX PhyFA-Req00001525 BEGIN 2013-11-07 : zhengying */
    CONST gsm_scell_info_t *stp_gsm_scell_info = NULL_PTR;
    gsm_conn_ho_tb_t *stp_gsm_conn_ho_tb = NULL_PTR;
    gsm_channel_parameters_t *stp_gsm_channel_params = NULL_PTR;
    gsm_conn_ho_params_t *stp_gsm_conn_ho_params = NULL_PTR;

    CONST gsm_conn_ch_tb_t *stp_gsm_conn_ch_tb = NULL_PTR;
    CONST gsm_channel_parameters_t *stp_gsm_params_ch = NULL_PTR;
    /* FIX PhyFA-Req00001525 END   2013-11-07 : zhengying */
    CONST gsm_cellsearch_ctrl_t *stp_cellsearch_ctrl = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    time_distance_inter_cells_t* stp_ho_time_info = NULL_PTR; 
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR;

    OAL_ASSERT(NULL_PTR!=stp_oal_msg, "gsm_fcp_handover null pointer");
    OAL_ASSERT(NULL_PTR!= stp_gsm_latest_rtx_frame_pos,"gsm_fcp_handover():stp_gsm_latest_rtx_frame_pos is NULL_PTR!");    
    
    stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();
    
    /* FIX PhyFA-Req00001525 BEGIN 2013-11-07 : zhengying */
    stp_gsm_conn_ho_tb     = gsm_get_conn_ho_tb();
    stp_gsm_channel_params = &stp_gsm_conn_ho_tb->st_channel_params;
    stp_gsm_conn_ho_params = &stp_gsm_conn_ho_tb->st_conn_ho_params;
    //stp_gsm_traffic_schedu = &stp_gsm_conn_ho_tb->st_traffic_schedu;
    
    stp_gsm_conn_ch_tb     = gsm_get_conn_ch_tb();    
    stp_gsm_params_ch      = &stp_gsm_conn_ch_tb->st_channel_params;
    /* FIX PhyFA-Req00001525 END   2013-11-07 : zhengying */
    /* FIX A2KPH-Bug00000509 BEGIN    2008-07-14: fushilong */
    u16_channel_type  = stp_gsm_channel_params->u16_ch_type;
    u16_handover_type = stp_gsm_conn_ho_params->u16_handover_type;
    /* FIX A2KPH-Bug00000509 END    2008-07-14: fushilong */
    /* FIX PhyFA-Bug00002938    BEGIN 2011-09-09: wuxiaorong */ 
    
    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
    stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */

        
    switch(OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg))
    {
        case MSG_ISR_FCP_FRAMETIMER_IND:
            /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
            /* FIX LM-Bug00001751    BEGIN 2012-02-25: linlan */
            u16_standby_id = stp_gsm_conn_ho_tb->u16_standby_id;
            /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
            if((UINT16)1 == stp_cellsearch_ctrl->u16_valid_flag)
            {
                stp_gsm_cellsearch = gsm_get_cellsearch_tbl_by_standby(u16_standby_id);    
                if((UINT16)1 == stp_gsm_cellsearch->u16_valid_flag)
                {
                    OAL_ASSERT((HO_SEARCH == stp_gsm_cellsearch->u16_search_type), "gsm_fcp_handover():Unexpected u16_search_type!\n");

                    u16_gsm_bg_state = stp_gsm_cellsearch->u16_bg_state;
                    stp_gsm_arfcn_cellsearch_tb = &(stp_gsm_cellsearch->st_arfcn_cellsearch[stp_gsm_cellsearch->u16_curr_arfcn_index]);
                    switch(u16_gsm_bg_state)
                    {
                        case GSM_BGS_NON:
                            break;
                        case GSM_BGS_SEARCH_RF:
                            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNEXPECTED_BG_STATE));
                            break;
                        case GSM_BGS_FCB:
                        case GSM_BGS_SB: 
                            /*FIX PhyFA-Bug00002891  BEGIN  2011-09-06 :fushilong*/
                            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)13));
                            /*FIX PhyFA-Bug00002891  END 2011-09-06 :fushilong*/
                            gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,NULL_PTR,stp_latest_rtx_frame_pos);
                            stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
                            stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn+(UINT32)1),u16_active_id);
                            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                            {
                                /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                                /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                                gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                                /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                                /* FIX NGM-Enh00000122  END    2014-10-10: linlan */

                                gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                                  stp_gsm_main_frametask_tmp1,
                                                  &u16_ts_bitmap[0], &u16_ts_bitmap[1]);                                
                            }
                            break;                            
                        case GSM_BGS_SYNC:
                            gsm_fcp_del_all_frametask((UINT16)UNSPECIFIED_STANDBY_ID);

                            stp_nc_balist = gsm_get_nc_balist(u16_standby_id);
                            stp_gsm_scell_info = gsm_get_gsm_scell_info(u16_standby_id);

                            if(((gsm_fn_compare((gsm_fn_operate(stp_gsm_channel_params->u32_starting_time)),u32_curr_fn))
                                   &&((UINT16)1 == stp_gsm_channel_params->u16_startingtime_valid))
                               ||((UINT16)0 == stp_gsm_channel_params->u16_startingtime_valid))
                            {
                                /* FIX PhyFA-Enh00001076  BEGIN  2011-08-8  : yuzhengfeng */    
                                /* FIX PhyFA-Enh00001076  END  2011-08-8  : yuzhengfeng */    
                                /* FIX LM-Enh00001106 2013-03-20 BEGIN:sunzhiqiang */
                                stp_nc_balist->s16_target_agc = stp_gsm_arfcn_cellsearch_tb->s16_agc;
                                stp_nc_balist->s16_oldcell_agc = stp_gsm_scell_info->s16_dedicated_reliable_agc;
                                /* FIX LM-Enh00001106 2013-03-20 END:sunzhiqiang */
                                /* FIX LM-Enh00001252   BEGIN   2013-08-05  : linlan*/
                                /* FIX LM-Bug00002855   BEGIN   2013-07-09  : sunzhiqiang*/
                                gsm_l1cc_send_stop_norm_cellsearch_msg(u16_standby_id, INVALID_WORD);
                                /* FIX LM-Bug00002855   END     2013-07-09  : sunzhiqiang*/  
                                /* FIX LM-Enh00001252   END     2013-08-05  : linlan*/
                                
                                /*fix A2KPH_Bug00001151  begin: fushilong 2008-11-25*/            
                                stp_gsm_conn_ho_tb->u16_conn_ho_tb_flg=1; 
                                /*fix L2000-Bug00001392  begin: songzhiyuan 2010-05-21*/            
                                if((UINT16)0x00 == stp_gsm_conn_ho_tb->u16_rat_type)
                                {
                                    
                                    /*save origin cell tineinfo and transfer origin cell timeinfo to target cell timeinfo*/
                                    gsm_fcp_handover_target_cell((UINT16)stp_gsm_conn_ho_params->u16_ho_bcch_idx,u16_standby_id);
                                }
                                /*fix L2000-Bug00001392  end: songzhiyuan 2010-05-21*/            
                                /*fix A2KPH_Bug00001151  end: fushilong 2008-11-25*/  
                            }
                            break;
                        case GSM_BGS_HANDOVER_SYNC_FAIL: 
                            /* FIX PhyFA-Enh00001076  BEGIN  2011-08-8  : yuzhengfeng */    
                            #if 0
                            stp_gsm_cellsearch->u16_bg_state = GSM_BGS_NON;
                            #endif
                            /* FIX PhyFA-Enh00001076  END  2011-08-8  : yuzhengfeng */     
                            /* FIX LM-Enh00001252   BEGIN   2013-08-05  : linlan*/
                            /* FIX LM-Bug00002855   BEGIN   2013-07-09  : sunzhiqiang*/
                            gsm_l1cc_send_stop_norm_cellsearch_msg(u16_standby_id, INVALID_WORD);
                            /* FIX LM-Bug00002855   END     2013-07-09  : sunzhiqiang*/  
                            /* FIX LM-Enh00001252   END     2013-08-05  : linlan*/
                            break;
                        default:
                            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(UNEXPECTED_BG_STATE));                            
                            break;
                    }
                    return OAL_SUCCESS;
                }
            }
            /* FIX LM-Bug00001471    BEGIN 2011-12-14: wuxiaorong */
            if((UINT16)0 == stp_gsm_conn_ho_tb->u16_conn_ho_tb_flg)
            {
                return OAL_SUCCESS;
            }
            /* FIX LM-Bug00001471    END 2011-12-14: wuxiaorong */
            /*FIX LM-Bug00000526  BEGIN  2010-05-14 :wanghuan*/
            if((UINT16)0 == stp_gsm_conn_ho_tb->u16_agc_init_flag)
            {
                 stp_gsm_conn_ho_tb->u16_agc_init_flag = 1;
                 gsm_fc_handover_init(stp_gsm_conn_ho_tb->u16_rat_type, u16_standby_id);
                 /* FIX LM-Bug00000775  BEGIN  2011-09-16  : wanghuan */
                 /*inform HLS to modify the EDEN parameter*/
                 gsm_fcp_handle_hls_res((UINT16)GSM_FCP_HANDOVER_CNF,(UINT32)0,(UINT32)0,u16_standby_id);
                 /* FIX LM-Bug00000775  END    2011-09-16  : wanghuan */
            }
            /*FIX LM-Bug00000526  END  2010-05-14 :wanghuan*/


            
            /*FIX LM-Enh00000369 BEGIN 2011-08-27: sunzhiqiang */
            #if 0
            if((UINT32)GSM_INVALID_FN == stp_gsm_conn_ho_params->u32_handover_tx_fn)
            {
                /* FIX PhyFA-Bug00000443    END     2008-05-14  : ningyaojun*/
                /* FIX PhyFA-Enh00000089   BEGIN  2008-04-07  : huangjinfu */
                if(NON_SYNC_HO == u16_handover_type)
                {
                    if((UINT32)GSM_INVALID_FN != stp_gsm_conn_ho_params->u32_handover_tx_fn)
                    {
                        stp_gsm_conn_ho_params->u32_t3124_fn =
                            gsm_calc_t3124(stp_gsm_conn_ho_params->u32_handover_tx_fn);
                    }
                }
                /* FIX PhyFA-Enh00000089   END    2008-04-07  : huangjinfu */
            }
            #endif
            /*FIX LM-Enh00000369 END 2011-08-27: sunzhiqiang */
            if((UINT16)NON_SYNC_HO == u16_handover_type)
            {
                /* Fix LM-Bug00000750 BEGIN 2011-09-14 : wanghuan*/
                #if 0
                if(GSM_FCP_INVALID_FN != g_stp_gsm_conn_ho_tb->st_conn_ho_params.u32_handover_tx_fn)
                {
                    stp_gsm_conn_ho_params->u32_t3124_fn =
                    gsm_calc_t3124(stp_gsm_conn_ho_params->u32_handover_tx_fn);
                }
                #endif
                /* Fix LM-Bug00000750 END 2011-09-14 : wanghuan*/
            }
            else if((UINT16)SYNC_HO == u16_handover_type)
            {
                /* Fix LM-Bug00000750 BEGIN 2011-09-14 : wanghuan*/
                stp_ho_time_info = gsm_get_handover_time_info();
                if(stp_ho_time_info->s16_delta_offset > 2500)
                {
                    stp_ho_time_info->s16_delta_offset-= 5000;
                }
                 /* FIX PhyFA-Enh00001154 BEGIN 2011-12-03: dujianzhong */
                s16_ta_value_new = ((SINT16)(stp_gsm_params_ch->u16_ta_value) +(stp_ho_time_info->s16_delta_offset * 2)) % 256;
                /*TA = 2d/c,where d is the distance from MS to BTS,and the c is the speed of wireless wave.*/
                /*rx_offset represents d/c in the old cell time,so increased TA = rx_offset*2.*/
                if((s16_ta_value_new > 252) ||(0 > s16_ta_value_new ))
                {
                    if((UINT16)0 == stp_gsm_conn_ho_params->u16_nci)
                    {
                        if(s16_ta_value_new > 252)/*max ta_value:63bit*/
                        {
                            s16_ta_value_new = 252;
                        }
                        else if(s16_ta_value_new < 0)
                        {
                            s16_ta_value_new = 0;
                        }
                    }
                    else
                    {
                        gsm_l1resp_handover_failed((UINT16)TA_OUT_OF_RANGE,u16_standby_id);
                        return OAL_SUCCESS;
                    }
                }
                stp_gsm_channel_params->u16_ta_value = (UINT16)s16_ta_value_new;
               /* FIX PhyFA-Enh00001154 END 2011-12-03: dujianzhong */
               /* Fix LM-Bug00000750 END   2011-09-14 : wanghuan*/
            }
            else if((UINT16)PRE_SYNC_HO == u16_handover_type)
            {
                if(stp_gsm_channel_params->st_timing_adv.is_value_valid)
                {
                    stp_gsm_channel_params->u16_ta_value =
                        (stp_gsm_channel_params->st_timing_adv.value * (UINT16)4);
                }
                else
                {
                    stp_gsm_channel_params->u16_ta_value = 4;
                }
            }
            else if((UINT16)PSEUDO_SYNC_HO == u16_handover_type)
            {

            }
            else
            {
                oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
            }
            /* FIX PhyFA-Enh00000089   END    2008-04-07  : huangjinfu */

            /*handover rx*/
            if( (((UINT16)0 != stp_gsm_channel_params->st_cipher_set.cipher_mode)
                &&((UINT16)1 == stp_gsm_conn_ho_tb->u16_conn_ho_tb_flg)))
            {
                gsm_fcp_handover_cipher(u32_next_fn);
            }
            if((UINT16)NON_SYNC_HO == u16_handover_type)
            {
                gsm_fcp_add_frametask(TSNTASK_HANDOVER_RX,u32_next_fn,u16_standby_id);
            }

            /*handover tx*/
            /* FIX L2000-Bug00000657 BEGIN 2010-01-12 : liujiaheng */
            switch(stp_gsm_conn_ho_params->u16_data_state)
            {
                case GSM_DATA_ENCODED:
                    if(OAL_TRUE == gsm_fcp_handover_tx_enable(u32_next_fn,u16_channel_type))
                    {
                        /* FIX LM-Bug00000036 BEGIN 2010-07-28 : songzhiyuan */
                        /* Fix LM-Req00000138   BEGIN  2011-02-26  : sunzhiqiang */
                        if(/* Fix LM-Bug00000489   BEGIN  2011-04-22  : sunzhiqiang */
                           (gsm_fn_compare(u32_next_fn, stp_gsm_channel_params->u32_newcell_starting_time))
                           &&((UINT16)1 == stp_gsm_channel_params->u16_newcell_startingtime_valid)
                           &&(stp_gsm_channel_params->u16_beforetime_valid != (UINT16)1))
                           /* Fix LM-Bug00000489   END  2011-04-22  : sunzhiqiang */
                        {
                        }
                        else
                        {
                            if(OAL_SUCCESS == gsm_fcp_add_special_frametask(TSNTASK_HANDOVER_TX,u32_next_fn,u16_standby_id))
                            {
                                stp_gsm_conn_ho_params->u16_ab_attempt_times = 1;
                                /* Fix LM-Bug00000750 BEGIN 2011-09-14 : wanghuan*/
                                #if 0
                                stp_gsm_conn_ho_params->u32_handover_tx_fn = u32_next_fn;
                                #endif
                                if((UINT16)NON_SYNC_HO == u16_handover_type)
                                {
                                    stp_gsm_conn_ho_params->u32_t3124_fn = gsm_calc_t3124(u32_next_fn);
                                }
                                /* Fix LM-Bug00000750 END   2011-09-14 : wanghuan*/
                                stp_gsm_conn_ho_params->u16_data_state = GSM_DATA_SENT;
                            }
                            stp_gsm_channel_params->u16_newcell_startingtime_valid = 0;
                            stp_gsm_channel_params->u32_newcell_starting_time = (UINT32)GSM_INVALID_FN;
                        }
                        /* Fix LM-Req00000138   END  2011-02-26  : sunzhiqiang */
                    }
                   break;
                case GSM_DATA_SENT:
                    if(OAL_TRUE == gsm_fcp_handover_tx_enable(u32_next_fn,u16_channel_type))
                    {
                        if(((UINT16)4 > stp_gsm_conn_ho_params->u16_ab_attempt_times) || ((UINT16)NON_SYNC_HO == u16_handover_type))
                        {
                            stp_gsm_conn_ho_params->u16_ab_attempt_times++;
                            st_frame_pos.s16_offset = 0;
                            st_frame_pos.u16_tsn = (UINT16)(stp_gsm_conn_ho_tb->st_traffic_schedu.u16_tch_rx_tn + (UINT16)3);
                            st_frame_pos.u32_fn = u32_next_fn;
                            gsm_fcp_check_add_tsntask(TSNTASK_HANDOVER_TX,&st_frame_pos,u16_standby_id);
                            //gsm_fcp_add_special_frametask(stp_tsn_prios,TSNTASK_HANDOVER_TX,u32_next_fn);
                        }
                        else
                        {
                            stp_gsm_conn_ho_params->u16_ab_attempt_times++;
                        }
                        /* FIX LM-Bug00000036 END 2010-07-28 : songzhiyuan */
                        if(((UINT16)NON_SYNC_HO != u16_handover_type) && ((UINT16)6 <= stp_gsm_conn_ho_params->u16_ab_attempt_times  ))
                        {
                            /*delay 2 frames change system state to dedicated*/
                            if(stp_gsm_conn_ho_params->u16_ab_attempt_times > (UINT16)6)
                            {
                                oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
                            }
                            else
                            { 
                                /*FIX PhyFA-Req00000936  BEGIN   2011-07-12  : ningyaojun*/                                
                                st_sync_handover_ind.u16_standby_id = u16_standby_id;
                                gsm_l1cc_send_msg(&st_sync_handover_ind,(UINT16)L1CC_GSM_L1RESP_TASK_ID,MSG_GSM_FCP_L1RESP_SYNC_HANDOVER_IND,(UINT16)sizeof(msg_gsm_fcp_l1resp_sync_handover_ind_t),u16_standby_id);
                                /*FIX PhyFA-Req00000936  END     2011-07-12  : ningyaojun*/                             
                                
                                /* 状态改变换成发消息，和直接设置g_u16_gsm_fcp_new_state差别只在帧中断边界情况下状态晚一帧生效 */
                                #if 0
                                                 /* FIX LM-Bug00001007 BEGIN 2010-10-14 : linlan */
                                //gsm_set_fcp_new_state(GSM_DEDICATED);/*cann't send message*/
                                /* FIX LM-Bug00001007 END 2010-10-14 : linlan */
                                #endif
                                gsm_send_state_change_req((UINT32)GSM_DEDICATED,u16_standby_id);
                            }
                        }
                    }
                    break;
                case GSM_DATA_UNENCODED:
                      gsm_fcp_enc_special_data(&u32_enc_blks,TSNTASK_HANDOVER_TX,u32_curr_fn,u16_standby_id);
                     break;
                default:
                     break;
            }            
            /* FIX L2000-Bug00000657 END   2010-01-12 : liujiaheng */

            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
            stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
            stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn+(UINT32)1),u16_active_id);
            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            {
                /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                  stp_gsm_main_frametask_tmp1,
                                  &u16_ts_bitmap[0], &u16_ts_bitmap[1]);                
            }
            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/       
            break;
        /*fix A2KPH_Bug00001151  begin: fushilong 2008-11-15*/            
        case MSG_GSM_MEAS_FCP_FCBSUCC_IND:


            
            /*FIX PhyFA-Req00000936  BEGIN   2011-06-14  : ningyaojun*/
            stp_fcbsucc_ind = (msg_gsm_meas_l1cc_fcb_result_ind_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));
            u16_standby_id = stp_fcbsucc_ind->u16_standby_id;

            if((UINT16)1 == stp_cellsearch_ctrl->u16_valid_flag)
            {
                stp_gsm_cellsearch = gsm_get_cellsearch_tbl_by_standby(u16_standby_id);    
                if((UINT16)1 == stp_gsm_cellsearch->u16_valid_flag)
                {
                    u16_gsm_bg_state = stp_gsm_cellsearch->u16_bg_state;
                    if(GSM_BGS_NON != u16_gsm_bg_state)
                    {
                        /*FIX PhyFA-Bug00002820  BEGIN   2011-08-15  : linlan*/
                        /*FIX LM-Bug00001481  BEGIN   2011-12-27  : sunzhiqiang*/
                        if(NOCELL_SEARCH == stp_gsm_cellsearch->u16_search_type)
                        {
                            u32_fcp_ret = gsm_fcp_nocell_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                        }
                        else
                        {
                            u32_fcp_ret = gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                        }
                        if(OAL_SUCCESS == u32_fcp_ret)
                        {
                            stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_calculated_sb_fn,u16_active_id);
                            stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_calculated_sb_fn+(UINT32)1),u16_active_id);
                            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_calculated_sb_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                            {
                                /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                                /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                                gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                                /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                                /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                                gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                                  stp_gsm_main_frametask_tmp1,
                                                  &u16_ts_bitmap[0], &u16_ts_bitmap[1]);                                
                            }   
                        }
                        /*FIX LM-Bug00001481  END   2011-12-27  : sunzhiqiang*/
                    }
                    /*FIX PhyFA-Bug00002820  END   2011-08-15  : linlan*/
                }
            }                
            /*FIX PhyFA-Req00000936  END     2011-06-14  : ningyaojun*/            
            break;
            /*fix A2KPH_Bug00001151  end: fushilong 2008-11-15*/
        default:
            /* FIX PhyFA-Enh00001698 BEGIN  2014-04-23  : wangjunli*/
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            OAL_PRINT((UINT16)OAL_GET_OAL_MSG_STANDBY_ID(stp_oal_msg),(UINT16)GSM_MODE,"gsm_fcp_handoverreceive unexpected msg:%ld",OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg));
            /* FIX PhyFA-Enh00001698 END  2014-04-23  : wangjunli*/
            break;
    }
    return OAL_SUCCESS;
}
/* FIX A2KPH-Bug00000509 END    2008-07-14: fushilong */




/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_transfer
* DESCRIPTION:       the handler of the state GSM_TRANSFER of the module FCP
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  stp_tsn_prios[10]: priority info of  [tsn7|tsn0|tsn1|tsn2|tsn3|tsn4|tsn5|tsn6|tsn7|tsn0]
      u32_next_fn:  the next frame number
      stp_oal_msg:        message received from other module
* Output Parameters: NULL
* Return value:      OAL_SUCCESS or OAL_FAILURE
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-11-05      zhengying               PhyFA-Req00001525   [NGM]V4 GSM代码入库
* 2014-05-09      linlan                  PhyFA-Bug00004614   [NMG]transfer下需要根据USF结果清除u16_next_block_tx_flag标志
* 2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
* 2014-09-22      gaowu                   NGM-Bug00000586     [NGM]l1cc_gsm需要将伪事件的配置按照非周期事件来配置
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_transfer(UINT32 u32_next_fn,oal_msg_t *stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos)
{
    UINT16 i;
    UINT16 u16_higer_tsn_allowed;
    UINT16 u16_mac_mode = INVALID_MAC_MODE;

    UINT32 u32_curr_fn;
    UINT32 u32_next_block_fn;
    msg_gsm_eq_fcp_usf_ind_t *stp_usf_ind=NULL_PTR;
    UINT16 u16_start_tsn,u16_end_tsn;
    UINT16 u16_tsn_no;
    UINT16 u16_add_txslot_bitmap=0;
    SINT16 s16_rtx_offset;
    UINT32 u32_pdtch_rtx_allowed = OAL_FAILURE;

    CONST gprs_fcp_rrbp_encbuff_info_st   *stp_gprs_rrbp_encbuff_info = NULL_PTR;
    CONST gprs_fcp_encbuff_info_st    *stp_gprs_fcp_encbuff_info = NULL_PTR;
    gprs_pdtch_para_t*  stp_pdtch_prarms= NULL_PTR;
    /*FIX LM-Bug00001481  BEGIN   2011-12-27  : sunzhiqiang*/
    OAL_STATUS u32_fcp_ret = OAL_FAILURE;
    /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wuxiaorong */
    CONST msg_gsm_meas_l1cc_fcb_result_ind_t *stp_fcbsucc_ind =  NULL_PTR;
    /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */
    CONST gsm_cellsearch_t    *stp_gsm_cellsearch     = NULL_PTR;
    UINT16 u16_gsm_bg_state;
    UINT32 u32_calculated_sb_fn = (UINT32)0;
    UINT16  u16_rxtask_type;
    /*FIX LM-Bug00001481  END   2011-12-27  : sunzhiqiang*/

    /*FIX A2KPH-Bug00002339  BEGIN 2009-07-20: songzhiyuan*/ 
    gprs_pdtch_para_t *stp_usf_pdtch_prarms = NULL_PTR;
    /*FIX A2KPH-Bug00002339  END 2009-07-20: songzhiyuan*/ 
    /*  FIX LM-Enh00000738    BEGIN 2012-05-30: wanghuan*/ 
    UINT16 u16_usf_detected = 0;
    /*  FIX LM-Enh00000738    END   2012-05-30: wanghuan*/ 

    /* FIX PhyFA-Bug00000249   BEGIN  2008-03-31  : huangjinfu */
    CONST gprs_fcp_encbuff_info_st *stp_encbuff_info = NULL_PTR;
    /* FIX PhyFA-Bug00000249   END  2008-03-31  : huangjinfu */
    /* FIX A2KPH-Bug00000377  BEGIN    2008-06-17  : ningyaojun*/
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp0 = NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp1 = NULL_PTR;
    /* FIX A2KPH-Bug00000377  BEGIN    2008-06-17  : ningyaojun*/

    /* FIX PHYFA-Enh00000220  BEGIN   2008-09-08  : ningyaojun*/
    /* FIX PHYFA-Enh00000220  END     2008-09-08  : ningyaojun*/

    /* Fix A2KPH Bug00001028 begin, guxiaobo, 20081028*/
    UINT32 u32_gsm_fcp_req_blks;
    /* Fix A2KPH Bug00001028 end, guxiaobo, 20081028*/
    /* FIX PhyFA-Req00000344 BEGIN  2009-06-06  : wuxiaorong */  
    
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    UINT16  u16_standby_id = UNSPECIFIED_STANDBY_ID;
    UINT16  u16_ts_bitmap[2] = {0,0}; 
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
    /*FIX LM-Bug00002571  BEGIN   2013-01-16  : ningyaojun*/
    UINT32 u32_interval_fn = (UINT32)0;
    /*FIX LM-Bug00002571  END     2013-01-16  : ningyaojun*/

    OAL_STATUS u32_judge_usf = OAL_FAILURE;
    /* FIX LM-Enh00001186 BEGIN  2013-06-09  yuzhengfeng*/

    /* FIX PhyFA-Req0000xxxx BEGIN 2013-07-25 : gaowu */
    UINT16 u16_usf_valid_cnt = (UINT16)0;
    
    gsm_tstask_latest_frame_pos_t*  stp_latest_frame_pos_list = NULL_PTR;
    CONST gprs_ps_signal_state_t *stp_gsm_ps_signal_state_ind = NULL_PTR;
    gsm_offset_in_frame_t st_frame_pos;
    /* FIX PhyFA-Req00001525 BEGIN 2013-11-05 : zhengying */

    gprs_pdtch_tb_t *stp_gprs_pdtch_tb = NULL_PTR;
    CONST gsm_cellsearch_ctrl_t* stp_cellsearch_ctrl = NULL_PTR;  
    /* FIX PhyFA-Req00001525 END   2013-11-05 : zhengying */
    #ifdef PECKER_SWITCH  
    CONST gsm_ded_ch_report_ind_tb_t* stp_gsm_ded_ch_report_ind = NULL_PTR;
    #endif
    CONST msg_gsm_eq_fcp_loopback_ind_t   *stp_eq_fcp_loopback_ind = NULL_PTR;
    CONST gsm_edge_loopback_tbl_t   *stp_gsm_edge_loopback_tbl = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    UINT32_PTR u32p_block_fn_from_usf_messge = NULL_PTR;
    CONST_UINT32_PTR u32p_block_fn_from_frame_timer = NULL_PTR;
    UINT16_PTR u16p_gsm_pdtch_sent_blks = NULL_PTR;
    gsm_offset_in_frame_t st_latest_fn_pos;    
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR; 
    /* FIX LTE-Enh00000846 BEGIN  2014-04-29: wangjunli */
    UINT16 u16_pdch_tx_task = TSNTASK_NULL;
    CONST gsm_common_info_t* stp_gsm_common_info = NULL_PTR;
    /* FIX LTE-Enh00000846 END  2014-04-29: wangjunli */
    CONST gsm_standby_info_t*  stp_gsm_standby_info = NULL_PTR;
    
    OAL_ASSERT(NULL_PTR!=stp_oal_msg, "gsm_fcp_transfer null pointer");
    OAL_ASSERT(NULL_PTR!= stp_gsm_latest_rtx_frame_pos,"gsm_fcp_transfer():stp_gsm_latest_rtx_frame_pos is NULL_PTR!");    
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    /*Other simcards are not considered now, the function will be added later*/
    /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
    stp_gprs_pdtch_tb = gsm_get_gprs_cfg_tbl();
    
    u16_standby_id = stp_gprs_pdtch_tb->u16_standby_id;    
    OAL_ASSERT((UNSPECIFIED_STANDBY_ID != u16_standby_id),"gsm_fcp_transfer():g_st_gprs_pdtch_tb.u16_standby_id is UNSPECIFIED_STANDBY_ID!");    
    /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/ 
    /* FIX PhyFA-Bug00005065    BEGIN   2014-06-27  : linlan*/
    stp_gsm_standby_info = gsm_get_standby_info(); 
    /* FIX PhyFA-Bug00005065    END     2014-06-27  : linlan*/

    
    /* FIX LTE-Enh00000846 BEGIN  2014-04-29: wangjunli */
    stp_gsm_common_info = gsm_get_common_info(u16_standby_id);
    if((UINT16)1 == stp_gsm_common_info->u16_gsm_fta_mode)
    {
        u16_pdch_tx_task = TSNTASK_PDTCH_FTA_TX;
    }
    else
    {
        u16_pdch_tx_task = TSNTASK_PDTCH_TX;
    }
    /* FIX LTE-Enh00000846 END  2014-04-29: wangjunli */
    /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
    #if 0
    u32_ret = oal_tpc_filter_msg(MSG_GSM_FCP_PSD_FRAMETASK_INFO,(UINT16)UNSPECIFIED_STANDBY_ID,(UINT16)GSM_MODE);
    #endif
    /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
    /* FIX PhyFA-Req00000344 END  2009-06-06  : wuxiaorong */
    
    u32_curr_fn = gsm_fn_operate((u32_next_fn + (UINT32)GSM_MAX_FN) - (UINT32)1);
    /* FIX PhyFA-Bug00002938    BEGIN 2011-09-09: wuxiaorong */ 
    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
    stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */
    /* FIX PhyFA-Bug00002938    END 2011-09-09: wuxiaorong */
    /* FIX LM-Bug00002968   BEGIN   2013-09-23  : linlan*/
    if(stp_gprs_pdtch_tb->u16_pdtch_tb_flg != GSM_TABLE_VALID)
    {
        OAL_PRINT(u16_standby_id,(UINT16)GSM_MODE,"u8_pdtch_tb_flg table invalid");
        return OAL_FAILURE;
    }
    /* FIX LM-Bug00002968   END     2012-09-23  : linlan*/

    switch(OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg))
    {
        case MSG_ISR_FCP_FRAMETIMER_IND:
            /* FIX A2KPH-Bug00001579   BEGIN   2009-02-27  : ningyaojun*/
            gprs_fcp_conf_new_para(u32_curr_fn,u16_standby_id);
            /* FIX LM-Bug00002182  BEGIN   2012-07-07  : guxiaobo */
            gprs_fcp_cfg_loopback_para(u16_standby_id);
            /* FIX LM-Bug00002182  END   2012-07-07  : guxiaobo */            
            u32_pdtch_rtx_allowed = gsm_get_pdchpara_from_gprstb(&stp_pdtch_prarms,u32_next_fn);
            /* FIX A2KPH-Bug00001579   END     2009-02-27  : ningyaojun*/

            /* 伪事件优先配置，防止分配给辅模式的GAP被占 */
            /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
            gsm_check_and_add_psd_irat_task(u16_standby_id, u32_next_fn, GSM_PSD_FN_TASK);
            gsm_check_and_add_psd_irat_task(u16_standby_id, u32_next_fn, GSM_PSD_TS_TASK);
            /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
            /*FIX LM-Req00000372  BEGIN   2011-09-23  : linlan*/ 
            #if 0
            gsm_fcp_add_frametask(TSNTASK_BCCH_RX,u32_next_fn,u16_standby_id);
            #endif
           
            stp_latest_frame_pos_list = (gsm_tstask_latest_frame_pos_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,
                                                                                 (UINT32)OAL_GET_TYPE_SIZE_IN_WORD(gsm_tstask_latest_frame_pos_t));  
            stp_latest_frame_pos_list->u16_latest_pos_num = 0;
            /*配置tranfer下SCELL_BCCH,TC_BCCH,NC_BCCH任务*/
            gsm_add_bcch_in_transfer_tsntask(u32_next_fn, stp_latest_frame_pos_list);
            /*FIX LM-Req00000372  END   2011-09-23  : linlan*/
            gsm_fcp_add_frametask(TSNTASK_PBCCH_RX,u32_next_fn,u16_standby_id);

            /* Fix A2KPH Bug00000580 begin, guxiaobo, 20081031*/
            /* FIX AM-Bug00000151 BEGIN  2010-07-16 : wanghuan */
            /* FIX AM-Bug00000386    BEGIN   2011-04-25  : wanghuan*/
            if((OAL_SUCCESS == u32_pdtch_rtx_allowed) 
               && (( EDGE_RLC_UNACK_MODE == stp_pdtch_prarms->st_edge_cfg_info.u16_dl_rlc_mode )
                  ||(EDGE_RLC_UNACK_MODE == stp_pdtch_prarms->st_edge_cfg_info.u16_ul_rlc_mode)))
            {
               ;
            }
            else 
            {
                /*FIX LM-Req00000372  BEGIN   2011-09-23  : linlan*/ 
                gsm_add_ccch_in_transfer_tsntask(u32_next_fn,stp_latest_frame_pos_list);
                /*FIX LM-Req00000372  END   2011-09-23  : linlan*/
            }
            /* FIX AM-Bug00000386    END     2011-04-25  : wanghuan*/
           
            /*检测事件优先级是否满足transfer下优先配搜网以及SYNC BCCH*/
            /* FIX PhyFA-Bug00005065    BEGIN   2014-06-27  : linlan*/
            for(i = (UINT16)0; i <(UINT16)MAX_STANDBY_AMT; i++)
            {
                if((UINT16)GSM_MASTER_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[i])
                {         
                    gsm_add_sync_cell_bcch_tsntask(u32_next_fn,stp_gsm_standby_info->u16_standby_id[i],stp_latest_frame_pos_list);  
                }
            }            
            /* FIX PhyFA-Bug00005065    END     2014-06-27  : linlan*/            
            gsm_fcp_get_latest_frame_pos(stp_latest_frame_pos_list,&st_latest_fn_pos);

            /*检测是否存在G+G,搜网与PS流程同时并发的处理*/       
            stp_gsm_ps_signal_state_ind = gsm_get_ps_signal_state(u16_standby_id);

            if(OAL_FALSE  == stp_gsm_ps_signal_state_ind->b_in_ps_signal_state)/*不在PS_SIG流程下，才优先配置收网*/
            {
                u32_fcp_ret = gsm_fcp_cellsearch_in_transfer(u32_next_fn,stp_oal_msg,u16_standby_id,&st_latest_fn_pos);                
            }
            /* FIX PhyFA-Bug00005191    BEGIN   2014-07-03  : linlan*/
            else/*如果ps过程中进入PS_SIG流程，需要把之前配置的前景搜网或者无网络收网的事件删除*/
            {
                gsm_del_cellsearch_fcb();
            }
            /* FIX PhyFA-Bug00005191    END     2014-07-03  : linlan*/
            oal_mem_free((CONST_VOID_PTR*)&stp_latest_frame_pos_list);  
            if(OAL_SUCCESS== u32_fcp_ret)
            {
                return OAL_SUCCESS;
            }
            /*GSM PS +其他模式前景搜网或者无网络搜网的处理 */            
            /*处理原则为PS_SIG > FG_SEARCH or NOCELL_SEARCH > PS_CONN, L1CC保证查询到PS_SIG后，不再尝试激活任何搜网配置的事物 */
            if(OAL_FALSE  == stp_gsm_ps_signal_state_ind->b_in_ps_signal_state)/*FG_SEARCH or NOCELL_SEARCH > PS_CONN*/
            {
                u32_fcp_ret = gsm_check_other_affairflow_higher_then_ps_data(u16_standby_id);
                if(OAL_SUCCESS== u32_fcp_ret)
                {
                    return OAL_SUCCESS;
                }
            }
            
            /*gsm_fcp_add_frametask(TSNTASK_TC_BCCH_RX,u32_next_fn,u16_standby_id);*/
            /*FIX LM-Bug00002137   END   2012-06-11  : sunzhiqiang*/
            /*FIX LM-Bug00001026   END   2012-02-14  : linlan*/
            stp_gsm_edge_loopback_tbl = gsm_get_edge_loopback_tbl();
            /*gsm_fcp_combine_frametask(stp_tsn_prios,GSM_TRANSFER,u32_next_fn);for PTCCH*/

            if(OAL_SUCCESS == u32_pdtch_rtx_allowed)
            {

                gsm_add_rtx_tsntask_in_transfer(u32_next_fn);
            }

            /* FIX PhyFA-Req00000825 BEGIN  2011-01-10 : wuxiaorong */           
            /* Fix PhyFA Req00000171 begin, guxiaobo, 20080826*/
            #ifdef PECKER_SWITCH 
            stp_gsm_ded_ch_report_ind = gsm_get_pecker_ded_ch_report_ind();
            if((UINT16)0 != stp_gsm_ded_ch_report_ind->u16_report_ctrl)
            {
                gsm_fcp_ded_ch_report_ind(u32_curr_fn, GSM_TRANSFER, u16_standby_id);
            }
            #endif
            /* Fix PhyFA Req00000171 end, guxiaobo, 20080826*/
            /* FIX PhyFA-Req00000825 END  2011-01-10 : wuxiaorong */
            
            gsm_fcp_set_meas_task(u32_next_fn, stp_latest_rtx_frame_pos);


            /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            {
                stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
                stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn+(UINT32)1),u16_active_id);
                
                /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                  stp_gsm_main_frametask_tmp1,
                                  &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
            }
            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/


            /* FIX A2KPH-Bug00000377  BEGIN    2008-06-17  : ningyaojun*/
            /* FIX PhyFA-Bug00000665   BEGIN   2008-08-06  : ningyaojun*/             
            if(OAL_SUCCESS == gsm_get_pdchpara_from_gprstb(&stp_pdtch_prarms, gsm_fn_operate(((u32_curr_fn + (UINT32)BURST_NUM_PER_BLOCK) + (UINT32)GSM_DATA_ENC_ADVANCE) + (UINT32)1)))
            {
               u16_mac_mode = stp_pdtch_prarms->u16_type_of_allocation;
            }
            else
            {
               u16_mac_mode = INVALID_MAC_MODE;
            }            

            /* FIX PhyFA-Req00000105    BEGIN   2008-05-24  : ningyaojun*/
            if(stp_gsm_edge_loopback_tbl->u16_loopback_state != (UINT16)EDGE_LOOPBACK_ON)
            /* FIX PhyFA-Req00000105    BEGIN   2008-05-24  : ningyaojun*/
            {
                gsm_add_enc_tsntask_in_transfer(u32_next_fn,u16_mac_mode);
            }
            break;

        case MSG_GSM_EQ_FCP_USF_DEC_IND:
            
            /* FIX PhyFA-Bug00000809   BEGIN   2008-10-27  : ningyaojun*/
            stp_usf_ind = (msg_gsm_eq_fcp_usf_ind_t *)OAL_GET_OAL_MSG_BODY(stp_oal_msg);
            u32_next_block_fn = gsm_fcp_get_next_block_fn(stp_usf_ind->u32_fn);
            u32_pdtch_rtx_allowed = gsm_get_pdchpara_from_gprstb(&stp_pdtch_prarms,u32_next_block_fn); 
            /*  FIX A2KPH-Bug00002339  BEGIN 2009-07-20: songzhiyuan*/ 
            gsm_get_pdchpara_from_gprstb(&stp_usf_pdtch_prarms,stp_usf_ind->u32_fn); 
            /*  FIX A2KPH-Bug00002339  END 2009-07-20: songzhiyuan*/ 
            if(OAL_SUCCESS == u32_pdtch_rtx_allowed)
            {
                u16_start_tsn = stp_pdtch_prarms->u16_tx_start_tsn;
                u16_end_tsn = stp_pdtch_prarms->u16_tx_end_tsn;
            }
            else
            {
                break;
            }
            u16_mac_mode = stp_pdtch_prarms->u16_type_of_allocation;
            /* FIX PhyFA-Bug00000809   END     2008-10-27  : ningyaojun*/
            
            /* FIX LM-Bug00000447  BEGIN    2011-03-25  : wanghuan */ 
            /*FIX LM-Bug00002571  BEGIN   2013-01-16  : ningyaojun*/
            u32_interval_fn = gsm_calc_interval_from_fn2peiodpos(u32_next_block_fn,u32_curr_fn, (UINT32)GSM_MAX_FN);
            if(  (gsm_fn_compare(u32_curr_fn, u32_next_block_fn) || (u32_next_block_fn == u32_curr_fn)) 
               &&((UINT32)2 >= u32_interval_fn))
            /*FIX LM-Bug00002571  END     2013-01-16  : ningyaojun*/
            {
                ;
            }
            else/*if the usf_ind is out of date,discard it and break directly after a GSM_FCP_TRANS_READY_IND is reported */
            {
                /*FIX PhyFA-Enh00001343 BEGIN 2012-09-03: dujianzhong */
                OAL_PRINT(u16_standby_id,(UINT16)GSM_MODE,"Usf_ind is too late, u32_next_block_fn=0x%lx, u32_curr_fn =0x%lx\r\n",(UINT32)u32_next_block_fn,(UINT32)u32_curr_fn); 
                /*FIX PhyFA-Enh00001343 END   2012-09-03: dujianzhong */
                if( ((UINT16)DYNAMIC_MODE == u16_mac_mode) || ((UINT16)EXT_DYNAMIC_MODE == u16_mac_mode) )
                {
                    /*u32_gsm_fcp_req_blks and s_u16_gsm_fcp_pdtch_sent_blks[u16_active_id] are set to 0*/  
                    gsm_fcp_handle_hls_res((UINT16)GSM_FCP_TRANS_READY_IND,u32_next_block_fn,(UINT32)0,u16_standby_id);          
                }                                                                        /*because that just fn is needed to report to HLS.*/
                break;
            }

            /* FIX PhyFA-Bug00000572   BEGIN  2008-07-05  : huangjinfu */
            s16_rtx_offset = -(SINT16)stp_gprs_pdtch_tb->u16_pdtch_ta;
            /* FIX PhyFA-Bug00000572   END  2008-07-05  : huangjinfu */
            /* Fix A2KPH Bug00001028 begin, guxiaobo, 20081028*/
            u32_gsm_fcp_req_blks = (UINT32)0;
            u16p_gsm_pdtch_sent_blks = gsm_get_pdtch_sent_blks();
            *u16p_gsm_pdtch_sent_blks = 0;
            /* Fix A2KPH Bug00001028 end, guxiaobo, 20081028*/

            /* FIX MS_Enh00000011    BEGIN   2009-03-28  : fushilong*/
            if((UINT16)1==stp_gprs_pdtch_tb->u16_test_mode_b_valid)
            {
                for(u16_tsn_no=u16_start_tsn;u16_tsn_no<=u16_end_tsn;u16_tsn_no++)
                {
                    stp_usf_ind->u16_usf[u16_tsn_no]= stp_pdtch_prarms->u16_usf_list[u16_tsn_no];
                }
            }
            /* FIX MS_Enh00000011    END   2009-03-28  : fushilong*/

            switch(u16_mac_mode)
            {
                case DYNAMIC_MODE:
                    for(u16_tsn_no=u16_start_tsn;u16_tsn_no<=u16_end_tsn;u16_tsn_no++)
                    {
                        /* FIX PhyFA-Bug00000249   BEGIN  2008-03-31  : huangjinfu */
                        /* FIX PHYFA-Bug00000666 BEGIN 2008-08-20: Han Haitao */
                        /* FIX A2KPH-Bug00001239   BEGIN   2008-12-09  : ningyaojun*/
                        stp_encbuff_info = NULL_PTR;
                        /* FIX A2KPH-Bug00001239   END     2008-12-09  : ningyaojun*/
                        for(i=0;i<=(u16_end_tsn-u16_start_tsn);i++)
                        {
                            stp_gprs_fcp_encbuff_info = gsm_get_encbuff_info(i);
                            if((UINT16)GSM_DATA_ENCODED == stp_gprs_fcp_encbuff_info->u16_data_state)
                            {
                                stp_encbuff_info = stp_gprs_fcp_encbuff_info;
                                break;
                            }
                        }
                        /* FIX PHYFA-Bug00000666 END 2008-08-20: Han Haitao */
                        if((UINT16)GSM_FCP_TSN_VALID == stp_pdtch_prarms->u16_ul_tsn_valid_list[u16_tsn_no])
                        {
                            /*  FIX LM-Enh00000738    BEGIN 2012-05-30: wanghuan*/
                            if(stp_usf_ind->u16_usf[u16_tsn_no] == stp_usf_pdtch_prarms->u16_usf_list[u16_tsn_no])
                            {
                                u16_usf_detected =1;
                            }
                            /*  FIX LM-Enh00000738    END   2012-05-30: wanghuan*/
                            /* FIX A2KPH-Bug00001239   BEGIN   2008-12-09  : ningyaojun*/
                            /*  FIX A2KPH-Bug00002339  BEGIN 2009-07-20: songzhiyuan*/ 
                            u32_judge_usf = gsm_fcp_judge_usf(stp_usf_pdtch_prarms,u16_tsn_no,stp_usf_ind->u16_usf[u16_tsn_no]);
                            stp_gprs_rrbp_encbuff_info = gsm_get_rrbp_encbuff_info((UINT16)((u16_tsn_no+ (UINT16)3) % (UINT16)4));

                            if((OAL_SUCCESS == u32_judge_usf)
                                &&(NULL_PTR != stp_encbuff_info)
                                &&((UINT16)GSM_DATA_ENCODED == stp_encbuff_info->u16_data_state) 
                                &&((u32_next_block_fn != stp_gprs_rrbp_encbuff_info->u32_rrbp_fn)
                                   ||((u16_tsn_no+(UINT16)3) != stp_gprs_rrbp_encbuff_info->u16_tsn))
                              )
                            {
                                /* FIX PhyFA-Req00001525 BEGIN 2013-09-07 : gaowu */
                                /* 第一次进入时将start_tsn置起 */
                                if ((UINT16)0 == u16_usf_valid_cnt)
                                {
                                    stp_gprs_pdtch_tb->u16_next_block_tx_start_tsn = u16_tsn_no;
                                }
                                stp_gprs_pdtch_tb->u32_next_block_fn = u32_next_block_fn;
                                stp_gprs_pdtch_tb->u16_next_block_tx_flag = (UINT16)1;
                                stp_gprs_pdtch_tb->u16_next_block_tx_ts_num++;
                                u16_usf_valid_cnt++;
                                /* FIX PhyFA-Req00001525 END   2013-09-07 : gaowu */
                                st_frame_pos.s16_offset = s16_rtx_offset;
                                st_frame_pos.u16_tsn = (UINT16)(u16_tsn_no+(UINT16)GSM_UL_DELAY_TSN_NUM);
                                st_frame_pos.u32_fn = u32_next_block_fn;

                                /*  FIX PhyFA-Bug00003606    BEGIN   2012-07-26: wangjunli*/
                                /* FIX LTE-Enh00000846 BEGIN  2014-04-29: wangjunli */
                                if(OAL_SUCCESS == gsm_fcp_check_add_tsntask((UINT16)u16_pdch_tx_task,&st_frame_pos,u16_standby_id))
                                /* FIX LTE-Enh00000846 END  2014-04-29: wangjunli */
                                {
                                    /* FIX HSDPA-Bug00000419   BEGIN   2008-10-20  : ningyaojun*/
                                    //u16_add_txslot_bitmap |= 1 << u16_tsn_no;
                                    u16_add_txslot_bitmap |= (UINT16)1 << u16_tsn_no;
                                    /* FIX HSDPA-Bug00000419   END     2008-10-20  : ningyaojun*/
                                    if(((UINT16)GSM_DATA_SENT == stp_encbuff_info->u16_data_state) && (stp_encbuff_info->u16_not_dummy_blk))
                                    {
                                        (*u16p_gsm_pdtch_sent_blks)++;
                                    }
                                }
                                /*  FIX PhyFA-Bug00003606    END   2012-07-26: wangjunli*/
                            }
                            else
                            {
                                stp_gprs_pdtch_tb->u16_next_block_tx_flag =  (UINT16)0;
                            }
                        }
                        /* FIX PhyFA-Bug00000249   END  2008-03-31  : huangjinfu */

                        /* Fix A2KPH Bug00001028 begin, guxiaobo, 20081028*/
                        if((UINT16)GSM_DATA_UNENCODED != stp_gprs_pdtch_tb->st_pdtch_data[u16_tsn_no - u16_start_tsn].u16_pdtch_data_state)
                        {
                            u32_gsm_fcp_req_blks++;
                        }
                        /* Fix A2KPH Bug00001028 end, guxiaobo, 20081028*/
                    }
                    break;
                case EXT_DYNAMIC_MODE:
                    u16_higer_tsn_allowed = GPRS_HIGER_TSN_NOT_ALLOWED;
                    for(u16_tsn_no=u16_start_tsn;u16_tsn_no<=u16_end_tsn;u16_tsn_no++)
                    {
                        /* FIX PhyFA-Bug00000249   BEGIN  2008-03-31  : huangjinfu */
                        /* FIX A2KPH-Bug00001788 BEGIN  2009-03-21  : wuxiaorong */
                        if((UINT16)GSM_FCP_TSN_VALID == stp_pdtch_prarms->u16_ul_tsn_valid_list[u16_tsn_no])
                        {
                            /*  FIX LM-Enh00000738    BEGIN 2012-05-30: wanghuan*/
                            if(stp_usf_ind->u16_usf[u16_tsn_no] == stp_usf_pdtch_prarms->u16_usf_list[u16_tsn_no])
                            {
                                u16_usf_detected =1;
                            }
                            /*  FIX LM-Enh00000738    END   2012-05-30: wanghuan*/
                            if((OAL_SUCCESS == gsm_fcp_judge_usf(stp_pdtch_prarms,u16_tsn_no,stp_usf_ind->u16_usf[u16_tsn_no]))
                              ||(u16_higer_tsn_allowed == (UINT16)GPRS_HIGER_TSN_ALLOWED))
                            {
                                u16_higer_tsn_allowed = GPRS_HIGER_TSN_ALLOWED;
                                /* FIX PhyFA-Bug00000317   BEGIN  2008-04-07  : huangjinfu */
                                /* FIX PhyFA-Enh00000105  2008-04-23  : huangjinfu */
                                /*data must not be sent if gprs data is not encoded for Enh00000105*/

                                /* FIX PhyFA-Req00001525 BEGIN 2013-09-07 : gaowu */
                                /* 第一次进入时将start_tsn置起 */
                                if ((UINT16)0 == u16_usf_valid_cnt)
                                {
                                    stp_gprs_pdtch_tb->u16_next_block_tx_start_tsn = u16_tsn_no;
                                }
                                stp_gprs_pdtch_tb->u32_next_block_fn = u32_next_block_fn;
                                stp_gprs_pdtch_tb->u16_next_block_tx_flag = (UINT16)1;
                                stp_gprs_pdtch_tb->u16_next_block_tx_ts_num++;
                                u16_usf_valid_cnt++;
                                /* FIX PhyFA-Req00001525 END   2013-09-07 : gaowu */
                                
                                stp_encbuff_info = NULL_PTR;
                                for(i=0;i<=(u16_end_tsn-u16_start_tsn);i++)
                                {
                                    stp_gprs_fcp_encbuff_info = gsm_get_encbuff_info(i);

                                    if((UINT16)GSM_DATA_ENCODED == stp_gprs_fcp_encbuff_info->u16_data_state)
                                    {
                                        stp_encbuff_info = stp_gprs_fcp_encbuff_info;
                                        break;
                                    }
                                } 

                                stp_gprs_rrbp_encbuff_info = gsm_get_rrbp_encbuff_info((UINT16)((u16_tsn_no+ (UINT16)3) % (UINT16)4));

                                if((NULL_PTR != stp_encbuff_info)
                                &&((UINT16)GSM_DATA_ENCODED == stp_encbuff_info->u16_data_state) 
                                &&((u32_next_block_fn != stp_gprs_rrbp_encbuff_info->u32_rrbp_fn)
                                ||((u16_tsn_no+(UINT16)3) != stp_gprs_rrbp_encbuff_info->u16_tsn))
                                /* FIX A2KPH-Bug00001788 END  2009-03-21  : wuxiaorong */
                                   )
                                {
                                    st_frame_pos.s16_offset = s16_rtx_offset;
                                    st_frame_pos.u16_tsn = (UINT16)(u16_tsn_no +(UINT16)GSM_UL_DELAY_TSN_NUM);
                                    st_frame_pos.u32_fn = u32_next_block_fn;
                                
                                    /* FIX LTE-Enh00000846 BEGIN  2014-04-29: wangjunli */
                                    if((OAL_SUCCESS == gsm_fcp_check_add_tsntask(u16_pdch_tx_task,
                                                                                &st_frame_pos,u16_standby_id)))
                                    /* FIX LTE-Enh00000846 END  2014-04-29: wangjunli */
                                    {
                                        /* FIX HSDPA-Bug00000419   BEGIN   2008-10-20  : ningyaojun*/                               
                                        u16_add_txslot_bitmap |= (UINT16)1 << u16_tsn_no;
                                        /* FIX HSDPA-Bug00000419   END     2008-10-20  : ningyaojun*/                                       
                                        if(((UINT16)GSM_DATA_SENT == stp_encbuff_info->u16_data_state) && (stp_encbuff_info->u16_not_dummy_blk))
                                        {
                                            (*u16p_gsm_pdtch_sent_blks)++;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                 stp_gprs_pdtch_tb->u16_next_block_tx_flag =  (UINT16)0;
                            }
                        }
                        /* Fix A2KPH Bug00001028 begin, guxiaobo, 20081028*/
                        if((UINT16)GSM_DATA_UNENCODED != stp_gprs_pdtch_tb->st_pdtch_data[u16_tsn_no - u16_start_tsn].u16_pdtch_data_state)
                        {
                            u32_gsm_fcp_req_blks++;
                        }                        
                        /* Fix A2KPH Bug00001028 end, guxiaobo, 20081028*/
                        /* FIX PhyFA-Bug00000249   END  2008-03-31  : huangjinfu */
                    }
                    break;
                case SINGLE_BLOCK:
                case MULTI_BLOCK:
                    break;
                default:
                    oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
                    return OAL_FAILURE;
            }
            
            u32p_block_fn_from_frame_timer = gsm_get_block_fn_from_frame_timer();
            u32p_block_fn_from_usf_messge = gsm_get_block_fn_from_usf_message();
            
            /* FIX PhyFA-Enh00000090   BEGIN  2008-03-31  : huangjinfu */
            *u32p_block_fn_from_usf_messge = u32_next_block_fn;
            if((*u32p_block_fn_from_frame_timer == *u32p_block_fn_from_usf_messge)
                &&(stp_pdtch_prarms->u16_rtx_start_tsn < stp_pdtch_prarms->u16_rtx_end_tsn))
            {
                /* FIX PhyFA-Req00000108    BEGIN   2008-07-17  : ningyaojun*/
                gsm_fcp_modify_pdtch_frametask(*u32p_block_fn_from_usf_messge,
                                               (UINT16)(stp_pdtch_prarms->u16_rtx_start_tsn+(UINT16)GSM_UL_DELAY_TSN_NUM),
                                               (UINT16)(stp_pdtch_prarms->u16_rtx_end_tsn+(UINT16)GSM_UL_DELAY_TSN_NUM)
                                              );


                /* FIX PhyFA-Req00000108    BEGIN   2008-07-17  : ningyaojun*/
             }

            /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
            u16_add_txslot_bitmap = (u16_add_txslot_bitmap<<3);
            if((UINT16)0 != u16_add_txslot_bitmap) 
            {
                stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_block_fn,u16_active_id);
                stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_block_fn+(UINT32)1),u16_active_id);
                if((UINT16)GSM_TSNTASK_DOWNLOADED == stp_gsm_main_frametask_tmp0->u16_downloaded_ind)
                {           
                    /*FIX PhyFA-Req00001045  BEGIN   2011-09-09  : ningyaojun*/
                    if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_block_fn, (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                    /*FIX PhyFA-Req00001045  END     2011-09-09  : ningyaojun*/ 
                    {
                        gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                          stp_gsm_main_frametask_tmp1,
                                          &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
                        /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
                        #if 0
                        if (OAL_SUCCESS == u32_ret)
                        {
                            oal_tpc_trace_data(MSG_GSM_FCP_PSD_FRAMETASK_INFO,
                                                (UINT16_PTR)stp_gsm_main_frametask_tmp0,
                                                (UINT16)(sizeof(gsm_main_frametask_t)/sizeof(UINT16)),
                                                u32_curr_fn,
                                                u16_standby_id,
                                                (UINT16)GSM_MODE);
                        }
                        #endif
                        /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
                    }

                    if((UINT16)GSM_TSNTASK_DOWNLOADED == stp_gsm_main_frametask_tmp1->u16_downloaded_ind)
                    {           
                        if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(gsm_fn_operate(u32_next_block_fn+(UINT32)1), (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                        {
                            gsm_rtx_event_req(stp_gsm_main_frametask_tmp1,
                                              gsm_get_main_frametask(gsm_fn_operate(u32_next_block_fn+(UINT32)2), u16_active_id),
                                              &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
                            /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
                            #if 0
                            if (OAL_SUCCESS == u32_ret)
                            {
                                oal_tpc_trace_data(MSG_GSM_FCP_PSD_FRAMETASK_INFO,
                                                   (UINT16_PTR)stp_gsm_main_frametask_tmp1,
                                                   (UINT16)(sizeof(gsm_main_frametask_t)/sizeof(UINT16)),
                                                   u32_curr_fn,
                                                   u16_standby_id,
                                                   (UINT16)GSM_MODE);
                            }
                            #endif
                            /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
                        }                 
                    }                

                }             
            }
            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/

            /* Fix PhyFA Enh00000257 begin, guxiaobo, 20081101*/
            if( ((UINT16)DYNAMIC_MODE == u16_mac_mode) || ((UINT16)EXT_DYNAMIC_MODE == u16_mac_mode) )
            {
                /*  FIX LM-Enh00000738    BEGIN 2012-05-30: wanghuan*/
                /* Fix A2KPH Bug00001028 begin, guxiaobo, 20081028*/
                gsm_fcp_handle_hls_res((UINT16)GSM_FCP_TRANS_READY_IND,u32_next_block_fn,((UINT32)u16_usf_detected<<16)+(u32_gsm_fcp_req_blks << 8) + (UINT32)*u16p_gsm_pdtch_sent_blks,u16_standby_id);            
                /* Fix A2KPH Bug00001028 end, guxiaobo, 20081028*/
                /*  FIX LM-Enh00000738    END   2012-05-30: wanghuan*/
            }
            /* Fix PhyFA Enh00000257 end, guxiaobo, 20081101*/
            break;

        /* FIX PhyFA-Req00000105    BEGIN   2008-05-24  : ningyaojun*/
        case MSG_GSM_EQ_FCP_LOOPBACK_IND:   
            stp_gsm_edge_loopback_tbl = gsm_get_edge_loopback_tbl();
            if( (UINT16)EDGE_LOOPBACK_ON == stp_gsm_edge_loopback_tbl->u16_loopback_state)
            {

                stp_eq_fcp_loopback_ind =(msg_gsm_eq_fcp_loopback_ind_t*)OAL_GET_OAL_MSG_BODY(stp_oal_msg);
                u32_next_block_fn = stp_eq_fcp_loopback_ind->u32_next_block_fn;

                /* FIX A2KPH-Bug00001579   BEGIN   2009-02-27  : ningyaojun*/
                u32_pdtch_rtx_allowed = gsm_get_pdchpara_from_gprstb(&stp_pdtch_prarms,u32_next_block_fn);
                /* FIX A2KPH-Bug00001579   END     2009-02-27  : ningyaojun*/

                u16_start_tsn = stp_pdtch_prarms->u16_tx_start_tsn;
                u16_end_tsn =   stp_pdtch_prarms->u16_tx_end_tsn;

                /* FIX HSDPA-Bug00000419   BEGIN   2008-10-20  : ningyaojun*/
                u16_add_txslot_bitmap = 0x00;
                /* FIX HSDPA-Bug00000419   END     2008-10-20  : ningyaojun*/
                s16_rtx_offset = -(SINT16)stp_gprs_pdtch_tb->u16_pdtch_ta;
                if(  (OAL_SUCCESS == u32_pdtch_rtx_allowed)
                   &&((stp_pdtch_prarms->u16_l1_direction & (UINT16)0x02) == (UINT16)0x02)
                  )
                {
                    for(u16_tsn_no=u16_start_tsn; u16_tsn_no<=u16_end_tsn; u16_tsn_no++)
                    {
                        if((UINT16)GSM_FCP_TSN_VALID == stp_pdtch_prarms->u16_ul_tsn_valid_list[u16_tsn_no])
                        {
                            st_frame_pos.s16_offset = s16_rtx_offset;
                            st_frame_pos.u16_tsn = (UINT16)(u16_tsn_no+(UINT16)GSM_UL_DELAY_TSN_NUM);
                            st_frame_pos.u32_fn = u32_next_block_fn;
                                    
                            /* FIX LTE-Enh00000846 BEGIN  2014-04-29: wangjunli */
                            gsm_fcp_check_add_tsntask(u16_pdch_tx_task,
                                                      &st_frame_pos,
                                                      u16_standby_id);
                            /* FIX LTE-Enh00000846 END  2014-04-29: wangjunli */
                        }
                        /* FIX HSDPA-Bug00000419   BEGIN   2008-10-20  : ningyaojun*/
                        u16_add_txslot_bitmap |= (UINT16)1 << u16_tsn_no;
                        /* FIX HSDPA-Bug00000419   END     2008-10-20  : ningyaojun*/
                    }
                }

                if(stp_pdtch_prarms->u16_rtx_start_tsn < stp_pdtch_prarms->u16_rtx_end_tsn)
                {

                    /* FIX PhyFA-Req00000108    BEGIN   2008-07-17  : ningyaojun*/
                    gsm_fcp_modify_pdtch_frametask((u32_next_block_fn),
                                                   (UINT16)(stp_pdtch_prarms->u16_rtx_start_tsn+(UINT16)GSM_UL_DELAY_TSN_NUM),
                                                   (UINT16)(stp_pdtch_prarms->u16_rtx_end_tsn+(UINT16)GSM_UL_DELAY_TSN_NUM));
                    /* FIX PhyFA-Req00000108    END     2008-07-17  : ningyaojun*/
                }

                if((UINT16)0 != u16_add_txslot_bitmap) 
                {
                    stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_block_fn,u16_active_id);
                    stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_block_fn+(UINT32)1),u16_active_id);
                    if((UINT16)GSM_TSNTASK_DOWNLOADED == stp_gsm_main_frametask_tmp0->u16_downloaded_ind)
                    {           
                        /* FIX LTE-Bug00002799 BEGIN  2014-03-24  : xiongjiangjiang */
                        if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_block_fn, (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                        /* FIX LTE-Bug00002799 END  2014-03-24  : xiongjiangjiang */
                        {
                            gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                              stp_gsm_main_frametask_tmp1,
                                              &u16_ts_bitmap[0], &u16_ts_bitmap[1]);

                            /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
                            #if 0
                            if (OAL_SUCCESS == u32_ret)
                            {
                                oal_tpc_trace_data(MSG_GSM_FCP_PSD_FRAMETASK_INFO,
                                                   (UINT16_PTR)stp_gsm_main_frametask_tmp0,
                                                   (UINT16)(sizeof(gsm_main_frametask_t)/sizeof(UINT16)),
                                                   u32_curr_fn,
                                                   u16_standby_id,
                                                   (UINT16)GSM_MODE);
                            }
                            #endif
                            /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
                                                    
                        }

                        if((UINT16)GSM_TSNTASK_DOWNLOADED == stp_gsm_main_frametask_tmp1->u16_downloaded_ind)
                        {           
                            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(gsm_fn_operate(u32_next_block_fn+(UINT32)1), (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                            {
                                gsm_rtx_event_req(stp_gsm_main_frametask_tmp1,
                                                  gsm_get_main_frametask(gsm_fn_operate(u32_next_block_fn+(UINT32)2),u16_active_id),
                                                  &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
                                /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
                                #if 0
                                if (OAL_SUCCESS == u32_ret)
                                {
                                    oal_tpc_trace_data(MSG_GSM_FCP_PSD_FRAMETASK_INFO,
                                                       (UINT16_PTR)stp_gsm_main_frametask_tmp1,
                                                       (UINT16)(sizeof(gsm_main_frametask_t)/sizeof(UINT16)),
                                                       u32_curr_fn,
                                                       u16_standby_id,
                                                       (UINT16)GSM_MODE);
                                }
                                #endif
                                /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
                            }                 
                        }                

                    }             
                }
                /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/  
            }         
            break;
        /*FIX LM-Bug00001481  BEGIN   2011-12-27  : sunzhiqiang*/
        case MSG_GSM_MEAS_FCP_FCBSUCC_IND:
            
            stp_fcbsucc_ind = (msg_gsm_meas_l1cc_fcb_result_ind_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));
            u16_standby_id = stp_fcbsucc_ind->u16_standby_id;
            u16_rxtask_type = stp_fcbsucc_ind->u16_rxtask_type;
            if((UINT16)GSM_CELLSEARCH == u16_rxtask_type)
            {
                stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();
                if((UINT16)1 == stp_cellsearch_ctrl->u16_valid_flag)
                {
                    stp_gsm_cellsearch = gsm_get_cellsearch_tbl_by_standby(u16_standby_id);    
                    if((UINT16)1 == stp_gsm_cellsearch->u16_valid_flag)
                    {
                        u16_gsm_bg_state = stp_gsm_cellsearch->u16_bg_state;
                        if((UINT16)GSM_BGS_NON != u16_gsm_bg_state)
                        {
                            if(NOCELL_SEARCH == stp_gsm_cellsearch->u16_search_type)
                            {
                                u32_fcp_ret = gsm_fcp_nocell_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                            }
                            else
                            {
                                u32_fcp_ret = gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                            }
                            if(OAL_SUCCESS == u32_fcp_ret)
                            {
                                stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_calculated_sb_fn,u16_active_id);
                                stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_calculated_sb_fn+(UINT32)1),u16_active_id);
                                if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_calculated_sb_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                                {
                                    gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                                      stp_gsm_main_frametask_tmp1,
                                                      &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
                                    /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
                                    #if 0
                                    if (OAL_SUCCESS == u32_ret)
                                    {
                                        oal_tpc_trace_data(MSG_GSM_FCP_PSD_FRAMETASK_INFO,
                                                           (UINT16_PTR)stp_gsm_main_frametask_tmp0,
                                                           (UINT16)(sizeof(gsm_main_frametask_t)/sizeof(UINT16)),
                                                           u32_curr_fn,
                                                           u16_standby_id,
                                                           (UINT16)GSM_MODE);
                                    }
                                    #endif
                                    /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
                                }   
                            }
                        }
                    }
                }      
            }
            else
            {
                OAL_PRINT(u16_standby_id,(UINT16)GSM_MODE,"in transfer state,fcbsucc_ind msg's rxtask_type:%ld",(UINT32)u16_rxtask_type);
            }               
            break;
        /*FIX LM-Bug00001481  END   2011-12-27  : sunzhiqiang*/
        /* FIX PhyFA-Req00000105    BEGIN   2008-05-24  : ningyaojun*/
        default:
            /* FIX PhyFA-Enh00001698 BEGIN  2014-04-23  : wangjunli*/
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            OAL_PRINT((UINT16)OAL_GET_OAL_MSG_STANDBY_ID(stp_oal_msg),(UINT16)GSM_MODE,"gsm_fcp_transfer receive unexpected msg:%ld",OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg));
            /* FIX PhyFA-Enh00001698 END  2014-04-23  : wangjunli*/
            break;
    }
    return OAL_SUCCESS;
}



/************************************************************
** Function Name: gsm_fcp_idle ()
** Description: the handler of the state GSM_IDLE of the module FCP
** Input :
      stp_tsn_prios[10]: priority info of  [tsn7|tsn0|tsn1|tsn2|tsn3|tsn4|tsn5|tsn6|tsn7|tsn0]
      u32_next_fn:  the next frame number
      stp_oal_msg:        message received from other module

** Output : NULL

** Return:  OAL_SUCCESS or OAL_FAILURE
** Notes:
2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*   2014-07-25      wuxiaorong              PhyFA-Bug00005426   [NGM]gsm辅模式状态下的sbsearch流程的任务配置需要变更.
*   2014-07-30      wuxiaorong              PhyFA-Bug00005464   [NGM]GSM的SB search当帧配当帧睡眠控制latest_rtx_frame_pos设置有问题 
*   2014-08-05      wuxiaorong              PhyFA-Req00001925   [NGM]T业务下G测量在当前接收未完成前就需要帧中断下次接收任务 .
*   2014-08-12      xiongjiangjiang         PhyFA-Bug00005562   [NGM]GSM辅模式时需要增加在假的帧中断下用下一次的上报时间点来更新最近收发帧
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_idle(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos)
{
    /* FIX PhyFA-Bug00002799 BEGIN  2011-08-12 : wuxiaorong */ 
    UINT16 i=0,k=0;
    UINT32 u32_curr_fn;
    /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wuxiaorong */
    CONST msg_gsm_meas_l1cc_fcb_result_ind_t *stp_fcbsucc_ind =  NULL_PTR;
    /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */
    gsm_tstask_latest_frame_pos_t*  stp_latest_frame_pos_list = NULL_PTR;
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR;

    /* Fix PHYFA-Req00000384 BEGIN 2009-09-18 : guxiaobo*/
    OAL_STATUS u32_status;
    /* Fix PHYFA-Req00000384 END 2009-09-18 : guxiaobo*/

    /*FIX PhyFA-Req00000936  BEGIN   2011-07-12  : ningyaojun*/
    UINT16 u16_meas_standby_id = UNSPECIFIED_STANDBY_ID;
    /*FIX PhyFA-Req00000936  END     2011-07-12  : ningyaojun*/ 
    
    /*FIX PhyFA-Req00000936  BEGIN   2011-07-11  : ningyaojun*/
    rr_gphy_pdtch_conn_req_t   *stp_pdtch_conn_req  = NULL_PTR;
    pdtch_ch_ctrl_struct            *stp_pdtch_conn_para = NULL_PTR;    
    /*FIX PhyFA-Req00000936  END     2011-07-11  : ningyaojun*/

    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    UINT32  u32_conn_ind_fn  = (UINT32)GSM_INVALID_FN;    
   
    UINT16  u16_standby_id      = UNSPECIFIED_STANDBY_ID;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp0 = NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp1 = NULL_PTR;    
    UINT16  u16_ts_bitmap[2] = {0,0}; 
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/ 

    /* FIX PhyFA-Req00000936 BEGIN  2011-06-09  : wuxiaorong */
    UINT16  u16_rxtask_type;
    UINT32  u32_calculated_sb_fn;
    /* FIX PhyFA-Req00000936 END  2011-06-09  : wuxiaorong */

   
    /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
    CONST time_info_struct* stp_timeinfo = NULL_PTR;
    UINT32 u32_enc_spec_blks = (UINT32)0;
    /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
    CONST gsm_nc_meas_sync_ctrl_t*  stp_nc_meas_sync_ctrl = NULL_PTR;
    /* FIX PhyFA-Req00001525 BEGIN 2013-11-05 : zhengying */
    CONST gsm_scell_info_t *stp_gsm_scell_info = NULL_PTR;
    /* FIX PhyFA-Req00001525 END   2013-11-05 : zhengying */
    CONST gsm_meas_req_t* stp_gsm_meas_req= NULL_PTR;
    CONST gsm_ncell_meas_req_t*   stp_ncell_meas_req = NULL_PTR;
    #ifdef PECKER_SWITCH  
    CONST gsm_ded_ch_report_ind_tb_t* stp_gsm_ded_ch_report_ind = NULL_PTR;
    #endif
    CONST gsm_cellsearch_ctrl_t *stp_cellsearch_ctrl = NULL_PTR;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    gprs_pdtch_tb_t *stp_gprs_pdtch_tb = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID; 
    /* FIX PhyFA-Bug00005665 BEGIN 2014-09-01 : wuxiaorong */
    SINT16  s16_fn_begin_offset= 0;
    /* FIX PhyFA-Bug00005665 END   2014-09-01 : wuxiaorong */
    /* FIX NGM-Bug00000585 BEGIN 2014-09-20 : wuxiaorong */
    UINT16  u16_rtx_bitmap = 0;
    /* FIX NGM-Bug00000585 END   2014-09-20 : wuxiaorong */


    OAL_ASSERT(NULL_PTR!=stp_oal_msg, "gsm_fcp_idle stp_oal_msg=null");    
    OAL_ASSERT(NULL_PTR!= stp_gsm_latest_rtx_frame_pos,"gsm_fcp_idle():stp_gsm_latest_rtx_frame_pos is NULL_PTR!");    
    stp_gsm_standby_info = gsm_get_standby_info(); 
    stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();
    #ifdef PECKER_SWITCH  
    stp_gsm_ded_ch_report_ind = gsm_get_pecker_ded_ch_report_ind();
    #endif
    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
    stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */
    
    stp_nc_meas_sync_ctrl = gsm_get_nc_meas_sync_ctrl();
    switch(OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg))
    {
        case MSG_ISR_FCP_FRAMETIMER_IND: 
            u32_curr_fn = gsm_fn_operate((u32_next_fn+(UINT32)GSM_MAX_FN)-(UINT32)1);
            /* 申请内存 */
            stp_latest_frame_pos_list = (gsm_tstask_latest_frame_pos_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,
                                                                                  (UINT32)OAL_GET_TYPE_SIZE_IN_WORD(gsm_tstask_latest_frame_pos_t));  

            stp_latest_frame_pos_list->u16_latest_pos_num = 0;
            /* FIX LM-Bug00001045    BEGIN   2011-10-20  : linlan*/
            stp_gprs_pdtch_tb = gsm_get_gprs_cfg_tbl();
            
            if ((UINT16)1 == stp_gprs_pdtch_tb->u16_pdtch_tb_flg)
            {
                
                u16_standby_id = stp_gprs_pdtch_tb->u16_standby_id;
                /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
                stp_timeinfo = gsm_get_scell_timeinfo(u16_standby_id);
                if (((SINT32)0 != stp_timeinfo->s32_delta_fn)|| ((SINT16)0 != stp_timeinfo->s16_offset_in_ts ))
                {
                    gsm_send_frame_timing_change_req(u16_standby_id, GSM_ENTER_CONN);
                }
                /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
                #ifdef L1CC_GSM_RTRT_UT_SWITCH
                stub_gsm_fcp_enc_special_data(&u32_enc_spec_blks,(UINT16)TSNTASK_RRBP_TX0,u32_curr_fn,u16_standby_id);
                stub_gsm_fcp_add_special_frametask((UINT16)TSNTASK_RRBP_TX0,u32_next_fn,u16_standby_id);
                #else
                gsm_fcp_enc_special_data(&u32_enc_spec_blks,(UINT16)TSNTASK_RRBP_TX0,u32_curr_fn,u16_standby_id);
                gsm_fcp_add_special_frametask((UINT16)TSNTASK_RRBP_TX0,u32_next_fn,u16_standby_id);
                #endif
            }
            /* FIX LM-Bug00001045    END   2011-10-20  : linlan*/
            for(k = 0; k <(UINT16)MAX_STANDBY_AMT; k++)
            {   
                u16_standby_id = stp_gsm_standby_info->u16_standby_id[k];
                if((UINT16)GSM_MASTER_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[k])
                {
                    gsm_add_master_standby_idle_tsntask(u32_next_fn,u16_standby_id,stp_latest_frame_pos_list);
                    gsm_add_master_standby_nc_bcch_tsntask(u32_next_fn,u16_standby_id,stp_latest_frame_pos_list);
                    gsm_add_sync_cell_bcch_tsntask(u32_next_fn,u16_standby_id,stp_latest_frame_pos_list);
                    /* Fix LM-Enh00001221 BEGIN 2013-07-24 : wangjunli*/
                    /* 添加PLMN过程BCCH接收 */
                    gsm_add_master_standby_plmn_bcch_tsntask(u32_next_fn,u16_standby_id,stp_latest_frame_pos_list);
                    /* Fix LM-Enh00001221 END 2013-07-24 : wangjunli*/
                }
                /* FIX PhyFA-Req00001026 BEGIN 2012-04-10  : wangjunli*/
                /* 辅模式需要尝试SI预读任务TSNTASK_TC_BCCH_RX配置 */
                else if(GSM_SLAVE_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[k]) 
                {
                    gsm_add_slave_standby_tsntask(u32_next_fn,u16_standby_id,stp_latest_frame_pos_list); 
                }
                /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/ 
            }
            /* FIX PhyFA-Bug00002799  END  2011-08-12 : wuxiaorong */ 
           
            /*FIX LM-Bug00000928  BEGIN   2011-10-08  : wanghairong*/
            gsm_fcp_get_latest_frame_pos(stp_latest_frame_pos_list,stp_latest_rtx_frame_pos);

             /* 释放内存 */
            oal_mem_free((CONST_VOID_PTR*)&stp_latest_frame_pos_list);  
            
            /* FIX PhyFA-Req00000985 BEGIN  2011-07-25 : wuxiaorong */   
            OAL_ASSERT((stp_latest_rtx_frame_pos->u32_fn!= u32_next_fn),"gsm_fcp_cs_idle():g_st_latest_frame_pos[u16_active_id] calculated fail!");
            /* FIX PhyFA-Req00000985 END  2011-07-25 : wuxiaorong */
            /*FIX LM-Bug00000928  END   2011-10-08  : wanghairong*/
            /*FIX PhyFA-Req00000936  BEGIN   2011-07-11  : ningyaojun*/
            /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
            /*这里要不要调定时??*/
            /*u16_standby_id = stp_gsm_standby_info->u16_fg_standby_id; *///???
            if( ((UINT16)1 == stp_gprs_pdtch_tb->u16_pdtch_tb_flg) 
               &&((UINT16)PDTCH_CONNECT_REQ_VALID == stp_gprs_pdtch_tb->u16_pdtch_conn_valid[0]))                    
            {
                u16_standby_id = stp_gprs_pdtch_tb->u16_standby_id;
                /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
                /*FIX LM-Bug00000971  BEGIN  2011-10-13 :fushilong*/
                gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
                /*FIX LM-Bug00000971  END 2011-10-13 :fushilong*/
                u32_conn_ind_fn = gsm_fn_operate(u32_curr_fn + (UINT32)5);
                stp_pdtch_conn_req = &stp_gprs_pdtch_tb->st_pdtch_conn_req[0];
                stp_pdtch_conn_para = &stp_pdtch_conn_req->pdtch_chan_params;
                if(stp_pdtch_conn_para->is_starting_frame_valid)
                {
                    if(gprs_fcp_activetime_is_arrive(stp_pdtch_conn_para->tbf_starting_frame, u32_conn_ind_fn))
                    {
                        /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
                        stp_timeinfo = gsm_get_scell_timeinfo(u16_standby_id);
                        if (((SINT32)0 != stp_timeinfo->s32_delta_fn)|| ((SINT16)0 != stp_timeinfo->s16_offset_in_ts ))
                        {
                            gsm_send_frame_timing_change_req(u16_standby_id, GSM_ENTER_CONN);
                        }
                        /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
                        /* FIX PhyFA-Bug00000127      2008-01-17  : huangjinfu begin*/
                        gsm_send_state_change_req((UINT32)GSM_TRANSFER, u16_standby_id);
                        /* FIX PhyFA-Bug00000127      2008-01-17  : huangjinfu end*/
                        /* FIX PhyFA-Req00001525 BEGIN  2013-11-18 : wuxiaorong */
                        gsm_check_enter_ps_data_or_ps_signal_affairflow(u16_standby_id);
                        /* 重新申请afc的控制权 */
                        gsm_send_mmc_afc_ctrl_req(OAL_TRUE,u16_standby_id,(UINT16)L1CC_MMC_AFC_CTRL_PS_CAUSE);
                        /* FIX PhyFA-Req00001525 END  2013-11-18 : wuxiaorong */
                        break;
                    }
                }
                else
                {
                    /*FIX PhyFA-Req00001045  BEGIN   2011-09-09  : ningyaojun*/
                    oal_error_handler(ERROR_CLASS_MINOR, GSM_FCP_ERR(INVALID_PARAM));
                    stp_pdtch_conn_para->tbf_starting_frame = gsm_fcp_get_current_block_fn(u32_curr_fn);
                    stp_pdtch_conn_para->is_starting_frame_valid = 1;
                    /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
                    stp_timeinfo = gsm_get_scell_timeinfo(u16_standby_id);
                    if (((SINT32)0 != stp_timeinfo->s32_delta_fn)|| ((SINT16)0 != stp_timeinfo->s16_offset_in_ts ))
                    {
                        gsm_send_frame_timing_change_req(u16_standby_id, GSM_ENTER_CONN);
                    }
                    /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
                    gsm_send_state_change_req((UINT32)GSM_TRANSFER, u16_standby_id);
                    /* FIX PhyFA-Req00001525 BEGIN  2013-11-18 : wuxiaorong */
                    gsm_check_enter_ps_data_or_ps_signal_affairflow(u16_standby_id);
                    /* 重新申请afc的控制权 */
                    gsm_send_mmc_afc_ctrl_req(OAL_TRUE,u16_standby_id,(UINT16)L1CC_MMC_AFC_CTRL_PS_CAUSE);
                    /* FIX PhyFA-Req00001525 END  2013-11-18 : wuxiaorong */
                    break;                    
                    /*FIX PhyFA-Req00001045  END     2011-09-09  : ningyaojun*/ 
                }
            }
            /*FIX PhyFA-Req00000936  END     2011-07-11  : ningyaojun*/

            /* FIX NGM-Bug00000585 BEGIN 2014-09-20 : wuxiaorong */
            u16_rtx_bitmap = gsm_get_frame_task_rtx_bitmap(gsm_get_main_frametask(u32_next_fn,u16_active_id));
            if(0 != u16_rtx_bitmap)
            {
                /*FIX LM-Req00000372   BEGIN   2011-09-23  : linlan*/                
                /*FIX NGM-Bug00000358  BEGIN   2014-08-22  : linlan*/
                gsm_check_del_running_sync_nslot_task(u32_next_fn,u16_rtx_bitmap);
                /*gsm_del_fcb_task_reset_cellsearch_tb();  */              
                /*FIX NGM-Bug00000358  END     2014-08-22  : linlan*/
                /*FIX LM-Req00000372   END   2011-09-23  : linlan*/
                /*  FIX LM-Bug00001225  BEGIN  2011-11-10 :fushilong  */
                /*  FIX LM-Bug00001225  END 2011-11-10 :fushilong  */

            }
            /* FIX NGM-Bug00000585 END   2014-09-20 : wuxiaorong */
            
            /* FIX PhyFA-Req00000936 BEGIN  2011-06-09  : wuxiaorong */
            /* FIX LM-Bug00000955    BEGIN 2011-10-09: wuxiaorong */ 
            /*    FIX LM-Bug00001225  BEGIN  2011-11-10 :fushilong    */
            if(OAL_FALSE == check_gsm_cellsearch_fcb_running())
            /*    FIX LM-Bug00001225  END 2011-11-10 :fushilong    */    
            {
                /* FIX PhyFA-Bug00004912 BEGIN 2014-06-05 : wuxiaorong */
                /* FIX PhyFA-Bug00005665 BEGIN 2014-09-01 : wuxiaorong */
                /* 睡眠唤醒后的假的帧中断消息不能按普通方式配测量.需要限定范围 */
                if(OAL_FALSE == gsm_check_pseudo_frametimer(stp_oal_msg,&s16_fn_begin_offset))
                {
                    gsm_fcp_set_meas_task(u32_next_fn,stp_latest_rtx_frame_pos);
                }
                else
                {
                    /* s16_fn_begin_offset不为0时,只进行主、辅模式的ReBSIC和RSSI测量;s16_fn_begin_offset=0时,内部调用gsm_fcp_set_meas_task() */
                    gsm_fcp_set_selfwakeup_nextfn_bsic_rssi_meas_task(u32_next_fn,s16_fn_begin_offset,stp_latest_rtx_frame_pos);
                }
                /* FIX PhyFA-Bug00005665 END   2014-09-01 : wuxiaorong */
                /* FIX PhyFA-Bug00004912 END   2014-06-05 : wuxiaorong */
            }
            /* FIX LM-Bug00000955    END 2011-10-09: wuxiaorong */

            if((UINT16)1 == stp_cellsearch_ctrl->u16_valid_flag)
            {
                /* FIX PhyFA-Bug00002868  BEGIN  2011-09-01  : yuzhengfeng */
                /*  FIX LM-Bug00001225  BEGIN  2011-11-10 :fushilong  */
                if( /*  FIX LM-Bug00001598  BEGIN  2012-01-18 :sunzhiqiang  */
                  (OAL_FALSE == check_gsm_cellsearch_fcb_running())
                  /*  FIX LM-Bug00001598  END  2012-01-18 :sunzhiqiang  */
                  &&((UINT16)0 == stp_nc_meas_sync_ctrl->u16_running_bitmap))
                /*  FIX LM-Bug00001225  END 2011-11-10 :fushilong  */
                {
                   gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,NULL_PTR,stp_latest_rtx_frame_pos);
                    /* FIX LM-Bug00001469    BEGIN 2011-12-14: wuxiaorong */
                    if(OAL_FALSE == check_gsm_cellsearch_fcb_running())
                    {
                        gsm_fcp_nocell_process(u32_next_fn,stp_oal_msg,NULL_PTR,stp_latest_rtx_frame_pos);
                    }
                    /* FIX LM-Bug00001469    END 2011-12-14: wuxiaorong */
                }
                /* FIX PhyFA-Bug00002868  END  2011-09-01  : yuzhengfeng */ 
                /* FIX LM-Bug00001044    BEGIN 2011-10-19: wuxiaorong */ 
                else
                {
                   gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
                }
                /* FIX LM-Bug00001044    END 2011-10-19: wuxiaorong */ 
            }

            /* FIX PhyFA-Req00000936 END  2011-06-09  : wuxiaorong */

            /* FIX PhyFA-Req00000985 BEGIN  2011-07-25 : wuxiaorong */   
            OAL_ASSERT((stp_latest_rtx_frame_pos->u32_fn != u32_next_fn)&&(stp_latest_rtx_frame_pos->u32_fn  != (UINT32)GSM_INVALID_FN),"gsm_fcp_cs_idle():g_st_latest_frame_pos[u16_active_id] calculated fail!");

            /* FIX PhyFA-Req00000985 END  2011-07-25 : wuxiaorong */  

            /*FIX PhyFA-Req00000936  BEGIN   2011-07-12  : ningyaojun*/  
            /* 下面这种情况，需要保证不睡眠;至于MSG_GSM_FCP_L1RESP_MEAS_FN_IND，和后面测报检查统一发送 */
            for(i=(UINT16)0; i<(UINT16)MAX_STANDBY_AMT; i++)
            {
                /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : caisiwen */
                if(GSM_MASTER_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[i])
                {
                    /* FIX PhyFA-Req00001026 END 2012-04-10  : caisiwen */
                    u16_meas_standby_id = stp_gsm_standby_info->u16_standby_id[i];

                    stp_gsm_meas_req = gsm_get_gsm_meas_req();
                    stp_ncell_meas_req = gsm_get_ncell_meas_req(u16_meas_standby_id);
                    /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
                    /* 同时只支持一张卡存在PCCCH信道，因此增加一个break*/
                    /* FIX PhyFA-Bug00002795 BEGIN  2011-08-13 : wuxiaorong */ 
                    stp_gsm_scell_info = gsm_get_gsm_scell_info(u16_meas_standby_id);
                    if((((UINT16)0 != (stp_gsm_meas_req->u16_valid_bitmap&((UINT16)1<<u16_meas_standby_id)))
                    /* FIX PhyFA-Bug00002795 BEGIN  2011-08-13 : wuxiaorong */
                          &&(((UINT16)0 != stp_ncell_meas_req->u16_nc_mode)||((UINT16)1 == stp_gsm_scell_info->u16_pccch_exsit)))               
                       #ifdef PECKER_SWITCH  
                       ||((UINT16)0 != stp_gsm_ded_ch_report_ind->u16_report_ctrl)
                       #endif
                      )                              
                    {
                       /* FIX AM-Bug00000229 BEGIN  2010-09-26  : gaochunsheng */
                       gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
                       /* FIX AM-Bug00000229 END  2010-09-26  : gaochunsheng */
                       break;/* break 是必要的*/
                    }
                    /* FIX PhyFA-Req00001232   END   2012-03-30  : linlan*/
                }
            }
            /*FIX PhyFA-Req00000936  END     2011-07-12  : ningyaojun*/        
            /*  FIX NGM-Bug00000370  BEGIN  2014-08-21 :sunzhiqiang  */
            gsm_update_latest_fn_by_slave_meas_rpt_pos(u32_next_fn,stp_latest_rtx_frame_pos); 
            /*  FIX NGM-Bug00000370  END  2014-08-21 :sunzhiqiang  */

            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/   
            #ifdef L1CC_GSM_RTRT_UT_SWITCH
            if(OAL_SUCCESS == stub_gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            #else
            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            #endif
            {
                stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
                stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn+(UINT32)1),u16_active_id);
                
                /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                  stp_gsm_main_frametask_tmp1,
                                  &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
            }
            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
 
            break;

        case MSG_GSM_MEAS_FCP_FCBSUCC_IND:
            /* FIX PhyFA-Req00000936 BEGIN  2011-06-02  : wuxiaorong */
            /*调用OAL接口，事件下载都在外面，从里面各个分支挪出来*/
            /* FIX PhyFA-Req00000936 BEGIN  2011-06-02  : wuxiaorong */
            /* FIX PhyFA-Req00000089  BEGIN  2008-03-31  : huangjinfu */
            /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wuxiaorong */
            
            stp_fcbsucc_ind = (msg_gsm_meas_l1cc_fcb_result_ind_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));

            u16_standby_id = stp_fcbsucc_ind->u16_standby_id;
            u16_rxtask_type = stp_fcbsucc_ind->u16_rxtask_type;
            if(GSM_CELLSEARCH == u16_rxtask_type)
            {
                if(((UINT16)1 == stp_cellsearch_ctrl->st_nocell_search.u16_valid_flag)
                    &&(u16_standby_id == stp_cellsearch_ctrl->st_nocell_search.u16_standby_id[0])) 
                {
                    #ifdef L1CC_GSM_RTRT_UT_SWITCH
                    u32_status = stub_gsm_fcp_nocell_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                    #else
                    u32_status = gsm_fcp_nocell_process(u32_next_fn,
                                                        stp_oal_msg,
                                                        &u32_calculated_sb_fn,
                                                        stp_latest_rtx_frame_pos);
                    #endif
                }
                else
                {
                    u32_status = gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                }
            }
            else
            {
                 OAL_ASSERT(GSM_IRAT_GAP_MEAS         != u16_rxtask_type,"u16_rxtask_type!= GSM_MEAS,error");
                 u32_status = gsm_fcp_meas_fcbsucc_ind(u32_next_fn,stp_fcbsucc_ind,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
            }
            /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */
            /*调用OAL接口，事件下载都统一在外面，从里面各个分支挪出来*/
            if(OAL_SUCCESS == u32_status)
            {
                /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
                #ifdef L1CC_GSM_RTRT_UT_SWITCH
                if(OAL_SUCCESS == stub_gsm_fcp_get_undownloaded_slot_range(u32_calculated_sb_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                #else
                if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_calculated_sb_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                #endif
                {
                    stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_calculated_sb_fn,u16_active_id);
                    stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_calculated_sb_fn+(UINT32)1),u16_active_id);
                    
                    /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                    /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                    gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                    /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                    /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                    gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                      stp_gsm_main_frametask_tmp1,
                                      &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
                }
                /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
            }        
            /* FIX PhyFA-Req00000936 END  2011-06-02  : wuxiaorong */ 
            
            break;
        /* FIX PhyFA-Req00000089  END  2008-03-31  : huangjinfu */
        /*when the state changing is just happened from GSM_TRANSFER*/
        case MSG_GSM_EQ_FCP_USF_DEC_IND:
            /* FIX LM-Bug00001524    BEGIN 2011-12-29: linlan */ 
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            /* FIX LM-Bug00001524    END 2011-12-29: linlan */ 
            break;
        /* FIX PhyFA-Req00001925 BEGIN 2014-08-05 : wuxiaorong */
        #if 0
        /* FIX PhyFA-Bug00005426 BEGIN 2014-07-25 : wuxiaorong */
        case MSG_GSM_L1RESP_FCP_SBFAIL_IND:
            if(OAL_FALSE == check_gsm_cellsearch_fcb_running())
            {
                stp_sbfail_msg = (msg_gsm_l1resp_fcp_sb_result_ind_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));
                gsm_fcp_set_meas_task(stp_sbfail_msg->u32_next_sb_search_fn,stp_latest_rtx_frame_pos);
                /* FIX PhyFA-Bug00005464  BEGIN 2014-07-30 : wuxiaorong */
                if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(stp_sbfail_msg->u32_next_sb_search_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                {
                    stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(stp_sbfail_msg->u32_next_sb_search_fn,u16_active_id);
                    stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(stp_sbfail_msg->u32_next_sb_search_fn+(UINT32)1),u16_active_id);
                    gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                    stp_gsm_main_frametask_tmp1,
                                    &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
                }
                /* 是当帧陪当帧,u32_next_sb_search_fn = u32_next_fn-1  */
                if(gsm_fn_compare(stp_sbfail_msg->u32_next_sb_search_fn,u32_next_fn))
                {
                    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
                } 
                /* FIX PhyFA-Bug00005464  END   2014-07-30 : wuxiaorong */
            }
            break;
        /* FIX PhyFA-Bug00005426 END   2014-07-25 : wuxiaorong */
        #endif
        /* FIX PhyFA-Req00001925 END   2014-08-05 : wuxiaorong */
        default:
            /* FIX PhyFA-Enh00001698 BEGIN  2014-04-23  : wangjunli*/
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            OAL_PRINT((UINT16)OAL_GET_OAL_MSG_STANDBY_ID(stp_oal_msg),(UINT16)GSM_MODE,"gsm_fcp_idle receive unexpected msg:%ld",OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg));
            /* FIX PhyFA-Enh00001698 END  2014-04-23  : wangjunli*/
            break;
    }

    return OAL_SUCCESS;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/



/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_cs_rach
* DESCRIPTION:       the handler of the state GSM_CS_RACH of the module FCP
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:   stp_tsn_prios[10]: priority info of  [tsn7|tsn0|tsn1|tsn2|tsn3|tsn4|tsn5|tsn6|tsn7|tsn0]
      u32_next_fn:  the next frame number
      stp_oal_msg:        message received from other module
* Output Parameters:NULL
* Return value:      OAL_SUCCESS or OAL_FAILURE
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-11-05      zhengying               PhyFA-Req00001525   [NGM]V4 GSM代码入库
*2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/


L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
STATIC OAL_STATUS gsm_fcp_cs_rach(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos)
{
    UINT32 u32_enc_blks;
    UINT32 u32_curr_fn;
    UINT16 u16_temp_standby_id;
    UINT16 k;
    UINT16  u16_standby_id = UNSPECIFIED_STANDBY_ID;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp0 = NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp1 = NULL_PTR;    
    UINT16  u16_ts_bitmap[2] = {0,0}; 
    UINT32 u32_scell_current_fn = (UINT32)0;
    OAL_STATUS u32_fcp_ret = OAL_FAILURE;
    /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wuxiaorong */
    CONST msg_gsm_meas_l1cc_fcb_result_ind_t *stp_fcbsucc_ind =  NULL_PTR;
    /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */
    CONST gsm_cellsearch_t    *stp_gsm_cellsearch     = NULL_PTR;
    UINT16 u16_gsm_bg_state;
    UINT32 u32_calculated_sb_fn = (UINT32)0;
    UINT16  u16_rxtask_type;
    
    CONST gsm_ccch_tb_t  *stp_gsm_ccch_tb = NULL_PTR;
    UINT16         u16_paging_period;
    UINT16         u16_paging_offset;  
    gsm_rach_tb_t *stp_gsm_rach_tb = NULL_PTR;
    CONST gsm_sc_bcch_tb_t *stp_gsm_sc_bcch_tb = NULL_PTR;
    CONST gsm_cellsearch_ctrl_t *stp_cellsearch_ctrl = NULL_PTR;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR;
       
   
    OAL_ASSERT(NULL_PTR!=stp_oal_msg, "gsm_fcp_cs_rach null pointer");
    OAL_ASSERT(NULL_PTR!= stp_gsm_latest_rtx_frame_pos,"gsm_fcp_cs_rach():stp_gsm_latest_rtx_frame_pos is NULL_PTR!");    
    stp_gsm_rach_tb = gsm_get_rach_tab();
    stp_gsm_standby_info = gsm_get_standby_info(); 
    stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();
    /*Other simcards are not considered now, the function will be added later*/
    /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
    u16_standby_id = stp_gsm_rach_tb->u16_standby_id;  

    OAL_ASSERT((UNSPECIFIED_STANDBY_ID != u16_standby_id),"gsm_fcp_cs_rach():g_st_gsm_rach_tb.u16_standby_id is UNSPECIFIED_STANDBY_ID!");    
    /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/     

    /*FIX LM-Bug00001426  BEGIN   2011-12-14  : sunzhiqiang*/
    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
    stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */

    /*FIX LM-Bug00001426  END   2011-12-14  : sunzhiqiang*/
    u32_curr_fn = gsm_fn_operate((u32_next_fn+(UINT32)GSM_MAX_FN)-(UINT32)1);
    stp_gsm_rach_tb = gsm_get_rach_tab();
    switch(OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg))
    {
        case MSG_ISR_FCP_FRAMETIMER_IND:
            /* FIX LM-Enh00000503  END  2011-12-26: linlan */
            for(k = (UINT16)0; k < (UINT16)MAX_STANDBY_AMT; k++)
            {
                if((UINT16)GSM_MASTER_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[k])
                {
                    u16_temp_standby_id = stp_gsm_standby_info->u16_standby_id[k];
                    stp_gsm_sc_bcch_tb = gsm_get_sc_bcch_tbl_cfg(u16_temp_standby_id);
                    if((UINT16)1 == stp_gsm_sc_bcch_tb->u16_sc_bcch_tb_flg)
                    {
                        gsm_fcp_add_frametask((UINT16)TSNTASK_BCCH_RX,u32_next_fn,u16_temp_standby_id);
                    }
                    stp_gsm_ccch_tb = gsm_get_ccch_tbl_cfg(u16_temp_standby_id);
                    if((UINT16)1 == stp_gsm_ccch_tb->u16_ccch_tb_flg)
                    {
                        /*背景卡和前景卡PS接入,当CCCH不属于寻呼时配置TSNTASK_CCCH_PCH_RX*/
                        if((u16_standby_id != u16_temp_standby_id) || ((UINT16)0 != stp_gsm_rach_tb->u16_acess_cause))
                        {
                            u32_scell_current_fn = gsm_get_scell_current_fn(u32_next_fn,(UINT16)0,(SINT16)0,u16_temp_standby_id);
                            u16_paging_period = stp_gsm_ccch_tb->u16_paging_period;
                            u16_paging_offset = stp_gsm_ccch_tb->u16_paging_offset;
                            if(((UINT16)(gsm_fn_operate(u32_scell_current_fn + (UINT32)1)%(UINT32)u16_paging_period) != u16_paging_offset)
                                &&((UINT16)(u32_scell_current_fn%(UINT32)u16_paging_period) != u16_paging_offset))
                            {
                                gsm_fcp_add_frametask((UINT16)TSNTASK_CCCH_RX,u32_next_fn,u16_temp_standby_id);
                            }
                            else
                            {
                                gsm_fcp_add_frametask((UINT16)TSNTASK_CCCH_PCH_RX,u32_next_fn,u16_temp_standby_id);  
                            }
                        }
                        
                        if((u16_standby_id == u16_temp_standby_id) && ((UINT16)0 == stp_gsm_rach_tb->u16_acess_cause))
                        {
                            gsm_fcp_add_frametask((UINT16)TSNTASK_CCCH_CS_RX,u32_next_fn,u16_temp_standby_id);
                        }
                    }
                }
            }
            
            gsm_fcp_add_frametask(TSNTASK_TC_BCCH_RX,u32_next_fn,u16_standby_id);

            /* 重新取值  */
            stp_gsm_ccch_tb = gsm_get_ccch_tbl_cfg(u16_standby_id);
            if(((UINT16)1 == stp_gsm_rach_tb->u16_rach_tb_flg) &&
                (((UINT16)0 == stp_gsm_ccch_tb->u16_ccch_tb_flg) ||
                  ((UINT16)51 != stp_gsm_ccch_tb->u16_ccch_period)))
            {
                oal_error_handler(ERROR_CLASS_MINOR, GSM_FCP_ERR(INVALID_PARAM));
            }
            else
            {
                switch(stp_gsm_rach_tb->u16_data_state)
                {
                    case GSM_DATA_UNENCODED:
                        gsm_fcp_enc_special_data(&u32_enc_blks,TSNTASK_RACH_TX,u32_curr_fn,u16_standby_id);
                        break;
                    case GSM_DATA_ENCODED:
                        gsm_fcp_add_special_frametask(TSNTASK_RACH_TX,u32_next_fn,u16_standby_id);
                        break;
                    case GSM_DATA_SENT:
                        gsm_fcp_handle_hls_res((UINT16)GSM_FCP_RACH_CNF,u32_curr_fn,OAL_SUCCESS,u16_standby_id);             
                        stp_gsm_rach_tb->u16_data_state = GSM_DATA_INVALID;

                        stp_gsm_rach_tb->u16_rach_tb_flg = 0;
                        break;
                    case GSM_DATA_INVALID:
                        break;
                    default:
                        oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
                        break;
                }
            }
            
            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            {
                stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
                stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn+(UINT32)1),u16_active_id);
                
                /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                  stp_gsm_main_frametask_tmp1,
                                  &u16_ts_bitmap[0], &u16_ts_bitmap[1] );
            }
            break;
        /*when the state changing is just happened from GSM_TRANSFER*/
        case MSG_GSM_EQ_FCP_USF_DEC_IND:
            break;
        case MSG_GSM_MEAS_FCP_FCBSUCC_IND:
            
            stp_fcbsucc_ind = (msg_gsm_meas_l1cc_fcb_result_ind_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));
            u16_standby_id = stp_fcbsucc_ind->u16_standby_id;
            u16_rxtask_type = stp_fcbsucc_ind->u16_rxtask_type;
            if((UINT16)GSM_CELLSEARCH == u16_rxtask_type)
            {
                if((UINT16)1 == stp_cellsearch_ctrl->u16_valid_flag)
                {
                    stp_gsm_cellsearch = gsm_get_cellsearch_tbl_by_standby(u16_standby_id);    
                    if((UINT16)1 == stp_gsm_cellsearch->u16_valid_flag)
                    {
                        u16_gsm_bg_state = stp_gsm_cellsearch->u16_bg_state;
                        if((UINT16)GSM_BGS_NON != u16_gsm_bg_state)
                        {
                            if(NOCELL_SEARCH == stp_gsm_cellsearch->u16_search_type)
                            {
                                u32_fcp_ret = gsm_fcp_nocell_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                            }
                            else
                            {
                                u32_fcp_ret = gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                            }
                            if(OAL_SUCCESS == u32_fcp_ret)
                            {
                                stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_calculated_sb_fn,u16_active_id);
                                stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_calculated_sb_fn+(UINT32)1),u16_active_id);
                                if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_calculated_sb_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                                {
                                    /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                                    /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                                    gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                                    /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                                    /* FIX NGM-Enh00000122  END    2014-10-10: linlan */

                                    gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                                      stp_gsm_main_frametask_tmp1,
                                                      &u16_ts_bitmap[0], &u16_ts_bitmap[1]);

                                }   
                            }
                        }
                    }
                }        
            }
            else
            {
                OAL_PRINT(u16_standby_id,(UINT16)GSM_MODE,"in rach state,fcbsucc_ind msg's rxtask_type:%ld",(UINT32)u16_rxtask_type);
            }
            break;

        default:
            /* FIX PhyFA-Enh00001698 BEGIN  2014-04-23  : wangjunli*/
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            OAL_PRINT((UINT16)OAL_GET_OAL_MSG_STANDBY_ID(stp_oal_msg),(UINT16)GSM_MODE,"gsm_fcp_cs_rach receive unexpected msg:%ld",OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg));
            /* FIX PhyFA-Enh00001698 END  2014-04-23  : wangjunli*/
            break;
    }

    return OAL_SUCCESS;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/


/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_ps_rach
* DESCRIPTION:       the handler of the state GSM_PS_RACH of the module FCP
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters: 
        stp_tsn_prios[10]: priority info of  [tsn7|tsn0|tsn1|tsn2|tsn3|tsn4|tsn5|tsn6|tsn7|tsn0]
      u32_next_fn:  the next frame number
      stp_oal_msg:        message received from other module
* Output Parameters: NULL
* Return value:      OAL_SUCCESS or OAL_FAILURE
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-11-04      zhengying               PhyFA-Req00001525   [NGM]V4 GSM代码入库
*2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
STATIC OAL_STATUS gsm_fcp_ps_rach(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos)
{
    UINT32 u32_enc_blks;
    UINT32 u32_curr_fn;
    UINT32 u32_send_ret;
    SINT16 s16_rtx_offset_list[4];
    UINT16  u16_tsn_list[4];
    UINT16 u16_tsn_num;
    VOID_PTR vp_para[4];
    /* FIX PhyFA-Bug00000446    BEGIN   2008-06-26  : ningyaojun*/
    UINT32 u32_enc_spec_blks = (UINT32)0; //useless
    /* FIX PhyFA-Bug00000446    END     2008-06-26  : ningyaojun*/

    UINT16 u16_prach_enc_flag = 0;
    /*FIX A2KPH-Bug00001556    BEGIN  2009-02-25 :fushilong*/
    //UINT16 u16_tx_ts_bitmap = 0; 
    /*FIX A2KPH-Bug00001556    END 2009-02-25 :fushilong*/
    OAL_STATUS u32_status = OAL_FAILURE;
    //gsm_main_frametask_t* stp_frame_task = NULL_PTR;

    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    UINT16  u16_standby_id = UNSPECIFIED_STANDBY_ID;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp0 = NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp1 = NULL_PTR;    
    UINT16  u16_ts_bitmap[2] = {0,0}; 
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/  
    CONST gprs_pccch_tb_t* stp_gprs_pccch_tlb = NULL_PTR;
    gprs_prach_tb_t *stp_gprs_prach_tb = NULL_PTR;
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;

    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    OAL_ASSERT(NULL_PTR!=stp_oal_msg, "gsm_fcp_ps_rach null pointer");
    OAL_ASSERT(NULL_PTR!= stp_gsm_latest_rtx_frame_pos,"gsm_fcp_ps_rach():stp_gsm_latest_rtx_frame_pos is NULL_PTR!");    
    /*Other simcards are not considered now, the function will be added later*/
    /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
    stp_gprs_prach_tb = gsm_get_prach_tb();
    u16_standby_id = stp_gprs_prach_tb->u16_standby_id;  
    OAL_ASSERT(((UINT16)UNSPECIFIED_STANDBY_ID != u16_standby_id),"gsm_fcp_ps_rach():g_st_gsm_rach_tb.u16_standby_id is UNSPECIFIED_STANDBY_ID!");    
    /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/  
    
    u32_curr_fn = gsm_fn_operate((u32_next_fn+(UINT32)GSM_MAX_FN)-(UINT32)1);
    switch(OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg))
    {
        case MSG_ISR_FCP_FRAMETIMER_IND:
            gsm_fcp_add_frametask(TSNTASK_PCCCH_RX,u32_next_fn,u16_standby_id);
            gsm_fcp_add_frametask(TSNTASK_PBCCH_RX,u32_next_fn,u16_standby_id);

            /* FIX PhyFA-Bug00000446    BEGIN   2008-06-26  : ningyaojun*/
            gsm_fcp_enc_special_data(&u32_enc_spec_blks,TSNTASK_RRBP_TX0,u32_curr_fn,u16_standby_id);
            gsm_fcp_add_special_frametask(TSNTASK_RRBP_TX0,u32_next_fn,u16_standby_id);
            /* FIX PhyFA-Bug00000446    END     2008-06-26  : ningyaojun*/

            /* FIX A2KPH-Bug00000479    BEGIN   2008-07-22  : wuxiaorong */
            gsm_fcp_add_frametask(TSNTASK_TC_BCCH_RX,u32_next_fn,u16_standby_id);
            /* FIX A2KPH-Bug00000479    END     2008-07-22  : wuxiaorong */

            switch(stp_gprs_prach_tb->u16_prach_data_state)
            {
                case GSM_DATA_UNENCODED:
                    gsm_fcp_enc_special_data(&u32_enc_blks,TSNTASK_PRACH_TX,u32_curr_fn,u16_standby_id);
                   break;
                case GSM_DATA_ENCODED:
                    stp_gprs_pccch_tlb = gsm_get_gprs_pccch_tbl(u16_standby_id);
                    /* Fix PhyFA Bug00000440 begin, guxiaobo, 20080911 */
                    if(  ((UINT16)MPAL_PAGING_REORG    == stp_gprs_pccch_tlb->u16_page_mode)
                       ||((UINT16)MPAL_FULL_PCCCH_BLKS == stp_gprs_pccch_tlb->u16_page_mode))
                    {
                        if(stp_gprs_prach_tb->u16_is_about_to_arrive)
                        {
                            u16_prach_enc_flag = 1;
                        }
                        else
                        {
                           u16_prach_enc_flag = 0;
                        }
                    }
                    else
                    {
                        u16_prach_enc_flag = 1;
                    }

                    if(u16_prach_enc_flag)
                    {
                        /* Fix A2KPH Bug00001019 end, guxiaobo, 20081021*/
                        if(stp_gprs_prach_tb->u16_send_prach_req == (UINT16)1)
                        {
                            if(OAL_SUCCESS == gsm_fcp_add_special_frametask(TSNTASK_PRACH_TX,u32_next_fn,u16_standby_id))
                            {
                                stp_gprs_prach_tb->u16_prach_data_state = GSM_DATA_SENT;
                            }
                        }
                        else
                        {
                            if(OAL_SUCCESS == gsm_fcp_get_spec_tb_info(&u16_tsn_num,u16_tsn_list,s16_rtx_offset_list,vp_para,
                                TSNTASK_PRACH_TX,u32_next_fn,(UINT16)GSM_DATA_ENCODED,u16_standby_id))
                            {
                                stp_gprs_prach_tb->u16_prach_data_state = GSM_DATA_SENT;
                            }
                        }
                    }

                   break;
                case GSM_DATA_SENT:
                    stp_gprs_prach_tb->u16_is_about_to_arrive = 0;

                    if((UINT16)1 == stp_gprs_prach_tb->u16_send_prach_req)
                    {
                           u32_send_ret = OAL_SUCCESS;
                    }
                    else
                    {
                           u32_send_ret = OAL_FAILURE;
                    }
                    gsm_fcp_handle_hls_res((UINT16)GSM_FCP_PRACH_CNF,u32_curr_fn,u32_send_ret,u16_standby_id);
                    stp_gprs_prach_tb->u16_prach_data_state = GSM_DATA_INVALID;
                    stp_gprs_prach_tb->u16_is_prev_rach_valid = 1;
                    /* FIX PECK3_Bug00000548  BEGIN  2009-08-26  : fushilong */
                    stp_gprs_prach_tb->u16_prach_tb_flg = 0;
                    /* FIX PECK3_Bug00000548  END  2009-08-26  : fushilong */
                   break;
                case GSM_DATA_INVALID:
                    break;
                default:
                    oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));
                   break;
            }
            /* FIX PhyFA-Req00000985 BEGIN  2011-07-25 : wuxiaorong */  
            /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
            stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */
            /* FIX PhyFA-Req00000985 END  2011-07-25 : wuxiaorong */ 

            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/   
            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            {
                stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
                stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn+(UINT32)1),u16_active_id);
                
                /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                  stp_gsm_main_frametask_tmp1,
                                  &u16_ts_bitmap[0], &u16_ts_bitmap[1]);

            }
            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/            
            break;
        case MSG_GSM_DEC_FCP_USF_DEC_IND:
            /* u16_prach_data_state must be GSM_DATA_ENCODED */
            if((UINT16)GSM_DATA_ENCODED != stp_gprs_prach_tb->u16_prach_data_state)
            {
                oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_DUMMY_DATA));
            }

            /* indicate that prach tx fn must be one of the four frames of the next allocated
            PDCH block, so we should be ready to add prach tx task */
            stp_gprs_prach_tb->u16_is_about_to_arrive = 1;



            if(stp_gprs_prach_tb->u32_prach_fn == u32_curr_fn)
            {
                /*FIX LM-Bug00000405  BEGIN  2011-02-21 :fushilong*/
                /*如果PRACH需要发送，则添加TSNTASK_PRACH_TX任务*/
                if((UINT16)1 == stp_gprs_prach_tb->u16_send_prach_req )
                {
                    u32_status = gsm_fcp_add_special_frametask(TSNTASK_PRACH_TX,u32_curr_fn,u16_standby_id);
                }
                
                /*根据u32_statu的状态来进行PRACH事件的下载*/
                if(OAL_SUCCESS == u32_status)
                {
                    /* in 42.1.1.4.3, 52.1.1.6.3, it will not be executed here, but codes are added for safety concern */
                    //stp_frame_task = gsm_get_main_frametask(u32_curr_fn);
                    /*FIX PhyFA-Req00000936  BEGIN   2011-06-22  : ningyaojun*/
                    if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_curr_fn, (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                    {
                        stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_curr_fn,u16_active_id);
                        stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_curr_fn+(UINT32)1),u16_active_id);
                        
                        /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                        /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                        gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                        /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                        /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                        gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                          stp_gsm_main_frametask_tmp1,
                                          &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
                        
                    }                              
                    /*FIX PhyFA-Req00000936  END     2011-06-22  : ningyaojun*/                    
                }
               
               /*根据u32_status的状态来确定GSM_FCP_PRACH_CNF的参数*/
               /*特别注意，目前的代码修改已经考虑到下面这种情况*/
               /* 当延迟达到一定程度时，DD已经来不及添加任务，此时会直接上报GSM_FCP_PRACH_CNF*/
               /*消息中携带的RACH发送状态为FAILURE。*/
               if(OAL_SUCCESS != u32_status)
               {
                    u32_send_ret = OAL_FAILURE;
               }
               else
               {
                   u32_send_ret = OAL_SUCCESS;
               }               

               /*如果是当前帧添加PRACH任务，则直接发送GSM_FCP_PRACH_CNF并改变状态为DATA_INVALID；*/
               /*同时about_to_arrive,u16_is_prev_rach_valid,u16_prach_tb_flg都需要作相应的更新*/
               /*如果通过将状态直接更改为DATA_SENT的方式来发送GSM_FCP_PRACH_CNF，则PRACH发送帧号会与实际帧号有偏差*/
               /*CRTUG 用例为52.1.1.6.3*/
               stp_gprs_prach_tb->u16_is_about_to_arrive = 0;
               gsm_fcp_handle_hls_res((UINT16)GSM_FCP_PRACH_CNF,u32_curr_fn,u32_send_ret,u16_standby_id);
               stp_gprs_prach_tb->u16_prach_data_state = GSM_DATA_INVALID;
               stp_gprs_prach_tb->u16_is_prev_rach_valid = 1;
               stp_gprs_prach_tb->u16_prach_tb_flg = 0;
               /*FIX LM-Bug00000405  END 2011-02-21 :fushilong*/
            }
            else
            {
                if((UINT16)1 == stp_gprs_prach_tb->u16_send_prach_req )
                {
                    u32_status = gsm_fcp_add_special_frametask(TSNTASK_PRACH_TX,u32_next_fn,u16_standby_id);
                }
                else
                {
                    /*FIX PhyFA-Bug00001188  BEGIN  2009-05-12 :fushilong*/
                    if(stp_gprs_prach_tb->u32_prach_fn == u32_next_fn )
                    {
                       stp_gprs_prach_tb->u16_prach_data_state = GSM_DATA_SENT;
                    }        
                    /*FIX PhyFA-Bug00001188  END 2009-05-12 :fushilong*/
                }

                if(OAL_SUCCESS == u32_status)
                {
                   stp_gprs_prach_tb->u16_prach_data_state =GSM_DATA_SENT;

                    /*FIX PhyFA-Req00000936  BEGIN   2011-06-22  : ningyaojun*/               
                    if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                    {
                        stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
                        stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn+(UINT32)1),u16_active_id);
                        
                        /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                        /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                        gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                        /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                        /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                        gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                          stp_gsm_main_frametask_tmp1,
                                          &u16_ts_bitmap[0], &u16_ts_bitmap[1]);

                    }                          
                    /*FIX PhyFA-Req00000936  END     2011-06-22  : ningyaojun*/                    
                }
            }
            break;            
        default:
            /* FIX AM-Bug00000248  BEGIN  2010-09-15  : gaochunsheng*/
            /* FIX PhyFA-Enh00001698 BEGIN  2014-04-23  : wangjunli*/
            /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
            stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */
            OAL_PRINT((UINT16)OAL_GET_OAL_MSG_STANDBY_ID(stp_oal_msg),(UINT16)GSM_MODE,"gsm_fcp_ps_rach receive unexpected msg:%ld",OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg));
            /* FIX PhyFA-Enh00001698 END  2014-04-23  : wangjunli*/
            /* FIX AM-Bug00000248  BEGIN  2010-09-15  : gaochunsheng*/
            break;
    }
    return OAL_SUCCESS;
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/

/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_cs
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*   2014-07-30      wuxiaorong              PhyFA-Bug00005464   [NGM]GSM的SB search当帧配当帧睡眠控制latest_rtx_frame_pos设置有问题 
*   2014-08-05      wuxiaorong              PhyFA-Req00001925   [NGM]T业务下G测量在当前接收未完成前就需要帧中断下次接收任务 .
*   2014-08-12      xiongjiangjiang         PhyFA-Bug00005562   [NGM]GSM辅模式时需要增加在假的帧中断下用下一次的上报时间点来更新最近收发帧
*   2014-09-20      wuxiaorong              NGM-Bug00000585     [NGM]G辅模SB Search跨帧配置出现N帧配N+2帧导致后续N+1帧配N+2帧时把先前配置任务删除
***********************************************************************************************************************/
/* FIX PhyFA-Req00000936 BEGIN  2011-06-09  : wuxiaorong */
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_cs(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos)
{

    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp0 = NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp1 = NULL_PTR;    
    UINT16  u16_ts_bitmap[2] = {0,0}; 
    UINT32  u32_calculated_sb_fn = (UINT32)(UINT32)GSM_INVALID_FN;
    OAL_STATUS u32_fcp_ret = OAL_FAILURE;
    CONST gsm_normal_search_ctrl_t  *stp_normal_search = NULL_PTR;    
    CONST gsm_cellsearch_t  *stp_cellsearch_tbl= NULL_PTR;    
    gsm_nocell_search_ctrl_t  *stp_nocell_search= NULL_PTR;
    /*FIX LM-Bug00002010  END   2012-05-11  : sunzhiqiang*/
    UINT16 i = 0;
    UINT16 u16_standby_id = UNSPECIFIED_STANDBY_ID;
    msg_gsm_meas_l1cc_fcb_result_ind_t CONST*stp_fcbsucc_ind =  NULL_PTR;
    UINT16   u16_rxtask_type;
    mmc_time_info_t st_mmc_time_info;
    gsm_cellsearch_ctrl_t *stp_cellsearch_ctrl = NULL_PTR; 
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    gsm_tstask_latest_frame_pos_t*  stp_latest_frame_pos_list = NULL_PTR;
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR;
    /*FIX PhyFA-Bug00005514  BEGIN   2014-08-05  : linlan*/
    CONST gsm_nc_meas_sync_ctrl_t*  stp_nc_meas_sync_ctrl = NULL_PTR;
    /*FIX PhyFA-Bug00005514  END     2014-08-05  : linlan*/
    /* FIX PhyFA-Bug00005665 BEGIN 2014-09-01 : wuxiaorong */
    SINT16  s16_fn_begin_offset = 0;
    /* FIX PhyFA-Bug00005665 END   2014-09-01 : wuxiaorong */
    /* FIX NGM-Bug00000585 BEGIN 2014-09-20 : wuxiaorong */
    UINT16  u16_rtx_bitmap = 0;
    /* FIX NGM-Bug00000585 END   2014-09-20 : wuxiaorong */

    
    OAL_ASSERT((NULL_PTR != stp_oal_msg), "gsm_fcp_cs:stp_oal_msg is null");    
    OAL_ASSERT(NULL_PTR!= stp_gsm_latest_rtx_frame_pos,"gsm_fcp_cs():stp_gsm_latest_rtx_frame_pos is NULL_PTR!");    
    stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();   
    stp_gsm_standby_info = gsm_get_standby_info();
    stp_nocell_search = &(stp_cellsearch_ctrl->st_nocell_search);
    stp_normal_search = &(stp_cellsearch_ctrl->st_normal_search);
    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
    stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)13));
    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */


    switch(OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg))
    {
        case MSG_ISR_FCP_FRAMETIMER_IND:

            
            /* 申请内存 */
            stp_latest_frame_pos_list = (gsm_tstask_latest_frame_pos_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,
                                                                                  (UINT32)OAL_GET_TYPE_SIZE_IN_WORD(gsm_tstask_latest_frame_pos_t));  
            stp_latest_frame_pos_list->u16_latest_pos_num = 0;

            /* FIX LM-Bug00001266    BEGIN 2011-11-14: wuxiaorong */
            for(i = 0; i< (UINT16)MAX_STANDBY_AMT; i++)
            {
                u16_standby_id = stp_gsm_standby_info->u16_standby_id[i];
                if((UINT16)GSM_MASTER_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[i])
                {
                    /* 实际上应该只可能有TSNTASK_BCCH_RX/TSNTASK_PBCCH_RX,在INIT_SEARCH的搜网GSM_BGS_SYNC子状态时也会尝配置;放在这里，保证它在PLMN过程BCCH之前先配 */
                    gsm_add_master_standby_idle_tsntask(u32_next_fn,u16_standby_id,stp_latest_frame_pos_list);  
                    gsm_add_sync_cell_bcch_tsntask(u32_next_fn,u16_standby_id,stp_latest_frame_pos_list);
                    /* Fix LM-Enh00001221 BEGIN 2013-07-24 : wangjunli*/
                    /*添加PLMN过程BCCH接收*/
                    gsm_add_master_standby_plmn_bcch_tsntask(u32_next_fn,u16_standby_id,stp_latest_frame_pos_list);
                    /* Fix LM-Enh00001221 END 2013-07-24: wangjunli*/
                    /* FIX LM-Bug00001339  END 2011-11-30 :wanghairong */
                }
                /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
                /*辅模式需要尝试SI预读任务TSNTASK_TC_BCCH_RX配置*/
                else if((UINT16)GSM_SLAVE_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[i]) 
                {
                    gsm_add_slave_standby_tsntask(u32_next_fn,u16_standby_id,stp_latest_frame_pos_list); 
                }
                /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/
            }
         
            gsm_fcp_get_latest_frame_pos(stp_latest_frame_pos_list,stp_latest_rtx_frame_pos);
            /* FIX LM-Bug00000986    END 2011-10-12: wuxiaorong */
            
            /*FIX PhyFA-Bug00005514  BEGIN   2014-08-05  : linlan*/
            /* FIX NGM-Bug00000585 BEGIN 2014-09-20 : wuxiaorong */
            u16_rtx_bitmap = gsm_get_frame_task_rtx_bitmap(gsm_get_main_frametask(u32_next_fn,u16_active_id));
            if(0 != u16_rtx_bitmap)
            {
                /*FIX NGM-Bug00000358  BEGIN   2014-08-22  : linlan*/
                gsm_check_del_running_sync_nslot_task(u32_next_fn,u16_rtx_bitmap);
                /*gsm_del_fcb_task_reset_cellsearch_tb();  */              
                /*FIX NGM-Bug00000358  END     2014-08-22  : linlan*/
            }
            /* FIX NGM-Bug00000585 END   2014-09-20 : wuxiaorong */

            /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : caisiwen */
            if(OAL_FALSE == check_gsm_cellsearch_fcb_running())
            {
                /* FIX PhyFA-Bug00004912 BEGIN 2014-06-05 : wuxiaorong */
                /* 配置辅模式测量 */
                /* FIX PhyFA-Bug00005665 BEGIN 2014-09-01 : wuxiaorong */
                /* 睡眠唤醒后的假的帧中断消息不能按普通方式配测量.需要限定范围 */
                if(OAL_FALSE == gsm_check_pseudo_frametimer(stp_oal_msg,&s16_fn_begin_offset))
                {
                    gsm_fcp_set_meas_task(u32_next_fn,stp_latest_rtx_frame_pos);
                }
                else
                {
                    /* s16_fn_begin_offset不为0时,只进行辅模式的ReBSIC和RSSI测量;s16_fn_begin_offset=0时,内部调用gsm_fcp_set_meas_task() */
                    gsm_fcp_set_selfwakeup_nextfn_bsic_rssi_meas_task(u32_next_fn,s16_fn_begin_offset,stp_latest_rtx_frame_pos);
                }
                /* FIX PhyFA-Bug00005665 END   2014-09-01 : wuxiaorong */
                /* FIX PhyFA-Bug00004912 END   2014-06-05 : wuxiaorong */ 
            }
            /* FIX PhyFA-Req00001026 END 2012-04-10  : caisiwen */
            /*FIX LM-Bug00001436  BEGIN   2011-12-14  : sunzhiqiang*/          
            stp_nc_meas_sync_ctrl = gsm_get_nc_meas_sync_ctrl();
            if((OAL_FALSE == check_gsm_cellsearch_fcb_running())
                &&((UINT16)0 == stp_nc_meas_sync_ctrl->u16_running_bitmap))
            {
            /*FIX PhyFA-Bug00005514  END     2014-08-05  : linlan*/
                /* FIX LM-Bug00000688  BEGIN  2011-08-22 : wuxiaorong */
                if(((UINT16)1 == stp_normal_search->u16_valid_flag)&&(stp_normal_search->u16_search_standby_num > (UINT16)0))
                {
                    /*FIX LM-Bug00002010  BEGIN   2012-05-11  : sunzhiqiang*/
                    /* FIX NGM-Bug00000677 BEGIN 2014-10-09 : wuxiaorong */
                    if(((UINT16)1 == stp_nocell_search->u16_valid_flag)&&(stp_nocell_search->u16_standby_num_from_mmc > (UINT16)0))
                    /* FIX NGM-Bug00000677 END   2014-10-09 : wuxiaorong */
                    {
                        /* u16_standby_num_from_mmc>0, u16_cur_standby_id 必定不是UNSPECIFIED_STANDBY_ID */
                        stp_cellsearch_tbl = &(stp_cellsearch_ctrl->st_cellsearch_tbl[stp_nocell_search->u16_cur_standby_id]);
                        if(((UINT16)1 == stp_cellsearch_tbl->u16_valid_flag) 
                          &&((UINT16)GSM_BG_FCB_SETTING == stp_cellsearch_tbl->u16_fcb_set_flg))
                        {
                            dd_event_stop_by_standby(u16_active_id,(UINT16)((UINT16)0x0001<<stp_nocell_search->u16_cur_standby_id),(UINT16)GSM_TIMING,&st_mmc_time_info);
                            gsm_fcp_del_all_frametask((UINT16)UNSPECIFIED_STANDBY_ID);
                            gsm_fcp_nocell_next_search_or_sleep(stp_nocell_search);
                        }
                    }
                    /*FIX LM-Bug00002010  END   2012-05-11  : sunzhiqiang*/
                    gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,NULL_PTR,stp_latest_rtx_frame_pos);
                }
                else
                {
                    /* FIX LM-Enh00000462    BEGIN 2011-11-09: wuxiaorong */
                    gsm_fcp_nocell_process(u32_next_fn,stp_oal_msg,NULL_PTR,stp_latest_rtx_frame_pos);
                    /* FIX LM-Enh00000462    END 2011-11-09: wuxiaorong */
                }
                /* FIX LM-Bug00000688 END  2011-08-22 : wuxiaorong */ 
            }
            else
            {
                gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            }
            /*FIX LM-Bug00001436  END   2011-12-14  : sunzhiqiang*/       
            /*  FIX NGM-Bug00000370  BEGIN  2014-08-21 :sunzhiqiang  */
            gsm_update_latest_fn_by_slave_meas_rpt_pos(u32_next_fn,stp_latest_rtx_frame_pos); 
            /*  FIX NGM-Bug00000370  END  2014-08-21 :sunzhiqiang  */

            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/   
            #ifdef L1CC_GSM_RTRT_UT_SWITCH
            if(OAL_SUCCESS == stub_gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            #else
            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            #endif
            {
                stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
                stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn + (UINT32)1),u16_active_id);
                
                /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                  stp_gsm_main_frametask_tmp1,
                                  &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
                
            }
            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/  

             /* 释放内存 */
            oal_mem_free((CONST_VOID_PTR*)&stp_latest_frame_pos_list);  
            
            break;

        case MSG_GSM_MEAS_FCP_FCBSUCC_IND:

            /*FIX LM-Bug00001470  BEGIN   2011-12-15  : sunzhiqiang*/
            
            stp_fcbsucc_ind = (msg_gsm_meas_l1cc_fcb_result_ind_t *)OAL_GET_OAL_MSG_BODY(stp_oal_msg);
            u16_standby_id = stp_fcbsucc_ind->u16_standby_id;
            /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */
            u16_rxtask_type = stp_fcbsucc_ind->u16_rxtask_type;

            
            if(GSM_CELLSEARCH == u16_rxtask_type)
            {
                if((UINT16)GSM_TABLE_VALID == stp_cellsearch_ctrl->u16_valid_flag)
                {
                    stp_cellsearch_tbl = gsm_get_cellsearch_tbl_by_standby(u16_standby_id);
                    if(NOCELL_SEARCH == stp_cellsearch_tbl->u16_search_type)
                    {
                        u32_fcp_ret = gsm_fcp_nocell_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                    }
                    else
                    {
                        u32_fcp_ret = gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                    }
                    /*FIX LM-Bug00001470  END   2011-12-15  : sunzhiqiang*/
                }
            }
            else 
            {
                
                 OAL_ASSERT(GSM_IRAT_GAP_MEAS != u16_rxtask_type,"u16_rxtask_type!= GSM_MEAS,error");
                 u32_fcp_ret = gsm_fcp_meas_fcbsucc_ind(u32_next_fn,stp_fcbsucc_ind,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                
            }
            /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */


            
            if(OAL_SUCCESS == u32_fcp_ret)
            {
                OAL_ASSERT((UINT32)(UINT32)GSM_INVALID_FN != u32_calculated_sb_fn,"gsm_fcp_cs,u32_calculated_sb_fn error");
                
                /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
                #ifdef L1CC_GSM_RTRT_UT_SWITCH
                if(OAL_SUCCESS == stub_gsm_fcp_get_undownloaded_slot_range(u32_calculated_sb_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                #else
                if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_calculated_sb_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                #endif
                {
                    stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_calculated_sb_fn,u16_active_id);
                    stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_calculated_sb_fn +(UINT32)1),u16_active_id);
                    
                    /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                    /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                    gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                    /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                    /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                    gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                      stp_gsm_main_frametask_tmp1,
                                      &u16_ts_bitmap[0], &u16_ts_bitmap[1]);                  

                }
                /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
            }
            break;

        /* FIX PhyFA-Req00001925 BEGIN 2014-08-05 : wuxiaorong */
        #if 0
        /* FIX PhyFA-Bug00005426 BEGIN 2014-07-25 : wuxiaorong */
        case MSG_GSM_L1RESP_FCP_SBFAIL_IND:
            if(OAL_FALSE == check_gsm_cellsearch_fcb_running())
            {
                stp_sbfail_msg = (msg_gsm_l1resp_fcp_sb_result_ind_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));
                gsm_fcp_set_meas_task(stp_sbfail_msg->u32_next_sb_search_fn,stp_latest_rtx_frame_pos);
                /* FIX PhyFA-Bug00005464  BEGIN 2014-07-30 : wuxiaorong */
                if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(stp_sbfail_msg->u32_next_sb_search_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                {
                    stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(stp_sbfail_msg->u32_next_sb_search_fn,u16_active_id);
                    stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(stp_sbfail_msg->u32_next_sb_search_fn+(UINT32)1),u16_active_id);
                    gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                    stp_gsm_main_frametask_tmp1,
                                    &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
                }
                /* 是当帧陪当帧,u32_next_sb_search_fn = u32_next_fn-1  */
                if(gsm_fn_compare(stp_sbfail_msg->u32_next_sb_search_fn,u32_next_fn))
                {
                    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
                } 
                 /* FIX PhyFA-Bug00005464  END   2014-07-30 : wuxiaorong */
            }
            break;
        /* FIX PhyFA-Bug00005426 END   2014-07-25 : wuxiaorong */
        #endif
        /* FIX PhyFA-Req00001925 END   2014-08-05 : wuxiaorong */
        default:
            /* FIX PhyFA-Enh00001698 BEGIN  2014-04-23  : wangjunli*/
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            OAL_PRINT((UINT16)OAL_GET_OAL_MSG_STANDBY_ID(stp_oal_msg),(UINT16)GSM_MODE,"gsm_fcp_cs receive unexpected msg:%ld",OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg));
            /* FIX PhyFA-Enh00001698 END  2014-04-23  : wangjunli*/
            break;
    }

    return OAL_SUCCESS;
}

/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_nocell
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*   2014-07-17      wuxiaorong              PhyFA-Bug00005346   [NGM]G+T下G在nocell，T待机时出现0x13f00
*   2014-07-25      wuxiaorong              PhyFA-Bug00005426   [NGM]gsm辅模式状态下的sbsearch流程的任务配置需要变更.
*   2014-07-30      wuxiaorong              PhyFA-Bug00005464   [NGM]GSM的SB search当帧配当帧睡眠控制latest_rtx_frame_pos设置有问题 
*   2014-08-12      xiongjiangjiang         PhyFA-Bug00005562   [NGM]GSM辅模式时需要增加在假的帧中断下用下一次的上报时间点来更新最近收发帧
***********************************************************************************************************************/
/* FIX PhyFA-Req00000936 BEGIN  2011-06-09  : wuxiaorong */
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_nocell(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos)
{
    /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
    UINT16 u16_k = 0;
    /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/
    
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp0 = NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp1 = NULL_PTR;    
    UINT16  u16_ts_bitmap[2] = {0,0}; 
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
    /* FIX LM-Bug00001744  BEGIN   2012-02-25  : wanghairong */
    UINT16 u16_standby_id = UNSPECIFIED_STANDBY_ID;
    oal_primitive_id_t    msg_primitive_id;
    /* FIX LM-Bug00001744  END   2012-02-25  : wanghairong */
    UINT32  u32_calculated_sb_fn = (UINT32)(UINT32)GSM_INVALID_FN;
    OAL_STATUS u32_fcp_ret = OAL_FAILURE;
    /*FIX LM-Bug00000992  BEGIN  2011-10-13 :fushilong*/
    BOOLEAN b_nocell_fcb_running = OAL_FALSE; 
    CONST gsm_cellsearch_t  *stp_cellsearch_tbl= NULL_PTR;
    gsm_nocell_search_ctrl_t  *stp_nocell_search= NULL_PTR;
    CONST gsm_nc_meas_sync_ctrl_t*  stp_nc_meas_sync_ctrl = NULL_PTR;
    /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wuxiaorong */
    msg_gsm_meas_l1cc_fcb_result_ind_t CONST*stp_fcbsucc_ind =  NULL_PTR;
    UINT16   u16_rxtask_type;
    /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */
    mmc_time_info_t st_mmc_time_info;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    gsm_cellsearch_ctrl_t *stp_cellsearch_ctrl = NULL_PTR; 
    gsm_tstask_latest_frame_pos_t*  stp_latest_frame_pos_list = NULL_PTR;
    UINT16 u16_active_id;
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR;
    /* FIX PhyFA-Bug00005665 BEGIN 2014-09-01 : wuxiaorong */
    SINT16  s16_fn_begin_offset= 0;
    /* FIX PhyFA-Bug00005665 END   2014-09-01 : wuxiaorong */
    
    u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    stp_gsm_standby_info = gsm_get_standby_info();
    stp_nc_meas_sync_ctrl = gsm_get_nc_meas_sync_ctrl();
    stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();
    
    OAL_ASSERT(NULL_PTR!= stp_gsm_latest_rtx_frame_pos,"gsm_fcp_nocell():stp_gsm_latest_rtx_frame_pos is NULL_PTR!");   
    
    stp_nocell_search = &(stp_cellsearch_ctrl->st_nocell_search);
    /*FIX LM-Bug00000992  END 2011-10-13 :fushilong*/    
    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
    stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)13));
    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */
    /* FIX LM-Bug00001744  BEGIN   2012-02-25  : wanghairong */
    if(NULL_PTR == stp_oal_msg)
    {
        oal_error_handler(ERROR_CLASS_CRITICAL, GSM_FCP_ERR(INVALID_PARAM));  
        return OAL_FAILURE;
    }
    u16_standby_id   = OAL_GET_OAL_MSG_STANDBY_ID(stp_oal_msg);
    msg_primitive_id = OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg);
    switch(msg_primitive_id)
    /* FIX LM-Bug00001744  END   2012-02-25  : wanghairong */
    {
        case MSG_ISR_FCP_FRAMETIMER_IND:
            /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
            /* 申请内存 */
            stp_latest_frame_pos_list = (gsm_tstask_latest_frame_pos_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,
                                                                                  (UINT32)OAL_GET_TYPE_SIZE_IN_WORD(gsm_tstask_latest_frame_pos_t));  
            stp_latest_frame_pos_list->u16_latest_pos_num = 0;
            /*add TC_BCCH_RX task for pre-read SI within slave_active_state when the tb is valid*/
            for(u16_k = 0; u16_k <(UINT16)MAX_STANDBY_AMT; u16_k++)
            {                
                if((UINT16)GSM_SLAVE_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[u16_k]) //slave_active_state
                {
                    u16_standby_id = stp_gsm_standby_info->u16_standby_id[u16_k];  
                    gsm_add_slave_standby_tsntask(u32_next_fn,u16_standby_id,stp_latest_frame_pos_list); 
                }
            }

            OAL_ASSERT(stp_latest_frame_pos_list->u16_latest_pos_num <= MAX_STANDBY_AMT,"");
            gsm_fcp_get_latest_frame_pos(stp_latest_frame_pos_list,stp_latest_rtx_frame_pos);

            /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/

            /* 释放内存 */
            oal_mem_free((CONST_VOID_PTR*)&stp_latest_frame_pos_list);  
            
            /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : caisiwen */
            if(OAL_FALSE == check_gsm_cellsearch_fcb_running())
            {
                /* FIX PhyFA-Bug00004912 BEGIN 2014-06-05 : wuxiaorong */
                /* 配置辅模式测量 */ 
                /* FIX PhyFA-Bug00005665 BEGIN 2014-09-01 : wuxiaorong */
                /* 睡眠唤醒后的假的帧中断消息不能按普通方式配测量.需要限定范围 */
                if(OAL_FALSE == gsm_check_pseudo_frametimer(stp_oal_msg,&s16_fn_begin_offset))
                {
                    gsm_fcp_set_meas_task(u32_next_fn,stp_latest_rtx_frame_pos);
                }
                else
                {
                    /* s16_fn_begin_offset不为0时,只进行辅模式的ReBSIC和RSSI测量;s16_fn_begin_offset=0时,内部调用gsm_fcp_set_meas_task() */
                    gsm_fcp_set_selfwakeup_nextfn_bsic_rssi_meas_task(u32_next_fn,s16_fn_begin_offset,stp_latest_rtx_frame_pos);
                }
                /* FIX PhyFA-Bug00005665 END   2014-09-01 : wuxiaorong */
                /* FIX PhyFA-Bug00004912 END   2014-06-05 : wuxiaorong */
            }
            /* FIX PhyFA-Req00001026 END 2012-04-10  : caisiwen */

            /* FIX PhyFA-Bug00005346 BEGIN 2014-07-17 : wuxiaorong */
            if((UINT16)GSM_TABLE_VALID == stp_cellsearch_ctrl->u16_valid_flag)
            {            
                if((UINT16)0 == stp_nc_meas_sync_ctrl->u16_running_bitmap)
                {   /* FIX LM-Bug00000992  BEGIN  2011-10-13 :fushilong */
                    if(((UINT16)1 == (stp_cellsearch_ctrl->st_normal_search.u16_valid_flag)))
                    {
                        /* FIX NGM-Bug00000677 BEGIN 2014-10-09 : wuxiaorong */
                        if(((UINT16)1 == stp_nocell_search->u16_valid_flag)&&(stp_nocell_search->u16_standby_num_from_mmc > (UINT16)0))
                        /* FIX NGM-Bug00000677 END   2014-10-09 : wuxiaorong */
                        {
                            /* u16_standby_num_from_mmc>0, u16_cur_standby_id 必定不是UNSPECIFIED_STANDBY_ID */
                            stp_cellsearch_tbl = &(stp_cellsearch_ctrl->st_cellsearch_tbl[stp_nocell_search->u16_cur_standby_id]);
                            if(((UINT16)1 == stp_cellsearch_tbl->u16_valid_flag) 
                              &&((UINT16)GSM_BG_FCB_SETTING == stp_cellsearch_tbl->u16_fcb_set_flg))
                            {
                                b_nocell_fcb_running = OAL_TRUE; 
                            }
                            /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
                            if(OAL_TRUE == b_nocell_fcb_running)
                            {
                                dd_event_stop_by_standby(u16_active_id,(UINT16)((UINT16)0x0001<<stp_nocell_search->u16_cur_standby_id),(UINT16)GSM_TIMING,&st_mmc_time_info);
                                gsm_fcp_del_all_frametask((UINT16)UNSPECIFIED_STANDBY_ID);                
                                gsm_fcp_nocell_next_search_or_sleep(stp_nocell_search);
                            }
                            /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/           
                        }
                        /* FIX LM-Bug00000992  END 2011-10-13 :fushilong */
                        gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,NULL_PTR,stp_latest_rtx_frame_pos);
                    }
                    else
                    {
                        gsm_fcp_nocell_process(u32_next_fn,stp_oal_msg,NULL_PTR,stp_latest_rtx_frame_pos);
                        if(stp_latest_rtx_frame_pos->u32_fn == gsm_fn_operate(u32_next_fn + (UINT32)13))
                        {
                            if((UINT32)GSM_INVALID_FN == stp_nocell_search->u32_wakeup_fn)
                            {
                                 gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
                            }
                            else if((UINT32)SLEEP_FRAME_NUM_INFINITE == stp_nocell_search->u32_wakeup_fn)
                            {
                                ;
                            }
                            else
                            {
                                if(gsm_fn_compare(stp_latest_rtx_frame_pos->u32_fn,stp_nocell_search->u32_wakeup_fn))
                                {
                                    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,stp_nocell_search->u32_wakeup_fn);
                                    /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
                                    OAL_PRINT(u16_standby_id,
                                    (UINT16)GSM_MODE,
                                    "gsm_fcp_nocell(): re-calculate s_u32_latest_rtx_fn[u16_active_id]=u32_wakeup_fn,g_u32_gsm_fn[u16_active_id]=0x%lx,u32_next_fn=0x%lx,u32_wakeup_fn =%ld", 
                                    (UINT32)g_u32_gsm_fn[u16_active_id],
                                    (UINT32)u32_next_fn,                                   
                                    (UINT32)stp_nocell_search->u32_wakeup_fn);  
                                    /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
                                }
                            }
                        }
                    }
                }
                else
                {
                   gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
                }
            }
            /* FIX PhyFA-Bug00005346 END   2014-07-17 : wuxiaorong */
            /*  FIX NGM-Bug00000370  BEGIN  2014-08-21 :sunzhiqiang  */
            gsm_update_latest_fn_by_slave_meas_rpt_pos(u32_next_fn,stp_latest_rtx_frame_pos);
            /*  FIX NGM-Bug00000370  END  2014-08-21 :sunzhiqiang  */

            /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
            #ifdef L1CC_GSM_RTRT_UT_SWITCH
            if(OAL_SUCCESS == stub_gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            #else
            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            #endif   
            {
                stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
                stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn+(UINT32)1),u16_active_id);
                
                /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                  stp_gsm_main_frametask_tmp1,
                                  &u16_ts_bitmap[0], &u16_ts_bitmap[1]);                
            }
            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/               
            break;
        case MSG_GSM_MEAS_FCP_FCBSUCC_IND:

            /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */
            stp_fcbsucc_ind = (msg_gsm_meas_l1cc_fcb_result_ind_t *)OAL_GET_OAL_MSG_BODY(stp_oal_msg);
            u16_standby_id = stp_fcbsucc_ind->u16_standby_id;
            u16_rxtask_type = stp_fcbsucc_ind->u16_rxtask_type;
            if(GSM_CELLSEARCH == u16_rxtask_type)
            {
                if((UINT16)GSM_TABLE_VALID == stp_cellsearch_ctrl->u16_valid_flag)
                {
                    stp_cellsearch_tbl = gsm_get_cellsearch_tbl_by_standby(u16_standby_id);
                    if(NOCELL_SEARCH == stp_cellsearch_tbl->u16_search_type)
                    {
                        u32_fcp_ret = gsm_fcp_nocell_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                    }
                    else
                    {
                        u32_fcp_ret = gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                    }
                }
            }
            else 
            {
                
                 OAL_ASSERT(GSM_IRAT_GAP_MEAS         != u16_rxtask_type,"u16_rxtask_type!= GSM_MEAS,error");
                 u32_fcp_ret = gsm_fcp_meas_fcbsucc_ind(u32_next_fn,stp_fcbsucc_ind,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                
            }
            /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */

            
            if(OAL_SUCCESS == u32_fcp_ret)
            {
                OAL_ASSERT((UINT32)(UINT32)GSM_INVALID_FN != u32_calculated_sb_fn,"gsm_fcp_nocell,u32_calculated_sb_fn error");
                /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
                #ifdef L1CC_GSM_RTRT_UT_SWITCH
                if(OAL_SUCCESS == stub_gsm_fcp_get_undownloaded_slot_range(u32_calculated_sb_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                #else
                if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_calculated_sb_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                #endif
                {
                    stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_calculated_sb_fn,u16_active_id);
                    stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_calculated_sb_fn+(UINT32)1),u16_active_id);
                    
                    /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                    /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                    gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                    /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                    /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                    gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                      stp_gsm_main_frametask_tmp1,
                                      &u16_ts_bitmap[0], &u16_ts_bitmap[1]);                    

                }
                /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
            }
            break;
        /* FIX PhyFA-Req00001925 BEGIN 2014-08-05 : wuxiaorong */
        #if 0
        /* FIX PhyFA-Bug00005426 BEGIN 2014-07-25 : wuxiaorong */
        case MSG_GSM_L1RESP_FCP_SBFAIL_IND:
            if(OAL_FALSE == check_gsm_cellsearch_fcb_running())
            {
                stp_sbfail_msg = (msg_gsm_l1resp_fcp_sb_result_ind_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));
                gsm_fcp_set_meas_task(stp_sbfail_msg->u32_next_sb_search_fn,stp_latest_rtx_frame_pos);
                /* FIX PhyFA-Bug00005464  BEGIN 2014-07-30 : wuxiaorong */
                
               
                if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(stp_sbfail_msg->u32_next_sb_search_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                {
                    stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(stp_sbfail_msg->u32_next_sb_search_fn,u16_active_id);
                    stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(stp_sbfail_msg->u32_next_sb_search_fn+(UINT32)1),u16_active_id);
                    gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                    stp_gsm_main_frametask_tmp1,
                                    &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
                }
                /* 是当帧陪当帧,u32_next_sb_search_fn = u32_next_fn-1  */
                if(gsm_fn_compare(stp_sbfail_msg->u32_next_sb_search_fn,u32_next_fn))
                {
                    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
                } 
                /* FIX PhyFA-Bug00005464  END   2014-07-30 : wuxiaorong */
             }
             break;
        /* FIX PhyFA-Bug00005426 END   2014-07-25 : wuxiaorong */
        #endif
        /* FIX PhyFA-Req00001925 END   2014-08-05 : wuxiaorong */
        default:
            /* FIX LM-Bug00001744  BEGIN   2012-02-25  : wanghairong */
            /* FIX PhyFA-Enh00001698 BEGIN  2014-04-23  : wangjunli*/
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            OAL_PRINT((UINT16)OAL_GET_OAL_MSG_STANDBY_ID(stp_oal_msg),(UINT16)GSM_MODE,"gsm_fcp_nocell receive unexpected msg:%ld",OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg));
            /* FIX PhyFA-Enh00001698 END  2014-04-23  : wangjunli*/
            /* FIX LM-Bug00001744  END   2012-02-25  : wanghairong */
            break;
    }

    return OAL_SUCCESS;
}
/* FIX PhyFA-Req00000936 END  2011-06-09  : wuxiaorong */ 

/************************************************************
** Function Name: gsm_fcp_init_state ()
** Description: the handler of the state GSM_INIT of the module FCP
** Input :
      u32_next_fn:  the next frame number
      stp_oal_msg:        message received from other module
** Output : NULL
** Return: OAL_SUCCESS or OAL_FAILURE
** Notes:
2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_init_state(CONST_UINT32 u32_next_fn,oal_msg_t CONST * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos)
{
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR;
    OAL_ASSERT((NULL_PTR != stp_oal_msg), "gsm_fcp_init_state : stp_oal_msg is NULL");  
    OAL_ASSERT((NULL_PTR != stp_gsm_latest_rtx_frame_pos), "gsm_fcp_init_state : stp_oal_msg is NULL");  
    OAL_ASSERT(u32_next_fn < (UINT32)GSM_MAX_FN,"u32_next_fn >=GSM_MAX_FN");

    /* FIX PhyFA-Enh00001698 BEGIN  2014-04-23  : wangjunli*/
    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */    
    stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */    
    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,(UINT32) SLEEP_FRAME_NUM_INFINITE);
    switch(OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg))
    {
        case MSG_ISR_FCP_FRAMETIMER_IND:
            break;
        default:
            OAL_PRINT((UINT16)OAL_GET_OAL_MSG_STANDBY_ID(stp_oal_msg),(UINT16)GSM_MODE,"gsm_fcp_init_state receive unexpected msg:%ld",OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg));
            /* FIX PhyFA-Enh00001698 END  2014-04-23  : wangjunli*/
            break;
    }

    return OAL_SUCCESS;
}

/************************************************************
** Function Name: gsm_fcp_init ()
** Description: the initial function of the module FCP
** Input : NULL
** Output : NULL
** Return: OAL_SUCCESS or OAL_FAILURE
** Notes:
2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
OAL_STATUS gsm_fcp_init(CONST_UINT16 u16_standby_id)
{
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    gsm_frametimer_change_info_t*  stp_frametimer_change_info;
    
    
    
    if(UNSPECIFIED_STANDBY_ID == u16_standby_id)
    {
        /*oal_mem_mode_init(OAL_MEM_GSMIDLE_TDIDLE);*/   
        #ifdef L1CC_GSM_RTRT_UT_SWITCH
        stub_gsm_fcp_del_all_frametask((UINT16)UNSPECIFIED_STANDBY_ID);
        #else
        gsm_fcp_del_all_frametask((UINT16)UNSPECIFIED_STANDBY_ID);
        #endif

        /* FIX AM-Bug00000320  BEGIN  2010-11-16 : wuxiaorong */
        gsm_set_fcp_3rd_state(GSM_INVALID_STATE);
        /* FIX AM-Bug00000320  END  2010-11-16 : wuxiaorong */

        gsm_init_rtx_overlap_gap_info();
        g_u16_state_change_standby_id[u16_active_id] = UNSPECIFIED_STANDBY_ID;
    } 


    stp_frametimer_change_info = gsm_get_frametimer_change_info();
    stp_frametimer_change_info->s32_delta_offset = 0;
    stp_frametimer_change_info->s32_delta_fn = (SINT32)GSM_INVALID_FN;

    
    return OAL_SUCCESS;
}

/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_slave
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*   2014-07-03      wuxiaorong              PhyFA-Bug00005194   [NGM] 异模式idle下的GSM辅模式测量FCB成功的u16_rxtask_type为GSM_GAP_MEAS
*   2014-07-25      wuxiaorong              PhyFA-Bug00005426   [NGM]gsm辅模式状态下的sbsearch流程的任务配置需要变更.
*   2014-07-30      wuxiaorong              PhyFA-Bug00005464   [NGM]GSM的SB search当帧配当帧睡眠控制latest_rtx_frame_pos设置有问题 
*   2014-08-05      wuxiaorong              PhyFA-Req00001925   [NGM]T业务下G测量在当前接收未完成前就需要帧中断下次接收任务 .
*   2014-08-12      xiongjiangjiang         PhyFA-Bug00005562   [NGM]GSM辅模式时需要增加在假的帧中断下用下一次的上报时间点来更新最近收发帧
*   2014-09-11      linlan                  NGM-Bug00000499     [NGM]GSM在异模式业务状态下更新了lastest_fn导致另一待前景搜网FCB配置不下去
*   2014-09-20      wuxiaorong              NGM-Bug00000585     [NGM]G辅模SB Search跨帧配置出现N帧配N+2帧导致后续N+1帧配N+2帧时把先前配置任务删除
***********************************************************************************************************************/

/* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_slave(UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,gsm_latest_rtx_frame_pos_t * CONST stp_gsm_latest_rtx_frame_pos)
{
    UINT16  u16_k = 0;
    UINT16  u16_standby_id      = UNSPECIFIED_STANDBY_ID;
    CONST msg_gsm_meas_l1cc_fcb_result_ind_t *stp_fcbsucc_ind = NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp0 = NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp1 = NULL_PTR; 
    UINT16  u16_ts_bitmap[2] = {0,0}; 
    UINT16  u16_rxtask_type;
    UINT32  u32_calculated_sb_fn = (UINT32)(UINT32)GSM_INVALID_FN;
    OAL_STATUS u32_status = OAL_FAILURE;
    CONST gsm_cellsearch_t  *stp_cellsearch_tbl= NULL_PTR;
    CONST gsm_nc_meas_sync_ctrl_t*  stp_nc_meas_sync_ctrl = NULL_PTR;

    CONST gsm_cellsearch_ctrl_t *stp_cellsearch_ctrl = NULL_PTR;
    gsm_tstask_latest_frame_pos_t*  stp_latest_frame_pos_list = NULL_PTR;
    /* FIX LTE-Bug00001291    BEGIN 2012-09-22: wangjunli */ 
    CONST gsm_nocell_search_ctrl_t *  stp_nocell_search = NULL_PTR;
    /* FIX LTE-Bug00001291    END 2012-09-22: wangjunli */ 
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR;
    /* FIX PhyFA-Bug00005665 BEGIN 2014-09-01 : wuxiaorong */
    SINT16  s16_fn_begin_offset= 0;
    /* FIX PhyFA-Bug00005665 END   2014-09-01 : wuxiaorong */
    /* FIX NGM-Bug00000585 BEGIN 2014-09-20 : wuxiaorong */
    UINT16  u16_rtx_bitmap = 0;
    /* FIX NGM-Bug00000585 END   2014-09-20 : wuxiaorong */

    
    
    OAL_ASSERT(NULL_PTR!=stp_oal_msg, "gsm_fcp_slave null msg!");    
    OAL_ASSERT((NULL_PTR != stp_gsm_latest_rtx_frame_pos), "gsm_fcp_slave : stp_gsm_latest_rtx_frame_pos is NULL");  
    stp_gsm_standby_info = gsm_get_standby_info(); 
    stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();
    stp_nc_meas_sync_ctrl = gsm_get_nc_meas_sync_ctrl();
    stp_nocell_search = &stp_cellsearch_ctrl->st_nocell_search;
    /* FIX NGM-Bug00000499 BEGIN 2014-09-11 : linlan */
    stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
    /* FIX NGM-Bug00000499 END   2014-09-11 : linlan */
        
    switch(OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg))
    {
        case MSG_ISR_FCP_FRAMETIMER_IND: 
        /*帧定时消息触发辅模式状态相应任务配置*/
        {

            /* 申请内存 */
            stp_latest_frame_pos_list = (gsm_tstask_latest_frame_pos_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,
                                                                                  (UINT32)OAL_GET_TYPE_SIZE_IN_WORD(gsm_tstask_latest_frame_pos_t));  
            stp_latest_frame_pos_list->u16_latest_pos_num = 0;

            /*add TC_BCCH_RX task for pre-read SI within slave_active_state when the tb is valid*/
            for(u16_k = 0; u16_k <(UINT16)MAX_STANDBY_AMT; u16_k++)
            {                
                if((UINT16)GSM_SLAVE_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[u16_k]) /* slave_active_state */
                {
                    u16_standby_id = stp_gsm_standby_info->u16_standby_id[u16_k];  
                    gsm_add_slave_standby_tsntask(u32_next_fn,u16_standby_id,stp_latest_frame_pos_list); 
                }
            }
            /* 先临时匹配修改 */
            OAL_ASSERT(stp_latest_frame_pos_list->u16_latest_pos_num <= MAX_STANDBY_AMT,"");
  
            gsm_fcp_get_latest_frame_pos(stp_latest_frame_pos_list,stp_latest_rtx_frame_pos);
            /* FIX NGM-Bug00000585 BEGIN 2014-09-20 : wuxiaorong */
            u16_rtx_bitmap = gsm_get_frame_task_rtx_bitmap(gsm_get_main_frametask(u32_next_fn,u16_active_id));
            if(0 != u16_rtx_bitmap)
            { 
                /*FIX NGM-Bug00000358  BEGIN   2014-08-22  : linlan*/
                gsm_check_del_running_sync_nslot_task(u32_next_fn,u16_rtx_bitmap);
                /*gsm_del_fcb_task_reset_cellsearch_tb();  */              
                /*FIX NGM-Bug00000358  END     2014-08-22  : linlan*/
            } 
            /* FIX NGM-Bug00000585 END   2014-09-20 : wuxiaorong */


            /*set_slave_meas_task*/
            if(OAL_FALSE == check_gsm_cellsearch_fcb_running()) 
            {
                /* FIX PhyFA-Bug00004912 BEGIN 2014-06-05 : wuxiaorong */
                /*set_slave_meas_task.*/
                /* FIX PhyFA-Bug00005665 BEGIN 2014-09-01 : wuxiaorong */
                /* 睡眠唤醒后的假的帧中断消息不能按普通方式配测量.需要限定范围 */
                if(OAL_FALSE == gsm_check_pseudo_frametimer(stp_oal_msg,&s16_fn_begin_offset))
                {
                    gsm_fcp_set_meas_task(u32_next_fn,stp_latest_rtx_frame_pos);
                }
                else
                {
                    /* s16_fn_begin_offset不为0时,只进行辅模式的ReBSIC和RSSI测量;s16_fn_begin_offset=0时,内部调用gsm_fcp_set_meas_task() */
                    gsm_fcp_set_selfwakeup_nextfn_bsic_rssi_meas_task(u32_next_fn,s16_fn_begin_offset,stp_latest_rtx_frame_pos);
                }
                /* FIX PhyFA-Bug00005665 END   2014-09-01 : wuxiaorong */
                /* FIX PhyFA-Bug00004912 END   2014-06-05 : wuxiaorong */ 
            }

            /*add cell search task within slave_active_state when the tb is valid*/
            if((UINT16)GSM_TABLE_VALID == stp_cellsearch_ctrl->u16_valid_flag)
            {            
               if((OAL_FALSE == check_gsm_cellsearch_fcb_running())
                  &&((UINT16)0 == stp_nc_meas_sync_ctrl->u16_running_bitmap))
               {
                    /*normal_search within slave mode pro*/
                    gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,NULL_PTR,stp_latest_rtx_frame_pos);
                    
                    if(OAL_FALSE == check_gsm_cellsearch_fcb_running())
                    {                    
                        gsm_fcp_nocell_process(u32_next_fn,stp_oal_msg,NULL_PTR,stp_latest_rtx_frame_pos);
                        /* FIX LTE-Bug00001291    BEGIN 2012-09-22: wangjunli */                         
                        if(stp_latest_rtx_frame_pos->u32_fn == gsm_fn_operate(u32_next_fn + (UINT32)13))
                        {
                            //OAL_PRINT(0,(UINT16)GSM_MODE,"u32_wakeup_fn=%lu s_u32_latest_rtx_fn[u16_active_id]=%lu",stp_nocell_search->u32_wakeup_fn,s_u32_latest_rtx_fn[u16_active_id]);
                            if((UINT32)GSM_INVALID_FN == stp_nocell_search->u32_wakeup_fn)
                            {
                                gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
                            }
                            else if((UINT32)SLEEP_FRAME_NUM_INFINITE == stp_nocell_search->u32_wakeup_fn)
                            {
                                ;
                            }
                            else
                            {
                                if(gsm_fn_compare(stp_latest_rtx_frame_pos->u32_fn,stp_nocell_search->u32_wakeup_fn))
                                {
                                    gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,stp_nocell_search->u32_wakeup_fn);    
                                    OAL_PRINT(u16_standby_id,
                                    (UINT16)GSM_MODE,
                                    "gsm_fcp_slave(): re-calculate s_u32_latest_rtx_fn[u16_active_id]=u32_wakeup_fn,g_u32_gsm_fn[u16_active_id]=0x%lx,u32_next_fn=0x%lx,u32_wakeup_fn =%ld", 
                                    (UINT32)g_u32_gsm_fn[u16_active_id],
                                    (UINT32)u32_next_fn,                                   
                                    (UINT32)stp_nocell_search->u32_wakeup_fn);  
                                }
                            }
                        }
                        /* FIX LTE-Bug00001291    END 2012-09-22: wangjunli */ 
                    }
               }
               else
               {
                   gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
               }
            }
            /*  FIX NGM-Bug00000370  BEGIN  2014-08-21 :sunzhiqiang  */
            gsm_update_latest_fn_by_slave_meas_rpt_pos(u32_next_fn,stp_latest_rtx_frame_pos); 
            /*  FIX NGM-Bug00000370  END  2014-08-21 :sunzhiqiang  */
            OAL_ASSERT((stp_latest_rtx_frame_pos->u32_fn!= u32_next_fn)&&(stp_latest_rtx_frame_pos->u32_fn != (UINT32)GSM_INVALID_FN),"gsm_fcp_cs_idle():s_u32_latest_rtx_fn[u16_active_id] calculated fail!");

         
            /*download rtx event */
            #ifdef L1CC_GSM_RTRT_UT_SWITCH
            if(OAL_SUCCESS == stub_gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            #else
            if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
            #endif
            {
                stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
                stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn+(UINT32)1),u16_active_id);
                
                /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                stp_gsm_main_frametask_tmp1,
                                &u16_ts_bitmap[0], &u16_ts_bitmap[1]);                

            }

            /* 释放内存 */
            oal_mem_free((CONST_VOID_PTR*)&stp_latest_frame_pos_list);  
             
            break;
        }         
        case MSG_GSM_MEAS_FCP_FCBSUCC_IND:
        {            
            stp_fcbsucc_ind = (msg_gsm_meas_l1cc_fcb_result_ind_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));            
            u16_standby_id = stp_fcbsucc_ind->u16_standby_id;
            u16_rxtask_type = stp_fcbsucc_ind->u16_rxtask_type;
            /* FIX PhyFA-Bug00005194 BEGIN 2014-07-03 : wuxiaorong */
            if(GSM_CELLSEARCH == GSM_RXTASK_TYPE_MASK(u16_rxtask_type))
            {
                if((UINT16)GSM_TABLE_VALID == stp_cellsearch_ctrl->u16_valid_flag)
                {
                    stp_cellsearch_tbl = gsm_get_cellsearch_tbl_by_standby(u16_standby_id);
                    if(NOCELL_SEARCH == stp_cellsearch_tbl->u16_search_type)
                    {
                        u32_status = gsm_fcp_nocell_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                    }
                    else
                    {
                        u32_status = gsm_fcp_bg_process(u32_next_fn,stp_oal_msg,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                    }
                }
            }
            else 
            {
                
                 OAL_ASSERT((GSM_MEAS == GSM_RXTASK_TYPE_MASK(u16_rxtask_type))||(GSM_IRAT_GAP_MEAS == GSM_RXTASK_TYPE_MASK(u16_rxtask_type)),"u16_rxtask_type!= GSM_MEAS and GSM_IRAT_GAP_MEAS,error");
                 u32_status = gsm_fcp_meas_fcbsucc_ind(u32_next_fn,stp_fcbsucc_ind,&u32_calculated_sb_fn,stp_latest_rtx_frame_pos);
                
            }
            /* FIX PhyFA-Bug00005194 END   2014-07-03 : wuxiaorong */
            /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */
            
            /*download rtx event */
            if(OAL_SUCCESS == u32_status)
            {
                OAL_ASSERT((UINT32)(UINT32)GSM_INVALID_FN != u32_calculated_sb_fn,"gsm_fcp_slave,u32_calculated_sb_fn error");
                #ifdef L1CC_GSM_RTRT_UT_SWITCH
                if(OAL_SUCCESS == stub_gsm_fcp_get_undownloaded_slot_range(u32_calculated_sb_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                #else
                if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_calculated_sb_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                #endif
                {
                    stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_calculated_sb_fn,u16_active_id);
                    stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_calculated_sb_fn+(UINT32)1),u16_active_id);
                    
                    /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                    /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                    gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                    /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                    /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                    gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                    stp_gsm_main_frametask_tmp1,
                                    &u16_ts_bitmap[0], &u16_ts_bitmap[1]);                    

                }
            }
            break;
        }
        /* FIX PhyFA-Req00001925 BEGIN 2014-08-05 : wuxiaorong */
        #if 0
        /* FIX PhyFA-Bug00005426 BEGIN 2014-07-25 : wuxiaorong */
        case MSG_GSM_L1RESP_FCP_SBFAIL_IND:
            if(OAL_FALSE == check_gsm_cellsearch_fcb_running())
            {
                stp_sbfail_msg = (msg_gsm_l1resp_fcp_sb_result_ind_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));
                gsm_fcp_set_meas_task(stp_sbfail_msg->u32_next_sb_search_fn,stp_gsm_latest_rtx_frame_pos);
                /* FIX PhyFA-Bug00005464  BEGIN 2014-07-30 : wuxiaorong */
                if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(stp_sbfail_msg->u32_next_sb_search_fn, (UINT16)7, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                {
                    stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(stp_sbfail_msg->u32_next_sb_search_fn,u16_active_id);
                    stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(stp_sbfail_msg->u32_next_sb_search_fn+(UINT32)1),u16_active_id);
                    gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                    stp_gsm_main_frametask_tmp1,
                                    &u16_ts_bitmap[0], &u16_ts_bitmap[1]);
                }
                /* 是当帧陪当帧,u32_next_sb_search_fn = u32_next_fn-1  */
                if(gsm_fn_compare(stp_sbfail_msg->u32_next_sb_search_fn,u32_next_fn))
                {
                    gsm_set_latest_rtx_frame_pos_fnhead(stp_gsm_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
                } 
                /* FIX PhyFA-Bug00005464  END   2014-07-30 : wuxiaorong */
            }
             break;
        /* FIX PhyFA-Bug00005426 END   2014-07-25 : wuxiaorong */
        #endif
        /* FIX PhyFA-Req00001925 END   2014-08-05 : wuxiaorong */
        default:
            /* FIX PhyFA-Enh00001698 BEGIN  2014-04-23  : wangjunli*/
            gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_next_fn + (UINT32)1));
            OAL_PRINT((UINT16)OAL_GET_OAL_MSG_STANDBY_ID(stp_oal_msg),(UINT16)GSM_MODE,"gsm_fcp_slave receive unexpected msg:%ld",OAL_GET_OAL_MSG_PRIMITIVE_ID(stp_oal_msg));
            /* FIX PhyFA-Enh00001698 END  2014-04-23  : wangjunli*/
            break;
    }
    return OAL_SUCCESS;
}
/* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/

/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_time_adj
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION 
OAL_STATUS gsm_fcp_time_adj(SINT32 s32_delta_fn, SINT16 s16_offset_in_frame)
{   
    /*FIX LM-Bug00000934  BEGIN  2011-10-08 :wanghairong*/
    u_time_info_t st_time_info;
    /*FIX LM-Bug00000934  END  2011-10-08 :wanghairong*/
    /*FIX LM-Bug00001464   BEGIN  2011-12-14 :linlan*/
    gsm_search_rf_tb_t *stp_search_rf_tb = NULL_PTR;
    UINT16 u16_arfcn_scaned_times;
    /*FIX LM-Bug00001464   END  2011-12-14 :linlan*/
    /*FIX LM-Bug00001353  BEGIN  2011-11-30 :sunzhiqiang*/
    gsm_cellsearch_t *stp_cellsearch_tbl = NULL_PTR;
    UINT32  u32_temp_sb_rx_fn = (UINT32)0;
    SINT16  s16_temp_sb_rx_offset = 0;
    UINT16  i = 0;
    UINT16  u16_standby_id = UNSPECIFIED_STANDBY_ID;
    UINT16  u16_normal_search_valid = 0;
    UINT16  u16_nocell_search_valid = 0;   
    /*FIX LM-Bug00001353  END  2011-11-30 :sunzhiqiang*/
    /* FIX LM-Bug00001771   BEGIN  2011-12-14 : linlan */
    SINT32 s32_rx_fn_offset;
    UINT16  u16_new_tsn;
    SINT16 s16_new_offset;
    /* FIX PhyFA-Req00001026 BEGIN 2012-04-10  : caisiwen */
    gsm_slave_interrat_meas_req_t *stp_slave_interrat_meas = NULL_PTR;
    UINT32 u32_temp_slave_rpt_fn = (UINT32)0;
    UINT16 u16_page_offset        = 0;
    gsm_ncell_rssi_meas_t  *stp_ncell_rssi_meas = NULL_PTR;
    /* FIX PhyFA-Req00001026 END 2012-04-10  : caisiwen */
    /* FIX PhyFA-Req00001525 BEGIN 2013-10-09 : wuxiaorong */
    UINT16 u32_first_startfn;
    /* FIX PhyFA-Req00001525 END   2013-10-09 : wuxiaorong */
    gsm_meas_req_t* stp_gsm_meas_req= NULL_PTR;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    CONST gsm_cellsearch_ctrl_t *stp_cellsearch_ctrl = NULL_PTR;  
    gsm_frametimer_change_info_t* stp_frametimer_change_info  = NULL_PTR;
    CONST gsm_l1cc_sleep_ctrl_info_t* stp_gsm_sleep_ctrl  = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    
     
    stp_gsm_standby_info = gsm_get_standby_info();
    stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();
    stp_gsm_sleep_ctrl= gsm_get_sleep_ctrl_tbl(u16_active_id);
    
    /* FIX LM-Bug00001771   END  2011-12-14 : linlan */
    /*FIX LM-Bug00000839  BEGIN  2011-09-22 :linlan*/ 
    /*DD time adjust range (-2500,2500]*/
    while(s16_offset_in_frame > (SINT16)GSM_QBITS_HALF_FN)
    {
    /*FIX LM-Bug00000839  END  2011-09-22 :linlan*/
        s16_offset_in_frame -= (SINT16)GSM_QBITS_PER_FN;
        s32_delta_fn = (SINT32)gsm_fn_operate((UINT32)(((SINT32)(UINT32)GSM_MAX_FN + s32_delta_fn) - (SINT32)1));
    }

    /*FIX LM-Bug00000839  BEGIN  2011-09-22 :linlan*/ 
    while(s16_offset_in_frame <= -(SINT16)GSM_QBITS_HALF_FN)
    /*FIX LM-Bug00000839  END  2011-09-22 :linlan*/
    {
        s16_offset_in_frame += (SINT16)GSM_QBITS_PER_FN;
        s32_delta_fn = (SINT32)gsm_fn_operate((UINT32)(((SINT32)(UINT32)GSM_MAX_FN + s32_delta_fn) + (SINT32)1));
    }
    /*FIX PhyFA-Bug00002757  BEGIN  2011-08-01 :fushilong*/    

    stp_frametimer_change_info = gsm_get_frametimer_change_info();
    stp_frametimer_change_info->s32_delta_offset = (SINT32)s16_offset_in_frame;;
    stp_frametimer_change_info->s32_delta_fn = s32_delta_fn;
    /*FIX PhyFA-Bug00002757  END 2011-08-01 :fushilong*/
    /*FIX LM-Bug00000934  BEGIN  2011-10-08 :wanghairong*/
    dd_net_time_get(u16_active_id,(UINT16)GSM_TIMING, &st_time_info);
    /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
    OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,
              (UINT16)GSM_MODE,
              "gsm_fcp_time_adj(): current time from dd:u32_fn=0x%lx,s32_offset=%ld; Adj value:delta_fn=0x%lx,offset=%ld\n",
              (UINT32)st_time_info.st_fn_offset.u32_subfn,
              (UINT32)st_time_info.st_fn_offset.s32_offset,
              (UINT32)stp_frametimer_change_info->s32_delta_fn,
              (UINT32)stp_frametimer_change_info->s32_delta_offset);
    /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
    /*FIX LM-Bug00000934  END  2011-10-08 :wanghairong*/

    /* FIX PhyFA-Req00001316 BEGIN  2012-11-26 : wuxiaorong */
    /* FIX PhyFA-Bug00003987 BEGIN  2013-06-03  : wangjunli*/
    if(GSM_WAKE_UP != stp_gsm_sleep_ctrl->u16_sleep_req)
    /* FIX PhyFA-Bug00003987 END  2013-06-03  : wangjunli*/
    {
       oal_wake_up(u16_active_id,(UINT16)GSM_TIMING);
       /* FIX LTE-Bug00001719 BEGIN 2013-03-18:wangjunli*/
       /*g_st_gsm_sleep_ctrl[u16_active_id].b_sleep_req = OAL_FALSE;*/
       /* FIX LTE-Bug00001719 END 2013-03-18:wangjunli*/
    }
    /* FIX PhyFA-Req00001316 END  2012-11-26 : wuxiaorong */
        

    /*FIX PhyFA-Bug00002757  BEGIN  2011-08-01 :fushilong*/    
    /*FIX PhyFA-Bug00002757  END 2011-08-01 :fushilong*/
    /*FIX LM-Bug00001353  BEGIN  2011-11-30 :sunzhiqiang*/ 
    u16_normal_search_valid = stp_cellsearch_ctrl->st_normal_search.u16_valid_flag;
    u16_nocell_search_valid = stp_cellsearch_ctrl->st_nocell_search.u16_valid_flag;
    if(((UINT16)1 == stp_cellsearch_ctrl->u16_valid_flag)
      &&(((UINT16)1 == u16_normal_search_valid) ||((UINT16)1 == u16_nocell_search_valid)))
    {
        for(i = 0; i< (UINT16)MAX_STANDBY_AMT; i++)
        {
            /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
            if((UINT16)GSM_STANDBY_INACTIVE != stp_gsm_standby_info->u16_active_flag[i])
            /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/
            {
                u16_standby_id  = stp_gsm_standby_info->u16_standby_id[i];
                stp_cellsearch_tbl = gsm_get_cellsearch_tbl_by_standby(u16_standby_id); 
                /* FIX LM-Bug00001464   BEGIN  2011-12-14 : linlan */
                if((UINT16)1 == stp_cellsearch_tbl->u16_valid_flag)
                {
                    if((UINT16)GSM_BGS_SB == stp_cellsearch_tbl->u16_bg_state)
                    {
                         u32_temp_sb_rx_fn = gsm_fn_operate(((UINT32)(UINT32)GSM_MAX_FN + stp_cellsearch_tbl->u32_sb_rx_fn) - (UINT32)1);
                         /*s16_temp_sb_rx_offset 一定是正的 */
                         s16_temp_sb_rx_offset = (SINT16)((GSM_QBITS_PER_FN + ((SINT16)stp_cellsearch_tbl->u16_sb_rx_tsn*GSM_QBITS_PER_TS)) + stp_cellsearch_tbl->s16_sb_rx_offset);
                         
                         u32_temp_sb_rx_fn = gsm_fn_operate((UINT32)((((SINT32)(UINT32)GSM_MAX_FN + (SINT32)u32_temp_sb_rx_fn) + s32_delta_fn) -(SINT32)1));
                         s16_temp_sb_rx_offset = (s16_temp_sb_rx_offset + GSM_QBITS_PER_FN) - s16_offset_in_frame;

                         stp_cellsearch_tbl->u32_sb_rx_fn = gsm_fn_operate(((UINT32)GSM_MAX_FN + u32_temp_sb_rx_fn) + (UINT32)((SINT32)s16_temp_sb_rx_offset/(SINT32)GSM_QBITS_PER_FN));
                         s16_temp_sb_rx_offset = s16_temp_sb_rx_offset - ((s16_temp_sb_rx_offset/GSM_QBITS_PER_FN)*GSM_QBITS_PER_FN);
                         stp_cellsearch_tbl->s16_sb_rx_offset = s16_temp_sb_rx_offset%GSM_QBITS_PER_TS;
                         stp_cellsearch_tbl->u16_sb_rx_tsn = (UINT16)(s16_temp_sb_rx_offset/GSM_QBITS_PER_TS);
                         /* FIX LM-Bug00001771   BEGIN  2011-12-14 : linlan */
                         gsm_format_timeinfor(&s32_rx_fn_offset, &u16_new_tsn, &s16_new_offset, (UINT16)stp_cellsearch_tbl->u16_sb_rx_tsn , stp_cellsearch_tbl->s16_sb_rx_offset);
                         stp_cellsearch_tbl->u32_sb_rx_fn  = gsm_fn_operate((UINT32)((SINT32)stp_cellsearch_tbl->u32_sb_rx_fn + (SINT32)(UINT32)GSM_MAX_FN + s32_rx_fn_offset));
                         stp_cellsearch_tbl->s16_sb_rx_offset = s16_new_offset;
                         stp_cellsearch_tbl->u16_sb_rx_tsn = (UINT16)u16_new_tsn;
                         /* FIX LM-Bug00001771   END  2011-12-14 : linlan */                         
                    }
                    else if((UINT16)GSM_BGS_SEARCH_RF == stp_cellsearch_tbl->u16_bg_state)
                    {
                        stp_search_rf_tb = &(stp_cellsearch_tbl->st_search_rf_tb);
                        u16_arfcn_scaned_times = stp_search_rf_tb->u16_arfcn_scaned_times;
                        stp_search_rf_tb->u32_last_arfcn_set_fn[(UINT32)u16_arfcn_scaned_times] = gsm_fn_operate((UINT32)((SINT32)((UINT32)GSM_MAX_FN + stp_search_rf_tb->u32_last_arfcn_set_fn[u16_arfcn_scaned_times]) + s32_delta_fn));
                    }
                    /* FIX LM-Bug00001464   END  2011-12-14 : linlan */
                }
                
             }
         }
    }
    /*FIX LM-Bug00001353  END  2011-11-30 :sunzhiqiang*/

    
    stp_gsm_meas_req = gsm_get_gsm_meas_req();
    /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : caisiwen */
    /* FIX PhyFA-Enh00001430  BEGIN  2012-01-28  : wuxiaorong */
    if((UINT16)0 != (stp_gsm_meas_req->u16_valid_bitmap&GSM_SLAVE_MEAS_BIT_MASK))
    /* FIX PhyFA-Enh00001430  END  2013-01-28  : wuxiaorong */
    {
        for(i = 0; i< (UINT16)MAX_STANDBY_AMT; i++)
        {
            /* FIX PhyFA-Enh00001430  BEGIN  2012-01-28  : wuxiaorong */
            stp_slave_interrat_meas = &(stp_gsm_meas_req->st_slave_interrat_meas_req[i]);
            stp_ncell_rssi_meas = gsm_get_ncell_rssi_meas_cfg(i);
            if( (UINT16)0 != (stp_gsm_meas_req->u16_valid_bitmap & ((UINT16)1 << (i+(UINT16)GSM_SLAVE_MEAS_BIT_OFFSET))))
            /* FIX PhyFA-Enh00001430  END  2013-01-28  : wuxiaorong */
            {   
                u32_temp_slave_rpt_fn = stp_slave_interrat_meas->u32_reported_fn;
                stp_slave_interrat_meas->u32_reported_fn =  gsm_fn_operate((UINT32)((((SINT32)(UINT32)GSM_MAX_FN + (SINT32)u32_temp_slave_rpt_fn) + s32_delta_fn))); 

                /* FIX PhyFA-Req00001026 BEGIN  2012-07-17  : caisiwen */
                u32_temp_slave_rpt_fn = stp_slave_interrat_meas->u32_next_reported_pos;
                stp_slave_interrat_meas->u32_next_reported_pos = gsm_fn_operate((UINT32)((((SINT32)(UINT32)GSM_MAX_FN + (SINT32)u32_temp_slave_rpt_fn) + s32_delta_fn)));
                /* FIX PhyFA-Req00001026 END  2012-07-17  : caisiwen */
                /* FIX PhyFA-Req00001525 BEGIN 2013-10-09 : wuxiaorong */
                u32_first_startfn = stp_ncell_rssi_meas->u32_startfn_list[0];
                stp_ncell_rssi_meas->u32_startfn_list[0] =  gsm_fn_operate((UINT32)((((SINT32)(UINT32)GSM_MAX_FN + (SINT32)u32_first_startfn) + s32_delta_fn))); 

                /* FIX PhyFA-Req00001525 END   2013-10-09 : wuxiaorong */
                u16_page_offset = stp_ncell_rssi_meas->u16_paging_offset;
                stp_ncell_rssi_meas->u16_paging_offset = (UINT16)(gsm_fn_operate((UINT32)((((SINT32)(UINT32)GSM_MAX_FN + (SINT32)u16_page_offset) + s32_delta_fn)))%((UINT32)stp_ncell_rssi_meas->u16_period));
            }
            
        } 
    }
    /* FIX PhyFA-Req00001026 END 2012-04-10  : caisiwen */
    
    return OAL_SUCCESS; 
}






/* FIX AM-Enh00000083    BEGIN  2010-07-12  : wuxiaorong */
/*************************************************************************************
Function Name: gsm_fcp_del_tsntask_ind
Input parameters: 
       oal_msg_t *stp_oal_msg
Output parameters:
       NONE
Return value:
       VOID
Global Variables:
              
Description:
       process the msg of MSG_GSM_L1CC_FCP_DEL_TSNTASK_IND from EQ/CC_DEC.
             
Remark:      
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*   2014-05-20      wuxiaorong              PhyFA-Bug00004781   [NGM]GSM的事务预占的超帧号未赋值0出现DD ASSERT: gsm superfn is not 0 
*   2014-06-13      linlan                  PhyFA-Bug00004976   [NGM]调用DD接口dd_gsm_event_del入参使用错误 
*   2014-06-18      wuxiaorong              NGM-Bug00000109     [TDM]SDCCH下FCB测量存在问题
*   2014-07-08      gaowu                   PhyFA-Bug00005217   [NGM]l1cc_gsm发送withdraw消息需要判断是否为withdraw_req消息
*   2014-07-15      gaowu                   PhyFA-Bug00005326   [NGM]T+G双待，TDS回收GAP时GSM未正确填写CNF消息.
*   2014-07-21      wuxiaorong              NGM-Bug00000206     [TDM]GSM模式转走后的FCP_DEL_TASK_NORMAL拖尾任务删除需要判断待激活状态
*************************************************************************************/ 
L1CC_GSM_DRAM_CODE_SECTION
STATIC VOID gsm_fcp_del_tsntask_ind(CONST oal_msg_t * CONST stp_oal_msg)
{
    UINT32  u32_del_1st_fn;
    UINT16   u16_del_tsn;
    UINT16   u16_del_burst_num;  
    UINT16   i;
    UINT16   u16_del_type;      /* 0: normal task del;1:special task del;2:immi del;*/
    UINT16   u16_table_clr_type;/* 0: do not clear;1:clear a frame 's table;2:clear all frames'table;*/
    UINT16  u16_tstask;
    UINT16  u16_ch_type;
    UINT16  u16_arfcn;
    UINT16  u16_standby_id;    
    UINT16  u16_event_type;
    UINT32  u32_fn_in_gap;    
    UINT16  u16_tsn_in_gap;    
    UINT16  u16_event_type_in_gap;    
    UINT16  u16_gap_withdraw_flag;
    UINT16  u16_tstask_num;
    BOOLEAN b_affair_delete_ind = OAL_FALSE;
    BOOLEAN b_mmc_conflict_judge = OAL_TRUE;
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
    fn_offset_info_t st_gsm_fn_info = {0};
    CONST gsm_gap_withdraw_info_t *stp_gsm_gap_withdraw_info = NULL_PTR;
    /*FIX LM-Bug00000639  BEGIN 2011-07-21:dujianzhong */ 
    l1cc_rat_gap_withdraw_cnf_info_t   *stp_gsm_gap_withdraw_cnf_info;
    l1cc_gsm2mmc_gap_withdraw_cnf_t     st_gsm2mmc_gap_withdraw_cnf;
    CONST  msg_gsm_l1cc_fcp_del_tsntask_ind_t  *stp_del_tsntask_ind = NULL_PTR;
    CONST  gsm_slot_rtx_t    *stp_slot_rtx_tmp = NULL_PTR;
    /*FIX LM-Bug00000639  END   2011-07-21:dujianzhong */
    /* FIX PhyFA-Req00001016 BEGIN 2011-8-22: wuxiaorong */ 
    UINT32 u32_fcp_del_ret;
    /* FIX PhyFA-Req00001016 END 2011-8-22: wuxiaorong */
    /* FIX LTE-Bug00001401  BEGIN  2012-12-10 : wuxiaorong */
    fn_offset_info_t  st_net_fn_offset_del = {(UINT16)0,(UINT16)0,(UINT32)0,(SINT32)0};
    mmc_time_info_t   st_mmc_time_info;
    /* FIX LTE-Bug00001401  END  2012-12-10 : wuxiaorong */
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    u_time_info_t   st_gsm_time_info;
    CONST gsm_tstask_info_in_gap_t    *stp_gsm_tstask_info_in_gap = NULL_PTR;
    /* FIX NGM-Bug00000206 BEGIN 2014-07-21 : wuxiaorong */
    CONST gsm_standby_info_t* stp_standby_info = NULL_PTR;
    /* FIX NGM-Bug00000206 END   2014-07-21 : wuxiaorong */
    SINT16 s16_offset;

    
    
    OAL_ASSERT(NULL_PTR!=stp_oal_msg, "gsm_fcp_del_tsntask_ind null pointer");
    
    stp_del_tsntask_ind = (msg_gsm_l1cc_fcp_del_tsntask_ind_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));
    
    u32_del_1st_fn   = stp_del_tsntask_ind->u32_fn;
    //u16_fn_num = stp_del_tsntask_ind->u16_fn_num;
    u16_del_tsn = stp_del_tsntask_ind->u16_tsn; 
    u16_del_burst_num = stp_del_tsntask_ind->u16_del_burst_num;
    u16_del_type = stp_del_tsntask_ind->u16_del_type;
    u16_table_clr_type = stp_del_tsntask_ind->u16_frametask_table_clr_type;
    u16_arfcn = stp_del_tsntask_ind->u16_arfcn;
    u16_ch_type = stp_del_tsntask_ind->u16_ch_type;
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/    
    u16_tstask = stp_del_tsntask_ind->u16_tstask;
    u16_standby_id = stp_del_tsntask_ind->u16_standby_id;
    u16_event_type = stp_del_tsntask_ind->u16_event_type;   
    s16_offset = stp_del_tsntask_ind->s16_offset;
    
    u16_gap_withdraw_flag = stp_del_tsntask_ind->u16_gap_withdraw_flag;
    stp_gsm_gap_withdraw_info = &stp_del_tsntask_ind->st_gsm_gap_withdraw_info;
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/  
    b_affair_delete_ind = stp_del_tsntask_ind->b_affair_delete_ind;

    /* FIX PHYFA-Enh00000795 END 2010-08-28 : liujiaheng */
    
    switch(u16_del_type)
    {
        /* FIX LM-Bug00000957 BEGIN 2011-10-13: sunzhiqiang */ 
        case FCP_DEL_TASK_NONE:
            break;
        /* FIX LM-Bug00000957 END 2011-10-13: sunzhiqiang */ 
        /* FIX LM-Bug00000100 BEGIN 2010-08-28 : liujiaheng */
        case FCP_DEL_TASK_NORMAL:
            /* FIX NGM-Bug00000206 BEGIN 2014-07-21 : wuxiaorong */
            stp_standby_info = gsm_get_standby_info();
            if(GSM_STANDBY_INACTIVE == stp_standby_info->u16_active_flag[u16_standby_id])
            {
                /* 待已经去激活，那么不需要再删除.也避免调用gsm_check_whether_need_mmc_conflict_judge而assert */
                break;   
            }
            /* FIX NGM-Bug00000206 END   2014-07-21 : wuxiaorong */
            b_mmc_conflict_judge = gsm_check_whether_need_mmc_conflict_judge(u16_active_id,u16_standby_id);
            /* Fix LM-Enh00000057 BEGIN 2010-09-22 : guxiaobo */
            for(i = 0;i < u16_del_burst_num;i++)
            {
                /*FIX LM-Bug00000639  BEGIN 2011-07-21:dujianzhong */
                ///*stp_frame_task = */gsm_get_main_frametask((u32_del_1st_fn + (UINT32)i),u16_active_id);
                stp_slot_rtx_tmp = gsm_get_tsntask(gsm_fn_operate(u32_del_1st_fn + (UINT32)i), (SINT16)u16_del_tsn);
                /* Fix LM-Bug00000319 BEGIN 2010-12-29: dujianzhong */
                /* clear l1cc frame task info */
                if( ((UINT16)GSM_TSNTASK_EXPIRED != stp_slot_rtx_tmp->u16_expired)
                   &&(u16_arfcn == stp_slot_rtx_tmp->u16_arfcn)
                   &&(u16_tstask == stp_slot_rtx_tmp->u16_tstask)
                   &&(u16_ch_type == stp_slot_rtx_tmp->u16_ch_type)               
                   &&(u16_standby_id == stp_slot_rtx_tmp->u16_standby_id) 
                  )
                 /*FIX LM-Bug00000639  END   2011-07-21:dujianzhong */ 
                {
                    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/ 
                    /* FIX PhyFA-Bug00002758  BEGIN  2011-08-01 : linlan */ 
                    if((UINT16)GSM_TSNTASK_DOWNLOADED == stp_slot_rtx_tmp->u16_downloaded_ind)
                    {
                        /* FIX LM-Bug00000729  BEGIN  2011-08-29  : yuzhengfeng */
                         /* FIX LM-Bug00001262 BEGIN 2011-11-30: linlan *//*fix again. dd can't recognise TX_GSM_GMSK_NB and TX_GSM_8PSK_NB*/
                        if(((UINT16)TX_GSM_GMSK_NB == u16_event_type)||((UINT16)TX_GSM_8PSK_NB == u16_event_type))
                        {
                            u16_event_type = (UINT16)TX_GSM_NSLOT;
                        }
                        /* FIX LM-Bug00001262 END 2011-11-30: linlan */
                        /* FIX LTE-Bug00001401  BEGIN  2012-12-10 : wuxiaorong */
                        if((OAL_STATUS)OAL_SUCCESS == dd_gsm_event_del(u16_active_id,(UINT16)u16_event_type, (UINT32)(u32_del_1st_fn + (UINT32)i),(UINT16)u16_del_tsn,&st_net_fn_offset_del))  
                        /* FIX LTE-Bug00001401  END  2012-12-10 : wuxiaorong */
                        {
                            /*FIX LM-Enh00001061  BEGIN   2013-01-30  : ningyaojun*/     
                            if(OAL_TRUE == b_affair_delete_ind)
                            {
                                /* 删除事务，需要模式间的MMC事务判决 */
                                if(OAL_TRUE == b_mmc_conflict_judge)
                                {
                                    /* FIX NGM-Bug00000206 BEGIN 2014-07-21 : wuxiaorong */
                                    gsm_del_mmc_affair(u16_standby_id,u16_tstask,gsm_fn_operate(u32_del_1st_fn + (UINT32)i),u16_del_tsn,s16_offset);
                                    /* FIX NGM-Bug00000206 END   2014-07-21 : wuxiaorong */
                                }
                            }                          
                            /*FIX LM-Enh00001061  END     2013-01-30  : ningyaojun*/
                            gsm_fcp_del_tsntask(gsm_fn_operate(u32_del_1st_fn + (UINT32)i),(SINT16)u16_del_tsn,(UINT16)UNSPECIFIED_STANDBY_ID);
                        }
                        /* FIX LM-Bug00000729  END  2011-08-29  : yuzhengfeng */
                    }
                    else
                    {
                        gsm_fcp_del_tsntask(gsm_fn_operate(u32_del_1st_fn + (UINT32)i),(SINT16)u16_del_tsn,(UINT16)UNSPECIFIED_STANDBY_ID);
                    }
                    /* FIX PhyFA-Bug00002758  END  2011-08-01 : linlan */ 
                    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/   
                }
                /* Fix LM-Bug00000319 END   2010-12-29: dujianzhong */
            }

            
            /* Fix LM-Enh00000057 END 2010-09-22 : guxiaobo */
            break;
        case FCP_DEL_TASK_SPECIAL:            
            /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/  
            /* FIX PhyFA-Req00001016 BEGIN 2011-8-22: wuxiaorong */
            /* FIX LTE-Bug00001401  BEGIN  2012-12-10 : wuxiaorong */
            /* FIX NGM-Bug00000109 BEGIN 2014-06-18 : wuxiaorong */
            u32_fcp_del_ret = dd_gsm_event_del(u16_active_id,u16_event_type,u32_del_1st_fn,(UINT16)u16_del_tsn,&st_net_fn_offset_del);
            /* FIX NGM-Bug00000109 END   2014-06-18 : wuxiaorong */
            /* FIX LTE-Bug00001401  END  2012-12-10 : wuxiaorong */
            if(OAL_SUCCESS != u32_fcp_del_ret)
            {   /* FIX LM-Bug00001544  BEGIN  2012-01-04 : fushilong */ 
                if(((UINT32)DD_DEL_FAIL_NEAR_FINISH  == u32_fcp_del_ret)
                   ||((UINT32)DD_DEL_FAIL_MATCH_FAIL == u32_fcp_del_ret)
                   ||((UINT32)DD_DEL_FAIL_DEL_AGAIN == u32_fcp_del_ret))
                /* FIX LM-Bug00001544  END  2012-01-04 : fushilong */   
                {
                    /*In the following conditions that 
                    (1)FCB is going to the end 
                    (2)FCB has been deleted or gone finished
                    L1CC need not to handle the failure as exception, noted by ningyaojun, 20110824
                    */
                }
                else
                {
                    oal_error_handler((UINT16)ERROR_CLASS_MAJOR, GSM_FCP_ERR(RTX_EVENT_DELETE_FAILED));
                }
            }
            /* FIX PhyFA-Req00001016 END 2011-8-22: wuxiaorong */
            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/

            
            break;
        case FCP_DEL_TASK_IMMI:
            /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/  
            dd_event_stop_by_standby(u16_active_id,(UINT16)((UINT16)0x0001<<u16_standby_id),(UINT16)GSM_TIMING,&st_mmc_time_info);
            /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
            
            break;
        case FCP_DEL_TASK_CONTINUE:

            dd_net_time_get(u16_active_id, (UINT16)GSM_TIMING, &st_gsm_time_info);
            /* 获取当前时间点 */
            oal_mem_copy(&st_gsm_fn_info, &st_gsm_time_info.st_fn_offset, OAL_GET_TYPE_SIZE_IN_WORD(fn_offset_info_t));
            oal_mem_copy(&st_net_fn_offset_del, &st_gsm_time_info.st_fn_offset, OAL_GET_TYPE_SIZE_IN_WORD(fn_offset_info_t));

            if (GSM_TABLE_VALID == u16_gap_withdraw_flag)
            {
                u16_tstask_num = stp_gsm_gap_withdraw_info->u16_tstask_num;

                for (i = (UINT16)0; i < u16_tstask_num; i++)
                {
                    stp_gsm_tstask_info_in_gap = &stp_gsm_gap_withdraw_info->st_gsm_tstask_info[i];
                    u16_event_type_in_gap = stp_gsm_tstask_info_in_gap->u16_event_type;
                    u32_fn_in_gap = stp_gsm_tstask_info_in_gap->u32_fn;
                    u16_tsn_in_gap = stp_gsm_tstask_info_in_gap->u16_tsn;

                    if (stp_gsm_tstask_info_in_gap->u16_download_ind)
                    {
                        if ((OAL_STATUS)OAL_SUCCESS == dd_gsm_event_del(u16_active_id,u16_event_type_in_gap,u32_fn_in_gap,u16_tsn_in_gap,&st_net_fn_offset_del))
                        {
                            gsm_fcp_del_tsntask(u32_fn_in_gap,(SINT16)u16_tsn_in_gap,(UINT16)UNSPECIFIED_STANDBY_ID);
                        }
                    }
                    else
                    {
                        gsm_fcp_del_tsntask(u32_fn_in_gap,(SINT16)u16_tsn_in_gap,(UINT16)UNSPECIFIED_STANDBY_ID);
                    }

                    gsm_get_fn_info_compare_result(&st_gsm_fn_info, &st_net_fn_offset_del, &st_gsm_fn_info);
                }               

                if (OAL_TRUE == stp_gsm_gap_withdraw_info->b_need_withdraw_cnf)                                
                {
                    /* 给对应的主模式回复WITHDRAW_CNF */
                    st_gsm2mmc_gap_withdraw_cnf.u16_src_standby_id = u16_standby_id;
                    st_gsm2mmc_gap_withdraw_cnf.u16_dest_standby_id = stp_gsm_gap_withdraw_info->u16_dest_standby_id;
                    st_gsm2mmc_gap_withdraw_cnf.u16_dest_mode = stp_gsm_gap_withdraw_info->u16_dest_mode;
                    st_gsm2mmc_gap_withdraw_cnf.u16_frame_type = (UINT16)GSM_TIMING;
                    /* FIX NGM-Bug00000456 BEGIN 2014-09-11 : gaowu */
                    stp_gsm_gap_withdraw_cnf_info = &st_gsm2mmc_gap_withdraw_cnf.st_rat_gap_withdraw_cnf_info;

                    dd_frame2mmc_time_transform(u16_active_id, (UINT16)GSM_TIMING, &st_gsm_fn_info, &stp_gsm_gap_withdraw_cnf_info->st_gap_withdraw_mmc_time);
                    /* FIX NGM-Bug00000456 END   2014-09-11 : gaowu */
                    stp_gsm_gap_withdraw_cnf_info->u16_gap_type_amt = stp_gsm_gap_withdraw_info->u16_gap_type_amt;
                    oal_mem_copy(&stp_gsm_gap_withdraw_cnf_info->st_withdraw_gap_info[0],
                                  &stp_gsm_gap_withdraw_info->st_gsm_withdraw_gap_info[0],
                                  (UINT16)(OAL_GET_TYPE_SIZE_IN_WORD(l1cc_rat_withdraw_gap_type_info_t) * stp_gsm_gap_withdraw_info->u16_gap_type_amt));

                    gsm_l1cc_send_msg(&st_gsm2mmc_gap_withdraw_cnf, (UINT16)L1CC_MMC_TASK_ID, MSG_GSM2MMC_GAP_WITHDRAW_CNF, (UINT16)sizeof(l1cc_gsm2mmc_gap_withdraw_cnf_t), u16_standby_id);
                }
            }

            break;
        default:
            oal_error_handler(ERROR_CLASS_MINOR, GSM_FCP_ERR(INVALID_PARAM));
            break;
            
    }
    /* clear frametask table*/
    switch(u16_table_clr_type)
    {
        case FCP_FRAMETASK_TABLE_CLEAR_NONE:
            /* do nothing*/
            break;
        case FCP_FRAMETASK_TABLE_CLEAR_ONE:
            gsm_fcp_del_frametask(u32_del_1st_fn,(UINT16)UNSPECIFIED_MODE);  
            break;
        case FCP_FRAMETASK_TABLE_CLEAR_ALL:
            gsm_fcp_del_all_frametask(u16_standby_id);
            break;  
        default:
            oal_error_handler(ERROR_CLASS_MINOR, GSM_FCP_ERR(INVALID_PARAM));
    }
    return;
}


/* FIX AM-Enh00000083   END  2010-07-12  : wuxiaorong */


L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_state_change(CONST oal_msg_t * CONST stp_oal_msg)
{
    /* FIX LM-Bug00001404 BEGIN   2011-12-06: wuxiaorong */ 
    CONST gsm_pl_state_change_t *stp_state_change_msg = NULL_PTR;
    UINT16 u16_changed_state;
    UINT16 u16_gsm_state;
    UINT16 u16_gsm_fcp_new_state;
    UINT16 u16_standby_id; 

    if(NULL_PTR == stp_oal_msg)
    {
        return OAL_FAILURE;
    }
    stp_state_change_msg= (gsm_pl_state_change_t *)OAL_GET_OAL_MSG_BODY(stp_oal_msg);
    u16_changed_state    = (UINT16)(stp_state_change_msg->changed_state);
    u16_standby_id = stp_state_change_msg->u16_standby_id;
   
    OAL_ASSERT(((UINT16)MAX_STANDBY_AMT > u16_standby_id), "MAX_STANDBY_AMT <= u16_standby_id");  
    gsm_fc_init(u16_standby_id); 
    
    u16_gsm_state = gsm_get_gsm_state();
    u16_gsm_fcp_new_state = gsm_get_fcp_new_state();
    if(u16_gsm_fcp_new_state != u16_gsm_state)
    {
        gsm_set_fcp_3rd_state(u16_changed_state);
        /*FIX PhyFA-Enh00001343 BEGIN 2012-09-03: dujianzhong */
        OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,
                  (UINT16)GSM_MODE,
                  "gsm_fcp_state_change():State changing is too quickly!u16_gsm_state: 0x%lx,u16_gsm_state: 0x%lx,g_u16_gsm_fcp_3rd_state: 0x%lx\r\nFILE_ID: %ld, LINE_NO: %ld\r\n",
                  (UINT32)u16_gsm_state,
                  (UINT32)u16_gsm_fcp_new_state,
                  (UINT32)gsm_get_fcp_3rd_state(),
                  (UINT32)THIS_FILE_NAME_ID, 
                  (UINT32)__LINE__);
        /*FIX PhyFA-Enh00001343 END   2012-09-03: dujianzhong */
    }
    else
    {
        if(u16_changed_state != u16_gsm_fcp_new_state)
        {
            gsm_set_fcp_new_state(u16_changed_state);
        }
        else/* 发送转状态消息时，被FCP打断，状态已经变了，如GSM_NOCELL */
        {
            OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,
                      (UINT16)GSM_MODE,
                      "gsm_fcp_state_change():u16_gsm_state:0x%lx, u16_gsm_fcp_new_state:0x%lx, u16_changed_state:0x%lx\n",
                      (UINT32)u16_gsm_state,
                      (UINT32)u16_gsm_fcp_new_state,
                      (UINT32)u16_changed_state);
        }
    }
    /* FIX LM-Bug00001404 END   2011-12-06: wuxiaorong */
    return OAL_SUCCESS;
}

/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_state_changing
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/

/*FIX PhyFA-Req00001045  BEGIN   2011-09-16  : ningyaojun*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC BOOLEAN gsm_fcp_state_changing(VOID)
{

    UINT16   u16_standby_id = UNSPECIFIED_STANDBY_ID;
    UINT16   u16_standby_bitmap = 0;
    BOOLEAN  b_return_val = OAL_FALSE;
    mmc_time_info_t st_mmc_time_info;
    UINT16   u16_fcp_3rd_state;
    UINT16   u16_fcp_new_state;
    UINT16   u16_gsm_state;
    UINT16   u16_i;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    UINT16_PTR u16p_gsm_eq_dec_databuff = NULL_PTR;
    UINT16_PTR u16p_gsm_enc_ilbuffer = NULL_PTR;
    four_burst_transmission_pattern_info_t  *stp_four_burst_trans_pattern_info = NULL_PTR;
        /* FIX PhyFA-Req00001525 BEGIN 2013-11-07 : zhengying */
    CONST gsm_conn_ho_tb_t *stp_gsm_conn_ho_tb = NULL_PTR;

    /* FIX LM-Enh00001287 BEGIN 2013-08-29: wangjunli */
    gsm_ccch_tb_t* stp_temp_ccch_tb = NULL_PTR;
    /* FIX LM-Enh00001287 END 2013-08-29: wangjunli */
    
    
    
    stp_gsm_standby_info  = gsm_get_standby_info();
    u16_fcp_new_state = gsm_get_fcp_new_state();
    u16_gsm_state     = gsm_get_gsm_state();
    u16_fcp_3rd_state = gsm_get_fcp_3rd_state();
    

    stp_gsm_conn_ho_tb     = gsm_get_conn_ho_tb();
    /* FIX PhyFA-Req00001525 END   2013-11-07 : zhengying */
    /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
    /*u16_standby_id = stp_gsm_standby_info->u16_fg_standby_id;*/
    if(u16_fcp_new_state != u16_gsm_state)
    {
       

        /* FIX LM-Enh00001287 BEGIN 2013-08-29: wangjunli */
        /*转TRANSFER，CCCH还是FULL_CCCH, 重新按normal配置*/
        if((GSM_TRANSFER == u16_fcp_new_state)
          &&((GSM_IDLE == u16_gsm_state)||(GSM_CS_RACH == u16_gsm_state)||(GSM_PS_RACH == u16_gsm_state)))
        {
            for(u16_i=0; u16_i<(UINT16)MAX_STANDBY_AMT; u16_i++)
            {
                stp_temp_ccch_tb = gsm_get_ccch_tbl_cfg(u16_i);
                /* FIX LM-Bug00003047 BEGIN  2013-12-23  wangjunli*/
                if(((UINT16)1 == stp_temp_ccch_tb->u16_ccch_tb_flg)
                  &&((MPAL_FULL_CCCH_BLKS == stp_temp_ccch_tb->u16_curr_paging_mode)
                     ||(MPAL_PAGING_REORG == stp_temp_ccch_tb->u16_curr_paging_mode)))
                /* FIX LM-Bug00003047 END  2013-12-23  wangjunli*/
                {
                    gsm_ccch_set(stp_temp_ccch_tb->u16_ccch_conf,
                                 stp_temp_ccch_tb->u16_bs_pa_mfrms,
                                 (UINT16)MPAL_NORMAL_PAGING,
                                 stp_temp_ccch_tb->u16_bsagblksres,
                                 stp_temp_ccch_tb->u32_imsimod1000,u16_i);
                    stp_temp_ccch_tb->u16_next_ccch_pos_idx = GSM_FCP_POS_INDEX_INVALID;
                    OAL_PRINT(u16_i,(UINT16)GSM_MODE,"reconfig ccch as normal_paging,ccch_pos_size=%u, offset=%d",stp_temp_ccch_tb->u16_ccch_pos_size,stp_temp_ccch_tb->u16_ccch_pos[0]);
                 }
             }
        }
        /*从TRANSFER转IDLE，恢复FULL_CCCH配置*/
        if((GSM_IDLE== u16_fcp_new_state)&&(GSM_TRANSFER == u16_gsm_state))
        {
            for(u16_i=0; u16_i<(UINT16)MAX_STANDBY_AMT; u16_i++)
            {
                stp_temp_ccch_tb = gsm_get_ccch_tbl_cfg(u16_i);
                /* FIX LM-Bug00003047 BEGIN  2013-12-23  wangjunli*/
                if(((UINT16)1 == stp_temp_ccch_tb->u16_ccch_tb_flg)
                  &&((MPAL_FULL_CCCH_BLKS == stp_temp_ccch_tb->u16_curr_paging_mode)
                     ||(MPAL_PAGING_REORG == stp_temp_ccch_tb->u16_curr_paging_mode)))
                /* FIX LM-Bug00003047 END  2013-12-23  wangjunli*/
                {
                    gsm_ccch_set(stp_temp_ccch_tb->u16_ccch_conf,
                                 stp_temp_ccch_tb->u16_bs_pa_mfrms,
                                 (UINT16)MPAL_FULL_CCCH_BLKS,
                                 stp_temp_ccch_tb->u16_bsagblksres,
                                 stp_temp_ccch_tb->u32_imsimod1000,u16_i);
                    stp_temp_ccch_tb->u16_next_ccch_pos_idx = GSM_FCP_POS_INDEX_INVALID;
                    OAL_PRINT(u16_i,(UINT16)GSM_MODE,"reconfig ccch as full_ccch,ccch_pos_size=%u, offset=%d",stp_temp_ccch_tb->u16_ccch_pos_size,stp_temp_ccch_tb->u16_ccch_pos[0]);
                 }
             }
        }
        /* FIX LM-Enh00001287 END 2013-08-29: wangjunli */
        /* FIX PhyFA-Req00001525 BEGIN  2013-11-18 : wuxiaorong */
        /* 进入和退出事务流不在转状态时操作，而是在流程中添加，有些事务流不会伴随系统状态的转变的  */
        /* FIX PhyFA-Req00001525 END  2013-11-18 : wuxiaorong */
        gsm_set_gsm_state(u16_fcp_new_state);
        /*FIX LM-Req00000372  BEGIN   2011-09-23  : linlan*/             
        gsm_del_fcb_task_reset_cellsearch_tb();
        /*FIX LM-Req00000372  END   2011-09-23  : linlan*/
        /*FIX LM-Bug00000956  BEGIN  2011-10-13 :fushilong*/
        u16_standby_bitmap = 0;
        for(u16_i=0; u16_i<(UINT16)MAX_STANDBY_AMT; u16_i++)
        {
            /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
            if((UINT16)GSM_STANDBY_INACTIVE != stp_gsm_standby_info->u16_active_flag[u16_i])
            /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/            
            {
                u16_standby_bitmap |=((UINT16)1<<(stp_gsm_standby_info->u16_standby_id[u16_i]));
            }
        }
        /* 不分卡，由于帧任务表已经删除，下载的事件也全删掉   */
        dd_event_stop_by_standby(u16_active_id,u16_standby_bitmap,(UINT16)GSM_TIMING,&st_mmc_time_info);
   
        /*FIX LM-Bug00000956  END 2011-10-13 :fushilong*/                
        /* FIX PhyFA-Bug00002849  BEGIN  2011-08-23  : yuzhengfeng */
        gsm_fcp_del_all_frametask((UINT16)UNSPECIFIED_STANDBY_ID);
        /* FIX PhyFA-Bug00002849  END  2011-08-23  : yuzhengfeng */
        
        /* 重新获取系统状态 */
        u16_gsm_state = gsm_get_gsm_state();
        
        /*FIX LM-Bug00001213  BEGIN 2011-11-17 :wuxiaorong */
        if((UINT16)GSM_HANDOVER == u16_gsm_state)
        {
            /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
            u16_standby_id = stp_gsm_conn_ho_tb->u16_standby_id;
            /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/
            gsm_hlp_expired_nc_meas(u16_standby_id);
        }
        /*FIX LM-Bug00001213  END 2011-11-17 :wuxiaorong */
        /*FIX PhyFA-Req00000936  BEGIN   2011-07-11  : ningyaojun*/
        gsm_eq_para_reset();
        /*FIX PhyFA-Req00000936  END     2011-07-11  : ningyaojun*/
        /* FIX PhyFA-Bug00003430   BEGIN     2012-06-01  : wangjunli*/
        #if 0
        /*FIX LM-Bug00002207 BEGIN 2012-07-05 dujianzhong */
        OAL_ASSERT((((UINT16)0 == u16_active_standby_num)||((UINT16)MAX_STANDBY_AMT > u16_standby_id)), "MAX_STANDBY_AMT <= u16_standby_id");  
        if((UINT16)MAX_STANDBY_AMT > g_u16_state_change_standby_id[u16_active_id])
        {
            gsm_fc_init(g_u16_state_change_standby_id[u16_active_id]); 
        }
        /*rework:to elimilate PC_LINT messages 2012-07-11 dujianzhong */
        else if(((UINT16)GSM_HANDOVER == u16_gsm_state)&&((UINT16)MAX_STANDBY_AMT > u16_standby_id))
        {
            gsm_fc_init(u16_standby_id);
        }
        g_u16_state_change_standby_id[u16_active_id] = UNSPECIFIED_STANDBY_ID;
        #endif
        /* FIX LM-Bug00002207  END     2012-07-05 dujianzhong */
        /* FIX PhyFA-Bug00003430   END     2012-06-01  : wangjunli*/

        /* FIX LM-Enh00000681  BEGIN   2012-05-09: wanghuan*/
        if((UINT16)GSM_TRANSFER == u16_gsm_state)
        {
            stp_four_burst_trans_pattern_info = gsm_get_four_burst_trans_pattern_info();

            stp_four_burst_trans_pattern_info->u16_count =(UINT16)0;
            stp_four_burst_trans_pattern_info->u16_pattern =(UINT16)0;
            oal_mem_set((VOID_PTR)&stp_four_burst_trans_pattern_info->s16_relative_timing_info[0][0],(UINT16)(sizeof(stp_four_burst_trans_pattern_info->s16_relative_timing_info)/sizeof(UINT16)),(UINT16)0x7fff);
        }
        /* FIX LM-Enh00000681  END   2012-05-09: wanghuan*/
        /* FIX A2KPH-Bug00000297 BEGIN  2008-05-31  : huangjinfu */
        /* FIX LM-Bug00001339  BEGIN 2011-11-30 :wanghairong */
        if(GSM_DEDICATED == u16_gsm_state)
        /* FIX LM-Bug00001339  END 2011-11-30 :wanghairong */
        {
            u16p_gsm_eq_dec_databuff = gsm_get_gsm_eq_dec_databuff((UINT16)0, (UINT16)0);
            u16p_gsm_enc_ilbuffer = gsm_get_gsm_enc_ilbuffer();
            oal_mem_set(u16p_gsm_enc_ilbuffer,  (UINT16)GSM_WORD_SIZE_OF_ENC_ILBUFF,(UINT16)0);
            /*FIX LM-Bug00001293  BEGIN   2011-11-17  : sunzhiqiang*/
           oal_mem_set((VOID_PTR)u16p_gsm_eq_dec_databuff,  (UINT16)((GSM_PDTCH_EQ2DEC_BUFF_NUM * (GSM_EQ_DEC_PDTCH_DATA_BYTE_SIZE/2)) * GSM_DL_TS_NUM_MAX),(UINT16)0x0000);
            /* FIX NGM-Bug00000256   BEGIN  2014-08-05: sunzhiqiang */
            cache_flush((UINT32_PTR)u16p_gsm_eq_dec_databuff,GSM_CACHE_LINE_SIZE_BYTE_ALIGN((UINT32)((GSM_PDTCH_EQ2DEC_BUFF_NUM * GSM_EQ_DEC_PDTCH_DATA_BYTE_SIZE) * GSM_DL_TS_NUM_MAX)));
            /* FIX NGM-Bug00000256   END  2014-08-05: sunzhiqiang */
            /*FIX LM-Bug00001293  END   2011-11-17  : sunzhiqiang*/  
        }
        /*FIX LM-Bug00001124  END   2011-10-28  : sunzhiqiang*/
        /* FIX A2KPH-Bug00000297 END  2008-05-31  : huangjinfu */

        
        if (OAL_SUCCESS == oal_tpc_filter_msg((UINT32)MSG_GSM_FCP_PSD_SCELL_INFO,(UINT16)UNSPECIFIED_STANDBY_ID,(UINT16)GSM_MODE))
        {
            if((UINT16)0!= stp_gsm_standby_info->u16_active_standby_num)
            {
                /* 如果有辅模式卡激活，取辅模式激活卡ID */
                if((UNSPECIFIED_STANDBY_ID == u16_standby_id))
                {
                    for(u16_i = 0; u16_i < (UINT16)MAX_STANDBY_AMT; u16_i++)
                    {
                        if((UINT16)GSM_STANDBY_INACTIVE != stp_gsm_standby_info->u16_active_flag[u16_i])
                        {
                            u16_standby_id = stp_gsm_standby_info->u16_standby_id[u16_i];
                            break;
                        }
                    }       
                }        
        
                OAL_ASSERT((UINT16)MAX_STANDBY_AMT > u16_standby_id, "MAX_STANDBY_AMT <= u16_standby_id");
                gsm_fcp_psd_export_scell_info(u16_standby_id);
            }
        }
        b_return_val = OAL_TRUE;
    }

    /*FIX LM-Bug00001800     BEGIN   2012-03-08  : ningyaojun*/
    if(GSM_INVALID_STATE != u16_fcp_3rd_state)
    {
        gsm_set_fcp_new_state(u16_fcp_3rd_state);
        gsm_set_fcp_3rd_state(GSM_INVALID_STATE);
        /*It's optional to handle the "new" u16_gsm_fcp_new_state immediately or not:
          (1)by calling gsm_fcp_state_changing(), g_u16_gsm_state will be updated again for the 2nd time;
          (2)not calling gsm_fcp_state_changing(), u16_gsm_fcp_new_state will be handled in next frame */
        b_return_val = gsm_fcp_state_changing();  
    }   
    /*FIX LM-Bug00001800     END     2012-03-08  : ningyaojun*/
   
    return b_return_val;
}
/*FIX PhyFA-Req00001045  END     2011-09-16  : ningyaojun*/ 


/************************************************************
** Function Name: gsm_fcp_pre_add_mmc_affair (UINT16 u16_tstask,UINT16 u16_standby_id)
** Description: init ccch u16_next_ccch_pos_idx
** Input : u16_tstask,u16_standby_id,u32_fn
** Output : NULL
** Return: 
** Notes:
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC VOID gsm_fcp_pre_add_mmc_affair(IN CONST UINT16 u16_tstask,IN CONST UINT16 u16_standby_id,IN CONST UINT32 u32_fn)
{
    
    SINT16 s16_rtx_offset;
    gsm_tb_pos_t st_tb_pos_info = {0, 0, NULL_PTR, NULL_PTR};
    UINT32 u32_fcp_ret=OAL_FAILURE;

    UINT32 u32_next_pos_fn = (UINT32)GSM_INVALID_FN;
    CONST_UINT16 u16_active_id   = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    UINT16 u16_next_pos_index    = GSM_FCP_POS_INDEX_INVALID;

    msg_gsm_common_fcp_affair_operate_req_t *stp_affair_operate_req = NULL_PTR;
    gsm_tsn_info_t st_tsn_info;
    CONST gsm_tstask_attribute_t* stp_tstask_attribute = NULL_PTR;
    
    st_tsn_info.u16_tstask = u16_tstask;
    st_tsn_info.u16_standby_id = u16_standby_id;
    st_tsn_info.u32_fn = u32_fn;

    u32_fcp_ret = gsm_fcp_check_isarrive(&st_tsn_info,&st_tb_pos_info);
    s16_rtx_offset = st_tsn_info.s16_rtx_offset;
    if(OAL_SUCCESS == u32_fcp_ret)
    {
        return;
    }
    else
    {
        /* FIX NGM-Bug00000089  BEGIN 2014-06-06 : zhengying */
        if ((UINT16)0 == st_tb_pos_info.u16_period)
        {
            return;        
        }  
        /* FIX NGM-Bug00000089  END   2014-06-06 : zhengying */
        u16_next_pos_index      = *st_tb_pos_info.u16p_next_pos_index;
    }
    

    u32_next_pos_fn =   (((UINT32)st_tb_pos_info.u16_period)*(u32_fn/((UINT32)st_tb_pos_info.u16_period))) + (UINT32)st_tb_pos_info.u16p_pos_list[u16_next_pos_index]; 

    /* FIX PhyFA-Bug00004763 BEGIN 2014-05-27 : zhengying */
    /* 要考虑到当前帧号在这个周期内的最后一个index的后面；所以我们要
       计算的是下个周期内的第一个index的位置 */
    if((u32_fn%((UINT32)st_tb_pos_info.u16_period)) > ((UINT32)st_tb_pos_info.u16p_pos_list[st_tb_pos_info.u16_pos_size - 1]))
    {
        u32_next_pos_fn = u32_next_pos_fn + (UINT32)st_tb_pos_info.u16_period;
    }
    
    if(u32_next_pos_fn < (UINT32)GSM_MAX_FN)
    {
        u32_next_pos_fn = u32_next_pos_fn;
    } 
    /* 要转圈的时候，index从0开始 */
    else
    {
        u32_next_pos_fn = (UINT32)st_tb_pos_info.u16p_pos_list[0];
    }
    /* FIX PhyFA-Bug00004763 END   2014-05-27 : zhengying */



    
    /*to be checked : V2使用了按照valid tsn数组循环的方式，其实是不需要的?这里去掉了是不是正确 */

    stp_affair_operate_req = (msg_gsm_common_fcp_affair_operate_req_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,(UINT16)OAL_MEM_DRAM_LEVEL,(UINT32) OAL_GET_TYPE_SIZE_IN_WORD(msg_gsm_common_fcp_affair_operate_req_t));     

    stp_tstask_attribute = gsm_get_tstask_attribute(u16_tstask);

    stp_affair_operate_req->u16_operate_type              = AFFAIR_ADD;
    stp_affair_operate_req->u16_standby_id                    = u16_standby_id;
    stp_affair_operate_req->u16_affair_repeat_count       = stp_tstask_attribute->u16_repeat_frms;
    stp_affair_operate_req->st_raw_affair.u16_standby_id      = u16_standby_id;
    stp_affair_operate_req->st_raw_affair.u16_active_id   = u16_active_id;
    //stp_affair_operate_req->st_raw_affair.u16_logch_id    = LOGCH_0; 
    stp_affair_operate_req->st_raw_affair.b_fuzzy_match   = OAL_TRUE;
    stp_affair_operate_req->st_raw_affair.u16_mode_timing = GSM_TIMING;
    /* FIX PhyFA-Bug00004763 BEGIN 2014-05-19 : zhengying */
    stp_affair_operate_req->st_raw_affair.e_affair_id = stp_tstask_attribute->e_affair_id;
    stp_affair_operate_req->st_raw_affair.u16_rtx_status  =  stp_tstask_attribute->u16_rtx_status;
    /* FIX PhyFA-Bug00004763 END   2014-05-19 : zhengying */
    stp_affair_operate_req->st_raw_affair.u16_freq        = INVALID_WORD;
    /* FIX PhyFA-Enh00001876  BEGIN 2014-05-06 : gaowu */
    stp_affair_operate_req->st_raw_affair.st_time_len.u16_superfn          = 0;/*superfn必须为0*/
    /* FIX PhyFA-Enh00001876  END   2014-05-06 : gaowu */
    stp_affair_operate_req->st_raw_affair.st_time_len.u32_sub_fn           = u32_next_pos_fn; 
    stp_affair_operate_req->st_raw_affair.st_time_len.s32_start_offset     = (SINT32)(((SINT32)((SINT32)st_tsn_info.u16_start_tsn*(SINT32)GSM_QBITS_PER_TS))+(SINT32)s16_rtx_offset); //if u16_tsn>7
    stp_affair_operate_req->st_raw_affair.st_time_len.u32_duration         = (UINT32)stp_tstask_attribute->fp_get_rtx_len(u16_tstask);

    gsm_l1cc_send_msg(stp_affair_operate_req,(UINT16)L1CC_GSM_FCP_TASK_ID, MSG_GSM_COMMON_FCP_AFFAIR_OPERATE_REQ, (UINT16)sizeof(msg_gsm_common_fcp_affair_operate_req_t),u16_standby_id);
    
    oal_mem_free((CONST_VOID_PTR*)&stp_affair_operate_req);           
       
    return;
}
/*FIX LM-Enh00001119  END 2013-04-22 :fushilong*/    

/*FIX A2KPH-Bug00001978     END  2009-06-12 :fushilong*/

/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_frametimer_change
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/


/*FIX PhyFA-Bug00002796  BEGIN   2011-08-29  : ningyaojun*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_frametimer_change(VOID)
{
    UINT32 u32_valid_fn = (UINT32)GSM_INVALID_FN;
    UINT16 u16_standby_bitmap = 0;
    UINT16 i;    
    UINT16  u16_standby_id      = UNSPECIFIED_STANDBY_ID;

    /*FIX  PhyFA-Bug00003005  BEGIN  2011-09-22 :wuxiaorong */
    UINT32 u32_gsm_cur_fn;
    UINT32 u32_gsm_calculated_fn;
    /*FIX PhyFA-Bug00003005  END  2011-09-22 :wuxiaorong */
    /*FIX LM-Bug00001625  BEGIN   2012-02-15  : sunzhiqiang*/
    l1cc_mmc_del_affair_info_t  st_del_affair_info;    
    l1cc_mmc_del_affair_type_t  st_del_type_info;
    /*FIX LM-Bug00001625  END   2012-02-15  : sunzhiqiang*/
    /*FIX LM-Enh00000622  BEGIN  2012-05-03 :linlan*/
    gsm_nocell_search_ctrl_t* stp_nocell_search = NULL_PTR;
    gsm_cellsearch_t         *stp_cellsearch_tbl= NULL_PTR;
    /*FIX LM-Enh00000622  END    2012-05-03 :linlan*/
    /* FIX LM-Bug00002075  BEGIN   2012-05-24  : wanghairong */
    UINT32 u32_curr_fn = (UINT32)0;
    /* FIX LM-Bug00002075  END   2012-05-24  : wanghairong */
    mmc_time_info_t st_mmc_time_info;
    fn_info_t st_get_fn_info;
    gsm_frametimer_change_info_t* stp_frametimer_change_info  = NULL_PTR;
    gsm_cellsearch_ctrl_t *stp_cellsearch_ctrl = NULL_PTR;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    
    stp_frametimer_change_info = gsm_get_frametimer_change_info();
    stp_gsm_standby_info = gsm_get_standby_info();
    stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();
    for(i=0; i<(UINT16)MAX_STANDBY_AMT; i++)
    {
        /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
        if(GSM_STANDBY_INACTIVE != stp_gsm_standby_info->u16_active_flag[i])
        /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/        
        {
            u16_standby_bitmap |= (UINT16)(0X0001<<(stp_gsm_standby_info->u16_standby_id[i]));

            /*FIX LM-Bug00001625  BEGIN   2012-02-15  : sunzhiqiang*/
            /*Clear all affairs of GSM in MMC*/
            st_del_affair_info.u16_active_id        = u16_active_id;
            st_del_affair_info.u16_mode_timing      = GSM_TIMING;                           
            st_del_affair_info.u16_del_method       = L1CC_MMC_DEL_MATCH_OTHERS_IE;
            
            st_del_type_info.u16_del_type_bitmap = MMC_DEL_TYPE_MODE_TYPE;
            st_del_type_info.u16_standby_id        = stp_gsm_standby_info->u16_standby_id[i];//必须要赋值
            /*FIX LTE-Bug00001873  BEGIN   2013-05-03  : wangjunli*/
            st_del_type_info.u16_mode_timing = GSM_TIMING;
            /*FIX LTE-Bug00001873  END   2013-05-03  : wangjunli*/
            //st_del_type_info.e_affair_id       = ...;//这个没赋值，属于全删
            st_del_affair_info.stp_del_type_info = &st_del_type_info;
            l1cc_mmc_del_affair(&st_del_affair_info);
            /*FIX LM-Bug00001625  END   2012-02-15  : sunzhiqiang*/
    
        }
    }
    dd_event_stop_by_standby(u16_active_id,u16_standby_bitmap,(UINT16)GSM_TIMING,&st_mmc_time_info);
   
    gsm_fcp_del_all_frametask((UINT16)UNSPECIFIED_STANDBY_ID);
 
    
    /*FIX LM-Bug00001527  BEGIN   2011-12-28  : yuzhengfeng*/
    gsm_fcp_init_tab_pos_index();            
    /*FIX LM-Bug00001527  END   2011-12-28  : yuzhengfeng*/ 
   

    /*FIX PhyFA-Bug00002757  BEGIN  2011-08-01 :fushilong*/
    /*FIX  PhyFA-Bug00003005  BEGIN  2011-09-22 :wuxiaorong */
    dd_fn_get(u16_active_id,(UINT16)GSM_TIMING,&st_get_fn_info);
    u32_gsm_cur_fn = st_get_fn_info.u32_subfn;
    u32_gsm_calculated_fn = gsm_fn_operate((UINT32)(((SINT32)u32_gsm_cur_fn +(SINT32) (UINT32)GSM_MAX_FN) + (SINT32)stp_frametimer_change_info->s32_delta_fn));
    dd_fn_adj(u16_active_id,(UINT16)GSM_TIMING, ((SINT32)u32_gsm_calculated_fn - (SINT32)u32_gsm_cur_fn));
    dd_fn_get(u16_active_id,(UINT16)GSM_TIMING,&st_get_fn_info);
    g_u32_gsm_fn[u16_active_id] = st_get_fn_info.u32_subfn;
    /* temporarily add,only for test view */
    OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,(UINT16)GSM_MODE,"u32_gsm_cur_fn:%ld,u32_gsm_calculated_fn:%ld,g_u32_gsm_fn[u16_active_id]:%ld",u32_gsm_cur_fn,u32_gsm_calculated_fn,g_u32_gsm_fn[u16_active_id]);
    /*FIX PhyFA-Bug00003005  END  2011-09-22 :wuxiaorong */
    /*FIX PhyFA-Bug00002757  END 2011-08-01 :fushilong */


    /*FIX LM-Bug00000992  BEGIN  2011-10-13 :fushilong*/
    /*FIX LM-Enh00000622  BEGIN  2012-05-03 :linlan*/
    stp_nocell_search = &(stp_cellsearch_ctrl->st_nocell_search);
    /* FIX NGM-Bug00000677 BEGIN 2014-10-09 : wuxiaorong */
    if (((UINT16)1 == stp_nocell_search->u16_valid_flag)&&(stp_nocell_search->u16_standby_num_from_mmc > (UINT16)0))
    /* FIX NGM-Bug00000677 END   2014-10-09 : wuxiaorong */
    {
        /* u16_standby_num_from_mmc>0, u16_cur_standby_id 必定不是UNSPECIFIED_STANDBY_ID */
        u16_standby_id = stp_nocell_search->u16_cur_standby_id;
        stp_cellsearch_tbl  =  &(stp_cellsearch_ctrl->st_cellsearch_tbl[u16_standby_id]);
        if ((UINT16)GSM_BGS_SLEEP == stp_cellsearch_tbl->u16_bg_state)
        {
            #ifdef L1CC_GSM_RTRT_UT_SWITCH
            stub_gsm_fcp_nocell_next_search_or_sleep(stp_nocell_search);
            #else
            gsm_fcp_nocell_next_search_or_sleep(stp_nocell_search);
            #endif
        }
        else
        {
            /*FIX LM-Bug00002355  BEGIN  2012-09-24 :linlan*/
            if (stp_nocell_search->u16_standby_num_from_mmc > (UINT16)0)
            {
                gsm_fcp_reconfig_nocell_search_in_cellresel(stp_cellsearch_tbl);
            }
            /*FIX LM-Bug00002355  EDN    2012-09-24 :linlan*/
        }
    }    
    /*FIX LM-Enh00000622  END  2012-05-03 :linlan*/
    /*FIX LM-Bug00000992  END  2011-10-13 :fushilong*/
    

    /* FIX LM-Bug00002075  BEGIN   2012-05-24  : wanghairong */
    u32_curr_fn = g_u32_gsm_fn[u16_active_id];    
    /* u32_valid_fn = gsm_fn_operate(g_u32_gsm_fn[u16_active_id]+1); */
     /* 帧号调整使用局部变量保存和比较，以防止由于某些原因导致g_u32_gsm_fn[u16_active_id] 在调用gsm_timer_adj前后发生改变而引起异常 */
    u32_valid_fn = gsm_fn_operate(u32_curr_fn + (UINT32)1);
    /* FIX LM-Bug00002075  END   2012-05-24  : wanghairong */
    if((SINT32)0 != stp_frametimer_change_info->s32_delta_offset)
    {
        u32_valid_fn = gsm_timer_adj((UINT16)GSM_MODE,(UINT16)0,stp_frametimer_change_info->s32_delta_offset);
    }
    /*FIX LM-Bug00001213  BEGIN 2011-11-17 :wuxiaorong */
    
    /* FIX LM-Bug00002075  BEGIN   2012-05-24  : wanghairong */
    if(u32_valid_fn == gsm_fn_operate(u32_curr_fn +(UINT32)1)) /* timing adjustment takes effect next fn */
    /* if(u32_valid_fn == gsm_fn_operate(g_u32_gsm_fn[u16_active_id] +(UINT32)1))*/ /* timing adjustment takes effect next fn */
    {
        OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,
                 (UINT16)GSM_MODE,
                 "gsm_fcp_frametimer_change():timing adjustment takes effect in next fn, delta FN=0x%lx, Qbit=%ld", 
                 stp_frametimer_change_info->s32_delta_fn,
                 stp_frametimer_change_info->s32_delta_offset);  

        stp_frametimer_change_info->s32_delta_fn = (SINT32)(UINT32)GSM_INVALID_FN;
    }
    else if(u32_valid_fn == gsm_fn_operate(u32_curr_fn + (UINT32)2)) /* timing adjustment takes effect next next fn */
    /* else if(u32_valid_fn == gsm_fn_operate(g_u32_gsm_fn[u16_active_id] + (UINT32)2))*/ /* timing adjustment takes effect next next fn */
    /* FIX LM-Bug00002075  END   2012-05-24  : wanghairong */
    {
        /* Delay one frame, next MSG_ISR_FCP_FRAMETIMER_IND will do nothing */
        OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,
                 (UINT16)GSM_MODE,
                 "gsm_fcp_frametimer_change():timing adjustment takes effect in next next fn, delta FN=0x%lx, Qbit=%ld", 
                 stp_frametimer_change_info->s32_delta_fn,
                  stp_frametimer_change_info->s32_delta_offset);           
        stp_frametimer_change_info->s32_delta_fn = (SINT32)0;
        stp_frametimer_change_info->s32_delta_offset = (SINT32)0;
    }
    else if((UINT32)GSM_INVALID_FN == u32_valid_fn)
    {
        OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,
                 (UINT16)GSM_MODE,
                 "gsm_fcp_frametimer_change():timing adjustment must try again, in next fn, delta FN=0x%lx, Qbit=%ld", 
                 stp_frametimer_change_info->s32_delta_fn,
                  stp_frametimer_change_info->s32_delta_offset);  
        /* 下一次只调整帧头，帧号不需要调了 */
        stp_frametimer_change_info->s32_delta_fn = (SINT32)0;
    }
    /*FIX LM-Bug00001213  END 2011-11-17 :wuxiaorong */
    else
    {  
        /* out of range */
        OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,(UINT16)GSM_MODE,"gsm_fcp_frametimer_change():out of range that timing adjustment takes effect, cur fn =0x%lx, valid fn=0x%lx", g_u32_gsm_fn[u16_active_id], u32_valid_fn);
        oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(VALUE_OUT_OF_RANGE));
    }

    return OAL_SUCCESS;
}
/*FIX PhyFA-Bug00002796  END     2011-08-29  : ningyaojun*/ 
/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_frametimer_pre_handle
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
*   2014-06-13      linlan                  PhyFA-Bug00004976   [NGM]调用DD接口dd_gsm_event_del入参使用错误 
*   2014-09-25      wuxiaorong              NGM-Bug00000627     NGM】【TDM】GSM的帧中断在GAP回收的CNF时间点前不配任务时需要更新stp_latest_rtx_frame_pos->u32_fn为下下帧保证不睡眠
***********************************************************************************************************************/

L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_frametimer_pre_handle(UINT16_PTR u16p_msg_rx_cfg)
{
    UINT16 u16_cbch_type =0;
    /*FIX LM-Bug00002721  BEGIN   2013-04-15  : sunzhiqiang*/
    UINT32 u32_sacch_fn;
    /*FIX LM-Bug00002721  END   2013-04-15  : sunzhiqiang*/
    oal_msg_t   *stp_oal_msg = NULL_PTR;
    p_gphy_rr_cbch_bitmap_exh_ind_t *stp_cbch_bitmap_exh_ind = NULL_PTR;
    UINT32 u32_gsm_fn;
    fn_offset_info_t st_gsm_fn_info = {0};
    /*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
    UINT16 i;    
    UINT16  u16_standby_id = UNSPECIFIED_STANDBY_ID;
    /*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
    /*FIX LM-Bug00000814  BEGIN   2011-09-22  : sunzhiqiang*/
    OAL_STATUS u32_fcb_del_ret = OAL_FAILURE;
    //UINT16     u16_fg_standby_id;
    gsm_ncell_meas_req_t *stp_ncell_meas_req = NULL_PTR;
    /*FIX LM-Bug00000814  END   2011-09-22  : sunzhiqiang*/
 
    gsm_nc_meas_sync_task_t  *stp_set_fcb_task = NULL_PTR;
    /* FIX LTE-Bug00001401  BEGIN  2012-12-10 : wuxiaorong */
    fn_offset_info_t  st_net_fn_offset_del = {(UINT16)0,(UINT16)0,(UINT32)0,(SINT32)0};
    /* FIX LTE-Bug00001401  END  2012-12-10 : wuxiaorong */
    CONST gsm_monitor_cbch_tb_t*  stp_cbch_tbl = NULL_PTR;
    gsm_nc_meas_sync_ctrl_t*  stp_nc_meas_sync_ctrl = NULL_PTR;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    CONST cc_data_for_audio_handover_t      *stp_gsm_cc_data_for_audio = NULL_PTR;
    CONST gsm_frametimer_change_info_t* stp_frametimer_change_info  = NULL_PTR;
    gsm_conn_ch_tb_t* stp_gsm_conn_ch_tb = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    mmc_time_info_t   st_mmc_time_info;
    /* FIX NGM-Bug00000195   BEGIN   2014-07-18  : linlan*/    
    UINT16 u16_valid_bitmap;
    UINT16 u16_1_left_shift;
    /* FIX NGM-Bug00000195   END     2014-07-18  : linlan*/
    /* FIX NGM-Bug00000627 BEGIN 2014-09-25 : wuxiaorong */
    UINT32 u32_gsm_next_fn;
    gsm_latest_rtx_frame_pos_t* stp_gsm_latest_rtx_frame_pos = NULL_PTR;
    gsm_offset_in_frame_t* stp_latest_rtx_frame_pos = NULL_PTR;
    /* FIX NGM-Bug00000627 END   2014-09-25 : wuxiaorong */

    
    OAL_ASSERT(NULL_PTR!= u16p_msg_rx_cfg, "u16p_msg_rx_cfg = null");
    /*FIX PhyFA-Req00001045  BEGIN   2011-09-16  : ningyaojun*/
    *u16p_msg_rx_cfg = 1;
    stp_gsm_standby_info = gsm_get_standby_info(); 
    stp_nc_meas_sync_ctrl = gsm_get_nc_meas_sync_ctrl();

    
    if(OAL_TRUE == gsm_fcp_state_changing())
    {
        /* FIX NGM-Bug00000127 BEGIN  2014-06-24: sunzhiqiang */
        gsm_update_gsm_rx_info_in_state_change();
        /* FIX NGM-Bug00000127 END  2014-06-24: sunzhiqiang */
        *u16p_msg_rx_cfg = 0;
    }
    /*FIX PhyFA-Req00001045  END     2011-09-16  : ningyaojun*/ 
   

    stp_gsm_conn_ch_tb = gsm_get_conn_ch_tb();
    stp_frametimer_change_info = gsm_get_frametimer_change_info();
    /* FIX PhyFA-Bug00000738   END  2008-09-16  : huangjinfu */
    if((UINT32)GSM_INVALID_FN != (UINT32)stp_frametimer_change_info->s32_delta_fn)
    {
        /*FIX PhyFA-Bug00002796  BEGIN   2011-08-29  : ningyaojun*/
        gsm_fcp_frametimer_change();
        /* FIX PhyFA-Bug00002938    BEGIN 2011-09-09: wuxiaorong */ 
        *u16p_msg_rx_cfg = 0;
        /* FIX PhyFA-Bug00002938    END 2011-09-09: wuxiaorong */
        /*FIX PhyFA-Bug00002796  END     2011-08-29  : ningyaojun*/ 
        /*FIX LM-Bug00002721  BEGIN   2013-04-15  : sunzhiqiang*/
        if(OAL_TRUE == stp_gsm_conn_ch_tb->b_handoverfail_flag)
        {
            u32_sacch_fn = gsm_fcp_calc_sacchstart_fn();
            if((UINT32)GSM_INVALID_FN != u32_sacch_fn)
            {
                stp_gsm_conn_ch_tb->u32_sacchstart_fn = u32_sacch_fn;
            }
        }
        else
        {
            stp_gsm_conn_ch_tb->u32_sacchstart_fn = (UINT32)GSM_INVALID_FN;
        }
        /*FIX LM-Bug00002721  END   2013-04-15  : sunzhiqiang*/
    }
    /* FIX PhyFA-Enh00000252 END  2008-11-03  : wuxiaorong */
    /*Fix LM-Req00000214 BEGIN 2011-03-28: dujianzhong  */

    stp_gsm_cc_data_for_audio = gsm_get_save_cc_data();
    
    if((UINT16)1 == stp_gsm_cc_data_for_audio->u16_handover_change_flag)
    {
        gsm_fcp_newcell_block_position_handle(u16_active_id);
    }
    /*Fix LM-Req00000214 END 2011-03-28: dujianzhong  */


    /*FIX LM-Bug00000814  BEGIN   2011-09-22  : sunzhiqiang*/
    /* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
    if(((UINT16)1 == stp_gsm_conn_ch_tb->u16_conn_ch_tb_flg)&&(OAL_TRUE == stp_gsm_conn_ch_tb->b_is_change))
    {
        u16_standby_id = stp_gsm_conn_ch_tb->u16_standby_id;
        stp_ncell_meas_req = gsm_get_ncell_meas_req(u16_standby_id);
        /*FIX A2KPH-Bug00001978     BEGIN 2009-06-12 :fushilong*/
        if(OAL_TRUE == gsm_meas_report_pos_changed(u16_standby_id))
        /*FIX A2KPH-Bug00001978     END 2009-06-12 :fushilong*/        
        {
            /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wuxiaorong */
            /* FIX NGM-Bug00000195   BEGIN   2014-07-18  : linlan*/
            u16_valid_bitmap = stp_nc_meas_sync_ctrl->u16_valid_bitmap;
            if((UINT16)0 != u16_valid_bitmap)
            {
                for (i = 0; i < GSM_NC_SYNC_SEARCH_ROUTE_NUM; i++)
                {                    
                    stp_set_fcb_task = gsm_get_nc_meas_sync_task(i);                    
                    u16_1_left_shift = (UINT16)1 << i;
                    if((UINT16)0 != (stp_nc_meas_sync_ctrl->u16_running_bitmap & u16_1_left_shift))
                    {                    
                       /* Fix PHYFA-Req00000384 BEGIN 2009-09-18 : guxiaobo*/
                       /* FIX LTE-Bug00001401  BEGIN  2012-12-10 : wuxiaorong */
                       u32_fcb_del_ret = dd_gsm_event_del(u16_active_id,(UINT16)RX_GSM_SYNC_NSLOT,stp_set_fcb_task->st_start_point[0].u32_fn,stp_set_fcb_task->st_start_point[0].u16_tsn,&st_net_fn_offset_del);
                       /* FIX LTE-Bug00001401  END  2012-12-10 : wuxiaorong */
                       if(OAL_SUCCESS != u32_fcb_del_ret)
                       {
                            /* FIX PhyFA-Bug00002758  BEGIN  2011-08-01 : linlan */        
                            /* FIX PhyFA-Req00001016 BEGIN 2011-8-22: wuxiaorong */ 
                            /* FIX LM-Bug00001505 BEGIN 2011-12-22: fushilong */ 
                            if(((UINT32)DD_DEL_FAIL_NEAR_FINISH  == u32_fcb_del_ret)
                               ||((UINT32)DD_DEL_FAIL_MATCH_FAIL == u32_fcb_del_ret)
                               ||((UINT32)DD_DEL_FAIL_DEL_AGAIN == u32_fcb_del_ret))
                            /* FIX LM-Bug00001505 END 2011-12-22: fushilong */                    
                            {
                                /*In the following conditions that 
                                (1)FCB is going to the end 
                                (2)FCB has been deleted or gone finished
                                L1CC need not to handle the failure as exception, noted by ningyaojun, 20110824
                                */
                            }
                            else
                            {
                                oal_error_handler((UINT16)ERROR_CLASS_MAJOR, GSM_FCP_ERR(RTX_EVENT_DELETE_FAILED));
                            }
                            /* FIX PhyFA-Req00001016 END 2011-8-22: wuxiaorong */
                            
                            /* FIX PhyFA-Bug00002758  END  2011-08-01 : linlan */    
                            return OAL_FAILURE;
                       }
                        
                       if(OAL_SUCCESS != gsm_fcp_del_frametask(stp_set_fcb_task->st_start_point[0].u32_fn,(UINT16)UNSPECIFIED_STANDBY_ID))
                       {
                           oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(FN_EXPIRED));
                       }
                       /* Fix PHYFA-Req00000384 END 2009-09-18 : guxiaobo*/
                    }
                    /* FIX PhyFA-Req00001026 END  2012-04-10  : wuxiaorong */
                    /* bit0 清0 */
                    /* Fix LTE-Bug00001784 BEGIN 2013-04-03 : wangjunli*/
                    stp_set_fcb_task->st_meas_cell_info.u16_no_meas = 0;
                    stp_set_fcb_task->u16_continual_count = 0;
                    /* 对应bit位清0 */
                    stp_nc_meas_sync_ctrl->u16_start_bitmap  &=~u16_1_left_shift;  
                    stp_nc_meas_sync_ctrl->u16_running_bitmap &= ~u16_1_left_shift;
                    /* Fix LTE-Bug00001784 END 2013-04-03 : wangjunli*/                    
                }
            }
            /* FIX NGM-Bug00000195   END   2014-07-18  : linlan*/

            /*
            channel changing  from sdcch to tch ,the following reporting params should be updated:
            u16_period
            u16_paging_offset
            */
            gsm_hlp_init_rssi_meas(u16_standby_id);
            /* g_st_ncell_rssi_meas.u16_ncell_num set be zero by gsm_hlp_init_rssi_meas(). */
            stp_gsm_conn_ch_tb->b_is_change = OAL_FALSE;

            
            /* FIX LM-Bug00001443  BEGIN   2011-12-14  : wanghairong */
            stp_ncell_meas_req->u32_reported_fn = gsm_get_scell_current_fn(g_u32_gsm_fn[u16_active_id],(UINT16)0,(SINT16)0, u16_standby_id);
            /* FIX LM-Bug00001443  END   2011-12-14  : wanghairong */
        }
        /*FIX LM-Bug00000814  END   2011-09-22  : sunzhiqiang*/
        /*FIX LM-Bug00003093  BEGIN   2014-02-28  : wangjunli*/
        /*如果是TCH信道切换，配置了FCB则删掉并清除标志*/        
        else if(((UINT16)2 == stp_ncell_meas_req->u16_meas_in_dedicate)&&((UINT16)MPAL_SDCCH != stp_gsm_conn_ch_tb->st_channel_params.u16_ch_type))
        {
            stp_set_fcb_task = gsm_get_nc_meas_sync_task((UINT16)0);
            /* Fix LTE-Bug00001784 BEGIN 2013-04-03 : wangjunli*/
            if((UINT16)1 == (stp_nc_meas_sync_ctrl->u16_running_bitmap&(UINT16)1))
            {
                dd_event_stop_by_standby(u16_active_id,(UINT16)((UINT16)0x0001<<u16_standby_id),(UINT16)GSM_TIMING,&st_mmc_time_info);
                stp_set_fcb_task->st_meas_cell_info.u16_no_meas = 0;
                stp_set_fcb_task->u16_continual_count = 0;
            }
            /* 对应bit位清0 */
            stp_nc_meas_sync_ctrl->u16_start_bitmap  &=~(UINT16)1; 
            stp_nc_meas_sync_ctrl->u16_running_bitmap &= ~(UINT16)1;
            /* Fix LTE-Bug00001784 END 2013-04-03 : wangjunli*/
            gsm_hlp_init_rssi_meas(u16_standby_id);
            stp_gsm_conn_ch_tb->b_is_change = OAL_FALSE;
            stp_ncell_meas_req->u32_reported_fn = gsm_get_scell_current_fn(g_u32_gsm_fn[u16_active_id],(UINT16)0,(SINT16)0, u16_standby_id);
        }
        /*FIX LM-Bug00003093  END   2014-02-28  : wangjunli*/
    }
    /* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/

    /* FIX PhyFA-Req00000134 BEGIN  2008-06-25 : wuxiaorong */
    /* FIX PhyFA-Req00001111 BEGIN  2011-11-09 : wanghuan */ 
    for(i = 0; i <(UINT16)MAX_STANDBY_AMT; i++)
    {
       if((UINT16)GSM_MASTER_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[i])
       {
            u16_standby_id = stp_gsm_standby_info->u16_standby_id[i];

            stp_cbch_tbl = gsm_get_cbch_tbl_cfg(u16_standby_id);
            if((UINT16)1 == stp_cbch_tbl->u16_cbch_tb_flg)
            {
               if((UINT16)1 == stp_cbch_tbl->u16_cbch_bitmap_tb_flg)
                {
                    /* send msg to l1resp,l1resp transfer it to hls*/
                    //???u16_standby_id=...?
                    if(OAL_TRUE == gsm_cbs_check_bitmap_state(&u16_cbch_type,u16_standby_id))
                    {

                        stp_cbch_bitmap_exh_ind = gsm_fcp_create_hls_msg(&stp_oal_msg,P_GPHY_RR_CBCH_BITMAP_EXHAUST_IND,(UINT16)sizeof(p_gphy_rr_cbch_bitmap_exh_ind_t),u16_standby_id);
                        stp_cbch_bitmap_exh_ind->cbch_type = (UINT16)(u16_cbch_type&(UINT16)0xff);
                        oal_msg_send(stp_oal_msg,(UINT16)L1CC_GSM_L1RESP_TASK_ID);/* 发送给hls的消息，先发给l1resp,然后转发给高层*/

                    }
                }
            }
       }
    }
    
    /* FIX PhyFA-Req00001111 END  2011-11-09 : wanghuan */ 
    /* FIX PhyFA-Req00000134 END  2008-06-25 : wuxiaorong */
    /* FIX NGM-Bug00000627 BEGIN 2014-09-25 : wuxiaorong */
    /* GAP回收消息内容的判断 */
    u32_gsm_fn = gsm_get_gsm_fn(u16_active_id);
    u32_gsm_next_fn = gsm_fn_operate(u32_gsm_fn + (UINT32)1);
    st_gsm_fn_info.u32_subfn = u32_gsm_next_fn;
    st_gsm_fn_info.s32_offset = (SINT32)0;
    /* 如果当前帧号所对应的时间点小于GAP回收点，不配置任务等
       到下一帧再配置 */
    if (OAL_TRUE == gsm_check_gap_withdraw_point_is_arrive(&st_gsm_fn_info))
    {
        /* 进了这个分支不睡眠.虽然*u16p_msg_rx_cfg = 0,gsm_fcp_mian()不会调睡眠，但是st_latest_rtx_frame_pos没有更新，需要更新. */
        stp_gsm_latest_rtx_frame_pos = gsm_get_latest_rtx_frame_pos();            
        stp_latest_rtx_frame_pos = &(stp_gsm_latest_rtx_frame_pos->st_latest_rtx_frame_pos);
        gsm_set_latest_rtx_frame_pos_fnhead(stp_latest_rtx_frame_pos,gsm_fn_operate(u32_gsm_next_fn + (UINT32)1));
      
        *u16p_msg_rx_cfg = 0;
    }
    /* FIX NGM-Bug00000627 END   2014-09-25 : wuxiaorong */

    return OAL_SUCCESS;
}


/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_frametimer_post_handle
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return Parameters: <name1>        <description1>
* Chage Histroy
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-09-04      gaowu                   PhyFA-Req0000XXXX   V4开发
*2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_frametimer_post_handle(VOID)
{
    UINT16 u16_gsm_state;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    UINT32  u32_curr_fn;
    
    
    
    u16_gsm_state = gsm_get_gsm_state();
    
    u32_curr_fn = gsm_get_gsm_fn(u16_active_id);      
            
    /* fcp gap控制总函数 */
    gsm_fcp_gap_ctrl_handle(u32_curr_fn, u16_gsm_state);  

   /* FIX PhyFA-Req00001525 BEGIN 2013-10-21 : wuxiaorong */
    gsm_check_meas_report_period(u32_curr_fn);
    /* FIX PhyFA-Req00001525 END   2013-10-21 : wuxiaorong */
    

    #ifdef HEARTBEAT_WATCH_DOG
    gsm_update_phyfa_heartbeat_info(u16_active_id);
    #endif
    gsm_check_gsm_work_heartbeat_info(u16_active_id);

    return OAL_SUCCESS;
}


/* FIX PhyFA-Inc00000013 END  2008-03-03 : yangguang modify */


/*FIX PhyFA-Req00000936  BEGIN   2011-05-29  : ningyaojun*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_affair_operate_req(oal_msg_t *stp_oal_msg)
{
    UINT16   u16_operate_type;      /*2:="AFFAIR_DELETEE",1:="AFFAIR_ACTIVATE",0:="AFFAIR_ADD" */
    UINT16   u16_standby_id;
    UINT16   u16_i;
    /* FIX PhyFA-Bug00003624  BEGIN  2012-08-06  : wuxiaorong */
    l1cc_rat2mmc_preempt_affair_t* stp_preempt_affair = NULL_PTR;     
    l1cc_mmc_act_result_t  st_act_result;
    msg_gsm_common_fcp_affair_operate_req_t *stp_affair_operate_req = NULL_PTR;
    l1cc_mmc_del_affair_info_t st_del_affair_info;

    OAL_ASSERT(NULL_PTR!=stp_oal_msg, "gsm_fcp_affair_operate_req null pointer");

    
    stp_affair_operate_req = (msg_gsm_common_fcp_affair_operate_req_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));
    u16_operate_type   = stp_affair_operate_req->u16_operate_type;
    u16_standby_id = stp_affair_operate_req->u16_standby_id;
    
    switch(u16_operate_type)
    {
        case AFFAIR_ADD:
            stp_preempt_affair = (l1cc_rat2mmc_preempt_affair_t *)oal_mem_alloc((UINT16)OAL_MEM_SHRAM_LEVEL,
                                                                                (UINT16)OAL_MEM_DRAM_LEVEL,
                                                                                (UINT32)OAL_GET_TYPE_SIZE_IN_WORD(l1cc_rat2mmc_preempt_affair_t));     
            oal_mem_set(stp_preempt_affair,OAL_GET_TYPE_SIZE_IN_WORD(l1cc_rat2mmc_preempt_affair_t),(UINT16)0);
            stp_preempt_affair->u16_affair_cnt = stp_affair_operate_req->u16_affair_repeat_count;
            for(u16_i=0;u16_i< stp_affair_operate_req->u16_affair_repeat_count; u16_i++)
            {
                oal_mem_copy(&(stp_preempt_affair->st_raw_affair[u16_i]),
                            &(stp_affair_operate_req->st_raw_affair),(UINT16)OAL_GET_TYPE_SIZE_IN_WORD(l1cc_mmc_raw_affair_info_t));
                stp_preempt_affair->st_raw_affair[u16_i].st_time_len.u32_sub_fn = gsm_fn_operate(stp_affair_operate_req->st_raw_affair.st_time_len.u32_sub_fn + (UINT32)u16_i);
            }
            gsm_l1cc_send_msg(stp_preempt_affair,(UINT16)L1CC_MMC_TASK_ID,(UINT32)MSG_RAT2MMC_PREEMPT_AFFAIR_REQ, 
                              (UINT16)sizeof(l1cc_rat2mmc_preempt_affair_t),u16_standby_id);
            oal_mem_free((CONST_VOID_PTR*)&stp_preempt_affair);   
            break;
        case AFFAIR_ACTIVATE:            
            l1cc_mmc_activate_affair(&stp_affair_operate_req->st_raw_affair,&st_act_result); 
            break;
        case AFFAIR_DELETE:

            st_del_affair_info.u16_active_id       = stp_affair_operate_req->st_raw_affair.u16_active_id;
            st_del_affair_info.u16_mode_timing     = GSM_TIMING;          
            
            st_del_affair_info.u16_del_method      = L1CC_MMC_DEL_MATCH_RAW_AFFAIR;
            st_del_affair_info.stp_raw_affair_info = &(stp_affair_operate_req->st_raw_affair);            
            l1cc_mmc_del_affair(&st_del_affair_info);
            break;  
        default:
            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_AFFAIR_OPERATE));            
    }
    return OAL_SUCCESS; 
    /* FIX PhyFA-Bug00003624  END  2012-08-06  : wuxiaorong */
}
/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/


/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_frame_timing_change_req
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
*   2014-05-16      zhengying               PhyFA-Enh00001893   [NGM]公共宏中的ASSERT相关修改 
***********************************************************************************************************************/


/* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_frame_timing_change_req(IN CONST oal_msg_t* CONST stp_oal_msg)
{
    OAL_STATUS u32_ret = OAL_FAILURE;
    SINT16      i;
    UINT16      u16_standby_id;      /* 0: normal task del;1:special task del;2:immi del;*/
    UINT16      u16_cause;
    UINT16      u16_standby_bitmap = 0x0000;
    CONST msg_gsm_common_fcp_frame_timing_change_req_t *stp_frame_timing_change_req = NULL_PTR;
    
    l1cc_mmc_del_affair_info_t  st_del_affair_info;    
    l1cc_mmc_del_affair_type_t  st_del_type_info;
    /*FIX PhyFA-Bug00002796  BEGIN   2011-08-15  : ningyaojun*/
    CONST time_info_struct    *stp_time_info = NULL_PTR;
    SINT32  s32_delta_fn;
    SINT16  s16_delta_offset;
    /*FIX PhyFA-Bug00002796  END     2011-08-15  : ningyaojun*/ 
    mmc_time_info_t st_mmc_time_info;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    CONST gsm_frametimer_change_info_t* stp_frametimer_change_info  = NULL_PTR;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    
    /*FIX NGM-Bug00000400  BEGIN   2014-08-26  : linlan*/
    gsm_conn_ho_tb_t *stp_gsm_conn_ho_tb = NULL_PTR;
    CONST gsm_conn_ho_params_t *stp_gsm_conn_ho_params = NULL_PTR; 
    gsm_conn_ch_tb_t *stp_gsm_conn_ch_tb = NULL_PTR;
    time_info_struct *stp_time_info_ho = NULL_PTR;
    gsm_nc_meas_sync_ctrl_t*  stp_nc_meas_sync_ctrl = NULL_PTR;
    gsm_channel_parameters_t   *stp_channel_params;
    gsm_nc_balist_t*  stp_nc_balist =NULL_PTR;  
    /*FIX NGM-Bug00000400  END     2014-08-26  : linlan*/

    OAL_ASSERT(NULL_PTR != stp_oal_msg,"gsm_fcp_frame_timing_change_req,stp_oal_msg = NULL_PTR");
    
    stp_frame_timing_change_req = (msg_gsm_common_fcp_frame_timing_change_req_t *)(OAL_GET_OAL_MSG_BODY(stp_oal_msg));
    u16_cause   = stp_frame_timing_change_req->u16_change_cause;
    u16_standby_id  = stp_frame_timing_change_req->u16_standby_id;
    stp_gsm_standby_info = gsm_get_standby_info();
    stp_frametimer_change_info = gsm_get_frametimer_change_info();

    switch(u16_cause)
    {
        case GSM_ENTER_CONN:
            gsm_del_fcb_task_reset_cellsearch_tb();           
            /*<1> Clear all events in OAL an DD*/
            for(i=0; i<MAX_STANDBY_AMT; i++)
            {
                if(GSM_STANDBY_INACTIVE != stp_gsm_standby_info->u16_active_flag[i])
                {
                    u16_standby_bitmap |= (UINT16)(0X0001<<(stp_gsm_standby_info->u16_standby_id[i]));

                    /*<3> Clear all affairs of GSM in MMC*/
                    st_del_affair_info.u16_active_id = u16_active_id;
                    st_del_affair_info.u16_mode_timing = GSM_TIMING;
                    st_del_affair_info.u16_del_method = L1CC_MMC_DEL_MATCH_OTHERS_IE;
                    //st_del_type_info.u16_rtx_chan_bitmap = 0x01;
                    st_del_type_info.u16_del_type_bitmap = MMC_DEL_TYPE_MODE_TYPE;
                    /*FIX LTE-Bug00001873  BEGIN   2013-05-03  : wangjunli*/
                    st_del_type_info.u16_mode_timing = GSM_TIMING;
                    /*FIX LTE-Bug00001873  END   2013-05-03  : wangjunli*/  
                    st_del_type_info.u16_standby_id = (stp_gsm_standby_info->u16_standby_id[i]);
                    //st_del_type_info.e_affair_id = ;
                    st_del_affair_info.stp_del_type_info = &st_del_type_info;
                    l1cc_mmc_del_affair(&st_del_affair_info);    
                }
            }
            dd_event_stop_by_standby(u16_active_id,u16_standby_bitmap,(UINT16)GSM_TIMING,&st_mmc_time_info);
   
            
            /*<2> Clear local frametask table*/
            gsm_fcp_del_all_frametask((UINT16)UNSPECIFIED_STANDBY_ID);

               

            stp_time_info = gsm_get_scell_timeinfo(u16_standby_id);
            s32_delta_fn = stp_time_info->s32_delta_fn;
            s16_delta_offset = (SINT16)((SINT16)stp_time_info->u16_tsn * GSM_QBITS_PER_TS) + stp_time_info->s16_offset_in_ts;
            gsm_fcp_time_adj(s32_delta_fn, s16_delta_offset);   

            /*<4> Transform the timeinfo of serving-cell and neighbour-cells*/
            gsm_transform_all_cells_timeinfo((UINT16)INVALID_WORD, u16_standby_id, u16_cause, g_u32_gsm_fn[u16_active_id]);           

            if((UINT32)GSM_INVALID_FN != (UINT32)stp_frametimer_change_info->s32_delta_fn)
            {
                OAL_IRQ_SAVE_AREA; /* protect g_u32_gsm_fn[u16_active_id] from callback interruptted */
                OAL_IRQ_DISABLE; 
                g_u32_gsm_fn[u16_active_id] = gsm_fn_operate((UINT32)((SINT32)g_u32_gsm_fn[u16_active_id] + (SINT32)(UINT32)GSM_MAX_FN + stp_frametimer_change_info->s32_delta_fn));
                OAL_IRQ_RESTORE;
            }  
            u32_ret = OAL_SUCCESS;
            break;
        
        /*FIX NGM-Bug00000400  BEGIN   2014-08-26  : linlan*/
        case GSM_HANDOVER_TRANSFORM:
              
            stp_gsm_conn_ho_tb     = gsm_get_conn_ho_tb();
            stp_gsm_conn_ho_params = &stp_gsm_conn_ho_tb->st_conn_ho_params;
            stp_gsm_conn_ch_tb     = gsm_get_conn_ch_tb();           
            stp_nc_meas_sync_ctrl = gsm_get_nc_meas_sync_ctrl();         
            stp_nc_balist = gsm_get_nc_balist(u16_standby_id);
            if(GSM_TABLE_VALID == stp_gsm_conn_ho_tb->u16_conn_ho_tb_flg)
            {
                stp_channel_params = &stp_gsm_conn_ho_tb->st_channel_params;

                if((UINT16)0 != stp_nc_meas_sync_ctrl->u16_running_bitmap)
                { 
                    dd_event_stop_by_standby(u16_active_id,((UINT16)((UINT16)0x0001<<u16_standby_id)),(UINT16)GSM_TIMING,&st_mmc_time_info);
                    stp_nc_meas_sync_ctrl->u16_running_bitmap &= (~(UINT16)1);/* bit0清0 */
                }
                /* FIX LM-Bug00000634 END 2011-07-18: wanghuan */ 
                /* FIX PhyFA-Bug00000798 BEGIN 2008-10-13: ChenLei */
                if((UINT16)GSM_HANDOVER_SCELL_INDEX == stp_gsm_conn_ho_params->u16_ho_bcch_idx)
                {
                    ;
                }
                else
                {
                    /* FIX PhyFA-Bug00000753 BEGIN 2008-09-26: ChenLei */
                    stp_time_info = gsm_get_ncell_timeinfo(stp_gsm_conn_ho_params->u16_ho_bcch_idx,u16_standby_id);
                    /* FIX PhyFA-Bug00000753 END 2008-09-26: ChenLei */
                    OAL_ASSERT((NULL_PTR != stp_time_info),"gsm_fcp_dedicated():Invalid timeinfo of u16_ho_bcch_idx!\n");    
                    
                    /* FIX PhyFA-Bug00000802 BEGIN 2008-11-07: ChenLei */
                    /* FIX PHYFA-Bug00000815 BEGIN 2008-10-31 :ChenLei */
                    /* FIX LM-Bug00000900 BEGIN  2011-09-29: sunzhiqiang */
                    /* FIX PhyFA-Req00001525  BEGIN 2013-11-08 : zhengying */
                    stp_time_info_ho                   = gsm_get_handover_cell_timeinfo();
                    stp_time_info_ho->u32_serv_fn      = stp_time_info->u32_serv_fn;
                    /* FIX LM-Bug00000900 END  2011-09-29: sunzhiqiang */
                    stp_time_info_ho->u16_tsn          = stp_time_info->u16_tsn;
                    stp_time_info_ho->s16_offset_in_ts = stp_time_info->s16_offset_in_ts;
                    stp_time_info_ho->s32_delta_fn     = stp_time_info->s32_delta_fn;
                    /* FIX PhyFA-Req00001525  END   2013-11-08 : zhengying */
                    /* FIX PHYFA-Bug00000815 END 2008-10-31 :ChenLei */
                    /* FIX PhyFA-Bug00000802 END 2008-11-07: ChenLei */

                    gsm_fcp_handover_target_cell((UINT16)stp_gsm_conn_ho_params->u16_ho_bcch_idx,u16_standby_id);

                    if((UINT16)1 == stp_channel_params->u16_startingtime_valid)
                    {
                        stp_channel_params->u32_starting_time = gsm_fn_operate((UINT32)((SINT32)(UINT32)GSM_MAX_FN + (SINT32)stp_channel_params->u32_starting_time + stp_time_info->s32_delta_fn));
                    }                
                }
                /* FIX PhyFA-Bug00000798 END 2008-10-13: ChenLei */
                /* FIX LM-Enh00001106 2013-03-20 BEGIN:sunzhiqiang */
                stp_nc_balist->s16_target_agc = stp_nc_balist->s16_nc_agc_list[stp_gsm_conn_ho_params->u16_ho_bcch_idx];

                stp_gsm_conn_ch_tb->u16_ct_buffer_clear_flg  = 1; 
                stp_gsm_conn_ch_tb->u16_ilbuffer_clear_flg = 1;

                /* FIX A2KPH-Bug00000412   END  2008-07-05: fushilong end*/
                }
                break;
                /*FIX NGM-Bug00000400  END     2014-08-26  : linlan*/            
        default:
            oal_error_handler(ERROR_CLASS_MAJOR, GSM_FCP_ERR(INVALID_PARAM));   
    }
    return u32_ret;
}
/* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/


/*FIX PhyFA-Req00000936  END     2011-05-29  : ningyaojun*/
/* FIX PhyFA-Bug00005191    BEGIN   2014-07-03  : linlan*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_del_cellsearch_fcb(VOID)
{
    OAL_STATUS u32_fcb_del_ret = OAL_FAILURE;
    UINT16 i;
    UINT16 u16_standby_id;
    gsm_cellsearch_t* stp_cellsearch_tbl;
    fn_offset_info_t  st_net_fn_offset_del = {(UINT16)0,(UINT16)0,(UINT32)0,(SINT32)0};    
    CONST gsm_cellsearch_ctrl_t *stp_cellsearch_ctrl = NULL_PTR;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;

    stp_gsm_standby_info = gsm_get_standby_info();

    stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();
    if((UINT16)1 == stp_cellsearch_ctrl->u16_valid_flag)
    {
        for(i=0; i< (UINT16)MAX_STANDBY_AMT; i++)
        {
            /* FIX PhyFA-Req00001026 BEGIN  2012-04-10  : wangjunli*/
            if((UINT16)GSM_STANDBY_INACTIVE != stp_gsm_standby_info->u16_active_flag[i])
            /* FIX PhyFA-Req00001026 END  2012-04-10  : wangjunli*/
            {
                u16_standby_id = stp_gsm_standby_info->u16_standby_id[i];

                stp_cellsearch_tbl = gsm_get_cellsearch_tbl_by_standby(u16_standby_id);
                if(((UINT16)1 == stp_cellsearch_tbl->u16_valid_flag) 
                &&((UINT16)GSM_BG_FCB_SETTING == stp_cellsearch_tbl->u16_fcb_set_flg))
                {
                    /* FIX LTE-Bug00001401  BEGIN  2012-12-10 : wuxiaorong */
                    u32_fcb_del_ret = dd_gsm_event_del(OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID,(UINT16)RX_GSM_SYNC_NSLOT,stp_cellsearch_tbl->u32_fcb_start_fn,stp_cellsearch_tbl->u16_fcb_rx_tsn,&st_net_fn_offset_del);
                    /* FIX LTE-Bug00001401  END  2012-12-10 : wuxiaorong */
                    if(OAL_SUCCESS == u32_fcb_del_ret)
                    {
                        stp_cellsearch_tbl->u16_fcb_set_flg =GSM_BG_FCB_SET_REQUIRED;
                    }
                    else
                    {    
                        if(((UINT32)DD_DEL_FAIL_NEAR_FINISH  == u32_fcb_del_ret)
                           ||((UINT32)DD_DEL_FAIL_MATCH_FAIL == u32_fcb_del_ret))
                        {
                            /*In the following conditions that 
                            (1)FCB is going to the end 
                            (2)FCB has been deleted or gone finished
                            L1CC need not to handle the failure as exception, noted by ningyaojun, 20110824
                            */
                            stp_cellsearch_tbl->u16_fcb_set_flg =GSM_BG_FCB_SET_REQUIRED;
                        }
                        else
                        {
                            /*FIX LM-Bug00001449  BEGIN   2011-12-15  : linlan*/
                            OAL_PRINT(u16_standby_id,(UINT16)GSM_MODE,"del FCB fail cause :%ld",u32_fcb_del_ret);
                            oal_error_handler((UINT16)ERROR_CLASS_MINOR, GSM_FCP_ERR(RTX_EVENT_DELETE_FAILED));
                            /*FIX LM-Bug00001449  END   2011-12-15  : linlan*/
                        }

                    }
                    break;   
                }
            }
        }
    }
    return u32_fcb_del_ret;
}
/* FIX PhyFA-Bug00005191    END     2014-07-03  : linlan*/

/*FIX LM-Req00000372  BEGIN   2011-09-23  : linlan*/
/************************************************************
** Function Name: gsm_del_fcb_task_reset_cellsearch_tb ()
** Description: delete fcb task and reset cellsearch table
** Input : NULL
** Output : NULL
** Return: 
** Notes:
*   2014-06-13      wuxiaorong              PhyFA-Bug00005006   NGM]GSM的FCB测量任务删除u16_running_bitmap/u16_start_bitmap的对应bit清0 
*************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_del_fcb_task_reset_cellsearch_tb(VOID)
{
    /* FIX PhyFA-Req00001925 BEGIN 2014-08-05 : wuxiaorong */
    gsm_meas_eliminate_arfcn_from_meas_sync_task((UINT16)UNSPECIFIED_STANDBY_ID);
    /* FIX PhyFA-Req00001925 END   2014-08-05 : wuxiaorong */ 
    /* FIX PhyFA-Bug00005191    BEGIN   2014-07-03  : linlan*/
    gsm_del_cellsearch_fcb();   
    /* FIX PhyFA-Bug00005191    END     2014-07-03  : linlan*/
    return OAL_SUCCESS;
}
/*FIX LM-Req00000372  END   2011-09-23  : linlan*/


/***********************************************************************************************************************
* FUNCTION:          gsm_check_del_running_sync_nslot_task
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2014-09-20      wuxiaorong              NGM-Bug00000585     [NGM]G辅模SB Search跨帧配置出现N帧配N+2帧导致后续N+1帧配N+2帧时把先前配置任务删除
*
***********************************************************************************************************************/
/*FIX NGM-Bug00000358  BEGIN   2014-08-22  : linlan*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC VOID gsm_check_del_running_sync_nslot_task(IN  CONST_UINT32 u32_next_fn,IN  CONST_UINT16 u16_rtx_bitmap)
{

    OAL_STATUS u32_fcb_del_ret = OAL_FAILURE;
    UINT16 i;
    UINT16  u16_running_bitmap;
    UINT16  u16_1_left_shift;
    CONST_UINT16  u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    fn_offset_info_t  st_net_fn_offset_del = {(UINT16)0,(UINT16)0,(UINT32)0,(SINT32)0};
    CONST gsm_nc_meas_sync_task_t*  stp_set_sync_task;
    CONST gsm_nc_meas_sync_ctrl_t*  stp_nc_meas_sync_ctrl = NULL_PTR;
    /* FIX NGM-Bug00000585 BEGIN 2014-09-20 : wuxiaorong */
    CONST gsm_slot_rtx_t* stp_slot_rtx = NULL_PTR;
    BOOLEAN  b_check_sync_nslot_del = OAL_FALSE;
    
    /* 1.首先检查u16_rtx_bitmap里有没有配置了周期接收的任务，如果配置了后续要检查有没有提前跨帧配置了测量/搜网的长事件 */
    b_check_sync_nslot_del = OAL_FALSE;
    for(i= 0; i < GSM_MAX_TIMESLOTS; i++)
    { 
        if(0 != (u16_rtx_bitmap&((UINT16)1<<i)))
        {
            stp_slot_rtx = gsm_get_tsntask(u32_next_fn,(SINT16)i);

            switch(stp_slot_rtx->u16_event_type)
            {
                case RX_GSM_NSLOT:
                case TX_GSM_AB:
                case TX_GSM_GMSK_NB:
                case TX_GSM_NSLOT:
                case TX_GSM_8PSK_NB:
                    b_check_sync_nslot_del = OAL_TRUE;       
                    break;
                default:
                    break;
            }
            if(OAL_TRUE == b_check_sync_nslot_del)
            {
                break;
            }
        }
    }
    
    if(OAL_FALSE == b_check_sync_nslot_del)
    {
        /* 1.1没有配置周期接收的任务，不需要检查有没有提前跨帧配置了测量/搜网的长事件.例如单待的G辅模式下 */
        return;
    }
    /* FIX NGM-Bug00000585 END   2014-09-20 : wuxiaorong */

    /* 2.检查邻区测量有没有RX_GSM_SYNC_NSLOT(长度至少2帧)，它一般提前跨帧配置的长事件需要删除;RX_GSM_NBIT的测量事件要删除的话，冲突检查里面gsm_fcp_del_rtx_conflict已经删除了 */
    stp_nc_meas_sync_ctrl = gsm_get_nc_meas_sync_ctrl();

    u16_running_bitmap = stp_nc_meas_sync_ctrl->u16_running_bitmap;
    if((UINT16)0 != u16_running_bitmap)
    {
        for(i = 0;i < GSM_NC_SYNC_SEARCH_ROUTE_NUM;i++)
        {
            u16_1_left_shift = ((UINT16)1 << i);
            if((UINT16)0 != (u16_running_bitmap&u16_1_left_shift))
            {
                stp_set_sync_task = gsm_get_nc_meas_sync_task((UINT16)i);
                u32_fcb_del_ret = dd_gsm_event_del(u16_active_id,(UINT16)RX_GSM_SYNC_NSLOT,stp_set_sync_task->st_start_point[0].u32_fn,stp_set_sync_task->st_start_point[0].u16_tsn,&st_net_fn_offset_del);
                if(OAL_SUCCESS == u32_fcb_del_ret)
                {
                    /* 对应bit位清0 */
                    gsm_invalid_nc_meas_sync_task(i);
                }
                else
                {    
                    /* FIX LM-Bug00001544  BEGIN  2012-01-04 : fushilong */
                    if(((UINT32)DD_DEL_FAIL_NEAR_FINISH  == u32_fcb_del_ret)
                    ||((UINT32)DD_DEL_FAIL_MATCH_FAIL == u32_fcb_del_ret)
                    ||((UINT32)DD_DEL_FAIL_DEL_AGAIN == u32_fcb_del_ret))
                    /* FIX LM-Bug00001544  END  2012-01-04 : fushilong */ 
                    {
                        /*In the following conditions that 
                        (1)FCB is going to the end 
                        (2)FCB has been deleted or gone finished
                        L1CC need not to handle the failure as exception, noted by ningyaojun, 20110824
                        */
                        OAL_PRINT((UINT16)UNSPECIFIED_STANDBY_ID,(UINT16)GSM_MODE,"u32_fcb_del_ret:%lx",u32_fcb_del_ret);
                        /* 对应bit位清0 */
                        gsm_invalid_nc_meas_sync_task(i);
                    }
                    else
                    {
                        oal_error_handler((UINT16)ERROR_CLASS_MAJOR, GSM_FCP_ERR(RTX_EVENT_DELETE_FAILED));
                    }

                }
            }
        }
    }

    /* 3.检查搜网有没有RX_GSM_SYNC_NSLOT(长度至少2帧)，它一般提前跨帧配置的长事件需要删除;RX_GSM_NBIT的测量事件要删除的话，冲突检查里面gsm_fcp_del_rtx_conflict已经删除了 */
    gsm_del_cellsearch_fcb(); 
    
    return;
}
/*FIX NGM-Bug00000358  END     2014-08-22  : linlan*/


/***********************************************************************************************************************
* FUNCTION:          gsm_del_mmc_affair
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2014-07-21      wuxiaorong              NGM-Bug00000206     [TDM]GSM模式转走后的FCP_DEL_TASK_NORMAL拖尾任务删除需要判断待激活状态
*
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC VOID gsm_del_mmc_affair(IN CONST_UINT16  u16_standby_id,IN CONST UINT16 u16_tstask,IN CONST UINT32 u32_del_fn,IN CONST UINT16 u16_del_tsn,IN CONST SINT16 s16_offset_in_ts)
{
    l1cc_mmc_raw_affair_info_t      st_raw_affair_info;
    l1cc_mmc_del_affair_info_t      st_del_affair_info;
    CONST gsm_tstask_attribute_t*   stp_tstask_attribute = NULL_PTR;
    UINT16 u16_active_id = 0;

    u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;

    
    stp_tstask_attribute = gsm_get_tstask_attribute(u16_tstask);     
    st_raw_affair_info.e_affair_id          = stp_tstask_attribute->e_affair_id;
    st_raw_affair_info.u16_standby_id       = u16_standby_id;
    st_raw_affair_info.u16_active_id        = u16_active_id;
    st_raw_affair_info.u16_mode_timing      = GSM_TIMING;
    
    st_raw_affair_info.b_fuzzy_match        = OAL_TRUE;
    st_raw_affair_info.u16_rtx_status       = stp_tstask_attribute->u16_rtx_status;  /*GSM_RX_GSM_DATA, GSM_TX_GSM_DATA, GSM_RX_TDD_DATA*/
    st_raw_affair_info.u16_freq             = INVALID_WORD;    
    st_raw_affair_info.st_time_len.u16_superfn          = 0;/*superfn必须为0*/
    st_raw_affair_info.st_time_len.u32_sub_fn           = u32_del_fn; 
    st_raw_affair_info.st_time_len.s32_start_offset     = (SINT32)(((SINT32)u16_del_tsn*(SINT32)GSM_QBITS_PER_TS)+(SINT32)s16_offset_in_ts); //???if u16_tsn>7
    st_raw_affair_info.st_time_len.u32_duration         = (UINT32)stp_tstask_attribute->fp_get_rtx_len(u16_tstask); 

    st_del_affair_info.u16_del_method      = L1CC_MMC_DEL_MATCH_RAW_AFFAIR;                                   
    st_del_affair_info.stp_raw_affair_info = &st_raw_affair_info;
    st_del_affair_info.u16_active_id        = u16_active_id;
    st_del_affair_info.u16_mode_timing      = GSM_TIMING;
    l1cc_mmc_del_affair(&st_del_affair_info);

    return;
}

/***********************************************************************************************************************
* FUNCTION:          gsm_check_irat_rx_pseudo_task
* DESCRIPTION:       <检查是否为伪事件，如果是伪事件就不下载到DD>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return Parameters: <name1>        <description1>
* Chage Histroy
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2013-12-26      gaowu                   PhyFA-Req00001525   [NGM]L1CC GSM四模版本入库
*
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC BOOLEAN gsm_check_irat_rx_pseudo_task(IN CONST_UINT16 u16_tstask)
{
    BOOLEAN b_is_arrive = OAL_FALSE;

    switch (u16_tstask)
    {
        case TSNTASK_PSD_TDS_TS_RX:
        case TSNTASK_PSD_TDS_FN_RX:
        case TSNTASK_PSD_LTE_FN_RX:
        case TSNTASK_PSD_WCDMA_FN_RX:
            
            b_is_arrive = OAL_TRUE;
            break;
        default:
            break;
    }

    return b_is_arrive;
}




/* FIX PhyFA-Req00001232   BEGIN   2012-03-30  : linlan*/
L1CC_GSM_DRAM_CODE_SECTION
VOID gsm_send_frame_timing_change_req(IN CONST_UINT16 u16_standby_id,IN CONST_UINT16 u16_change_cause)
{

    msg_gsm_common_fcp_frame_timing_change_req_t  st_msg_frame_timing_change;

    st_msg_frame_timing_change.u16_standby_id        = u16_standby_id;
    st_msg_frame_timing_change.u16_change_cause  = u16_change_cause;

    gsm_l1cc_send_msg(&st_msg_frame_timing_change, (UINT16)L1CC_GSM_FCP_TASK_ID, MSG_GSM_COMMON_FCP_FRAME_TIMING_CHANGE_REQ,(UINT16)sizeof(msg_gsm_common_fcp_frame_timing_change_req_t),u16_standby_id);
    return;
}
/* FIX PhyFA-Req00001232   END     2012-03-30  : linlan*/

L1CC_GSM_DRAM_CODE_SECTION
STATIC VOID_PTR gsm_fcp_create_hls_msg(oal_msg_t **stpp_oal_msg,UINT32 u32_primive_id,CONST_UINT16 u16_hls_struct_len/* sizeof */, UINT16 u16_standby_id)
{
    VOID_PTR vp_hls_struct;
    oal_msg_t *stp_tmp_oal_msg;

    OAL_ASSERT(NULL_PTR !=stpp_oal_msg,"stpp_oal_msg null");
    stp_tmp_oal_msg =  gsm_hls_msg_create(u16_hls_struct_len,u32_primive_id,u16_standby_id);
    vp_hls_struct=(VOID_PTR*)L1CC_GET_HLS_MSG_BODY((oal_hls_msg_t*)OAL_GET_OAL_MSG_BODY(stp_tmp_oal_msg));
    *stpp_oal_msg = stp_tmp_oal_msg;

    return vp_hls_struct;
}


/* FIX LM-Bug00002982  BEGIN   2013-10-15  : sunzhiqiang*/
L1CC_GSM_DRAM_CODE_SECTION
STATIC BOOLEAN gsm_check_other_standby_info(IN CONST_UINT16 u16_standby_id)
{
    UINT16  u16_k = 0;
    UINT16  u16_other_standby_id;
    /* FIX PhyFA-Bug00004838  BEGIN   2014-05-29  : xiongjiangjiang */
    UINT16  u16_master_standby_num = 0;
    /* FIX PhyFA-Bug00004838  END   2014-05-29  : xiongjiangjiang */
    BOOLEAN b_need_check_pch_conflict = OAL_TRUE;
    BOOLEAN b_check_other_affairflow   =OAL_FALSE; 
    l1cc_mmc_affair_flow_t st_enquiry_affair_flow_info;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;

    stp_gsm_standby_info = gsm_get_standby_info();

    for(u16_k = (UINT16)0; u16_k <(UINT16)MAX_STANDBY_AMT; u16_k++)
    {
        if((UINT16)GSM_MASTER_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[u16_k])
        {
            u16_other_standby_id = stp_gsm_standby_info->u16_standby_id[u16_k];

            /* FIX PhyFA-Bug00004838  BEGIN   2014-05-29  : xiongjiangjiang */
            u16_master_standby_num++;
            /* FIX PhyFA-Bug00004838  END   2014-05-29  : xiongjiangjiang */

            if( u16_other_standby_id != u16_standby_id)
            {
                b_check_other_affairflow = gsm_enquiry_mmc_affairflow(u16_other_standby_id,&st_enquiry_affair_flow_info);
                if((OAL_TRUE == b_check_other_affairflow)&&
                  ((MMC_AFFAIR_FLOW_PS_SIG_CNNT  == st_enquiry_affair_flow_info.u16_affair_flow)
                 ||(MMC_AFFAIR_FLOW_CS_CNNT == st_enquiry_affair_flow_info.u16_affair_flow)))
                {
                    b_need_check_pch_conflict = OAL_FALSE;
                    break;
                } 
            }
        }
    }

    /* FIX PhyFA-Bug00004838  BEGIN   2014-05-29  : xiongjiangjiang */
    /* 如果最多只有一个standby都是master，则返回false */
    if ((OAL_TRUE == b_need_check_pch_conflict)&&(u16_master_standby_num <= 1))
    {
        b_need_check_pch_conflict = OAL_FALSE;
    }
    /* FIX PhyFA-Bug00004838  END   2014-05-29  : xiongjiangjiang */

    return b_need_check_pch_conflict;
}

/***********************************************************************************************************************
* FUNCTION:          gsm_fcp_cellsearch_in_transfer
* DESCRIPTION:       <describing what the function is to do>
* NOTE:              <the limitations to use this function or other comments>
* Input Parameters:  <name1>        <description1>
* Output Parameters: <name1>        <description1>
* Return value:      <type>         <description>
* VERSION
* <DATE>          <AUTHOR>                <CR_ID>             <DESCRIPTION>
* 2014-09-20      wuxiaorong              NGM-Bug00000585     [NGM]G辅模SB Search跨帧配置出现N帧配N+2帧导致后续N+1帧配N+2帧时把先前配置任务删除
*
***********************************************************************************************************************/
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_fcp_cellsearch_in_transfer(IN CONST_UINT32 u32_next_fn,CONST oal_msg_t * CONST stp_oal_msg,IN CONST_UINT16 u16_transfer_standby_id, gsm_offset_in_frame_t* stp_latest_fn_pos)
{
    UINT16     u16_k;
    UINT16     u16_other_standby_id;
    BOOLEAN    b_check_other_affairflow   =OAL_FALSE;
    UINT16     u16_ts_bitmap[2] = {0,0}; 
    /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
    #if 0
    UINT32     u32_ret;
    UINT32     u32_curr_fn;
    #endif
    /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
    
    UINT32     u32_calculated_sb_fn = (UINT32)0;
    OAL_STATUS u32_fcp_ret = OAL_FAILURE;
    CONST gsm_cellsearch_t * stp_gsm_cellsearch  = NULL_PTR;
    CONST gsm_cellsearch_ctrl_t * stp_cellsearch_ctrl= NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp0 = NULL_PTR;
    CONST gsm_main_frametask_t* stp_gsm_main_frametask_tmp1 = NULL_PTR;
    CONST gsm_standby_info_t* stp_gsm_standby_info = NULL_PTR;
    l1cc_mmc_affair_flow_t st_enquiry_affair_flow_info;
    CONST_UINT16 u16_active_id = OAL_GET_CURR_RUNNING_TASK_ACTIVE_ID;
    /* FIX NGM-Bug00000585 BEGIN 2014-09-20 : wuxiaorong */
    UINT16  u16_rtx_bitmap = 0;
    /* FIX NGM-Bug00000585 END   2014-09-20 : wuxiaorong */

    
    stp_gsm_standby_info = gsm_get_standby_info();
    stp_cellsearch_ctrl = gsm_get_cellsearch_ctrl();  
    /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
    #if 0
    u32_ret = oal_tpc_filter_msg(MSG_GSM_FCP_PSD_FRAMETASK_INFO,(UINT16)UNSPECIFIED_STANDBY_ID,(UINT16)GSM_MODE);
    u32_curr_fn = gsm_fn_operate((u32_next_fn + (UINT32)GSM_MAX_FN) - (UINT32)1);    
    #endif
    /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */

    for(u16_k = (UINT16)0; u16_k <(UINT16)MAX_STANDBY_AMT; u16_k++)
    {
        /*transfer下主模式激活的卡是否发起了前景搜网流程/无网络收网，如果有对应流程，则分别进入相应处理分支，后续不再配置PS业务流程*/
        if((UINT16)GSM_MASTER_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[u16_k])
        {
            u16_other_standby_id = stp_gsm_standby_info->u16_standby_id[u16_k];
            if( u16_other_standby_id != u16_transfer_standby_id)
            {
                              
                stp_gsm_cellsearch = gsm_get_cellsearch_tbl_by_standby(u16_other_standby_id);
                b_check_other_affairflow = gsm_enquiry_mmc_affairflow(u16_other_standby_id,&st_enquiry_affair_flow_info);
                if((OAL_TRUE == b_check_other_affairflow)&&
                    ((MMC_AFFAIR_FLOW_FG_CELLSEARCH== st_enquiry_affair_flow_info.u16_affair_flow)
                    ||(MMC_AFFAIR_FLOW_NOCELL_SEARCH == st_enquiry_affair_flow_info.u16_affair_flow)))                            
                {
                    /* FIX NGM-Bug00000585 BEGIN 2014-09-20 : wuxiaorong */
                    u16_rtx_bitmap = gsm_get_frame_task_rtx_bitmap(gsm_get_main_frametask(u32_next_fn,u16_active_id));
                    if(0 != u16_rtx_bitmap)
                    {
                        /*FIX NGM-Bug00000358  BEGIN   2014-08-22  : linlan*/
                        gsm_check_del_running_sync_nslot_task(u32_next_fn,u16_rtx_bitmap);
                        /*gsm_del_fcb_task_reset_cellsearch_tb();  */              
                        /*FIX NGM-Bug00000358  END     2014-08-22  : linlan*/
                    }
                     /* FIX NGM-Bug00000585 END   2014-09-20 : wuxiaorong */

                    if((MMC_AFFAIR_FLOW_FG_CELLSEARCH  == st_enquiry_affair_flow_info.u16_affair_flow))
                    {
                        u32_fcp_ret = gsm_fcp_bg_process(u32_next_fn, stp_oal_msg, NULL_PTR,stp_latest_fn_pos);
                    }
                    else if((MMC_AFFAIR_FLOW_NOCELL_SEARCH == st_enquiry_affair_flow_info.u16_affair_flow))
                    {
                        u32_fcp_ret = gsm_fcp_nocell_process(u32_next_fn, stp_oal_msg, &u32_calculated_sb_fn,stp_latest_fn_pos);
                    }
                }
                /*没有事物流但是对于配置表有效，需要进入各分支检测是否到达配置搜网的时间点，添加对应事物流*/
                else if((GSM_TABLE_VALID == stp_cellsearch_ctrl->st_normal_search.u16_valid_flag) 
                        && (stp_cellsearch_ctrl->st_normal_search.u16_search_standby_num>(UINT16)0)
                        && (GSM_BGS_SLEEP == stp_gsm_cellsearch->u16_bg_state))
                {
                    u32_fcp_ret = gsm_fcp_bg_process(u32_next_fn, stp_oal_msg, NULL_PTR,stp_latest_fn_pos);
                }
                else if((GSM_TABLE_VALID ==stp_cellsearch_ctrl->st_nocell_search.u16_valid_flag)
                        && (stp_cellsearch_ctrl->st_nocell_search.u16_standby_num_from_mmc > 0)
                        && (GSM_BGS_SLEEP == stp_gsm_cellsearch->u16_bg_state))
                {       
                    u32_fcp_ret = gsm_fcp_nocell_process(u32_next_fn, stp_oal_msg, &u32_calculated_sb_fn,stp_latest_fn_pos);
                }
                /*事件下载*/                
                /* FIX PhyFA-Bug00005189  BEGIN  2014-07-01 : linlan */
                /*这里要保证之前SYNC_BCCH的事件在同步BSIC的间隙过程中能配置下去，所以不能用函数返回值*/
                /*if(OAL_SUCCESS == u32_fcp_ret)*/                    
                /* FIX PhyFA-Bug00005189  END    2014-07-01 : linlan */
                {
                    if(OAL_SUCCESS == gsm_fcp_get_undownloaded_slot_range(u32_next_fn, (UINT16)10, &u16_ts_bitmap[0], &u16_ts_bitmap[1]))
                    {                    
                        stp_gsm_main_frametask_tmp0 = gsm_get_main_frametask(u32_next_fn,u16_active_id);
                        stp_gsm_main_frametask_tmp1 = gsm_get_main_frametask(gsm_fn_operate(u32_next_fn+(UINT32)1),u16_active_id);
                        
                        /* FIX NGM-Enh00000122  BEGIN  2014-10-10: linlan */
                        /* FIX NGM-Bug00000351  BEGIN  2014-08-20: linlan */
                        gsm_fcp_check_wakeup_xc4210(u16_ts_bitmap[0],u32_next_fn);                
                        /* FIX NGM-Bug00000351  END    2014-08-20: linlan */                
                        /* FIX NGM-Enh00000122  END    2014-10-10: linlan */
                        gsm_rtx_event_req(stp_gsm_main_frametask_tmp0,
                                          stp_gsm_main_frametask_tmp1,
                                          &u16_ts_bitmap[0], &u16_ts_bitmap[1]);                        

                        /* FIX NGM-Bug00000586 BEGIN 2014-09-22 : gaowu */
                        #if 0
                        if (OAL_SUCCESS == u32_ret)
                        {
                            oal_tpc_trace_data(MSG_GSM_FCP_PSD_FRAMETASK_INFO,
                                               (UINT16_PTR)stp_gsm_main_frametask_tmp0,
                                               (UINT16)(sizeof(gsm_main_frametask_t)/sizeof(UINT16)),
                                               u32_curr_fn,
                                               u16_other_standby_id,
                                               (UINT16)GSM_MODE);
                        }
                        #endif
                        /* FIX NGM-Bug00000586 END   2014-09-22 : gaowu */
                    }   
                }
            }
        }
    }
    return u32_fcp_ret;
}




L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_add_master_standby_idle_tsntask(IN CONST_UINT32 u32_next_fn,IN CONST_UINT16 u16_standby_id,OUT gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list)
{

    UINT16 i=0,j=0;
    UINT32 u32_curr_fn;
    /* FIX LM-Bug00002982  BEGIN   2013-10-15  : sunzhiqiang*/
    BOOLEAN b_need_check_pch_conflict = OAL_TRUE;
    /* FIX LM-Bug00002982  END   2013-10-15  : sunzhiqiang*/
    CONST_UINT16 u16_tstask_rx_tb[GSM_CSIDLE_MAX_TSNTASK_NUM] =
    { 
      TSNTASK_BCCH_RX,
      TSNTASK_CCCH_RX,
      TSNTASK_PCCCH_RX,
      TSNTASK_CBCH_RX,
      TSNTASK_PBCCH_RX,
      TSNTASK_TC_BCCH_RX
    };
 
    gsm_offset_in_frame_t st_latest_fn ;
    UINT32 u32_scell_current_fn = (UINT32)0;
    gsm_ccch_tb_t  *stp_gsm_ccch_tb = NULL_PTR;
    CONST gsm_ccch_tb_t  *stp_same_cell_cch_tb = NULL_PTR;
    UINT16         u16_paging_period;
    UINT16         u16_paging_offset;  
    UINT16 u16_same_cell_standby_id = UNSPECIFIED_STANDBY_ID;
    OAL_STATUS      u32_fcp_ret = OAL_FAILURE;
    CONST gsm_scell_info_t* stp_gsm_scell_info = NULL_PTR;

   

    OAL_ASSERT(NULL_PTR != stp_latest_frame_pos_list,"");

    j = stp_latest_frame_pos_list->u16_latest_pos_num;
    u32_curr_fn = gsm_fn_operate((u32_next_fn+(UINT32)GSM_MAX_FN)-(UINT32)1);
    
    for(i=0;i<(UINT16)GSM_CSIDLE_MAX_TSNTASK_NUM;i++)
    {
        /*  FIX LM-Req00000393  BEGIN  2011-11-17 :fushilong  */
        if((UINT16)TSNTASK_CCCH_RX == u16_tstask_rx_tb[i])
        {
            /*FIX LM-Bug00001402  BEGIN   2011-12-06  : sunzhiqiang*/
            /* FIX LM-Bug00001443  BEGIN   2011-12-14  : wanghairong */
            stp_gsm_ccch_tb = gsm_get_ccch_tbl_cfg(u16_standby_id);

            if((UINT16)1 == stp_gsm_ccch_tb->u16_ccch_tb_flg)
            {
                u32_scell_current_fn = gsm_get_scell_current_fn(u32_next_fn,(UINT16)0,(SINT16)0,u16_standby_id);
                u16_paging_period = stp_gsm_ccch_tb->u16_paging_period;
                u16_paging_offset = stp_gsm_ccch_tb->u16_paging_offset;
                if(((UINT16)(gsm_fn_operate(u32_scell_current_fn + (UINT32)1)%(UINT32)u16_paging_period) == u16_paging_offset)
                    ||((UINT16)(u32_scell_current_fn%(UINT32)u16_paging_period) == u16_paging_offset))
                {
                    /*FIX LM-Bug00001361  BEGIN   2011-11-30  : sunzhiqiang*/
                    /* FIX LM-Enh00000503  BEGIN  2011-12-26: linlan */
                    /*#if 1*/
                    /* FIX LM-Enh00000435  BEGIN  2012-02-01  : sunzhiqiang */
                    #ifdef L1CC_GSM_RTRT_UT_SWITCH
                    u32_fcp_ret = stub_gsm_fcp_add_frametask((UINT16)TSNTASK_CCCH_PCH_RX,u32_next_fn,u16_standby_id); 
                    #else
                    u32_fcp_ret = gsm_fcp_add_frametask((UINT16)TSNTASK_CCCH_PCH_RX,u32_next_fn,u16_standby_id); 
                    #endif
                    /* FIX LM-Enh00000435  END  2012-02-01  : sunzhiqiang */
                    /*#endif*/
                    /* FIX LM-Enh00000503  END  2011-12-26: linlan */
                    /*FIX LM-Bug00001361  END   2011-11-30  : sunzhiqiang*/
                    /* FIX LM-Enh00000435  BEGIN  2012-02-01  : sunzhiqiang */
                    /* FIX LM-Enh00001195  BEGIN 2013-06-26  : linlan */
                    /*不区分paging mode进行冲突检测*/
                    /*if(((UINT16)MPAL_NORMAL_PAGING == stp_gsm_ccch_tb->u16_curr_paging_mode)||((UINT16)MPAL_EXTENDED_PAGING == stp_gsm_ccch_tb->u16_curr_paging_mode))*/
                    /* FIX LM-Bug00002982  BEGIN   2013-10-15  : sunzhiqiang*/
                    b_need_check_pch_conflict = gsm_check_other_standby_info(u16_standby_id);
                    if(OAL_TRUE == b_need_check_pch_conflict)
                    {
                        stp_gsm_scell_info = gsm_get_gsm_scell_info(u16_standby_id);
                        u16_same_cell_standby_id = stp_gsm_scell_info->u16_same_cell_standby_id;

                        stp_same_cell_cch_tb = gsm_get_ccch_tbl_cfg(u16_same_cell_standby_id);
                        /* 无相同小区，或者有相同小区且相同小区CCCH配置无效时，进行寻呼冲突检测 */
                        if((((UINT16)1 != stp_same_cell_cch_tb->u16_ccch_tb_flg)&&(u16_same_cell_standby_id != u16_standby_id))
                           ||(u16_same_cell_standby_id == u16_standby_id)
                          )
                        {
                            if(OAL_SUCCESS == stp_gsm_ccch_tb->u32_arrive_ret)/*pch position is arrived*/
                            {
                                /*FIX LM-Enh00000574  BEGIN   2012-02-29  : sunzhiqiang*/
                                if(GSM_INTRA_CONFLICT == u32_fcp_ret)/*add TSNTASK_CCCH_PCH_RX failed*/
                                /*FIX LM-Enh00000574  END   2012-02-29  : sunzhiqiang*/
                                {
                                    #ifdef L1CC_GSM_RTRT_UT_SWITCH
                                    stub_gsm_fcp_update_pch_conflict_count(u16_standby_id);
                                    #else
                                    gsm_fcp_update_pch_conflict_count(u16_standby_id);
                                    #endif
                                }
                                else
                                {
                                    /*add TSNTASK_CCCH_PCH_RX succeed, pch_conflict_count should be initilized*/
                                    if((UINT16)GSM_PCH_CONFLICT_THRESHOLD > stp_gsm_ccch_tb->u16_dsc_init_val)
                                    {
                                        stp_gsm_ccch_tb->u16_pch_conflict_count = GSM_PCH_CONFLICT_THRESHOLD;
                                    }
                                    else
                                    {
                                        stp_gsm_ccch_tb->u16_pch_conflict_count = stp_gsm_ccch_tb->u16_dsc_init_val;
                                    }
                                    /* FIX LM-Enh00001294  BEGIN  2013-09-03  : sunzhiqiang */
                                    if(GSM_PCH_CFG_CONFLICT_THRESHOLD < stp_gsm_ccch_tb->u32_pch_cfg_lost_cnt)
                                    {   
                                        gsm_fcp_handle_hls_res((UINT16)GSM_FCP_PCH_CONFLICT_IND,u32_curr_fn,OAL_SUCCESS,u16_standby_id);
                                        stp_gsm_ccch_tb->u32_pch_cfg_lost_cnt = (UINT32)0;
                                    }
                                    else
                                    {
                                        stp_gsm_ccch_tb->u32_pch_cfg_lost_cnt++;
                                    }
                                    /* FIX LM-Enh00001294  END  2013-09-03  : sunzhiqiang */
                                }
                                stp_gsm_ccch_tb->u32_arrive_ret = OAL_FAILURE;
                            } 
                        }
                    }
                    else
                    {
                        stp_gsm_ccch_tb->u32_arrive_ret = OAL_FAILURE;
                    }
                    /* FIX LM-Bug00002982  END   2013-10-15  : sunzhiqiang*/
                    /* FIX LM-Enh00000435  END  2012-02-01  : sunzhiqiang */
                }
                /* FIX LM-Bug00001443  END   2011-12-14  : wanghairong */
                else
                {
                    #ifdef L1CC_GSM_RTRT_UT_SWITCH
                    stub_gsm_fcp_add_frametask((UINT16)TSNTASK_CCCH_RX,u32_next_fn,u16_standby_id); 
                    #else
                    gsm_fcp_add_frametask((UINT16)TSNTASK_CCCH_RX,u32_next_fn,u16_standby_id); 
                    #endif
                }
                /* FIX NGM-Bug00000089  BEGIN 2014-06-06 : zhengying */
                /* FIX LM-Bug00002854  BEGIN 2013-07-06  : yuzhengfeng */
                if(OAL_TRUE == stp_gsm_ccch_tb->b_need_pre_add_mmc_affaire)
                {
                    gsm_fcp_pre_add_mmc_affair(TSNTASK_CCCH_PCH_RX,u16_standby_id,gsm_fn_operate(u32_curr_fn + (UINT32)6));    
                    stp_gsm_ccch_tb->b_need_pre_add_mmc_affaire= OAL_FALSE;
                }
                /* FIX LM-Bug00002854  END 2013-07-06  : yuzhengfeng */
                /*FIX LM-Bug00001402  END   2011-12-06  : sunzhiqiang*/
                /* FIX NGM-Bug00000089  END   2014-06-06 : zhengying */
            }

        }
        else
        {
            #ifdef L1CC_GSM_RTRT_UT_SWITCH
            stub_gsm_fcp_add_frametask(u16_tstask_rx_tb[i],u32_next_fn,u16_standby_id);
            #else
            gsm_fcp_add_frametask(u16_tstask_rx_tb[i],u32_next_fn,u16_standby_id);
            #endif
        }
        /*  FIX LM-Req00000393  END 2011-11-17 :fushilong    */ 
        gsm_fcp_get_tsntask_latestfn(u16_tstask_rx_tb[i],u32_curr_fn,u16_standby_id,&st_latest_fn);

        if((UINT32)GSM_INVALID_FN != st_latest_fn.u32_fn)
        {
            stp_latest_frame_pos_list->st_latest_frame_pos[j].u32_fn = st_latest_fn.u32_fn;
            stp_latest_frame_pos_list->st_latest_frame_pos[j].u16_tsn = st_latest_fn.u16_tsn;
            stp_latest_frame_pos_list->st_latest_frame_pos[j].s16_offset = st_latest_fn.s16_offset;
            j++;
        }  
    }

    stp_latest_frame_pos_list->u16_latest_pos_num = j;
    return OAL_SUCCESS;
}


L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_add_master_standby_nc_bcch_tsntask(IN CONST_UINT32 u32_next_fn,IN CONST_UINT16 u16_standby_id,OUT  gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list)
{
    UINT16 i=0,j=0;
    UINT32 u32_curr_fn;
    UINT16 u16_nc_tb_num;
    gsm_offset_in_frame_t st_latest_fn   = {(UINT32)0};
    CONST gsm_nc_bcch_tb_t *stp_gsm_nc_bcch_tb = NULL_PTR;
    CONST_UINT16 u16_tstask_nc_rx_tb[GSM_CSIDLE_MAX_TSNTASK_NC_NUM] =
    {
      TSNTASK_NC0_BCCH_RX,
      TSNTASK_NC1_BCCH_RX,
      TSNTASK_NC2_BCCH_RX,
      TSNTASK_NC3_BCCH_RX,
      TSNTASK_NC4_BCCH_RX,
      TSNTASK_NC5_BCCH_RX,
      TSNTASK_NC6_BCCH_RX
    };
    

    
    OAL_ASSERT(NULL_PTR != stp_latest_frame_pos_list,"");
    j = stp_latest_frame_pos_list->u16_latest_pos_num;
    u32_curr_fn = gsm_fn_operate((u32_next_fn+(UINT32)GSM_MAX_FN)-(UINT32)1);
    stp_gsm_nc_bcch_tb = gsm_get_nc_bcch_tbl_cfg(u16_standby_id);
    if((UINT16)GSM_TABLE_VALID == stp_gsm_nc_bcch_tb->u16_nc_bcch_tb_flg)
    {
        u16_nc_tb_num = stp_gsm_nc_bcch_tb->u16_avail_nc_num;
        for(i= 0; i < u16_nc_tb_num; i++)
        {
            gsm_fcp_add_frametask(u16_tstask_nc_rx_tb[i],u32_next_fn,u16_standby_id);
            gsm_fcp_get_tsntask_latestfn(u16_tstask_nc_rx_tb[i],u32_curr_fn,u16_standby_id,&st_latest_fn);
            if((UINT32)GSM_INVALID_FN != st_latest_fn.u32_fn)
            {
                stp_latest_frame_pos_list->st_latest_frame_pos[j].u32_fn = st_latest_fn.u32_fn;
                stp_latest_frame_pos_list->st_latest_frame_pos[j].u16_tsn = st_latest_fn.u16_tsn;
                stp_latest_frame_pos_list->st_latest_frame_pos[j].s16_offset = st_latest_fn.s16_offset;
                j++;
            }        
        }
    }
    stp_latest_frame_pos_list->u16_latest_pos_num = j;
    return OAL_SUCCESS;
}

L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_add_master_standby_plmn_bcch_tsntask(IN CONST_UINT32 u32_next_fn,IN CONST_UINT16 u16_standby_id,OUT  gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list)
{
    UINT16 i=0,j=0;
    UINT32 u32_curr_fn;
    CONST gsm_search_bcch_schedu_t * stp_bg_bcch_pos = NULL_PTR;
    gsm_offset_in_frame_t st_latest_fn   = {(UINT32)0};
    CONST gsm_search_bcch_tb_t *stp_bg_bcch_tb = NULL_PTR;
    CONST_UINT16 u16_tstask_bg_bcch_rx_tb[GSM_IDLE_MAX_TSNTASK_BG_BCCH_NUM] =
    {
        TSNTASK_BG_TC0_BCCH_RX,
        TSNTASK_BG_TC1_BCCH_RX,
        TSNTASK_BG_TC2_BCCH_RX,
        TSNTASK_BG_TC3_BCCH_RX,
        TSNTASK_BG_TC4_BCCH_RX,
        TSNTASK_BG_TC5_BCCH_RX
    };

    
    OAL_ASSERT(NULL_PTR != stp_latest_frame_pos_list,"");
    j = stp_latest_frame_pos_list->u16_latest_pos_num;
    u32_curr_fn = gsm_fn_operate((u32_next_fn+(UINT32)GSM_MAX_FN)-(UINT32)1);
    
    /* Fix LM-Enh00001221 BEGIN 2013-07-24 : wangjunli*/
    /*添加PLMN过程BCCH接收*/
    stp_bg_bcch_tb =     gsm_get_bg_bcch_tbl_cfg(u16_standby_id);
    if((UINT16)GSM_TABLE_VALID == stp_bg_bcch_tb->u16_valid_flag)
    {
        for(i = 0;i< MAX_ARFCN_NUM_OF_BG_BCCH; i++)/*bg_bcch_tb 不一定是按序存放的，需要遍历一遍*/
        {
            stp_bg_bcch_pos = &(stp_bg_bcch_tb->st_search_bcch_schedu_list[i]);
            if((UINT16)GSM_TABLE_VALID == stp_bg_bcch_pos->u16_valid_flag)
            {
                gsm_fcp_add_frametask(u16_tstask_bg_bcch_rx_tb[i],u32_next_fn,u16_standby_id);

                gsm_fcp_get_tsntask_latestfn(u16_tstask_bg_bcch_rx_tb[i],u32_curr_fn,u16_standby_id,&st_latest_fn);
                if((UINT32)GSM_INVALID_FN != st_latest_fn.u32_fn)
                {
                    stp_latest_frame_pos_list->st_latest_frame_pos[j].u32_fn = st_latest_fn.u32_fn;
                    stp_latest_frame_pos_list->st_latest_frame_pos[j].u16_tsn = st_latest_fn.u16_tsn;
                    stp_latest_frame_pos_list->st_latest_frame_pos[j].s16_offset = st_latest_fn.s16_offset;
                    j++;
                }   
            }
        }
    }
    /* Fix LM-Enh00001221 END 2013-07-24 : wangjunli*/
    stp_latest_frame_pos_list->u16_latest_pos_num = j;
    return OAL_SUCCESS;
}
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_add_ccch_in_transfer_tsntask(IN CONST_UINT32 u32_next_fn,OUT gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list)
{
    UINT16 k =0;
    UINT16 u16_j=0;
    UINT16 u16_bg_standby_id = 0;
    UINT32 u32_scell_current_fn;
    UINT16 u16_paging_period =0;
    UINT16 u16_paging_offset = 0;
    UINT32 u32_fcp_ret = OAL_FAILURE;
    UINT16 u16_same_cell_standby_id;
    UINT16 u16_standby_id = UNSPECIFIED_STANDBY_ID;
    UINT32 u32_curr_fn;
    CONST gsm_standby_info_t*  stp_gsm_standby_info = NULL_PTR;
    gsm_ccch_tb_t* stp_temp_ccch_tlb = NULL_PTR;
    CONST gsm_ccch_tb_t* stp_gsm_ccch_tb = NULL_PTR;
    CONST gsm_scell_info_t* stp_scell_info = NULL_PTR;
    CONST gprs_pdtch_tb_t* stp_gprs_pdtch_tb = NULL_PTR;
    gsm_offset_in_frame_t st_latest_fn_pos;

    OAL_ASSERT(stp_latest_frame_pos_list != NULL_PTR , "stp_latest_frame_pos_list == NULL_PTR");
    stp_gsm_standby_info = gsm_get_standby_info();   
    stp_gprs_pdtch_tb = gsm_get_gprs_cfg_tbl();
    u16_standby_id = stp_gprs_pdtch_tb->u16_standby_id;    
    u16_j = stp_latest_frame_pos_list->u16_latest_pos_num;
    u32_curr_fn = gsm_fn_operate((u32_next_fn + (UINT32)GSM_MAX_FN) - (UINT32)1);
    
    /*FIX LM-Req00000372  BEGIN   2011-09-23  : linlan*/ 
    for(k = (UINT16)0; k < (UINT16)MAX_STANDBY_AMT; k++)
    {
        if((UINT16)GSM_MASTER_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[k])
        {
            u16_bg_standby_id = stp_gsm_standby_info->u16_standby_id[k];

            stp_temp_ccch_tlb = gsm_get_ccch_tbl_cfg(u16_bg_standby_id);
            
            if((UINT16)1 == stp_temp_ccch_tlb->u16_ccch_tb_flg)
            {
                /* FIX LM-Enh00001186 BEGIN  2013-06-09  yuzhengfeng*/
               /* when page mode is FULL_CCCH in transfer, add task as NORMAL_CCCH*/
              /* FIX LM-Bug00003047 BEGIN  2013-12-23  wangjunli*/
               if((MPAL_FULL_CCCH_BLKS == stp_temp_ccch_tlb->u16_curr_paging_mode)
                 ||(MPAL_PAGING_REORG == stp_temp_ccch_tlb->u16_curr_paging_mode))
               /* FIX LM-Bug00003047 END  2013-12-23  wangjunli*/
               {
                    u32_scell_current_fn = gsm_get_scell_current_fn(u32_next_fn,(UINT16)0,(SINT16)0,u16_bg_standby_id);
                    u16_paging_period = stp_temp_ccch_tlb->u16_paging_period;
                    u16_paging_offset = stp_temp_ccch_tlb->u16_paging_offset;
                    if(((UINT16)(gsm_fn_operate(u32_scell_current_fn + (UINT32)1)%(UINT32)u16_paging_period) == u16_paging_offset)
                        ||((UINT16)(u32_scell_current_fn%(UINT32)u16_paging_period) == u16_paging_offset))
                    {
                        gsm_fcp_add_frametask((UINT16)TSNTASK_TRANSFER_CCCH_RX,u32_next_fn,u16_bg_standby_id);
                    }
               }
               else
               {
                    u32_fcp_ret = gsm_fcp_add_frametask((UINT16)TSNTASK_TRANSFER_CCCH_RX,u32_next_fn,u16_bg_standby_id);
                    if(u16_bg_standby_id != u16_standby_id)/* 非业务卡寻呼如果配置不了，则认为寻呼冲突，业务卡业务优先，业务卡即使冲突也不重选  */
                    {
                        if(((UINT16)MPAL_NORMAL_PAGING == stp_temp_ccch_tlb->u16_curr_paging_mode)||((UINT16)MPAL_EXTENDED_PAGING == stp_temp_ccch_tlb->u16_curr_paging_mode))
                        {
                            stp_scell_info = gsm_get_gsm_scell_info(u16_bg_standby_id);                                       
                            u16_same_cell_standby_id = stp_scell_info->u16_same_cell_standby_id;
                            stp_gsm_ccch_tb = gsm_get_ccch_tbl_cfg(u16_same_cell_standby_id);
                            /* 无相同小区，或者有相同小区且相同小区CCCH配置无效时，进行寻呼冲突检测 */
                            if((((UINT16)1 != stp_gsm_ccch_tb->u16_ccch_tb_flg)&&(u16_same_cell_standby_id != u16_bg_standby_id))
                             ||(u16_same_cell_standby_id == u16_bg_standby_id)
                              )
                            {
                                if(GSM_INTRA_CONFLICT == u32_fcp_ret)/* 到达寻呼点，但是寻呼配置失败 */
                                {
                                    gsm_fcp_update_pch_conflict_count(u16_bg_standby_id);
                                }
                                else if(OAL_SUCCESS == u32_fcp_ret)/* 到达寻呼点，寻呼配置成功 */
                                {
                                    /*add TSNTASK_CCCH_PCH_RX succeed, pch_conflict_count should be initilized*/
                                    /* FIX LM-Enh00001306 BEGIN  2013-09-13: wangjunli */
                                    stp_temp_ccch_tlb->u16_pch_conflict_count += (UINT16)2;
                                    if((UINT16)GSM_PCH_CONFLICT_THRESHOLD > stp_temp_ccch_tlb->u16_dsc_init_val)
                                    {              
                                        if((UINT16)GSM_PCH_CONFLICT_THRESHOLD < stp_temp_ccch_tlb->u16_pch_conflict_count)
                                        {
                                            stp_temp_ccch_tlb->u16_pch_conflict_count = (UINT16)GSM_PCH_CONFLICT_THRESHOLD;
                                        }
                                    }
                                    else
                                    {
                                        if((UINT16)stp_temp_ccch_tlb->u16_dsc_init_val < stp_temp_ccch_tlb->u16_pch_conflict_count)
                                        {
                                            stp_temp_ccch_tlb->u16_pch_conflict_count = (UINT16)stp_temp_ccch_tlb->u16_dsc_init_val;
                                        }
                                    }
                                    /* FIX LM-Enh00001306 END  2013-09-13: wangjunli */
                                }
                                else
                                {
                                    /* 没有到寻呼点 */
                                }
                            } 
                        }
                    }
               }
              /* FIX LM-Enh00001186 END  2013-06-09  yuzhengfeng*/
            } 
            gsm_fcp_get_tsntask_latestfn(TSNTASK_CCCH_RX,u32_curr_fn,u16_bg_standby_id,&st_latest_fn_pos);
            if((UINT32)GSM_INVALID_FN != st_latest_fn_pos.u32_fn)
            {
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].u32_fn    = st_latest_fn_pos.u32_fn;
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].u16_tsn   = st_latest_fn_pos.u16_tsn;
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].s16_offset= st_latest_fn_pos.s16_offset;
                u16_j++;
            }
            stp_latest_frame_pos_list->u16_latest_pos_num = u16_j;
        }
    }
    return OAL_SUCCESS;
}

L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_add_bcch_in_transfer_tsntask(IN CONST_UINT32 u32_next_fn,OUT gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list)
{

    UINT16 u16_bg_standby_id = 0;
    UINT16 k,i,u16_j;
    UINT32 u32_curr_fn;
    UINT16 u16_nc_tb_num;
    /* FIX PhyFA-Enh00001647 BEGIN 2014-04-28 : sunzhiqiang */
    UINT16 u16_tstask;
    UINT16  u16_standby_id = UNSPECIFIED_STANDBY_ID;
    CONST gprs_pdtch_tb_t *stp_gprs_pdtch_tb = NULL_PTR;
    /* FIX PhyFA-Enh00001647 END 2014-04-28 : sunzhiqiang */
    CONST gsm_standby_info_t*  stp_gsm_standby_info = NULL_PTR;
    gsm_offset_in_frame_t st_latest_fn_pos;
    CONST gsm_sc_bcch_tb_t *stp_gsm_sc_bcch_tb = NULL_PTR;    
    CONST gsm_tc_bcch_tb_t *stp_gsm_tc_bcch_tb = NULL_PTR;
    CONST gsm_nc_bcch_tb_t *stp_gsm_nc_bcch_tb = NULL_PTR;
    CONST gsm_transfer_nc_bcch_schedu_t* stp_transfer_nc_bcch_tb = NULL_PTR;

    CONST_UINT16 tsn_task_nc_rx_tb[GSM_CSIDLE_MAX_TSNTASK_NC_NUM] =
    {
      TSNTASK_NC0_BCCH_RX,
      TSNTASK_NC1_BCCH_RX,
      TSNTASK_NC2_BCCH_RX,
      TSNTASK_NC3_BCCH_RX,
      TSNTASK_NC4_BCCH_RX,
      TSNTASK_NC5_BCCH_RX,
      TSNTASK_NC6_BCCH_RX
    };

    OAL_ASSERT(NULL_PTR != stp_latest_frame_pos_list , "");
    u16_j = stp_latest_frame_pos_list->u16_latest_pos_num;    
    u32_curr_fn = gsm_fn_operate((u32_next_fn + (UINT32)GSM_MAX_FN) - (UINT32)1);
    stp_gsm_standby_info = gsm_get_standby_info();   
    /* FIX PhyFA-Enh00001647 BEGIN 2014-04-28 : sunzhiqiang */
    stp_gprs_pdtch_tb = gsm_get_gprs_cfg_tbl();
    u16_standby_id = stp_gprs_pdtch_tb->u16_standby_id;    
    /* FIX PhyFA-Enh00001647 END 2014-04-28 : sunzhiqiang */
    
    for(k = (UINT16)0; k <(UINT16)MAX_STANDBY_AMT; k++)
    {
        if((UINT16)GSM_MASTER_STANDBY_ACTIVE == stp_gsm_standby_info->u16_active_flag[k])
        {
            u16_bg_standby_id = stp_gsm_standby_info->u16_standby_id[k];
            /*TRANSFER NC BCCH*/
            stp_transfer_nc_bcch_tb = gsm_get_transfer_nc_bcch_tbl_cfg(u16_bg_standby_id);
            if((UINT16)1 == stp_transfer_nc_bcch_tb->u16_nc_bcch_tb_flg)
            {
                gsm_fcp_add_frametask((UINT16)TSNTASK_TRANSFER_NC_BCCH_RX,u32_next_fn,u16_bg_standby_id);
            }            
            gsm_fcp_get_tsntask_latestfn(TSNTASK_TRANSFER_NC_BCCH_RX,u32_curr_fn,u16_bg_standby_id,&st_latest_fn_pos);
            if((UINT32)GSM_INVALID_FN != st_latest_fn_pos.u32_fn)
            {
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].u32_fn    = st_latest_fn_pos.u32_fn;
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].u16_tsn   = st_latest_fn_pos.u16_tsn;
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].s16_offset= st_latest_fn_pos.s16_offset;
                u16_j++;
            }

            /*SCELL BCCH */
            stp_gsm_sc_bcch_tb = gsm_get_sc_bcch_tbl_cfg(u16_bg_standby_id);
            /* FIX PhyFA-Enh00001647 BEGIN 2014-04-28 : sunzhiqiang */            
            /* FIX NGM-Bug00000620   BEGIN 2014-09-26 : linlan */
            if(u16_standby_id == u16_bg_standby_id)
            /* FIX NGM-Bug00000620   END   2014-09-26 : linlan */
            {
                u16_tstask = TSNTASK_TRANSFER_BCCH_RX;
                
            }
            else
            {
                u16_tstask = TSNTASK_BCCH_RX;
            }
            if((UINT16)1 == stp_gsm_sc_bcch_tb->u16_sc_bcch_tb_flg)
            {
                gsm_fcp_add_frametask((UINT16)u16_tstask,u32_next_fn,u16_bg_standby_id);
            }
            gsm_fcp_get_tsntask_latestfn(u16_tstask,u32_curr_fn,u16_bg_standby_id,&st_latest_fn_pos);
            /* FIX PhyFA-Enh00001647 END 2014-04-28 : sunzhiqiang */
            if((UINT32)GSM_INVALID_FN != st_latest_fn_pos.u32_fn)
            {
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].u32_fn    = st_latest_fn_pos.u32_fn;
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].u16_tsn   = st_latest_fn_pos.u16_tsn;
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].s16_offset= st_latest_fn_pos.s16_offset;
                u16_j++;
            }
            /*TC BCCH*/
            stp_gsm_tc_bcch_tb = gsm_get_tc_bcch_tbl_cfg(u16_bg_standby_id);
            if((UINT16)1 == stp_gsm_tc_bcch_tb->u16_tc_bcch_tb_flg)
            {
                gsm_fcp_add_frametask((UINT16)TSNTASK_TC_BCCH_RX,u32_next_fn,u16_bg_standby_id);
            }
            gsm_fcp_get_tsntask_latestfn(TSNTASK_TC_BCCH_RX,u32_curr_fn,u16_bg_standby_id,&st_latest_fn_pos);
            if((UINT32)GSM_INVALID_FN != st_latest_fn_pos.u32_fn)
            {
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].u32_fn    = st_latest_fn_pos.u32_fn;
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].u16_tsn   = st_latest_fn_pos.u16_tsn;
                stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].s16_offset= st_latest_fn_pos.s16_offset;
                u16_j++;
            }

            /*NCELL BCCH*/
            stp_gsm_nc_bcch_tb = gsm_get_nc_bcch_tbl_cfg(u16_bg_standby_id);
            if((UINT16)GSM_TABLE_VALID == stp_gsm_nc_bcch_tb->u16_nc_bcch_tb_flg)
            {
                u16_nc_tb_num = stp_gsm_nc_bcch_tb->u16_avail_nc_num;
                for(i = (UINT16)0; i < u16_nc_tb_num; i++)
                {
                    gsm_fcp_add_frametask((UINT16)tsn_task_nc_rx_tb[i],u32_next_fn,u16_bg_standby_id);
                    gsm_fcp_get_tsntask_latestfn((UINT16)tsn_task_nc_rx_tb[i],u32_curr_fn,u16_bg_standby_id,&st_latest_fn_pos);
                    if((UINT32)GSM_INVALID_FN != st_latest_fn_pos.u32_fn)
                    {
                        u16_j = stp_latest_frame_pos_list->u16_latest_pos_num;
                        stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].u32_fn    = st_latest_fn_pos.u32_fn;
                        stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].u16_tsn   = st_latest_fn_pos.u16_tsn;
                        stp_latest_frame_pos_list->st_latest_frame_pos[u16_j].s16_offset= st_latest_fn_pos.s16_offset;
                        u16_j++;
                    }
                }
            }   
        }            
    }
    
    stp_latest_frame_pos_list->u16_latest_pos_num = u16_j;
    return OAL_SUCCESS;
}

L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_add_sync_cell_bcch_tsntask(IN CONST_UINT32 u32_next_fn,IN CONST_UINT16 u16_standby_id,OUT  gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list)
{
    UINT16 i=0,j=0;
    UINT32 u32_curr_fn;
    CONST gsm_search_bcch_schedu_t * stp_sync_bcch_pos = NULL_PTR;
    gsm_offset_in_frame_t st_latest_fn_pos;
    CONST gsm_search_bcch_tb_t *stp_sync_bcch_tb = NULL_PTR;
    CONST_UINT16 u16_tstask_bg_bcch_rx_tb[GSM_IDLE_MAX_TSNTASK_BG_BCCH_NUM] =
    {
        TSNTASK_SYNC_CELL0_BCCH_RX,
        TSNTASK_SYNC_CELL1_BCCH_RX,
        TSNTASK_SYNC_CELL2_BCCH_RX,
        TSNTASK_SYNC_CELL3_BCCH_RX,
        TSNTASK_SYNC_CELL4_BCCH_RX,
        TSNTASK_SYNC_CELL5_BCCH_RX
    };

    
    OAL_ASSERT(NULL_PTR != stp_latest_frame_pos_list,"");
    j = stp_latest_frame_pos_list->u16_latest_pos_num;
    u32_curr_fn = gsm_fn_operate((u32_next_fn+(UINT32)GSM_MAX_FN)-(UINT32)1);
    
    /* Fix LM-Enh00001221 BEGIN 2013-07-24 : wangjunli*/
    /*添加SYNC CELL过程BCCH接收*/
    stp_sync_bcch_tb =     gsm_get_sync_bcch_tbl_cfg(u16_standby_id);
    if((UINT16)GSM_TABLE_VALID == stp_sync_bcch_tb->u16_valid_flag)
    {
        for(i = 0;i< MAX_ARFCN_NUM_OF_BG_BCCH; i++)/*bg_bcch_tb 不一定是按序存放的，需要遍历一遍*/
        {
            stp_sync_bcch_pos = &(stp_sync_bcch_tb->st_search_bcch_schedu_list[i]);
            if((UINT16)GSM_TABLE_VALID == stp_sync_bcch_pos->u16_valid_flag)
            {
                gsm_fcp_add_frametask(u16_tstask_bg_bcch_rx_tb[i],u32_next_fn,u16_standby_id);

                gsm_fcp_get_tsntask_latestfn(u16_tstask_bg_bcch_rx_tb[i],u32_curr_fn,u16_standby_id,&st_latest_fn_pos);
                if((UINT32)GSM_INVALID_FN != st_latest_fn_pos.u32_fn)
                {
                    stp_latest_frame_pos_list->st_latest_frame_pos[j].u32_fn = st_latest_fn_pos.u32_fn;
                    stp_latest_frame_pos_list->st_latest_frame_pos[j].u16_tsn = st_latest_fn_pos.u16_tsn;
                    stp_latest_frame_pos_list->st_latest_frame_pos[j].s16_offset = st_latest_fn_pos.s16_offset;
                    j++;
                }   
            }
        }
    }
    /* Fix LM-Enh00001221 END 2013-07-24 : wangjunli*/
    stp_latest_frame_pos_list->u16_latest_pos_num = j;
    return OAL_SUCCESS;
}

/* FIX NGM-Bug00000496 BEGIN 2014-09-25 : guxiaobo */
L1CC_GSM_DRAM_CODE_SECTION
STATIC OAL_STATUS gsm_add_slave_standby_tsntask(IN CONST_UINT32 u32_next_fn,IN CONST_UINT16 u16_standby_id,OUT  gsm_tstask_latest_frame_pos_t* CONST stp_latest_frame_pos_list)
{
    UINT16   u16_i;
    UINT16   u16_resync_cell_ind;
    UINT16   u16_slave_scene_type;
    UINT16   u16_tstask;
    UINT32   u32_curr_fn;
    gsm_offset_in_frame_t st_latest_fn;
    UINT32   u32_resync_timer_out_fn;
    CONST gsm_tc_bcch_tb_t *stp_gsm_tc_bcch_tb = NULL_PTR;

    
    OAL_ASSERT(NULL_PTR != stp_latest_frame_pos_list,"");
    u16_i = stp_latest_frame_pos_list->u16_latest_pos_num;
    u32_curr_fn = gsm_fn_operate((u32_next_fn+(UINT32)GSM_MAX_FN)-(UINT32)1);
    stp_gsm_tc_bcch_tb = gsm_get_tc_bcch_tbl_cfg(u16_standby_id);
    if((UINT16)GSM_TABLE_VALID == stp_gsm_tc_bcch_tb->u16_tc_bcch_tb_flg)
    {
       u16_resync_cell_ind  = stp_gsm_tc_bcch_tb->u16_resync_cell_ind;
       u16_slave_scene_type = stp_gsm_tc_bcch_tb->u16_slave_scene_type;

       switch(u16_slave_scene_type)
       {
           case READ_SI_IN_RESELECTION_SCENE:
           case READ_SI_IN_PLMN_SCENE:
               u16_tstask = TSNTASK_TC_SEARCH_BCCH_RX;
               break;
           case READ_SI_IN_UTRA_CELL_FACH_SCENE:
           case READ_SI_IN_EUTRA_CGI_SCENE:
               u16_tstask = TSNTASK_IRAT_CNNT_TC_BCCH_RX;
               break;
           default:
               OAL_PRINT(u16_standby_id,(UINT16)GSM_MODE,"invalid value: u16_slave_scene_type=%d",u16_slave_scene_type);
               return OAL_FAILURE;
        }
        
        if((UINT16)0 != u16_resync_cell_ind) 
        { 
            /* GSM_SLAVE状态下，异模式在idle态/WCDMA or TDS异模式在业务态/LTE异模式在业务态才会有该分支 */
            if(stp_gsm_tc_bcch_tb->st_tc_timer_info.b_bsic_flg)
            {
                /* 判断有没有GAP */

                if(OAL_TRUE == gsm_check_slave_bcch_gap(u16_standby_id,u16_slave_scene_type,stp_gsm_tc_bcch_tb->u16_src_mode)/*有*/)
                {

                    gsm_fcp_add_frametask(u16_tstask,u32_next_fn,u16_standby_id);
                    gsm_fcp_get_tsntask_latestfn(u16_tstask,u32_curr_fn,u16_standby_id,&st_latest_fn);
                    if((UINT32)GSM_INVALID_FN != st_latest_fn.u32_fn)
                    {
                        stp_latest_frame_pos_list->st_latest_frame_pos[u16_i].u32_fn = st_latest_fn.u32_fn;
                        stp_latest_frame_pos_list->st_latest_frame_pos[u16_i].u16_tsn = st_latest_fn.u16_tsn;
                        stp_latest_frame_pos_list->st_latest_frame_pos[u16_i].s16_offset = st_latest_fn.s16_offset;
                        u16_i++;
                    } 

                }
            }
            else
            {
                u32_resync_timer_out_fn = gsm_fn_operate(stp_gsm_tc_bcch_tb->u32_resync_begin_fn + (UINT32)stp_gsm_tc_bcch_tb->u16_resync_need_fn_num);
            
                if(gsm_fn_compare(u32_resync_timer_out_fn,u32_curr_fn))
                {
                    OAL_PRINT(u16_standby_id,(UINT16)GSM_MODE,"TC resync_cell_ind:%d, timer out!u32_curr_fn:%ld,begin fn:%ld",u16_resync_cell_ind,u32_curr_fn,stp_gsm_tc_bcch_tb->u32_resync_begin_fn);
                }
            }
        }
        else
        {
            /* 判断有没有GAP */

            if(OAL_TRUE == gsm_check_slave_bcch_gap(u16_standby_id,u16_slave_scene_type,stp_gsm_tc_bcch_tb->u16_src_mode)/*有*/)
            {
                /* GSM_SLAVE状态下,一般是不会有u16_resync_cell_ind =0 分支 */
                gsm_fcp_add_frametask(u16_tstask,u32_next_fn,u16_standby_id);
                gsm_fcp_get_tsntask_latestfn(u16_tstask,u32_curr_fn,u16_standby_id,&st_latest_fn);

                if((UINT32)GSM_INVALID_FN != st_latest_fn.u32_fn)
                {
                    stp_latest_frame_pos_list->st_latest_frame_pos[u16_i].u32_fn = st_latest_fn.u32_fn;
                    stp_latest_frame_pos_list->st_latest_frame_pos[u16_i].u16_tsn = st_latest_fn.u16_tsn;
                    stp_latest_frame_pos_list->st_latest_frame_pos[u16_i].s16_offset = st_latest_fn.s16_offset;
                    u16_i++;
                } 

            }
        }
    }
    stp_latest_frame_pos_list->u16_latest_pos_num = u16_i;

    return OAL_SUCCESS;
}
/* FIX NGM-Bug00000496 END 2014-09-25 : guxiaobo */
