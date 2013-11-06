#ifndef TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_FAST_HPP_
#define TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_FAST_HPP_

#ifndef __cplusplus
#error C++-only header
#endif

#include <cassert>
#include <vector>
#include <c6x.h>

#include "internal/stdcpp.hpp"
#include "trik_vidtranscode_cv.h"


/* **** **** **** **** **** */ namespace trik /* **** **** **** **** **** */ {

/* **** **** **** **** **** */ namespace cv /* **** **** **** **** **** */ {


static uint16_t s_mult255_div[(1u<<8)];
static uint16_t s_mult43_div[(1u<<8)];


template <>
class BallDetector<TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_YUV422, TRIK_VIDTRANSCODE_CV_VIDEO_FORMAT_RGB565X> : public CVAlgorithm
{
  private:
    uint64_t m_detectRange;
    uint8_t  m_detectExpected;

    int32_t  m_targetX;
    int32_t  m_targetY;
    uint32_t m_targetPoints;

    TrikCvImageDesc m_inImageDesc;
    TrikCvImageDesc m_outImageDesc;

    std::vector<uint16_t> m_srcToDstColConv;
    std::vector<uint16_t> m_srcToDstRowConv;

    bool DEBUG_INLINE testifyRgbPixel(const uint32_t _rgb888,
                                      uint32_t& _out_rgb888) const
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
      const uint8_t u8_rgb_cmp = _cmpeq2(u32_rgb_max_max, u32_rgb_gb16);
      if (u8_rgb_cmp == 0)
        s16_hsv_hue_x256 = static_cast<int16_t>((0x10000*0)/3)
                         + static_cast<int16_t>(_dotpn2(u32_hsv_hue_mult43_div,
                                                        _packhl2(u32_rgb_gb16, u32_rgb_gb16)));
      else if (u8_rgb_cmp == 1)
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
      const uint64_t u64_hsv_range         = m_detectRange;
      const uint8_t  u8_hsv_det            = (  _cmpgtu4(u32_hsv, _hill(u64_hsv_range))
                                              | _cmpeq4( u32_hsv, _hill(u64_hsv_range)))
                                           & (  _cmpltu4(u32_hsv, _loll(u64_hsv_range))
                                              | _cmpeq4( u32_hsv, _loll(u64_hsv_range)));

      // SCORE cf5dee: 2.541, 2.485, 2.526, 2.471, 2.514
      if (u8_hsv_det != m_detectExpected)
        return false;

      _out_rgb888 = 0xffff00;
      return true;
    }

    void static __attribute__((always_inline)) writeRgb565Pixel(uint16_t* _rgb565ptr,
                                                                const uint32_t _rgb888)
    {
      *_rgb565ptr = ((_rgb888>>19)&0x001f) | ((_rgb888>>5)&0x07e0) | ((_rgb888<<8)&0xf800);
    }

    void __attribute__((always_inline)) drawRgbPixel(uint16_t _srcCol,
                                                     uint16_t _srcRow,
                                                     const TrikCvImageBuffer& _outImage,
                                                     const uint32_t _rgb888)
    {
      assert(_srcRow < m_srcToDstRowConv.size());
      const uint16_t dstRow = m_srcToDstRowConv[_srcRow];

      assert(_srcCol < m_srcToDstColConv.size());
      const uint16_t dstCol = m_srcToDstColConv[_srcCol];

      const uint32_t dstOfs = dstRow*m_outImageDesc.m_lineLength + dstCol*sizeof(uint16_t);
      uint16_t* rgbPtr = reinterpret_cast<uint16_t*>(_outImage.m_ptr + dstOfs);
      writeRgb565Pixel(rgbPtr, _rgb888);
    }

    void __attribute__((always_inline)) drawRgbTargetCross(int16_t _srcCol,
                                                           int16_t _srcRow,
                                                           const TrikCvImageBuffer& _outImage,
                                                           const uint32_t _rgb888)
    {
      for (int adj = 10; adj < 20; ++adj)
      {
        drawRgbPixel(range<int16_t>(0, _srcCol-adj, m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow-1  , m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
        drawRgbPixel(range<int16_t>(0, _srcCol-adj, m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow    , m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
        drawRgbPixel(range<int16_t>(0, _srcCol-adj, m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow+1  , m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
        drawRgbPixel(range<int16_t>(0, _srcCol+adj, m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow-1  , m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
        drawRgbPixel(range<int16_t>(0, _srcCol+adj, m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow    , m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
        drawRgbPixel(range<int16_t>(0, _srcCol+adj, m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow+1  , m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
        drawRgbPixel(range<int16_t>(0, _srcCol-1  , m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow-adj, m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
        drawRgbPixel(range<int16_t>(0, _srcCol    , m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow-adj, m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
        drawRgbPixel(range<int16_t>(0, _srcCol+1  , m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow-adj, m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
        drawRgbPixel(range<int16_t>(0, _srcCol-1  , m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow+adj, m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
        drawRgbPixel(range<int16_t>(0, _srcCol    , m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow+adj, m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
        drawRgbPixel(range<int16_t>(0, _srcCol+1  , m_inImageDesc.m_width-1),
                     range<int16_t>(0, _srcRow+adj, m_inImageDesc.m_height-1),
                     _outImage, _rgb888);
      }
    }

    void DEBUG_INLINE proceedRgbPixel(const uint16_t _srcRow,
                                      const uint16_t _srcCol,
                                      uint16_t* _dstImagePix,
                                      const uint32_t _rgb888)
    {
      uint32_t out_rgb888 = _rgb888;
      if (testifyRgbPixel(_rgb888, out_rgb888))
      {
        m_targetX += _srcCol;
        m_targetY += _srcRow;
        ++m_targetPoints;
      }

      writeRgb565Pixel(_dstImagePix, out_rgb888);
    }


    void DEBUG_INLINE proceedTwoYuyvPixels(const uint16_t _srcRow,
                                           const uint16_t _srcCol1,
                                           const uint16_t _srcCol2,
                                           uint16_t* _dstImagePix1,
                                           uint16_t* _dstImagePix2,
                                           const uint32_t _yuyv)
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

      proceedRgbPixel(_srcRow, _srcCol1, _dstImagePix1, u32_rgb_p1);

      proceedRgbPixel(_srcRow, _srcCol2, _dstImagePix2, u32_rgb_p2);
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
        m_srcToDstColConv[srcCol] = (srcCol*m_outImageDesc.m_width) / m_inImageDesc.m_width;

      m_srcToDstRowConv.resize(m_inImageDesc.m_height);
      for (uint16_t srcRow=0; srcRow < m_srcToDstRowConv.size(); ++srcRow)
        m_srcToDstRowConv[srcRow] = (srcRow*m_outImageDesc.m_height) / m_inImageDesc.m_height;

      s_mult255_div[0] = 0;
      for (uint16_t idx = 1; idx < (1u<<8); ++idx)
        s_mult255_div[idx] = (255u * (1u<<8)) / idx;

      s_mult43_div[0] = 0;
      for (uint16_t idx = 1; idx < (1u<<8); ++idx)
        s_mult43_div[idx] = (43u * (1u<<8)) / idx;

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

      uint8_t detectHueFrom = range<int16_t>(0, (_inArgs.detectHueFrom * 255) / 359, 255); // scaling 0..359 to 0..255
      uint8_t detectHueTo   = range<int16_t>(0, (_inArgs.detectHueTo   * 255) / 359, 255); // scaling 0..359 to 0..255
      uint8_t detectSatFrom = range<int16_t>(0, (_inArgs.detectSatFrom * 255) / 100, 255); // scaling 0..100 to 0..255
      uint8_t detectSatTo   = range<int16_t>(0, (_inArgs.detectSatTo   * 255) / 100, 255); // scaling 0..100 to 0..255
      uint8_t detectValFrom = range<int16_t>(0, (_inArgs.detectValFrom * 255) / 100, 255); // scaling 0..100 to 0..255
      uint8_t detectValTo   = range<int16_t>(0, (_inArgs.detectValTo   * 255) / 100, 255); // scaling 0..100 to 0..255

#ifdef DEBUG_REPEAT
      for (unsigned repeat = 0; repeat < DEBUG_REPEAT; ++repeat) {
#endif

      m_targetX = 0;
      m_targetY = 0;
      m_targetPoints = 0;

      if (detectHueFrom <= detectHueTo)
      {
        m_detectRange = _itoll((detectValFrom<<16) | (detectSatFrom<<8) | detectHueFrom,
                               (detectValTo  <<16) | (detectSatTo  <<8) | detectHueTo  );
        m_detectExpected = 0x8|0x7; // top byte is always compare equal
      }
      else
      {
        assert(detectHueFrom > 0 && detectHueTo < 255);
        m_detectRange = _itoll((detectValFrom<<16) | (detectSatFrom<<8) | (detectHueTo  +1),
                               (detectValTo  <<16) | (detectSatTo  <<8) | (detectHueFrom-1));
        m_detectExpected = 0x8|0x6; // top byte is always compare equal
      }

      assert(m_inImageDesc.m_height % 4 == 0); // verified in setup
#pragma MUST_ITERATE(,,4)
      for (uint16_t srcRow=0; srcRow < m_inImageDesc.m_height; ++srcRow)
      {
        assert(srcRow < m_srcToDstRowConv.size());
        const uint16_t dstRow = m_srcToDstRowConv[srcRow];

        const uint32_t  srcRowOfs = srcRow*m_inImageDesc.m_lineLength;
        const uint32_t* srcImage  = reinterpret_cast<uint32_t*>(_inImage.m_ptr + srcRowOfs);

        const uint32_t dstRowOfs   = dstRow*m_outImageDesc.m_lineLength;
        uint16_t*      dstImageRow = reinterpret_cast<uint16_t*>(_outImage.m_ptr + dstRowOfs);

        assert(m_inImageDesc.m_width % 32 == 0); // verified in setup
#pragma MUST_ITERATE(,,32/2)
        for (uint16_t srcCol=0; srcCol < m_inImageDesc.m_width; srcCol+=2)
        {
          assert(_srcCol+1 < m_srcToDstColConv.size());
          const uint16_t dstCol1 = m_srcToDstColConv[srcCol+0];
          const uint16_t dstCol2 = m_srcToDstColConv[srcCol+1];
          uint16_t* dstImagePix1 = &dstImageRow[dstCol1];
          uint16_t* dstImagePix2 = &dstImageRow[dstCol2];
          proceedTwoYuyvPixels(srcRow, srcCol+0, srcCol+1, dstImagePix1, dstImagePix2, *srcImage++);
        }
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


#endif // !TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_FAST_HPP_
