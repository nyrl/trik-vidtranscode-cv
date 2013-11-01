#ifndef TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_HPP_
#define TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_HPP_

#ifndef __cplusplus
#error C++-only header
#endif

#include <string.h>
#include <vector>

#include "internal/stdcpp.hpp"
#include "trik_vidtranscode_cv.h"


/* **** **** **** **** **** */ namespace trik /* **** **** **** **** **** */ {

/* **** **** **** **** **** */ namespace cv /* **** **** **** **** **** */ {


template <TrikCvImageFormat _inFormat, TrikCvImageFormat _outFormat>
class BallDetector : public CVAlgorithm,
                     private assert_inst<false> // Generic instance, non-functional
{
  public:
    virtual bool setup(const TrikCvImageDesc& _inImageDesc, const TrikCvImageDesc& _outImageDesc) { return false; }
    virtual bool run(const TrikCvImageBuffer& _inImage, TrikCvImageBuffer& _outImage,
                     const TrikCvAlgInArgs& _inArgs, TrikCvAlgOutArgs& _outArgs);

  private:
};





#warning TODO temporary specialization; to be reworked
#if 0
typedef XDAS_Int8* TrikCvImagePtr;
typedef XDAS_Int16 TrikCvImageDimension;
typedef XDAS_Int32 TrikCvImageSize;
#endif


template <>
class BallDetector<TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_YUV422, TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_RGB565X> : public CVAlgorithm
{
  private:
    TrikCvImageDesc m_inImageDesc;
    TrikCvImageDesc m_outImageDesc;

    std::vector<TrikCvImageDimension> m_widthConv;
    std::vector<TrikCvImageDimension> m_heightConv;

  public:
    virtual bool setup(const TrikCvImageDesc& _inImageDesc, const TrikCvImageDesc& _outImageDesc)
    {
      m_inImageDesc  = _inImageDesc;
      m_outImageDesc = _outImageDesc;

#warning Extra checks for dimensions?

      m_widthConv.resize(m_inImageDesc.m_width);
      for (TrikCvImageDimension srcIdx=0; srcIdx < m_inImageDesc.m_width; ++srcIdx)
        m_widthConv[srcIdx] = (srcIdx*m_outImageDesc.m_width) / m_inImageDesc.m_width;

      m_heightConv.resize(m_inImageDesc.m_height);
      for (TrikCvImageDimension srcIdx=0; srcIdx < m_inImageDesc.m_height; ++srcIdx)
        m_widthConv[srcIdx] = (srcIdx*m_outImageDesc.m_height) / m_inImageDesc.m_height;

      return true;
    }

    virtual bool run(const TrikCvImageBuffer& _inImage, TrikCvImageBuffer& _outImage,
                     const TrikCvAlgInArgs& _inArgs, TrikCvAlgOutArgs& _outArgs)
    {


#warning Stub
#if 1
      if (_inImage.m_size < _outImage.m_size)
        _outImage.m_size = _inImage.m_size;
      memcpy(_outImage.m_ptr, _inImage.m_ptr, _outImage.m_size);
#endif

      return true;
    }
};


} /* **** **** **** **** **** * namespace cv * **** **** **** **** **** */

} /* **** **** **** **** **** * namespace trik * **** **** **** **** **** */


#endif // !TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_HPP_
