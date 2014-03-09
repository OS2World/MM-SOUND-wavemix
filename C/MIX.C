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

ULONG EXPENTRY _export  WAVEMIXINITIALIZE(ULONG ulFlags)   {
PEXTENDMMIOINFO   pMixShared;
PESCMIXPARAMS     pMix;
ULONG             mix,ulRc;
HMTX              hMtx;
   /* 共用メモリーの収得 */
   if(DosGetNamedSharedMem(&pMixShared,SHAREMEMNAME,PAG_WRITE))  {
      /* 共用メモリーが使用不能 */
      return ERROR_INVALID_DATA;
   }
   /* メイン処理 */
   if(DosOpenMutexSem(SHAREMTX, &hMtx))   {
      hMtx=pMixShared->hMtx;
   }
   ulRc=DosRequestMutexSem(hMtx,SEM_INDEFINITE_WAIT);
   if(ulRc) {  return   ulRc;  }

   DosGetSharedMem(pMixShared->pMixParams,PAG_WRITE);
   pMix=(PESCMIXPARAMS)&pMixShared->pMixParams[0];
   if(!pMix) {
      DosReleaseMutexSem(hMtx);
      return ERROR_INVALID_DATA;
   }
   for(mix=0;mix<pMixShared->ulMixs;mix++) {
      memset(pMix,0,sizeof(ESCMIXPARAMS));
      pMix++;
   }
   DosReleaseMutexSem(hMtx);
return NO_ERROR; }

ULONG EXPENTRY _export  WAVEMIXCREATE(LHANDLE *phMix, ULONG ulFlags)   {
PEXTENDMMIOINFO   pMixShared;
PESCMIXPARAMS     pMix;
ULONG             mix,ulRc;
HMTX              hMtx;
   if(!phMix)   return ERROR_INVALID_HANDLE;
   /* 共用メモリーの収得 */
   if(DosGetNamedSharedMem(&pMixShared,SHAREMEMNAME,PAG_WRITE))  {
      /* 共用メモリーが使用不能 */
      *phMix=0L;
      return ERROR_INVALID_DATA;
   }
   /* メイン処理 */
   if(DosOpenMutexSem(SHAREMTX, &hMtx))   {
      hMtx=pMixShared->hMtx;
   }
   ulRc=DosRequestMutexSem(hMtx,SEM_INDEFINITE_WAIT);
   if(ulRc) {  return   ulRc;  }

   DosGetSharedMem(pMixShared->pMixParams,PAG_WRITE);
   pMix=(PESCMIXPARAMS)&pMixShared->pMixParams[0];
   if(!pMix) {
      DosReleaseMutexSem(hMtx);
      return ERROR_INVALID_DATA;
   }
   *phMix=NULL;
   for(mix=0;mix<pMixShared->ulMixs;mix++) {
      /* 有効チェック */
      if(pMix->ulStructLen)   {
         /* 使用中 */
      }  else  {
         memset(pMix,0,sizeof(ESCMIXPARAMS));
         pMix->ulStructLen=sizeof(ESCMIXPARAMS);
         pMix->loop.fEnable=FALSE;
         *phMix=(LHANDLE)pMix;
         DosReleaseMutexSem(hMtx);
         return NO_ERROR;
      }
      pMix++;
   }
   DosReleaseMutexSem(hMtx);
return ERROR_FILE_NOT_FOUND; }

ULONG EXPENTRY _export  WAVEMIXCAST(LHANDLE hMix, PLLDATA pWaveData)   {
PEXTENDMMIOINFO   pMixShared;
PESCMIXPARAMS     pMix;
LLDATA            ldDest;
HMTX              hMtx;
ULONG             ulRc;

   if(!hMix)   return ERROR_INVALID_HANDLE;
   /* 共用メモリーの収得 */
   if(DosGetNamedSharedMem(&pMixShared,SHAREMEMNAME,PAG_WRITE))
      return ERROR_INVALID_DATA;             // 共用メモリーが使用不能
   /* メイン処理 */
   if(DosOpenMutexSem(SHAREMTX, &hMtx))   {
      hMtx=pMixShared->hMtx;
   }
   ulRc=DosRequestMutexSem(hMtx,SEM_INDEFINITE_WAIT);
   if(ulRc) {  return   ulRc;  }

   pMix=(PESCMIXPARAMS)hMix;
   if(pMix->fEnable) {
      /* 前回のデーターは全て解放する */
      if(pMix->pMixCVBuffer)  EscFreeMem(pMix->pMixCVBuffer);
   }
   memset( pMix, 0, sizeof(ESCMIXPARAMS));
   pMix->ulStructLen=sizeof(ESCMIXPARAMS);

   /* 変換 */
   ldDest.pStruct=&pMixShared->hdrMMAudioDest;
   if(CnvWaveToCdda(&ldDest, pWaveData, NULL))  {
      memcpy(&ldDest,pWaveData,sizeof(LLDATA));
   }  else  {
//    EscFreeMem(pWaveData->pBuffer);
   }
   /* 設定 */
   pMix->pMixCVBuffer = ldDest.pBuffer;
   pMix->ulMixCVSize = ldDest.ulSize;
   pMix->ulMixCVPos = 0L;
   pMix->fEnable=TRUE;

   DosReleaseMutexSem(hMtx);

return NO_ERROR; }

ULONG EXPENTRY _export  WAVEMIXREGISTLOOPTIME(LHANDLE hMix, ULONG ulLoopStartTime, ULONG ulLoopEndTime)   {
PEXTENDMMIOINFO   pMixShared;
PESCMIXPARAMS     pMix;
HMTX              hMtx;
ULONG             ulRc;

   if(!hMix)   return ERROR_INVALID_HANDLE;
   if(DosGetNamedSharedMem(&pMixShared,SHAREMEMNAME,PAG_WRITE))
      return ERROR_INVALID_DATA;             // 共用メモリーが使用不能
   /* メイン処理 */
   if(DosOpenMutexSem(SHAREMTX, &hMtx))   {
      hMtx=pMixShared->hMtx;
   }
   ulRc=DosRequestMutexSem(hMtx,SEM_INDEFINITE_WAIT);
   if(ulRc) {  return   ulRc;  }

   pMix=(PESCMIXPARAMS)hMix;

   pMix->loop.fEnable=FALSE;
   pMix->loop.ulStartTime=ulLoopStartTime;
   pMix->loop.ulEndTime=ulLoopEndTime;
   pMix->loop.ulStartPos=ClcWaveSizeFromTime(pMix->loop.ulStartTime, &pMixShared->hdrMMAudioDest);
   pMix->loop.ulStartPos&=0xFFFFFFFCL;
   pMix->loop.ulStartPosSrc=ClcWaveSizeFromTime(pMix->loop.ulStartTime, &pMix->hdrMMAudio);
   pMix->loop.ulStartPosSrc&=0xFFFFFFFCL;
   pMix->loop.ulEndPos=ClcWaveSizeFromTime(pMix->loop.ulEndTime, &pMixShared->hdrMMAudioDest);
   pMix->loop.ulEndPos&=0xFFFFFFFCL;
   pMix->loop.ulEndPosSrc=ClcWaveSizeFromTime(pMix->loop.ulEndTime, &pMix->hdrMMAudio);
   pMix->loop.ulEndPosSrc&=0xFFFFFFFCL;
   if(pMix->loop.ulStartPos>=pMix->loop.ulEndPos) {                                      
      memset( &pMix->loop, 0, sizeof(ESCMIXLOOPPARAM));
   }  else  {
      pMix->loop.fEnable=TRUE;
   }

   DosRequestMutexSem(hMtx,SEM_INDEFINITE_WAIT);

return NO_ERROR; }

ULONG EXPENTRY _export  WAVEMIXCASTFILE(LHANDLE hMix, PCHAR pszFileName,  ULONG ulLoopStartTime, ULONG ulLoopEndTime)   {
HMMIO             hmmio;
MMIOINFO          mmInfo;
MMFORMATINFO      mmFmtInfo;
PESCMIXPARAMS     pMix;
LONG              lBytesRead,rc;
FOURCC            fccStorageSystem;
PEXTENDMMIOINFO   pMixShared;
HMTX              hMtx;
ULONG             ulRc,len,ulDriveNum;
PCHAR             ptr,ptr2;

   if(!hMix)   return ERROR_INVALID_HANDLE;
   memset(&mmInfo,0,sizeof(MMIOINFO));
   mmInfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;

   if(DosGetNamedSharedMem(&pMixShared,SHAREMEMNAME,PAG_WRITE))
      return ERROR_INVALID_DATA;             // 共用メモリーが使用不能

   hmmio = mmioOpen(pszFileName, &mmInfo, MMIO_READ);
   if(!hmmio)  return ERROR_FILE_NOT_FOUND;

   if(DosOpenMutexSem(SHAREMTX, &hMtx))   {
      hMtx=pMixShared->hMtx;
   }
   ulRc=DosRequestMutexSem(hMtx,SEM_INDEFINITE_WAIT);
   if(ulRc) { 
      mmioClose(hmmio,0);
      return ulRc;
   }

   pMix=(PESCMIXPARAMS)hMix;
   if(pMix->fEnable) {
      /* 前回のデーターは全て解放する */
      if(!pMix->hmmio)   {
         if(pMix->pMixCVBuffer)  EscFreeMem(pMix->pMixCVBuffer);
         pMix->pMixCVBuffer=NULL;
      }
   }
   pMix->fEnable=FALSE;
   pMix->ulMasterTime=0L;
   pMix->ulMasterPos=0L;
   pMix->ulStructLen=sizeof(ESCMIXPARAMS);

   pMix->hmmio=hmmio;
   rc=mmioGetHeader(hmmio, &pMix->hdrMMAudio, sizeof(MMAUDIOHEADER), &lBytesRead, 0, 0);
   mmioClose(pMix->hmmio,0);
   if(rc)   {
      memset( pMix, 0, sizeof(ESCMIXPARAMS));
      DosReleaseMutexSem(hMtx);
      return ERROR_INVALID_DATA;
   }

   pMix->loop.fEnable=FALSE;
   pMix->loop.ulStartTime=ulLoopStartTime;
   pMix->loop.ulEndTime=ulLoopEndTime;
   pMix->loop.ulStartPos=ClcWaveSizeFromTime(pMix->loop.ulStartTime, &pMixShared->hdrMMAudioDest);
   pMix->loop.ulStartPos&=0xFFFFFFFCL;
   pMix->loop.ulStartPosSrc=ClcWaveSizeFromTime(pMix->loop.ulStartTime, &pMix->hdrMMAudio);
   pMix->loop.ulStartPosSrc&=0xFFFFFFFCL;
   pMix->loop.ulEndPos=ClcWaveSizeFromTime(pMix->loop.ulEndTime, &pMixShared->hdrMMAudioDest);
   pMix->loop.ulEndPos&=0xFFFFFFFCL;
   pMix->loop.ulEndPosSrc=ClcWaveSizeFromTime(pMix->loop.ulEndTime, &pMix->hdrMMAudio);
   pMix->loop.ulEndPosSrc&=0xFFFFFFFCL;
   if(pMix->loop.ulStartPos>=pMix->loop.ulEndPos) {                                      
      memset( &pMix->loop, 0, sizeof(ESCMIXLOOPPARAM));
   }  else  {
      pMix->loop.fEnable=TRUE;
   }

   len=strlen(pszFileName);
   ptr=pszFileName;

   if(strstr(pszFileName,":\\") )  {
      strcpy(pMix->szFileName,pszFileName);
   }  else  {
      ulDriveNum=0L;
      DosQueryCurrentDisk(&ulDriveNum,&ulRc);
      if(len>=2)  {
         if(pszFileName[1]==':') {
            if(pszFileName[0]>='a' && pszFileName[0]<='z')  {
               ulDriveNum=pszFileName[0]-'a'+1;
            }  else  {
               ulDriveNum=pszFileName[0]-'A'+1;
            }
         }
      }
      strcpy(pMix->szFileName,"");
      ulRc=sizeof(pMix->szFileName);
      pMix->szFileName[0]=ulDriveNum+'A'-1;
      pMix->szFileName[1]=':';
      pMix->szFileName[2]='\\';
      DosQueryCurrentDir(ulDriveNum,&pMix->szFileName[3],&ulRc);
      strcat(pMix->szFileName,"\\");
      if(ptr2=strstr(ptr,":"))   {
         ptr=ptr2+1;
      }
      strcat(pMix->szFileName,ptr);
   }
// puts(pMix->szFileName);
   pMix->fHmmioRead=TRUE;
   pMix->fFirstLoad=TRUE;
   pMix->fEnable=TRUE;

   DosReleaseMutexSem(hMtx);

return NO_ERROR; }

ULONG EXPENTRY _export  WAVEMIXSTOP(LHANDLE hMix)   {
PEXTENDMMIOINFO   pMixShared;
PESCMIXPARAMS     pMix;
HMTX              hMtx;
ULONG             ulRc;

   if(!hMix)   return ERROR_INVALID_HANDLE;
   if(DosGetNamedSharedMem(&pMixShared,SHAREMEMNAME,PAG_WRITE))
      return ERROR_INVALID_DATA;             // 共用メモリーが使用不能
   /* メイン処理 */
   if(DosOpenMutexSem(SHAREMTX, &hMtx))   {
      hMtx=pMixShared->hMtx;
   }
   ulRc=DosRequestMutexSem(hMtx,1000);
   if(ulRc) {  return   ulRc;  }

   pMix=(PESCMIXPARAMS)hMix;
   pMix->fEnable=FALSE;

   DosReleaseMutexSem(hMtx);
return NO_ERROR; }

ULONG EXPENTRY _export  WAVEMIXSETEFFECT(LHANDLE hMix, FOURCC fccEffect, PVOID pExInfo)   {
PEXTENDMMIOINFO   pMixShared;
PESCMIXPARAMS     pMix;
HMTX              hMtx;
ULONG             ulRc=NO_ERROR;
LHANDLE           lHdlEmsEff;

   if(!hMix)   return ERROR_INVALID_HANDLE;
   if(DosGetNamedSharedMem(&pMixShared,SHAREMEMNAME,PAG_WRITE))
      return ERROR_INVALID_DATA;             // 共用メモリーが使用不能
   /* メイン処理 */
   if(DosOpenMutexSem(SHAREMTX, &hMtx))   {
      hMtx=pMixShared->hMtx;
   }
   ulRc=DosRequestMutexSem(hMtx,SEM_INDEFINITE_WAIT);
   if(ulRc) {  return   ulRc;  }

   pMix=(PESCMIXPARAMS)hMix;

   if(!fccEffect) {
      EmsClose(pMix->lHdlEmsEff);
      pMix->lHdlEmsEff=NULL;
      pMix->fccEffect=NULL;
   }  else  {
      ulRc=EmsOpen(&lHdlEmsEff, NULL, fccEffect, MMIO_WRITE|MMIO_NOIDENTIFY|MMIO_CREATE, pExInfo);
      if(!ulRc)  {
         if(pMix->lHdlEmsEff) {
            EmsClose(pMix->lHdlEmsEff);
            pMix->lHdlEmsEff=NULL;
            pMix->fccEffect=NULL;
         }
         pMix->fccEffect=fccEffect;
         pMix->lHdlEmsEff=lHdlEmsEff;
         EmsSetHeader(pMix->lHdlEmsEff, &pMixShared->hdrMMAudioDest, sizeof(MMAUDIOHEADER));
      }
   }
   DosReleaseMutexSem(hMtx);

return ulRc; }

ULONG EXPENTRY _export  WAVEMIXDESTROY(LHANDLE hMix)   {
PEXTENDMMIOINFO   pMixShared;
PESCMIXPARAMS     pMix;
HMTX              hMtx;
ULONG             ulRc;

   if(!hMix)   return ERROR_INVALID_HANDLE;
   if(DosGetNamedSharedMem(&pMixShared,SHAREMEMNAME,PAG_WRITE))
      return ERROR_INVALID_DATA;             // 共用メモリーが使用不能
   /* メイン処理 */
   if(DosOpenMutexSem(SHAREMTX, &hMtx))   {
      hMtx=pMixShared->hMtx;
   }
   ulRc=DosRequestMutexSem(hMtx,SEM_INDEFINITE_WAIT);
   if(ulRc) {  return   ulRc;  }

   pMix=(PESCMIXPARAMS)hMix;
   /* エフェクトのクローズ */
   if(pMix->lHdlEmsEff) {
      EmsClose(pMix->lHdlEmsEff);
      pMix->lHdlEmsEff=NULL;
      pMix->fccEffect=NULL;
   }
   /* メモリーの解放 */
   memset( pMix, 0, sizeof(ESCMIXPARAMS));

   DosReleaseMutexSem(hMtx);

return NO_ERROR; }

HMMIO EXPENTRY _export  WAVEMIXQUERYHMMIO(LHANDLE hMix)   {
PESCMIXPARAMS     pMix;
   if(!hMix)   return ERROR_INVALID_HANDLE;
   pMix=(PESCMIXPARAMS)hMix;
return    pMix->hmmio; }

ULONG EXPENTRY _export  WAVEMIXQUERYULONG(LHANDLE hMix,ULONG ulFlag)   {
PESCMIXPARAMS     pMix;
ULONG rc=0L;
   if(!hMix)   return ERROR_INVALID_HANDLE;
   pMix=(PESCMIXPARAMS)hMix;
   switch(ulFlag & 0x0000FFFFL) {
   case EWXL_STRUCTLEN:
      rc=pMix->ulStructLen;
      break;
   case EWXL_HMMIO:
      rc=(ULONG)pMix->hmmio;
      break;
   case EWXL_MASTER_TIME:
      rc=pMix->ulMasterTime;
      break;
   case EWXL_MASTER_POS:
      rc=pMix->ulMasterPos;
      break;
   case EWXL_LOOP_ENABLE:
      rc=pMix->loop.fEnable;
      break;
   case EWXL_LOOP_START_TIME:
      rc=pMix->loop.ulStartTime;
      break;
   case EWXL_LOOP_START_POS:
      rc=pMix->loop.ulStartPos;
      break;
   case EWXL_LOOP_END_TIME:
      rc=pMix->loop.ulEndTime;
      break;
   case EWXL_LOOP_END_POS:
      rc=pMix->loop.ulEndPos;
      break;
   default:
      rc=-1;
      break;
   }
return    rc; }

