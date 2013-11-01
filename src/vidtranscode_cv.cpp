#include "internal/vidtranscode_cv.h"

#include <cassert>
#include <memory>

#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>




#if 1
namespace trik
{
  class CVAlgorithm
  {
    public:
      virtual bool setup(const TrikCvImageDesc& _inImageDesc, const TrikCvImageDesc& _outImageDesc) = 0;
      virtual bool run(const TrikCvImageBuffer& _inImage, TrikCvImageBuffer& _outImage,
                       const TrikCvAlgInArgs& _inArgs, TrikCvAlgOutArgs& _outArgs) = 0;
  };

  template <TrikCvImageFormat _inFormat, TrikCvImageFormat _outFormat>
  class CVBallDetector : public CVAlgorithm
  {
    public:
      CVBallDetector() {};

      virtual bool setup(const TrikCvImageDesc& _inImageDesc, const TrikCvImageDesc& _outImageDesc) { return false; }
      virtual bool run(const TrikCvImageBuffer& _inImage, TrikCvImageBuffer& _outImage,
                       const TrikCvAlgInArgs& _inArgs, TrikCvAlgOutArgs& _outArgs) { return false; }
  };
};
#endif




struct TrikCvPersistentData
{
  TrikCvPersistentData() {}
  ~TrikCvPersistentData() {}

  TrikCvImageDesc m_inImageDesc;
  TrikCvImageDesc m_outImageDesc;

  std::auto_ptr<trik::CVAlgorithm> m_cvAlgorithm;
};




XDAS_Int32 trikCvHandleInit(TrikCvHandle* _handle)
{
  assert(_handle);

  _handle->m_persistentData = new TrikCvPersistentData;
  return IALG_EOK;
}


XDAS_Int32 trikCvHandleDestroy(TrikCvHandle* _handle)
{
  assert(_handle);

  delete reinterpret_cast<TrikCvPersistentData*>(_handle->m_persistentData);
  _handle->m_persistentData = 0;

  return IALG_EOK;
}


static TrikCvPersistentData& getHandlePersistentData(TrikCvHandle* _handle)
{
  assert(_handle && _handle->m_persistentData);

  return *reinterpret_cast<TrikCvPersistentData*>(_handle->m_persistentData);
}




template <typename _CVAlgorithm>
XDAS_Int32 createCVAlgorithm(TrikCvPersistentData& _pd)
{
  _pd.m_cvAlgorithm.reset(new _CVAlgorithm());
  if (!_pd.m_cvAlgorithm->setup(_pd.m_inImageDesc, _pd.m_outImageDesc))
  {
    Log_error0("CV algorithm setup failed");
    return IALG_EFAIL;
  }

  return IALG_EOK;
}




XDAS_Int32 handleSetupImageDescCreateCVAlgorithm(TrikCvPersistentData& _pd)
{
#define IF_IN_OUT_FORMAT(_CVAlgorithm, _inFormat, _outFormat) \
  if (_pd.m_inImageDesc.m_format == _inFormat && _pd.m_outImageDesc.m_format == _outFormat) \
    return createCVAlgorithm<_CVAlgorithm<_inFormat, _outFormat> >(_pd)

  IF_IN_OUT_FORMAT(trik::CVBallDetector, TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_YUV422, TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_RGB565X);
  IF_IN_OUT_FORMAT(trik::CVBallDetector, TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_RGB888, TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_RGB565X);

#undef IF_IN_OUT_FORMAT

  return IALG_EFAIL;
}




static XDAS_Int32 handleSetupImageDesc(TrikCvHandle* _handle)
{
  assert(_handle);
  TrikCvPersistentData& pd = getHandlePersistentData(_handle);
  XDAS_Int32 res;

  if (   _handle->m_params.base.numOutputStreams != 0
      && _handle->m_params.base.numOutputStreams != 1)
  {
    Log_error1("Invalid number of output stream: %d", _handle->m_params.base.numOutputStreams);
    return IALG_EFAIL;
  }

  pd.m_inImageDesc.m_format	= _handle->m_params.base.formatInput;
  pd.m_inImageDesc.m_width	= _handle->m_dynamicParams.inputWidth;
  pd.m_inImageDesc.m_height	= _handle->m_dynamicParams.inputHeight;
  pd.m_inImageDesc.m_lineLength	= _handle->m_dynamicParams.inputLineLength;

  if (_handle->m_params.base.numOutputStreams == 1)
  {
    pd.m_outImageDesc.m_format		= _handle->m_params.base.formatOutput[0];
    pd.m_outImageDesc.m_width		= _handle->m_dynamicParams.base.outputWidth[0];
    pd.m_outImageDesc.m_height		= _handle->m_dynamicParams.base.outputHeight[0];
    pd.m_outImageDesc.m_lineLength	= _handle->m_dynamicParams.outputLineLength[0];
  }
  else
  {
    pd.m_outImageDesc.m_format		= TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_UNKNOWN;
    pd.m_outImageDesc.m_width		= 0;
    pd.m_outImageDesc.m_height		= 0;
    pd.m_outImageDesc.m_lineLength	= 0;
  }

  if (   (pd.m_inImageDesc.m_width   < 0 || pd.m_inImageDesc.m_width   > _handle->m_params.base.maxWidthInput)
      || (pd.m_inImageDesc.m_height  < 0 || pd.m_inImageDesc.m_height  > _handle->m_params.base.maxHeightInput)
      || (pd.m_outImageDesc.m_width  < 0 || pd.m_outImageDesc.m_width  > _handle->m_params.base.maxWidthOutput[0])
      || (pd.m_outImageDesc.m_height < 0 || pd.m_outImageDesc.m_height > _handle->m_params.base.maxHeightOutput[0]))
  {
    Log_error4("Invalid image dimensions: %dx%d -> %dx%d",
               pd.m_inImageDesc.m_width,  pd.m_inImageDesc.m_height,
               pd.m_outImageDesc.m_width, pd.m_outImageDesc.m_height);
    return IALG_EFAIL;
  }

  if ((res = handleSetupImageDescCreateCVAlgorithm(pd)) != IALG_EOK)
  {
    Log_error0("Cannot create CV algorithm");
    return res;
  }

  Log_info0("Image description setup succeed");
  return IALG_EOK;
}




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

  Log_info0("Params accepted");
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
  XDAS_Int32 res;

  assert(_handle);

  if (_dynamicParams == NULL)
    _dynamicParams = &s_default;

  _handle->m_dynamicParams = *_dynamicParams;

#warning Actually check dynamic params and return IALG_EOK / IALG_EFAIL

  if ((res = handleSetupImageDesc(_handle)) != IALG_EOK)
  {
    Log_error0("Image description setup failed");
    return res;
  }

  Log_info0("Dynamic params accepted");
  return IALG_EOK;
}




XDAS_Int32 trikCvProceedImage(TrikCvHandle* _handle,
                              const TrikCvImageBuffer* _inImage, TrikCvImageBuffer* _outImage,
                              const TrikCvAlgInArgs* _inArgs, TrikCvAlgOutArgs* _outArgs)
{
  if (_handle == NULL || _inImage == NULL || _outImage == NULL || _inArgs == NULL || _outArgs == NULL)
  {
    Log_error0("Invalid image arguments");
    return IVIDTRANSCODE_EFAIL;
  }
  TrikCvPersistentData& pd = getHandlePersistentData(_handle);

  if (!pd.m_cvAlgorithm.get())
  {
    Log_error0("CV algorithm not created");
    return IVIDTRANSCODE_EFAIL;
  }

#warning TODO check buffer against image descriptions?

#warning TODO more sanity checks?


  if (!pd.m_cvAlgorithm->run(*_inImage, *_outImage, *_inArgs, *_outArgs))
  {
    Log_error0("CV algorithm run failed");
    return IVIDTRANSCODE_EFAIL;
  }

  return IVIDTRANSCODE_EOK;
}

