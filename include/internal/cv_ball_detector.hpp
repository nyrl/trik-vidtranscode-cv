#ifndef TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_HPP_
#define TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_HPP_

#ifndef __cplusplus
#error C++-only header
#endif

#include <cassert>
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


template <>
class BallDetector<TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_YUV422, TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_RGB565X> : public CVAlgorithm
{
  private:
    TrikCvImageDesc m_inImageDesc;
    TrikCvImageDesc m_outImageDesc;

    std::vector<TrikCvImageDimension> m_widthConv;
    std::vector<TrikCvImageDimension> m_heightConv;

    void proceedTwoYuyvPixels(const XDAS_UInt32 _yuyv, TrikCvImageDimension _srcCol, XDAS_UInt16* _rgb)
    {

#if 0
      // reference implementation
      typedef XDAS_Int16 Int16;
      union YUYV
      {
        struct
        {
          XDAS_UInt8 m_y1;
          XDAS_UInt8 m_u;
          XDAS_UInt8 m_y2;
          XDAS_UInt8 m_v;
        } m_unpacked;
        XDAS_UInt32 m_packed;
      } m_yuyv;
      m_yuyv.m_packed = _yuyv;

      const Int16 y1 = static_cast<Int16>(m_yuyv.m_unpacked.m_y1) - static_cast<Int16>(16);
      const Int16 d  = static_cast<Int16>(m_yuyv.m_unpacked.m_u)  - static_cast<Int16>(128);
      const Int16 y2 = static_cast<Int16>(m_yuyv.m_unpacked.m_y2) - static_cast<Int16>(16);
      const Int16 e  = static_cast<Int16>(m_yuyv.m_unpacked.m_v)  - static_cast<Int16>(128);

      const Int16 r12 =
                      + static_cast<Int16>(409/4) * e
                      + static_cast<Int16>(128/4);
      const Int16 g12 =
                      - static_cast<Int16>(100/4) * d
                      - static_cast<Int16>(208/4) * e
                      + static_cast<Int16>(128/4);
      const Int16 b12 =
                      + static_cast<Int16>(516/4) * d
                      + static_cast<Int16>(128/4);

      const Int16 y1c = static_cast<Int16>(298/4) * y1;
      const Int16 y2c = static_cast<Int16>(298/4) * y2;

#else
      // hopefully optimized implementation
      const XDAS_Int16 y1 = static_cast<XDAS_Int16>((_yuyv     )  & 0xff);
      const XDAS_Int16 u  = static_cast<XDAS_Int16>((_yuyv >>  8) & 0xff);
      const XDAS_Int16 y2 = static_cast<XDAS_Int16>((_yuyv >> 16) & 0xff);
      const XDAS_Int16 v  = static_cast<XDAS_Int16>((_yuyv >> 24) & 0xff);

      const XDAS_Int16 r12 =
                           + static_cast<XDAS_Int16>(409/4) * v
                           + static_cast<XDAS_Int16>(128/4 + (-128*409-16*298)/4);
      const XDAS_Int16 g12 =
                           - static_cast<XDAS_Int16>(100/4) * u
                           - static_cast<XDAS_Int16>(208/4) * v
                           + static_cast<XDAS_Int16>(128/4 + (+128*100+128*208-16*298)/4);
      const XDAS_Int16 b12 =
                           + static_cast<XDAS_Int16>(516/4) * u
                           + static_cast<XDAS_Int16>(128/4 + (-128*516-16*298)/4);

      const XDAS_Int16 y1c = static_cast<XDAS_Int16>(298/4) * y1;
      const XDAS_Int16 y2c = static_cast<XDAS_Int16>(298/4) * y2;
#endif

      const XDAS_UInt8 r1 = range<XDAS_Int16>(0, (r12 + y1c) >> 6, 255);
      const XDAS_UInt8 g1 = range<XDAS_Int16>(0, (g12 + y1c) >> 6, 255);
      const XDAS_UInt8 b1 = range<XDAS_Int16>(0, (b12 + y1c) >> 6, 255);

      _rgb[m_widthConv[_srcCol+0]] = (static_cast<XDAS_UInt16>(r1)     >>3)
                                   | (static_cast<XDAS_UInt16>(g1&0xfc)<<3)
                                   | (static_cast<XDAS_UInt16>(b1&0xf8)<<8);

      const XDAS_UInt8 r2 = range<XDAS_Int16>(0, (r12 + y2c) >> 6, 255);
      const XDAS_UInt8 g2 = range<XDAS_Int16>(0, (g12 + y2c) >> 6, 255);
      const XDAS_UInt8 b2 = range<XDAS_Int16>(0, (b12 + y2c) >> 6, 255);

      _rgb[m_widthConv[_srcCol+1]] = (static_cast<XDAS_UInt16>(r2)     >>3)
                                   | (static_cast<XDAS_UInt16>(g2&0xfc)<<3)
                                   | (static_cast<XDAS_UInt16>(b2&0xf8)<<8);
  }


  public:
    virtual bool setup(const TrikCvImageDesc& _inImageDesc, const TrikCvImageDesc& _outImageDesc)
    {
      m_inImageDesc  = _inImageDesc;
      m_outImageDesc = _outImageDesc;

      if (   m_inImageDesc.m_width < 0
          || m_inImageDesc.m_height < 0
          || m_inImageDesc.m_width  % 32 != 0
          || m_inImageDesc.m_height % 4  != 0)
        return false;

      m_widthConv.resize(m_inImageDesc.m_width);
      if (m_widthConv.size() != m_inImageDesc.m_width)
        return false;
      for (TrikCvImageDimension srcCol=0; srcCol < m_inImageDesc.m_width; ++srcCol)
        m_widthConv[srcCol] = (srcCol*m_outImageDesc.m_width) / m_inImageDesc.m_width;

      m_heightConv.resize(m_inImageDesc.m_height);
      if (m_heightConv.size() != m_inImageDesc.m_height)
        return false;
      for (TrikCvImageDimension srcRow=0; srcRow < m_inImageDesc.m_height; ++srcRow)
        m_heightConv[srcRow] = (srcRow*m_outImageDesc.m_height) / m_inImageDesc.m_height;

      return true;
    }

    virtual bool run(const TrikCvImageBuffer& _inImage, TrikCvImageBuffer& _outImage,
                     const TrikCvAlgInArgs& _inArgs, TrikCvAlgOutArgs& _outArgs)
    {
      if (m_inImageDesc.m_height * m_inImageDesc.m_lineLength > _inImage.m_size)
        return false;
      if (m_outImageDesc.m_height * m_outImageDesc.m_lineLength > _outImage.m_size)
        return false;

      for (TrikCvImageDimension srcRow=0; srcRow < m_inImageDesc.m_height; ++srcRow)
      {
        assert(srcRow < m_heightConv.size());
#warning somehow caching fails with release build
#if 0
        const TrikCvImageDimension dstRow = m_heightConv[srcRow];
#else
        const TrikCvImageDimension dstRow = (srcRow*m_outImageDesc.m_height) / m_inImageDesc.m_height;
#endif

        const TrikCvImageSize srcRowOfs = srcRow*m_inImageDesc.m_lineLength;
        const XDAS_UInt32* srcImage = reinterpret_cast<XDAS_UInt32*>(_inImage.m_ptr + srcRowOfs);

        const TrikCvImageSize dstRowOfs = dstRow*m_outImageDesc.m_lineLength;
        XDAS_UInt16* dstImage = reinterpret_cast<XDAS_UInt16*>(_outImage.m_ptr + dstRowOfs);

        assert(m_inImageDesc.m_width % 32 == 0); // verified in setup
        for (TrikCvImageDimension srcCol=0; srcCol < m_inImageDesc.m_width; srcCol+=2)
          proceedTwoYuyvPixels(*srcImage++, srcCol, dstImage);
      }

      return true;
    }
};


} /* **** **** **** **** **** * namespace cv * **** **** **** **** **** */

} /* **** **** **** **** **** * namespace trik * **** **** **** **** **** */


#endif // !TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_HPP_
