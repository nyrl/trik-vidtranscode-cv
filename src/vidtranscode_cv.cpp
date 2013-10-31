#include "internal/vidtranscode_cv.h"

#include <cassert>

#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>




XDAS_Int32 trikCvHandleSetupParams(TrikCvHandle* _handle,
                                   const TRIK_VIDTRANSCODE_CV_Params* _params)
{
  static const TRIK_VIDTRANSCODE_CV_Params s_default = {
    {							/* m_base, IVIDTRANSCODE_Params */
      sizeof(TRIK_VIDTRANSCODE_CV_Params),		/* size = sizeof this struct */
      1,						/* numOutputStreams = only one output stream by default */
      TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_YUV422,		/* formatInput = YUV422 */
      {
        TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_RGB565X,	/* formatOutput[0] = RGB565X */
        TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_UNKNOWN,	/* formatOutput[1] - disabled */
      },
      480,						/* maxHeightInput = up to 640wx480h */
      640,						/* maxWidthInput = see maxHeightInput */
      60000,						/* maxFrameRateInput = up to 60fps */
      -1,						/* maxBitRateInput = undefined */
      {
        480,						/* maxHeightOutput[0] = up to 640wx480h */
        -1,						/* maxHeightOutput[1] - disabled */
      },
      {
        640,						/* maxWidthOutput[0] = see maxHeightOutput */
        -1,						/* maxWidthOutput[1] - disabled */
      },
      {
        -1,						/* maxFrameRateOutput[0] = undefined */
        -1,						/* maxFrameRateOutput[1] = undefined */
      },
      {
        -1,						/* maxBitRateOutput[0] = undefined */
        -1,						/* maxBitRateOutput[1] = undefined */
      },
      XDM_BYTE						/* dataEndianness = byte only */
    }
  };

  assert(_handle);

  if (_params == NULL)
    _params = &s_default;

  _handle->m_params = *_params;

#warning Actually check params and return IALG_EOK / IALG_EFAIL

  return IALG_EOK;
}




XDAS_Int32 trikCvHandleSetupDynamicParams(TrikCvHandle* _handle,
                                          const TRIK_VIDTRANSCODE_CV_DynamicParams* _dynamicParams)
{
  static const TRIK_VIDTRANSCODE_CV_DynamicParams s_default = {
    {								/* m_base, IVIDTRANSCODE_DynamicParams */
      sizeof(TRIK_VIDTRANSCODE_CV_DynamicParams),		/* size = size of structure */
      0,							/* readHeaderOnlyFlag = false */
      {
        XDAS_FALSE,						/* keepInputResolutionFlag[0] = false, override resolution */
        XDAS_TRUE,						/* keepInputResolutionFlag[1] - disabled */
      },
      {
        240,							/* outputHeight[0] = default output 320wx240h */
        0,							/* outputHeight[1] - don't care */
      },
      {
        320,							/* outputWidth[0] = see outputHeight */
        0,							/* outputWidth[1] - don't care */
      },
      {
        XDAS_TRUE,						/* keepInputFrameRateFlag[0] = keep framerate */
        XDAS_TRUE,						/* keepInputFrameRateFlag[1] - don't care */
      },
      -1,							/* inputFrameRate = ignored when keepInputFrameRateFlag */
      {
        -1,							/* outputFrameRate[0] = ignored when keepInputFrameRateFlag */
        -1,							/* outputFrameRate[1] - don't care */
      },
      {
        -1,							/* targetBitRate[0] = ignored by codec */
        -1,							/* targetBitRate[1] - don't care */
      },
      {
        IVIDEO_NONE,						/* rateControl[0] = ignored by codec */
        IVIDEO_NONE,						/* rateControl[1] - don't care */
      },
      {
        XDAS_TRUE,						/* keepInputGOPFlag[0] = ignored by codec */
        XDAS_TRUE,						/* keepInputGOPFlag[1] - don't care */
      },
      {
        1,							/* intraFrameInterval[0] = ignored by codec */
        1,							/* intraFrameInterval[1] - don't care */
      },
      {
        0,							/* interFrameInterval[0] = ignored by codec */
        0,							/* interFrameInterval[1] - don't care */
      },
      {
        IVIDEO_NA_FRAME,					/* forceFrame[0] = ignored by codec */
        IVIDEO_NA_FRAME,					/* forceFrame[1] - don't care */
      },
      {
        XDAS_FALSE,						/* frameSkipTranscodeFlag[0] = ignored by codec */
        XDAS_FALSE,						/* frameSkipTranscodeFlag[1] - don't care */
      },
    },
    -1,								/* inputHeight - derived from params in initObj*/
    -1,								/* inputWidth  - derived from params in initObj*/
    -1,								/* inputLineLength - default, to be calculated base on width */
    {
      -1,							/* outputLineLength - default, to be calculated base on width */
      -1,							/* outputLineLength - default, to be calculated base on width */
    }
  };

  assert(_handle);

  if (_dynamicParams == NULL)
    _dynamicParams = &s_default;

  _handle->m_dynamicParams = *_dynamicParams;

#warning Actually check dynamic params and return IALG_EOK / IALG_EFAIL

  return IALG_EOK;
}


XDAS_Int32 trikCvProceedImage(const TrikCvImage* _inImage, TrikCvImage* _outImage,
                              const TRIK_VIDTRANSCODE_CV_InArgsAlg* _inArgs,
                              TRIK_VIDTRANSCODE_CV_OutArgsAlg* _outArgs)
{
#warning TODO
  return IVIDTRANSCODE_EOK;
}

