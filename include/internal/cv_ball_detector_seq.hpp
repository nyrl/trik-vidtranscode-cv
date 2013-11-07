#ifndef TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_SEQ_HPP_
#define TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_SEQ_HPP_

#ifndef __cplusplus
#error C++-only header
#endif

#include <cassert>
#include <c6x.h>

#include "internal/stdcpp.hpp"
#include "trik_vidtranscode_cv.h"


/* **** **** **** **** **** */ namespace trik /* **** **** **** **** **** */ {

/* **** **** **** **** **** */ namespace cv /* **** **** **** **** **** */ {


#define DEBUG_COMBINE_YUYV_RGB_HSV



static uint16_t s_mult43_div[(1u<<8)];
static uint16_t s_mult255_div[(1u<<8)];
static uint32_t s_rgb888[640*480];
static uint32_t s_hsv[640*480];


template <>
class BallDetector<TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_YUV422, TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_RGB565X> : public CVAlgorithm
{
  private:
    uint64_t m_detectRange;
    uint32_t m_detectExpected;

    int32_t  m_targetX;
    int32_t  m_targetY;
    uint32_t m_targetPoints;

    TrikCvImageDesc m_inImageDesc;
    TrikCvImageDesc m_outImageDesc;

    std::vector<uint16_t> m_srcToDstColConv;
    std::vector<uint16_t> m_srcToDstRowConv;


    static void __attribute__((always_inline)) writeRgb565Pixel(uint16_t* restrict _rgb565ptr,
                                                                const uint32_t _rgb888)
    {
      *_rgb565ptr = ((_rgb888>>19)&0x001f) | ((_rgb888>>5)&0x07e0) | ((_rgb888<<8)&0xf800);
    }

    void __attribute__((always_inline)) drawRgbPixel(const int32_t _srcCol,
                                                     const int32_t _srcRow,
                                                     const TrikCvImageBuffer& _outImage,
                                                     const uint32_t _rgb888) const
    {
      assert(_srcRow >= 0 && _srcRow < m_srcToDstRowConv.size());
      const uint32_t dstRow = m_srcToDstRowConv[_srcRow];

      assert(_srcCol >= 0 && _srcCol < m_srcToDstColConv.size());
      const uint32_t dstCol = m_srcToDstColConv[_srcCol];

      const uint32_t dstOfs = dstRow*m_outImageDesc.m_lineLength + dstCol*sizeof(uint16_t);
      uint16_t* restrict rgbPtr = reinterpret_cast<uint16_t*>(_outImage.m_ptr + dstOfs);
      writeRgb565Pixel(rgbPtr, _rgb888);
    }

    void __attribute__((always_inline)) drawRgbTargetCross(const int32_t _srcCol,
                                                           const int32_t _srcRow,
                                                           const TrikCvImageBuffer& _outImage,
                                                           const uint32_t _rgb888) const
    {
      const int32_t widthBot  = 0;
      const int32_t widthTop  = m_inImageDesc.m_width-1;
      const int32_t heightBot = 0;
      const int32_t heightTop = m_inImageDesc.m_height-1;

      for (int adj = 10; adj < 20; ++adj)
      {
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol-adj, widthTop),
                     range<int32_t>(heightBot, _srcRow-1  , heightTop),
                     _outImage, _rgb888);
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol-adj, widthTop),
                     range<int32_t>(heightBot, _srcRow    , heightTop),
                     _outImage, _rgb888);
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol-adj, widthTop),
                     range<int32_t>(heightBot, _srcRow+1  , heightTop),
                     _outImage, _rgb888);
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol+adj, widthTop),
                     range<int32_t>(heightBot, _srcRow-1  , heightTop),
                     _outImage, _rgb888);
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol+adj, widthTop),
                     range<int32_t>(heightBot, _srcRow    , heightTop),
                     _outImage, _rgb888);
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol+adj, widthTop),
                     range<int32_t>(heightBot, _srcRow+1  , heightTop),
                     _outImage, _rgb888);
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol-1  , widthTop),
                     range<int32_t>(heightBot, _srcRow-adj, heightTop),
                     _outImage, _rgb888);
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol    , widthTop),
                     range<int32_t>(heightBot, _srcRow-adj, heightTop),
                     _outImage, _rgb888);
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol+1  , widthTop),
                     range<int32_t>(heightBot, _srcRow-adj, heightTop),
                     _outImage, _rgb888);
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol-1  , widthTop),
                     range<int32_t>(heightBot, _srcRow+adj, heightTop),
                     _outImage, _rgb888);
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol    , widthTop),
                     range<int32_t>(heightBot, _srcRow+adj, heightTop),
                     _outImage, _rgb888);
        drawRgbPixel(range<int32_t>(widthBot,  _srcCol+1  , widthTop),
                     range<int32_t>(heightBot, _srcRow+adj, heightTop),
                     _outImage, _rgb888);
      }
    }

    static bool __attribute__((always_inline)) detectHsvPixel(const uint32_t _hsv,
                                                              const uint64_t _hsv_range,
                                                              const uint32_t _hsv_expect)
    {
      const uint32_t u32_hsv_det = _cmpltu4(_hsv, _hill(_hsv_range))
                                | _cmpgtu4(_hsv, _loll(_hsv_range));

      return (u32_hsv_det == _hsv_expect);
    }

    static uint64_t DEBUG_INLINE convert2xYuyvToRgb888(const uint32_t _yuyv)
    {
      const int64_t  s64_yuyv1   = _mpyu4ll(_yuyv,
                                             (static_cast<uint32_t>(static_cast<uint8_t>(409/4))<<24)
                                            |(static_cast<uint32_t>(static_cast<uint8_t>(298/4))<<16)
                                            |(static_cast<uint32_t>(static_cast<uint8_t>(516/4))<< 8)
                                            |(static_cast<uint32_t>(static_cast<uint8_t>(298/4))    ));
      const uint32_t u32_yuyv2   = _dotpus4(_yuyv,
                                             (static_cast<uint32_t>(static_cast<uint8_t>(-208/4))<<24)
                                            |(static_cast<uint32_t>(static_cast<uint8_t>(-100/4))<< 8));
      const uint32_t u32_rgb_h   = _add2(_packh2( 0,         _hill(s64_yuyv1)),
                                         (static_cast<uint32_t>(static_cast<uint16_t>(128/4 + (-128*409-16*298)/4))));
      const uint32_t u32_rgb_l   = _add2(_packlh2(u32_yuyv2, _loll(s64_yuyv1)),
                                          (static_cast<uint32_t>(static_cast<uint16_t>(128/4 + (+128*100+128*208-16*298)/4)<<16))
                                         |(static_cast<uint32_t>(static_cast<uint16_t>(128/4 + (-128*516-16*298)/4))));
      const uint32_t u32_y1y1    = _pack2(_loll(s64_yuyv1), _loll(s64_yuyv1));
      const uint32_t u32_y2y2    = _pack2(_hill(s64_yuyv1), _hill(s64_yuyv1));
      const uint32_t u32_rgb_p1h = _clr(_shr2(_add2(u32_rgb_h, u32_y1y1), 6), 16, 31);
      const uint32_t u32_rgb_p1l =      _shr2(_add2(u32_rgb_l, u32_y1y1), 6);
      const uint32_t u32_rgb_p2h = _clr(_shr2(_add2(u32_rgb_h, u32_y2y2), 6), 16, 31);
      const uint32_t u32_rgb_p2l =      _shr2(_add2(u32_rgb_l, u32_y2y2), 6);
      const uint32_t u32_rgb_p1 = _spacku4(u32_rgb_p1h, u32_rgb_p1l);
      const uint32_t u32_rgb_p2 = _spacku4(u32_rgb_p2h, u32_rgb_p2l);
      return _itoll(u32_rgb_p2, u32_rgb_p1);
    }

    static uint32_t DEBUG_INLINE convertRgb888ToHsv(const uint32_t _rgb888)
    {
      const uint32_t u32_rgb_or16    = _unpkhu4(_rgb888);
      const uint32_t u32_rgb_gb16    = _unpklu4(_rgb888);
      const uint32_t u32_rgb_max2    = _maxu4(_rgb888, _rgb888>>8);
      const uint16_t u16_rgb_max     = _clr(_maxu4(u32_rgb_max2, u32_rgb_max2>>8), 8, 31); // top 3 bytes were non-zeroes!
      const uint32_t u32_rgb_max_max = _pack2(u16_rgb_max, u16_rgb_max);

      const uint32_t u32_hsv_ooo_val_x256   = static_cast<uint16_t>(u16_rgb_max)<<8; // get max in 8..15 bits

      const uint32_t u32_rgb_min2    = _minu4(_rgb888, _rgb888>>8);
      const uint16_t u16_rgb_min     = _minu4(u32_rgb_min2, u32_rgb_min2>>8); // top 3 bytes are zeroes
      const uint16_t u16_rgb_delta   = u16_rgb_max-u16_rgb_min;

      /* optimized by table based multiplication with power-2 divisor, simulate 255*(max-min)/max */
      const uint16_t u16_hsv_sat_x256       = static_cast<uint16_t>(s_mult255_div[u16_rgb_max])
                                            * static_cast<uint16_t>(u16_rgb_delta);

      /* optimized by table based multiplication with power-2 divisor, simulate 43*(med-min)/(max-min) */
      const uint32_t u32_hsv_hue_mult43_div = _pack2(s_mult43_div[u16_rgb_delta],
                                                     s_mult43_div[u16_rgb_delta]);
      int16_t s16_hsv_hue_x256;
      const uint32_t u32_rgb_cmp = _cmpeq2(u32_rgb_max_max, u32_rgb_gb16);
      if (u32_rgb_cmp == 0)
          s16_hsv_hue_x256 = static_cast<int16_t>((0x10000*0)/3)
                           + static_cast<int16_t>(_dotpn2(u32_hsv_hue_mult43_div,
                                                          _packhl2(u32_rgb_gb16, u32_rgb_gb16)));
      else if (u32_rgb_cmp == 1)
          s16_hsv_hue_x256 = static_cast<int16_t>((0x10000*2)/3)
                           + static_cast<int16_t>(_dotpn2(u32_hsv_hue_mult43_div,
                                                          _packlh2(u32_rgb_or16, u32_rgb_gb16)));
      else
          s16_hsv_hue_x256 = static_cast<int16_t>((0x10000*1)/3)
                           + static_cast<int16_t>(_dotpn2(u32_hsv_hue_mult43_div,
                                                          _pack2(  u32_rgb_gb16, u32_rgb_or16)));

      const uint16_t u16_hsv_hue_x256      = static_cast<uint16_t>(s16_hsv_hue_x256);
      const uint32_t u32_hsv_sat_hue_x256  = _pack2(u16_hsv_sat_x256, u16_hsv_hue_x256);

      const uint32_t u32_hsv               = _packh4(u32_hsv_ooo_val_x256, u32_hsv_sat_hue_x256);
      return u32_hsv;
    }

#ifdef DEBUG_COMBINE_YUYV_RGB_HSV
    void DEBUG_INLINE convertImageYuyvToHsv(const TrikCvImageBuffer& _inImage)
    {
      const int8_t* restrict srcImageRow = _inImage.m_ptr;
      uint64_t* restrict rgb2x888ptr = reinterpret_cast<uint64_t*>(s_rgb888);
      uint64_t* restrict hsvx2ptr    = reinterpret_cast<uint64_t*>(s_hsv);
      const uint16_t height     = m_inImageDesc.m_height;
      const uint32_t lineLength = m_inImageDesc.m_lineLength;
      const uint16_t widthxu16  = m_inImageDesc.m_width*sizeof(uint16_t);

      assert(m_inImageDesc.m_height % 4 == 0); // verified in setup
#pragma MUST_ITERATE(4, ,4)
      for (uint16_t heightRemains=height; heightRemains>0; --heightRemains)
      {
        assert(reinterpret_cast<intptr_t>(srcImageRow) % 8 == 0); // let's pray...
        const uint64_t* restrict srcImagex2 = reinterpret_cast<const uint64_t*>(srcImageRow);
        const uint64_t* restrict srcImagex2To = reinterpret_cast<const uint64_t*>(srcImageRow + widthxu16);

        assert(m_inImageDesc.m_width % 32 == 0); // verified in setup
        assert(srcImagex2To-srcImagex2 == m_inImageDesc.m_width/4); // should be so...
#pragma MUST_ITERATE(32/4, ,32/4)
        for (; srcImagex2 != srcImagex2To; ++srcImagex2)
        {
          const uint64_t yuyv2x = *srcImagex2;

          const uint64_t rgb12 = convert2xYuyvToRgb888(_loll(yuyv2x));
          *rgb2x888ptr++ = rgb12;
          *hsvx2ptr++ = _itoll(convertRgb888ToHsv(_hill(rgb12)),
                               convertRgb888ToHsv(_loll(rgb12)));

          const uint64_t rgb34 = convert2xYuyvToRgb888(_hill(yuyv2x));
          *rgb2x888ptr++ = rgb34;
          *hsvx2ptr++ = _itoll(convertRgb888ToHsv(_hill(rgb34)),
                               convertRgb888ToHsv(_loll(rgb34)));
        }

        srcImageRow += lineLength;
      }
    }
#else
    void DEBUG_INLINE convertImageYuyvToRgb888(const TrikCvImageBuffer& _inImage)
    {
      const int8_t* restrict srcImageRow = _inImage.m_ptr;
      uint64_t* restrict rgb2x888ptr = reinterpret_cast<uint64_t*>(s_rgb888);
      const uint16_t height     = m_inImageDesc.m_height;
      const uint32_t lineLength = m_inImageDesc.m_lineLength;
      const uint16_t width4     = m_inImageDesc.m_width/4;

      assert(m_inImageDesc.m_height % 4 == 0); // verified in setup
#pragma MUST_ITERATE(4, ,4)
      for (uint16_t srcRow=0; srcRow < height; srcRow+=1)
      {
        assert(reinterpret_cast<intptr_t>(srcImageRow) % 8 == 0); // let's pray...
        const uint64_t* restrict srcImagex2 = reinterpret_cast<const uint64_t*>(srcImageRow);

        assert(m_inImageDesc.m_width % 32 == 0); // verified in setup
#pragma MUST_ITERATE(32/4, ,32/4)
        for (uint16_t remains = width4; remains > 0; --remains)
        {
          const uint64_t yuyv2x = *srcImagex2++;
          *rgb2x888ptr++ = convert2xYuyvToRgb888(_loll(yuyv2x));
          *rgb2x888ptr++ = convert2xYuyvToRgb888(_hill(yuyv2x));
        }

        srcImageRow += lineLength;
      }
    }

    void DEBUG_INLINE convertImageRgb888ToHsv()
    {
      const uint64_t* restrict rgb888x2ptr = reinterpret_cast<uint64_t*>(s_rgb888);
      uint64_t* restrict hsvx2ptr          = reinterpret_cast<uint64_t*>(s_hsv);

      uint32_t dimx2 = (m_inImageDesc.m_width*m_inImageDesc.m_height)/2;
      assert(dimx2 % (32*4/2) == 0); // m_width%32, m_height%4, two pixels per loop
#pragma MUST_ITERATE((32*4)/2, ,(32*4)/2)
      for (; dimx2 > 0; --dimx2)
      {
        const uint64_t rgb888x2 = *rgb888x2ptr++;
        *hsvx2ptr++ = _itoll(convertRgb888ToHsv(_hill(rgb888x2)),
                             convertRgb888ToHsv(_loll(rgb888x2)));
      }
    }
#endif

    void DEBUG_INLINE proceedImageHsv(TrikCvImageBuffer& _outImage)
    {
      const uint64_t* restrict rgb888x2ptr = reinterpret_cast<uint64_t*>(s_rgb888);
      const uint64_t* restrict hsvx2ptr    = reinterpret_cast<uint64_t*>(s_hsv);
      const uint16_t width          = m_inImageDesc.m_width;
      const uint16_t height         = m_inImageDesc.m_height;
      const uint32_t dstLineLength  = m_outImageDesc.m_lineLength;
      const uint64_t u64_hsv_range  = m_detectRange;
      const uint32_t u32_hsv_expect = m_detectExpected;
      int32_t  targetX = 0;
      int32_t  targetY = 0;
      uint32_t targetPoints = 0;

      assert(m_inImageDesc.m_height % 4 == 0); // verified in setup
#pragma MUST_ITERATE(4, ,4)
      for (uint16_t srcRow=0; srcRow < height; ++srcRow)
      {
        assert(srcRow < m_srcToDstRowConv.size());
        const uint16_t dstRow = m_srcToDstRowConv[srcRow];
        const uint32_t dstRowOfs = dstRow*dstLineLength;
        uint16_t* restrict dstImageRow = reinterpret_cast<uint16_t*>(_outImage.m_ptr + dstRowOfs);

        assert(m_inImageDesc.m_width % 32 == 0); // verified in setup
#pragma MUST_ITERATE(32/2, ,32/2)
        for (uint16_t srcCol=0; srcCol < width; srcCol+=2)
        {
          assert(_srcCol+1 < m_srcToDstColConv.size());
          const uint16_t dstCol1 = m_srcToDstColConv[srcCol+0];
          const uint16_t dstCol2 = m_srcToDstColConv[srcCol+1];
          const uint64_t rgb888x2 = *rgb888x2ptr++;
          const uint64_t hsvx2    = *hsvx2ptr++;

          if (detectHsvPixel(_loll(hsvx2), u64_hsv_range, u32_hsv_expect))
          {
            targetX += srcCol+0;
            targetY += srcRow;
            targetPoints += 1;
            writeRgb565Pixel(dstImageRow+dstCol1, 0xffff00);
          }
          else
            writeRgb565Pixel(dstImageRow+dstCol1, _loll(rgb888x2));

          if (detectHsvPixel(_hill(hsvx2), u64_hsv_range, u32_hsv_expect))
          {
            targetX += srcCol+1;
            targetY += srcRow;
            targetPoints += 1;
            writeRgb565Pixel(dstImageRow+dstCol2, 0xffff00);
          }
          else
            writeRgb565Pixel(dstImageRow+dstCol2, _hill(rgb888x2));
        }
      }

      m_targetX = targetX;
      m_targetY = targetY;
      m_targetPoints = targetPoints;
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

      m_srcToDstColConv.resize(m_inImageDesc.m_width);
      for (uint16_t srcCol=0; srcCol < m_srcToDstColConv.size(); ++srcCol)
        m_srcToDstColConv[srcCol] = (srcCol*m_outImageDesc.m_width) / m_inImageDesc.m_width; // m_width > 0 if came here

      m_srcToDstRowConv.resize(m_inImageDesc.m_height);
      for (uint16_t srcRow=0; srcRow < m_srcToDstRowConv.size(); ++srcRow)
        m_srcToDstRowConv[srcRow] = (srcRow*m_outImageDesc.m_height) / m_inImageDesc.m_height; // m_height > 0 if came here

      s_mult43_div[0] = 0;
      s_mult255_div[0] = 0;
      for (uint16_t idx = 1; idx < (1u<<8); ++idx)
      {
        s_mult43_div[ idx] = (43u  * (1u<<8)) / idx;
        s_mult255_div[idx] = (255u * (1u<<8)) / idx;
      }

      return true;
    }

    virtual bool run(const TrikCvImageBuffer& _inImage, TrikCvImageBuffer& _outImage,
                     const TrikCvAlgInArgs& _inArgs, TrikCvAlgOutArgs& _outArgs)
    {
      if (m_inImageDesc.m_height * m_inImageDesc.m_lineLength > _inImage.m_size)
        return false;
      if (m_outImageDesc.m_height * m_outImageDesc.m_lineLength > _outImage.m_size)
        return false;
      _outImage.m_size = m_outImageDesc.m_height * m_outImageDesc.m_lineLength;

      m_targetX = 0;
      m_targetY = 0;
      m_targetPoints = 0;

      uint8_t detectHueFrom = range<int32_t>(0, (static_cast<int32_t>(_inArgs.detectHueFrom) * 255) / 359, 255); // scaling 0..359 to 0..255
      uint8_t detectHueTo   = range<int32_t>(0, (static_cast<int32_t>(_inArgs.detectHueTo  ) * 255) / 359, 255); // scaling 0..359 to 0..255
      uint8_t detectSatFrom = range<int32_t>(0, (static_cast<int32_t>(_inArgs.detectSatFrom) * 255) / 100, 255); // scaling 0..100 to 0..255
      uint8_t detectSatTo   = range<int32_t>(0, (static_cast<int32_t>(_inArgs.detectSatTo  ) * 255) / 100, 255); // scaling 0..100 to 0..255
      uint8_t detectValFrom = range<int32_t>(0, (static_cast<int32_t>(_inArgs.detectValFrom) * 255) / 100, 255); // scaling 0..100 to 0..255
      uint8_t detectValTo   = range<int32_t>(0, (static_cast<int32_t>(_inArgs.detectValTo  ) * 255) / 100, 255); // scaling 0..100 to 0..255

      if (detectHueFrom <= detectHueTo)
      {
        m_detectRange = _itoll((detectValFrom<<16) | (detectSatFrom<<8) | detectHueFrom,
                               (detectValTo  <<16) | (detectSatTo  <<8) | detectHueTo  );
        m_detectExpected = 0x0;
      }
      else
      {
        assert(detectHueFrom > 0 && detectHueTo < 255);
        m_detectRange = _itoll((detectValFrom<<16) | (detectSatFrom<<8) | (detectHueTo  +1),
                               (detectValTo  <<16) | (detectSatTo  <<8) | (detectHueFrom-1));
        m_detectExpected = 0x1;
      }


#ifdef DEBUG_REPEAT
      for (unsigned repeat = 0; repeat < DEBUG_REPEAT; ++repeat) {
#endif

      if (m_inImageDesc.m_height > 0 && m_inImageDesc.m_width > 0)
      {
#ifdef DEBUG_COMBINE_YUYV_RGB_HSV
        convertImageYuyvToHsv(_inImage);
#else
        convertImageYuyvToRgb888(_inImage);
        convertImageRgb888ToHsv();
#endif
        proceedImageHsv(_outImage);
      }

#ifdef DEBUG_REPEAT
      } // repeat
#endif

      const uint32_t inImagePixels = m_inImageDesc.m_width * m_inImageDesc.m_height;
      if (inImagePixels > 0)
        _outArgs.targetMass = (static_cast<uint32_t>(m_targetPoints) * static_cast<uint32_t>(10000))
                            / inImagePixels; // scaling to 0..10000
      else
        _outArgs.targetMass = 0;

      if (m_targetPoints > 0)
      {
        int32_t targetX = m_targetX/m_targetPoints;
        int32_t targetY = m_targetY/m_targetPoints;
        drawRgbTargetCross(targetX, targetY, _outImage, 0xff00ff);
        _outArgs.targetX = ((targetX - static_cast<int32_t>(m_inImageDesc.m_width) /2) * 100*2) / static_cast<int32_t>(m_inImageDesc.m_width);
        _outArgs.targetY = ((targetY - static_cast<int32_t>(m_inImageDesc.m_height)/2) * 100*2) / static_cast<int32_t>(m_inImageDesc.m_height);
      }
      else
      {
        _outArgs.targetX = 0;
        _outArgs.targetY = 0;
      }

      return true;
    }
};


} /* **** **** **** **** **** * namespace cv * **** **** **** **** **** */

} /* **** **** **** **** **** * namespace trik * **** **** **** **** **** */


#endif // !TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_SEQ_HPP_
