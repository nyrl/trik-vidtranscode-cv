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
    void*					m_persistentData; /* Handle to C++ class */
} TrikCvHandle;

typedef TRIK_VIDTRANSCODE_CV_VideoFormat TrikCvImageFormat;

typedef struct TrikCvImageDesc {
    XDAS_Int32        m_width;
    XDAS_Int32        m_height;
    XDAS_Int32        m_lineLength;
    XDAS_Int32        m_format;
} TrikCvImageDesc;

typedef struct TrikCvImageBuffer {
    XDAS_Int8*        m_ptr;
    XDAS_Int32        m_size;
} TrikCvImageBuffer;

typedef TRIK_VIDTRANSCODE_CV_InArgsAlg  TrikCvAlgInArgs;

typedef TRIK_VIDTRANSCODE_CV_OutArgsAlg TrikCvAlgOutArgs;


XDAS_Int32 trikCvHandleInit(TrikCvHandle* _handle);
XDAS_Int32 trikCvHandleDestroy(TrikCvHandle* _handle);

XDAS_Int32 trikCvHandleSetupParams(TrikCvHandle* _handle, const TRIK_VIDTRANSCODE_CV_Params* _params);
XDAS_Int32 trikCvHandleSetupDynamicParams(TrikCvHandle* _handle, const TRIK_VIDTRANSCODE_CV_DynamicParams* _dynamicParams);

XDAS_Int32 trikCvProceedImage(TrikCvHandle* _handle,
                              const TrikCvImageBuffer* _inImage, TrikCvImageBuffer* _outImage,
                              const TrikCvAlgInArgs* _inArgs, TrikCvAlgOutArgs* _outArgs);


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // !TRIK_VIDTRANSCODE_CV_INTERNAL_VIDTRANSCODE_CV_H_
