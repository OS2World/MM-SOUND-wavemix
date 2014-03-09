/* ******************************************************************** */
/* MMIO SERVICE - MULTIMEDIA SUB SYSTEM DEVELOPERS INCLUDE HEADER       */
/* TEAM MMOS/2 TOKYO Multimedia Communications!                         */
/* ******************************************************************** */

#define  FOURCC_MMIOPROC   mmioFOURCC('W','M','I','X')
#define  MEDIATYPE         MMIO_MEDIATYPE_AUDIO
#define  TEXT_MMIOPROC     "Wave MIX MMIO Ver 1.00"
#define  NAME              "Wave MIX Service"
#define  EXT               "WMX"
#define  APPNAME           "WAVEMIX"

/* ******************************************************************** */
/* DEFINED                                                              */
/* ******************************************************************** */
#define  SUPPORTFLAG       MMIO_CANREADTRANSLATED | MMIO_CANREADWRITETRANSLATED \
                         | MMIO_CANREADUNTRANSLATED | MMIO_CANREADWRITEUNTRANSLATED

#define  MAX_DELAY         44100*2*50/1000
typedef struct {
   BOOL           fEnable;
   ULONG          ulStartTime;
   ULONG          ulEndTime;
   /* �f�[�^�[�ϊ���(CDDA) */
   ULONG          ulStartPos;
   ULONG          ulEndPos;
   /* �f�[�^�[�ϊ��O */
   ULONG          ulStartPosSrc;
   ULONG          ulEndPosSrc;
}  ESCMIXLOOPPARAM, *PESCMIXLOOPPARAM, **PPESCMIXLOOPPARAM;

typedef struct {
   ULONG          ulStructLen;   /* �\���̂̃T�C�Y */
   BOOL           fEnable;       /* �L�� */
   BOOL           fHandleAutoFree;     /* �����n���h���̎������ */
   BOOL           fBufferAutoFree;     /* �o�b�t�@�[�̎������ */
   BOOL           fHmmioRead;
   BOOL           fFirstLoad;

   HMMIO          hmmio;         /* MMIO���烍�[�h������ */
   HMMIO          hmmioOld;      /* �Â�HMMIO���N���[�Y���邽�߂ɕۑ� */
   MMAUDIOHEADER  hdrMMAudio;
   CHAR           szFileName[CCHMAXPATH];
   /* �ϊ���̃E�G�[�u�o�b�t�@�[�i�g�p�����j */
   PCHAR          pMixCVBuffer;
   ULONG          ulMixCVSize;
   ULONG          ulMixCVPos;

   ULONG             ulMasterTime;     /* �}�X�^�[���� */
   ULONG             ulMasterPos;      /* �}�X�^�[�ʒu */
   ESCMIXLOOPPARAM   loop;             /* ���[�v���t */
   /* �G�t�F�N�g */
   LHANDLE           lHdlEmsEff;
   FOURCC            fccEffect;
}  ESCMIXPARAMS, *PESCMIXPARAMS, **PPESCMIXPARAMS;

typedef struct _EXTENDMMIOINFO {
   HMMIO          hmmioSS;
   MMAUDIOHEADER  hdrMMAudioSrc;
   MMAUDIOHEADER  hdrMMAudioDest;
   ULONG          ulDestSize;
   /* EMS MMIO SERVICE */
   LHANDLE        lHdlEmsEff;
   BOOL           fDialogShow;         /* DIALOG WHEN ERROR (1=SHOW) */
   /* MIX */
   ULONG          ulMixs;
   PESCMIXPARAMS  pMixParams;
   PCHAR          pBuffer2;
   HMTX           hMtx;
}  EXTENDMMIOINFO,   *PEXTENDMMIOINFO;


#define  SHAREMEMNAME         "\\SHAREMEM\\WAVEMIX\\WAVEMIX.DAT"
#define  SHAREMTX             "\\SEM32\\MUTEXSEM\\WAVEMIX"

ULONG EXPENTRY _export  WAVEMIXINITIALIZE(ULONG ulFlags);
ULONG EXPENTRY _export  WAVEMIXCREATE(LHANDLE *hMix, ULONG ulFlags);
ULONG EXPENTRY _export  WAVEMIXCAST(LHANDLE hMix, PLLDATA pWaveData);
ULONG EXPENTRY _export  WAVEMIXCASTFILE(LHANDLE hMix, PCHAR pszFileName,  ULONG ulLoopStartTime, ULONG ulLoopEndTime);
ULONG EXPENTRY _export  WAVEMIXREGISTLOOPTIME(LHANDLE hMix, ULONG ulLoopStartTime, ULONG ulLoopEndTime);
ULONG EXPENTRY _export  WAVEMIXDESTROY(LHANDLE hMix);
HMMIO EXPENTRY _export  WAVEMIXQUERYHMMIO(LHANDLE hMix);
ULONG EXPENTRY _export  WAVEMIXSETEFFECT(LHANDLE hMix, FOURCC fccEffect, PVOID pExInfo);
ULONG EXPENTRY _export  WAVEMIXQUERYULONG(LHANDLE hMix,ULONG ulFlag);
