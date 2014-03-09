/* ******************************************************************** */
/* WAVE MIX MMIO PROC - MAIN SOURCE CODE                                */
/*                                                        Version 1.0.0 */
/*                                Copyrights(c) 1999-2000, Yuuriru Mint */
/*                         TEAM MMOS/2 TOKYO Multimedia Communications! */
/* ******************************************************************** */
/* Watcom C/C++ 10.0 英語版 + Warp 4 Toolkit                            */
/* Library: MMPM2.LIB LUCIER.LIB (2.3) LLCDIOCT.LIB                     */
/* ******************************************************************** */
#define INCL_ESC_NO_NETWORK
#define INCL_ESC_NO_WARP4
#include <LUCIER.H>

#include "MMIOPROC.H"

/* ******************************************************************** */
/* メイン関数                                                           */
/* ******************************************************************** */
LONG EXPENTRY _export MMIOENTRY(PVOID pVoid,USHORT usMsg,LONG mp1,LONG mp2)  {
LONG              lRC=MMIO_ERROR;
MMIOINFO          MMInfo,*pMMInfo=NULL,mmInfo;
PEXTENDMMIOINFO   pExMMInfo=NULL;
PMMFORMATINFO     pMMFmtInfo;
FOURCC            fourcc;
HMMIO             hmmio;
PMMAUDIOHEADER    pmmAudioHdrDest,pmmAudioHdrSrc;
PCHAR             pText,pszErrorStatus;
CHAR              szBuffer[257];
HAB               hab;
ULONG             mix;
PESCMIXPARAMS     pMix;
PSHORT   ptrSRead,ptrSWrite;
ULONG    ulSize,i;
ULONG             rc,ulPos,ulX;
LLDATA            ldDest,ldSrc;
PCHAR    ptrWriteResv;
ULONG    ulSWriteRemain;
BOOL     fLoop;
HMTX     hMtx;
   pMMInfo=(PMMIOINFO)pVoid;

   switch(usMsg)  {
   /* ***************************************************************** */
   /* オープン                                                          */
   /* ***************************************************************** */
   case MMIOM_OPEN:
      if(!pMMInfo)   break;
      pszErrorStatus="Unknown error.";
      /* ************************************************************** */
      /* 判別                                                           */
      /* ************************************************************** */
      if(!pMMInfo->fccChildIOProc)  {
         if (pMMInfo->ulFlags & MMIO_CREATE) {
            if(mmioDetermineSSIOProc((PCHAR)mp1,pMMInfo,&fourcc,NULL))
               fourcc = FOURCC_DOS;
         }  else  {
            if(mmioIdentifyStorageSystem((PCHAR)mp1,pMMInfo,&fourcc))
               return MMIO_ERROR;
         }
         if (!fourcc)  return MMIO_ERROR;
         else  pMMInfo->fccChildIOProc = fourcc;
      }
      /* ************************************************************** */
      /* 宛先ファイルのオープン                                         */
      /* ************************************************************** */
      memset(&MMInfo,0,sizeof(MMIOINFO));
      memmove(&MMInfo,pMMInfo,sizeof(MMInfo));
      MMInfo.ulFlags|=MMIO_NOIDENTIFY;
      MMInfo.pIOProc = NULL;
      MMInfo.fccIOProc = pMMInfo->fccChildIOProc;

      if (pMMInfo->ulFlags & MMIO_DELETE) {
         hmmio=mmioOpen((PCHAR)mp1,&MMInfo,MMInfo.ulFlags);
         if (!hmmio)  {                         
            pMMInfo->ulErrorRet = MMIOERR_DELETE_FAILED;
            return MMIO_ERROR;
         }  else
            return MMIO_SUCCESS;
      }

      /* ************************************************************** */
      /* 固有内部構造体の初期化                                         */
      /* ************************************************************** */
      if(DosGetNamedSharedMem(&pExMMInfo,SHAREMEMNAME,PAG_WRITE))  {
         /* 共用メモリーが使用不能 */
         return MMIO_ERROR;
      }
      if(!pExMMInfo)  break;
      pMMInfo->pExtraInfoStruct=(PVOID)pExMMInfo;
      /* ************************************************************** */
      /* WAVEMIX   ファイル設定を読み取る                               */
      /* ************************************************************** */
      hab=WinQueryAnchorBlock(HWND_DESKTOP);
//    hYsb=EscInitializeYSB(hab,(PCHAR)mp1,NULL,NULL);
//    if(!hYsb)   goto ErrorOpen;
      /* ************************************************************** */
      /* CLOSE YSB ACCESS                                               */
      /* ************************************************************** */
//    EscTerminateYSB(hYsb);
      /* ************************************************************** */
      /* リターン                                                       */
      /* ************************************************************** */
//    PrfWriteProfileString(HINI_USER,APPNAME,"ERRORSTRING","");
      lRC=MMIO_SUCCESS;
      ErrorOpen:
      break;                  
   /* ***************************************************************** */
   /* クローズ                                                          */
   /* ***************************************************************** */
   case MMIOM_CLOSE:
      if(!pMMInfo)   break;
      pExMMInfo=pMMInfo->pExtraInfoStruct;
      if(!pExMMInfo)  break;
      pMMInfo->pExtraInfoStruct=NULL;
      lRC=MMIO_SUCCESS;
      break;
   /* ***************************************************************** */
   /* ファイルの自動認識を行うチェックルーチン (ライトのみなので非サポ) */
   /* ***************************************************************** */
   case MMIOM_IDENTIFYFILE:
      if(!mp1 && !mp2)  break;
      if(!mp1) {
         mmioRead((HMMIO)mp2,szBuffer,10);
         if(strstr(szBuffer,"[TYPE]")) {
            lRC=MMIO_SUCCESS;
         }
      }  else  {
         pText=(PCHAR)mp1;
         ulSize=strlen(pText);
         if(ulSize>5)  {
            if(strstr(pText,".WMX"))  {
               lRC=MMIO_SUCCESS;
            }
         }
      }
      break;
   /* ***************************************************************** */
   /* このサービスのフォーマット情報を返す                              */
   /* ***************************************************************** */
   case MMIOM_GETFORMATINFO:
      pMMFmtInfo=(PMMFORMATINFO)mp1;
      if(!pMMFmtInfo) break;
      memset(pMMFmtInfo,0,sizeof (MMFORMATINFO));
      pMMFmtInfo->ulStructLen  = sizeof(MMFORMATINFO);
      pMMFmtInfo->fccIOProc    = FOURCC_MMIOPROC;
      pMMFmtInfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
      pMMFmtInfo->ulMediaType  = MEDIATYPE;
      pMMFmtInfo->ulFlags      = SUPPORTFLAG;
      strcpy((PSZ)pMMFmtInfo->szDefaultFormatExt,EXT);
      pMMFmtInfo->ulCodePage=MMIO_DEFAULT_CODE_PAGE;
      pMMFmtInfo->ulLanguage=MMIO_LC_US_ENGLISH;
      pMMFmtInfo->lNameLength=sizeof(NAME);
      lRC=MMIO_SUCCESS;
      break;
   /* ***************************************************************** */
   /* このサービスのフォーマット名を返す                                */
   /* ***************************************************************** */
   case MMIOM_GETFORMATNAME:
      lRC=0L;
      if(!mp1) break;
      if(sizeof(NAME)>=mp2)   strcpy((PCHAR)mp1,EXT);
      else  strcpy((PCHAR)mp1,NAME);
      lRC=strlen((PCHAR)mp1);
      break;
   /* ***************************************************************** */
   /* ヘッダを読みとる                                                  */
   /* ***************************************************************** */
   case MMIOM_GETHEADER:
      lRC=0L;
      if(!pMMInfo)   break;
      pExMMInfo=pMMInfo->pExtraInfoStruct;
      if(!pExMMInfo)  break;

      if(mp2<sizeof(MMAUDIOHEADER))  {
         pMMInfo->ulErrorRet=MMIOERR_INVALID_BUFFER_LENGTH;
         break;
      }
      if(!mp1)  {
         pMMInfo->ulErrorRet=MMIOERR_INVALID_STRUCTURE;
         break;
      }

      pmmAudioHdrDest=&pExMMInfo->hdrMMAudioDest;

      lRC=sizeof(MMAUDIOHEADER);
      memcpy((PVOID)mp1, pmmAudioHdrDest, lRC);
      break;
   /* ***************************************************************** */
   /* ヘッダを書き込む                                                  */
   /* ***************************************************************** */
   case MMIOM_SETHEADER:
      lRC=0L;
      if(!pMMInfo)   break;
      pMMInfo->ulErrorRet=MMIOERR_UNSUPPORTED_MESSAGE;
      lRC=MMIOERR_UNSUPPORTED_MESSAGE;
      break;
   /* ***************************************************************** */
   /* ヘッダサイズを返す                                                */
   /* ***************************************************************** */
   case MMIOM_QUERYHEADERLENGTH:
      lRC=sizeof(MMAUDIOHEADER);
      break;
   /* ***************************************************************** */
   /* 書き込み(アプリ側からみて)                                        */
   /* ***************************************************************** */
   case MMIOM_WRITE:
      if(!pMMInfo)   break;
      pMMInfo->ulErrorRet=MMIOERR_UNSUPPORTED_MESSAGE;
      lRC=MMIOERR_UNSUPPORTED_MESSAGE;
      break;
   /* ***************************************************************** */
   /* 読みとり(アプリ側からみて)                                        */
   /* ***************************************************************** */
   case MMIOM_READ:
      lRC=0L;
      if(!pMMInfo)   break;
      pExMMInfo=pMMInfo->pExtraInfoStruct;
      if(!pExMMInfo)  break;
      if(!mp2) return 0L;

      DosGetSharedMem(pExMMInfo,PAG_WRITE);

// DosBeep(800,50);
      DosOpenMutexSem(SHAREMTX, &hMtx);
      DosRequestMutexSem(hMtx,SEM_INDEFINITE_WAIT);
// DosBeep(400,50);

      pmmAudioHdrSrc=&pExMMInfo->hdrMMAudioSrc;
      pmmAudioHdrDest=&pExMMInfo->hdrMMAudioDest;

      /* 書き込み先の初期化 */
      memset( (PVOID)mp1, 0, mp2);

      /* 合成処理の開始 */
      DosGetSharedMem(pExMMInfo->pMixParams,PAG_WRITE);
      pMix=&pExMMInfo->pMixParams[0];
      if(!pMix) {
         DosReleaseMutexSem(pExMMInfo->hMtx);
         return 0L;
      }
      for(mix=0;mix<pExMMInfo->ulMixs;mix++) {
         /* 有効チェック */
         ulSWriteRemain=mp2;
         ptrWriteResv=(PCHAR)mp1;

         if(pMix->ulStructLen && (pMix->pMixCVBuffer||pMix->hmmio) && pMix->fEnable)   {
            /* ウエーブMMIOハンドルが指定されている場合は、WAVEMIXがデーターをロードする */
            if(pMix->hmmio && pMix->fHmmioRead)   {
               MMIOREAD:
               pMix->fHmmioRead=FALSE;
               /* ＭＭＩＯファイルロードをおこなう */
               ldDest.pStruct=&pExMMInfo->hdrMMAudioDest;
               ldSrc.pBuffer=NULL;
               rc=DosAllocSharedMem(&ldSrc.pBuffer,NULL,mp2+1000,PAG_COMMIT|PAG_WRITE|OBJ_GETTABLE);
               if(!ldSrc.pBuffer || rc)   {
                  ldSrc.pBuffer=NULL;
                  goto ERRORREAD;
               }
               ldSrc.pStruct=&pMix->hdrMMAudio;
//             DosGetSharedMem((PVOID)pMix->hmmio,PAG_WRITE);
               if(pMix->fFirstLoad) {
                  if(pMix->hmmioOld)   {
// DosBeep(200,100);
                     mmioClose(pMix->hmmioOld, 0);
                     pMix->hmmioOld=NULL;
                  }
                  memset( &mmInfo,0, sizeof(MMIOINFO));
                  mmInfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
// DosBeep(800,100);
                  pMix->hmmio = mmioOpen(pMix->szFileName, &mmInfo, MMIO_READ);
                  pMix->hmmioOld=pMix->hmmio;
                  pMix->fFirstLoad=FALSE;
                  rc=0;
               }
               if(pMix->hmmio)   {
                  rc=mmioRead( pMix->hmmio, ldSrc.pBuffer, mp2);
                  if(rc==0 && pMix->loop.fEnable)   {
                     pMix->ulMasterPos=pMix->loop.ulEndPos;
                     ulPos=mmioSeek( pMix->hmmio, SEEK_SET, pMix->ulMasterPos);
                     pMix->ulMasterPos =  (double)ulPos * (double)(44100L*2L*16L) / (
                        (double)pMix->hdrMMAudio.mmXWAVHeader.WAVEHeader.usChannels *
                        (double)pMix->hdrMMAudio.mmXWAVHeader.WAVEHeader.ulSamplesPerSec *
                        (double)pMix->hdrMMAudio.mmXWAVHeader.WAVEHeader.usBitsPerSample);
                     rc=mmioRead( pMix->hmmio, ldSrc.pBuffer, mp2);
                  }
               }
               ldSrc.ulSize=rc;
               if(rc<=0 || rc>mp2)  {
                  ERRORREAD:
                  /* これ以上ファイルはない */
                  if(ldSrc.pBuffer)  EscFreeMem(ldSrc.pBuffer);
                  if(pMix->pMixCVBuffer)  EscFreeMem(pMix->pMixCVBuffer);
                  if(pMix->hmmio)      {
// DosBeep(100,100);
                     mmioClose(pMix->hmmio, 0);
                     pMix->hmmio=NULL;
                  }
                  pMix->hmmioOld=NULL;
                  memset(pMix,0,sizeof(ESCMIXPARAMS));
                  pMix->ulStructLen=sizeof(ESCMIXPARAMS);
                  pMix->fEnable=FALSE;
                  goto READNEXT;
               }
               /* CDDAレベルに変換する */
               if(CnvWaveToCdda(&ldDest, &ldSrc, NULL))  {
                  EscFreeMem(ldSrc.pBuffer);
                  pMix->fEnable=FALSE;
                  goto READNEXT;
               }
               EscFreeMem(ldSrc.pBuffer);

               /* ウエーブエフェクトは変換後におこなう(負荷が高くなるが……) */
               if(pMix->lHdlEmsEff) {
               PCHAR ptr;
               ULONG ulReadSize;
                  ulReadSize=EmsWrite(pMix->lHdlEmsEff, ldDest.pBuffer, ldDest.ulSize);
                  EmsQueryMemoryFileBuffer(pMix->lHdlEmsEff, &ptr);
                  if(ptr)  {
                     if(ulReadSize >= ldDest.ulSize)  ulReadSize=ldDest.ulSize;
                     memcpy(ldDest.pBuffer, ptr, ulReadSize);
                  }
                  EmsUpdateMemoryFileBuffer(pMix->lHdlEmsEff);
               }

               pMix->pMixCVBuffer=ldDest.pBuffer;
               pMix->ulMixCVSize=ldDest.ulSize;
               pMix->ulMixCVPos=0L;
               if(!pMix->pMixCVBuffer) {
                  pMix->fEnable=FALSE;
                  goto READNEXT;
               }
               pMix->fEnable=TRUE;
            }  else  {
               /* バッファーからコピーする */
               if(!pMix->hmmio)   {
                  DosGetSharedMem(pMix->pMixCVBuffer,PAG_WRITE);
               }
            }
            if(!pMix->ulMixCVSize || pMix->ulMixCVPos>=pMix->ulMixCVSize)   {
               pMix->fEnable=FALSE;
               goto READNEXT;
            }
            /* 合成 */
            if(!pMix->pMixCVBuffer) {
               pMix->fEnable=FALSE;
               goto READNEXT;
            }
            ptrSRead=(PSHORT) &pMix->pMixCVBuffer[pMix->ulMixCVPos];
            ulSize=pMix->ulMixCVSize-pMix->ulMixCVPos;
            ptrSWrite=(PSHORT)ptrWriteResv;             
            if(ulSize>ulSWriteRemain) ulSize=ulSWriteRemain;

            fLoop=FALSE;

            if(pMix->loop.fEnable)   {
               if(pMix->ulMasterPos+ulSize >= pMix->loop.ulEndPos) {
                  ulX = (pMix->loop.ulEndPos - pMix->ulMasterPos)&0xFFFFFFFCL;
                  if(ulX>ulSize) {
                     ulSize=0L;
                  }  else  {
                     ulSize = (pMix->loop.ulEndPos - pMix->ulMasterPos)&0xFFFFFFFCL;
                  }
                  fLoop=TRUE;
               }
            }

            if(ulSize)  {
               pMix->ulMasterPos+=ulSize;
            }
            pMix->ulMasterTime=ClcWaveTimeFromSize(pMix->ulMasterPos, &pExMMInfo->hdrMMAudioDest);

            pMix->ulMixCVPos+=ulSize;
            ulSWriteRemain-=ulSize;
            ptrWriteResv+=ulSize;
            /* 合成ループ */
            for(i=0,ulSize/=2;i<ulSize;i++)   {
               *ptrSWrite=EweMerge16( *ptrSRead , *ptrSWrite);
               ptrSRead++;
               ptrSWrite++;
            }
            /* 解放 */
            if(pMix->ulMixCVPos>=pMix->ulMixCVSize || fLoop)   {
               if(pMix->hmmio)   {
                  if(pMix->pMixCVBuffer) EscFreeMem(pMix->pMixCVBuffer);
                  pMix->pMixCVBuffer=NULL;
                  pMix->ulMixCVPos=NULL;
                  pMix->ulMixCVSize=NULL;
                  pMix->fHmmioRead=TRUE;
                  pMix->fEnable=FALSE;
                  if(fLoop)   {
                     ulPos = mmioSeek( pMix->hmmio, pMix->loop.ulStartPosSrc, SEEK_SET);
                     if(ulPos==MMIO_ERROR || ulPos==MMIOERR_SEEK_FAILED) {
                        /* シークエラー */
                        pMix->fEnable=FALSE;
                        goto READNEXT;
                     }
                     pMix->ulMasterPos =  (double)ulPos * (double)(44100L*2L*16L) / (
                        (double)pMix->hdrMMAudio.mmXWAVHeader.WAVEHeader.usChannels *
                        (double)pMix->hdrMMAudio.mmXWAVHeader.WAVEHeader.ulSamplesPerSec *
                        (double)pMix->hdrMMAudio.mmXWAVHeader.WAVEHeader.usBitsPerSample);
                     pMix->ulMasterTime=ClcWaveTimeFromSize(pMix->ulMasterPos, &pExMMInfo->hdrMMAudioDest);
                  }
                  fLoop=FALSE;
                  goto MMIOREAD;
               }  else  {
                  EscFreeMem(pMix->pMixCVBuffer);
                  memset(pMix,0,sizeof(ESCMIXPARAMS));
                  pMix->ulStructLen=sizeof(ESCMIXPARAMS);
                  pMix->fEnable=FALSE;
               }
            }
         }
         /* 次のミックスポインターへ移動する */
         READNEXT:
         pMix++;
      }
      DosReleaseMutexSem(hMtx);
      lRC=mp2;
      break;
   /* ***************************************************************** */
   /* その他                                                            */
   /* ***************************************************************** */
   default: 
      if (!pMMInfo)  break;
      pExMMInfo=pMMInfo->pExtraInfoStruct;
      if(!pExMMInfo)  {
         pMMInfo->ulErrorRet=MMIOERR_UNSUPPORTED_MESSAGE;
         lRC=MMIOERR_UNSUPPORTED_MESSAGE;
         break;
      }
      if (pExMMInfo->hmmioSS)  {
         lRC=mmioSendMessage(pExMMInfo->hmmioSS,usMsg,mp1,mp2);
         if (!lRC)   pMMInfo->ulErrorRet=mmioGetLastError(pExMMInfo->hmmioSS);
      } else {
         pMMInfo->ulErrorRet=MMIOERR_UNSUPPORTED_MESSAGE;
         lRC=MMIOERR_UNSUPPORTED_MESSAGE;
      }
      break;
   }  /* end of switch */
return lRC; }

