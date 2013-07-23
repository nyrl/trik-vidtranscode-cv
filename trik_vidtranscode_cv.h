#ifndef TRIK_VIDTRANSCODE_CV_H_
#define TRIK_VIDTRANSCODE_CV_H_

#include <xdc/std.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividtranscode.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/*
 *  ======== TRIK_VIDTRANSCODE_CV ========
 *  Our implementation of the IVIDTRANSCODE interface
 */
extern IVIDTRANSCODE_Fxns TRIK_VIDTRANSCODE_CV_FXNS;
extern IALG_Fxns TRIK_VIDTRANSCODE_CV_IALG;


typedef struct TRIK_VIDTRANSCODE_CV_DynamicParams {
    IVIDTRANSCODE_DynamicParams	base;

    XDAS_Int32			inputHeight;
    XDAS_Int32			inputWidth;
    XDAS_Int32			inputLineLength;

    XDAS_Int32			outputLineLength[2];
} TRIK_VIDTRANSCODE_CV_DynamicParams;


typedef struct TRIK_VIDTRANSCODE_CV_Params {
    IVIDTRANSCODE_Params	base;
} TRIK_VIDTRANSCODE_CV_Params;


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // !TRIK_VIDTRANSCODE_CV_H_
