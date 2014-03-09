/* ******************************************************************** */
/* WATCOM C/C++ 10.5 DLL ENTRY POINT                                    */
/* ******************************************************************** */
#define INCL_ESC_NO_NETWORK
#define INCL_ESC_NO_WARP4
#include <LUCIER.H>
#include "MMIOPROC.H"

int __dll_initialize();
int __dll_terminate(void);
void APIENTRY __dll_cleanup(ULONG ulRes);

/* ******************************************************************** */
/* DLL初期化時に自動的に呼ばれる関数                                    */
/* エラー時は 0 を戻すと、DLLは起動されない。                           */
/* ******************************************************************** */
int __dll_initialize(void)  {
PEXTENDMMIOINFO   pExMMInfo=NULL;
PMMAUDIOHEADER    pmmAudioHdrDest;
ULONG             ulRc;
   /* ***************************************************************** */
   /* 解放時登録関数の予約                                              */
   /* ***************************************************************** */
   #ifndef STACK_LINK
   DosExitList(0x0000FF00|EXLST_ADD,__dll_cleanup);
   #endif

   if(DosAllocSharedMem(&pExMMInfo,SHAREMEMNAME,sizeof(EXTENDMMIOINFO),PAG_COMMIT|PAG_WRITE))  {
      DosGetNamedSharedMem(&pExMMInfo,SHAREMEMNAME,PAG_WRITE);
   }
   if(!pExMMInfo)    {
      return 0;
   }
   memset(pExMMInfo,0,sizeof(EXTENDMMIOINFO));

   ulRc=DosCreateMutexSem(SHAREMTX, &pExMMInfo->hMtx, DC_SEM_SHARED, FALSE);
// ulRc=DosOpenMutexSem(SHAREMTX, &pExMMInfo->hMtx);
   /* ************************************************************** */
   /* CHANNELS BTS SPS SETUP                                         */
   /* ************************************************************** */
   pmmAudioHdrDest=&pExMMInfo->hdrMMAudioDest;
   pmmAudioHdrDest->mmXWAVHeader.WAVEHeader.usChannels=2;
   pmmAudioHdrDest->mmXWAVHeader.WAVEHeader.ulSamplesPerSec=44100L;
   pmmAudioHdrDest->mmXWAVHeader.WAVEHeader.usBitsPerSample=16;
   pExMMInfo->ulMixs=32;
   if(DosAllocSharedMem(&pExMMInfo->pMixParams, NULL, pExMMInfo->ulMixs * sizeof(ESCMIXPARAMS),PAG_COMMIT|PAG_WRITE|OBJ_GETTABLE))  {
      DosGetSharedMem(pExMMInfo->pMixParams,PAG_WRITE);
   }
   memset( pExMMInfo->pMixParams, 0, pExMMInfo->ulMixs * sizeof(ESCMIXPARAMS) );
   ClcWaveInfoFromBasicInfo(pmmAudioHdrDest, 1000);

return 1; }

/* ******************************************************************** */
/* DLL解放時に自動的に呼ばれる関数                                      */
/* ******************************************************************** */
int __dll_terminate(void)  {
PEXTENDMMIOINFO   pExMMInfo=NULL;
PESCMIXPARAMS     pMix;
ULONG             mix;

   if(!DosGetNamedSharedMem(&pExMMInfo,SHAREMEMNAME,PAG_WRITE))  {
      if(pExMMInfo)  {
         DosGetSharedMem(pExMMInfo->pMixParams,PAG_WRITE);
         pMix=&pExMMInfo->pMixParams[0];
         for(mix=0;mix<pExMMInfo->ulMixs;mix++)   {   
            if(pMix->ulStructLen && pMix->fEnable)   {
               if(pMix->pMixCVBuffer)  DosFreeMem(pMix->pMixCVBuffer);
               pMix++;
            }
         }
         if(pExMMInfo->pMixParams)  DosFreeMem(pExMMInfo->pMixParams);
         DosCloseMutexSem(pExMMInfo->hMtx);
         DosFreeMem(pExMMInfo);
      }
   }
   #ifndef STACK_LINK
   #endif
return 1; }

/* ******************************************************************** */
/* __dll_cleanup                                                        */
/* ******************************************************************** */
void APIENTRY __dll_cleanup(ULONG ulRes)  {
   DosExitList(0x0000FF00|EXLST_EXIT,__dll_cleanup);
return;  }

