#ifndef TRIK_VIDTRANSCODE_CV_INTERNAL_VIDTRANSCODE_CV_H_
#define TRIK_VIDTRANSCODE_CV_INTERNAL_VIDTRANSCODE_CV_H_

#include <xdc/std.h>
#include <ti/xdais/xdas.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/ividtranscode.h>

#include "trik_vidtranscode_cv.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


typedef struct TrikCvHandle {
    IALG_Obj					m_alg;	/* MUST be first field of all XDAIS algs */

    TRIK_VIDTRANSCODE_CV_Params			m_params;
    TRIK_VIDTRANSCODE_CV_DynamicParams		m_dynamicParams;
} TrikCvHandle;


XDAS_Int32 trikCvHandleSetupParams(TrikCvHandle* _handle, const TRIK_VIDTRANSCODE_CV_Params* _params, const TRIK_VIDTRANSCODE_CV_DynamicParams* _dynamicParams);


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // !TRIK_VIDTRANSCODE_CV_INTERNAL_VIDTRANSCODE_CV_H_
