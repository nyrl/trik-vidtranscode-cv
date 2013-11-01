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
    std::vector<XDAS_UInt16> m_mult255_div;
    std::vector<XDAS_UInt16> m_mult43_div;

    bool testifyRgbPixel(const XDAS_UInt8 _r, const XDAS_UInt8 _g, const XDAS_UInt8 _b)
    {
      XDAS_UInt8 max;
      XDAS_UInt8 med;
      XDAS_UInt8 min;
      XDAS_UInt16 hsv_base;
      bool        hsv_is_incr;

#if 0

#define DEF_MIN_MED_MAX(_max, _med, _min, _hsv_base, _hsv_is_incr) \
      do { \
        max = _max; \
        med = _med; \
        min = _min; \
        hsv_base    = _hsv_base; \
        hsv_is_incr = _hsv_is_incr; \
      } while (0)

      if (_r > _b)
      {
        if (_b > _g) // r>b, b>g --> r>b>g
          DEF_MIN_MED_MAX(_r, _b, _g, 255, false);
        else if (_r > _g) // r>b, b<=g, r>g --> r>g>=b
          DEF_MIN_MED_MAX(_r, _g, _b, 0, true);
        else // r>b, b<=g, r<=g --> g>=r>b
          DEF_MIN_MED_MAX(_g, _r, _b, 85, false);
      }
      else if (_g > _b) // r<=b, g>b --> g>b>=r
        DEF_MIN_MED_MAX(_g, _b, _r, 85, true);
      else if (_r > _g) // r<=b, g<=b, r>g --> b>=r>g
        DEF_MIN_MED_MAX(_b, _r, _g, 171, true);
      else // r<=b, g<=b, r<=g --> b>=g>=r
        DEF_MIN_MED_MAX(_b, _g, _r, 171, false);
#undef DEF_MIN_MED_MAX

#else

      max = _r;
      med = _g;
      min = _b;
      hsv_base = 0;
      hsv_is_incr = true;

      // INTERMEDIATE max->r, med->g, min->b
      if (max < med)
      {
        std::swap(max, med);
        // INTERMEDIATE max->g, med->r, min->b
        if (med < min)
        {
          std::swap(med, min);
          // INTERMEDIATE max->g, med->b, min->r
          if (max < med)
          {
            std::swap(max, med);
            // FINAL max->b, med->g, min->r
            hsv_base = 171;
            hsv_is_incr = false;
          }
          else
          {
            // FINAL max->g, med->b, min->r
            hsv_base = 85;
          }
        }
        else
        {
          // FINAL max->g, med->r, min->b
          hsv_base = 85;
          hsv_is_incr = false;
        }
      }
      else if (med < min)
      {
        std::swap(med, min);
        // INTERMEDIATE max->r, med->b, min->g
        if (max < med)
        {
          std::swap(max, med);
          // FINAL max->b, med->r, min->g
          hsv_base = 171;
        }
        else
        {
          // FINAL max->r, med->b, min->g
          // hsv_base = 0;
          hsv_is_incr = false;
        }
      }
      else
      {
        // FINAL max->r, med->g, min->b
        // hsv_base = 0;
      }

#endif

      const XDAS_UInt16 hsv_v = max;

      /* optimized by table based multiplication with power-2 divisor, simulate 255*(max-min)/max */
      const XDAS_UInt16 hsv_s = (static_cast<XDAS_UInt16>(m_mult255_div[max]) * static_cast<XDAS_UInt16>(max-min)) >> 8;

      /* optimized by table based multiplication with power-2 divisor, simulate 43*(med-min)/(max-min) */
      const XDAS_UInt16 hsv_incr = (static_cast<XDAS_UInt16>(m_mult43_div[max-min]) * static_cast<XDAS_UInt16>(med-min)) >> 8;
      const XDAS_UInt16 hsv_h = hsv_is_incr ? hsv_base + hsv_incr : hsv_base - hsv_incr;

#warning TODO hsv check
#if 1
      if (hsv_v == 0xff && hsv_s == 0xff && hsv_h == 0xff)
        return true;
#endif
      return false;
    }

    void writeRgbPixel(XDAS_UInt16* _rgb, TrikCvImageDimension _srcCol, XDAS_UInt8 _r, XDAS_UInt8 _g, XDAS_UInt8 _b)
    {
      _rgb[m_widthConv[_srcCol]] = (static_cast<XDAS_UInt16>(_r)     >>3)
                                 | (static_cast<XDAS_UInt16>(_g&0xfc)<<3)
                                 | (static_cast<XDAS_UInt16>(_b&0xf8)<<8);
    }


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

      if (testifyRgbPixel(r1, g1, b1))
        writeRgbPixel(_rgb, _srcCol, 0xff, 0xff, 0x00);
      else
        writeRgbPixel(_rgb, _srcCol, r1, g1, b1);

      const XDAS_UInt8 r2 = range<XDAS_Int16>(0, (r12 + y2c) >> 6, 255);
      const XDAS_UInt8 g2 = range<XDAS_Int16>(0, (g12 + y2c) >> 6, 255);
      const XDAS_UInt8 b2 = range<XDAS_Int16>(0, (b12 + y2c) >> 6, 255);

      if (testifyRgbPixel(r2, g2, b2))
        writeRgbPixel(_rgb, _srcCol+1, 0xff, 0xff, 0x00);
      else
        writeRgbPixel(_rgb, _srcCol+1, r2, g2, b2);
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
      for (TrikCvImageDimension srcCol=0; srcCol < m_widthConv.size(); ++srcCol)
        m_widthConv[srcCol] = (srcCol*m_outImageDesc.m_width) / m_inImageDesc.m_width;

      m_heightConv.resize(m_inImageDesc.m_height);
      for (TrikCvImageDimension srcRow=0; srcRow < m_heightConv.size(); ++srcRow)
        m_heightConv[srcRow] = (srcRow*m_outImageDesc.m_height) / m_inImageDesc.m_height;

      m_mult255_div.resize(0x100);
      m_mult255_div[0] = 0;
      for (XDAS_UInt16 idx = 1; idx < m_mult255_div.size(); ++idx)
        m_mult255_div[idx] = (static_cast<XDAS_UInt16>(255) * static_cast<XDAS_UInt16>(1u<<8)) / idx;

      m_mult43_div.resize(0x100);
      m_mult43_div[0] = 0;
      for (XDAS_UInt16 idx = 1; idx < m_mult43_div.size(); ++idx)
        m_mult43_div[idx] = (static_cast<XDAS_UInt16>(43) * static_cast<XDAS_UInt16>(1u<<8)) / idx;

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
        const TrikCvImageDimension dstRow = m_heightConv[srcRow];

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
