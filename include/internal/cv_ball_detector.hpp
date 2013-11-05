#ifndef TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_HPP_
#define TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_HPP_

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

    std::vector<TrikCvImageDimension> m_srcToDstColConv;
    std::vector<TrikCvImageDimension> m_srcToDstRowConv;
    std::vector<XDAS_UInt16> m_mult255_div;
    std::vector<XDAS_UInt16> m_mult43_div;

    XDAS_UInt8 m_detectHueFrom;
    XDAS_UInt8 m_detectHueTo;
    XDAS_UInt8 m_detectSatFrom;
    XDAS_UInt8 m_detectSatTo;
    XDAS_UInt8 m_detectValFrom;
    XDAS_UInt8 m_detectValTo;
    XDAS_Int32  m_targetX;
    XDAS_Int32  m_targetY;
    XDAS_UInt32 m_targetPoints;

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
          hsv_base = 255;
          hsv_is_incr = false;
        }
      }
      else
      {
        // FINAL max->r, med->g, min->b
        hsv_base = 0;
      }

#endif

      const XDAS_UInt16 hsv_v = max;
      const bool hsv_v_det = (m_detectValFrom <= hsv_v) && (m_detectValTo >= hsv_v);

      /* optimized by table based multiplication with power-2 divisor, simulate 255*(max-min)/max */
      const XDAS_UInt16 hsv_s = (static_cast<XDAS_UInt16>(m_mult255_div[max]) * static_cast<XDAS_UInt16>(max-min)) >> 8;
      const bool hsv_s_det = (m_detectSatFrom <= hsv_s) && (m_detectSatTo >= hsv_s);

      /* optimized by table based multiplication with power-2 divisor, simulate 43*(med-min)/(max-min) */
      const XDAS_UInt16 hsv_incr = (static_cast<XDAS_UInt16>(m_mult43_div[max-min]) * static_cast<XDAS_UInt16>(med-min)) >> 8;
      const XDAS_UInt16 hsv_h = hsv_is_incr ? hsv_base + hsv_incr : hsv_base - hsv_incr;
      const bool hsv_h_det = (m_detectHueFrom <= m_detectHueTo)
                           ? (m_detectHueFrom <= hsv_h) && (m_detectHueTo >= hsv_h)
                           : (m_detectHueFrom <= hsv_h) || (m_detectHueTo >= hsv_h);

      return hsv_h_det && hsv_v_det && hsv_s_det;
    }

    bool FAST_testifyRgbPixel(const uint32_t _rgb888)
    {
#warning TODO
      return testifyRgbPixel((_rgb888>>16)&0xff, (_rgb888>>8)&0xff, (_rgb888)&0xff);
    }

    void writeRgbPixel(XDAS_UInt16* _rgb,
                       const XDAS_UInt8 _r, const XDAS_UInt8 _g, const XDAS_UInt8 _b)
    {
      *_rgb = (static_cast<XDAS_UInt16>(_r)     >>3)
            | (static_cast<XDAS_UInt16>(_g&0xfc)<<3)
            | (static_cast<XDAS_UInt16>(_b&0xf8)<<8);
    }

    void writeRgbPixel(TrikCvImageDimension _srcCol,
                       XDAS_UInt16* _rgbRow,
                       const XDAS_UInt8 _r, const XDAS_UInt8 _g, const XDAS_UInt8 _b)
    {
      assert(_srcCol < m_srcToDstColConv.size());
      const TrikCvImageDimension dstCol = m_srcToDstColConv[_srcCol];
      writeRgbPixel(&_rgbRow[dstCol], _r, _g, _b);
    }

    void FAST_writeRgbPixel(TrikCvImageDimension _srcCol,
                            XDAS_UInt16* _rgbRow,
                            const uint32_t _rgb888)
    {
      assert(_srcCol < m_srcToDstColConv.size());
      const TrikCvImageDimension dstCol = m_srcToDstColConv[_srcCol];

      _rgbRow[dstCol] = ((_rgb888>>19)&0x001f) | ((_rgb888>>5)&0x07e0) | ((_rgb888<<8)&0xf800);
    }

    void drawRgbPixel(TrikCvImageDimension _srcCol,
                      TrikCvImageDimension _srcRow,
                      const TrikCvImageBuffer& _outImage,
                      const XDAS_UInt8 _r, const XDAS_UInt8 _g, const XDAS_UInt8 _b)
    {
      assert(_srcRow < m_srcToDstRowConv.size());
      const TrikCvImageDimension dstRow = m_srcToDstRowConv[_srcRow];

      assert(_srcCol < m_srcToDstColConv.size());
      const TrikCvImageDimension dstCol = m_srcToDstColConv[_srcCol];

      const TrikCvImageSize dstOfs = dstRow*m_outImageDesc.m_lineLength + dstCol*sizeof(XDAS_UInt16);
      XDAS_UInt16* rgbPtr = reinterpret_cast<XDAS_UInt16*>(_outImage.m_ptr + dstOfs);
      writeRgbPixel(rgbPtr, _r, _g, _b);
    }

    void drawRgbTargetCross(TrikCvImageDimension _srcCol,
                            TrikCvImageDimension _srcRow,
                            const TrikCvImageBuffer& _outImage,
                            const XDAS_UInt8 _r, const XDAS_UInt8 _g, const XDAS_UInt8 _b)
    {
      for (int adj = 10; adj < 20; ++adj)
      {
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol-adj, m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow-1  , m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol-adj, m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow    , m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol-adj, m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow+1  , m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol+adj, m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow-1  , m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol+adj, m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow    , m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol+adj, m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow+1  , m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol-1  , m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow-adj, m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol    , m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow-adj, m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol+1  , m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow-adj, m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol-1  , m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow+adj, m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol    , m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow+adj, m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
        drawRgbPixel(range<TrikCvImageDimension>(0, _srcCol+1  , m_inImageDesc.m_width-1),
                     range<TrikCvImageDimension>(0, _srcRow+adj, m_inImageDesc.m_height-1),
                     _outImage, _r, _g, _b);
      }
    }


    void proceedRgbPixel(const TrikCvImageDimension _srcCol,
                         const TrikCvImageDimension _srcRow,
                         XDAS_UInt16* _rgbRow,
                         const XDAS_UInt8 _r, const XDAS_UInt8 _g, const XDAS_UInt8 _b)
    {
      if (testifyRgbPixel(_r, _g, _b))
      {
        m_targetX += _srcCol;
        m_targetY += _srcRow;
        ++m_targetPoints;
        writeRgbPixel(_srcCol, _rgbRow, 0xff, 0xff, 0x00);
      }
      else
        writeRgbPixel(_srcCol, _rgbRow, _r, _g, _b);
    }

    void /*__attribute__((noinline))*/ FAST_proceedRgbPixel(const TrikCvImageDimension _srcCol,
                                                        const TrikCvImageDimension _srcRow,
                                                        XDAS_UInt16* _rgbRow,
                                                        const uint32_t _rgb888)
    {
      if (FAST_testifyRgbPixel(_rgb888))
      {
        m_targetX += _srcCol;
        m_targetY += _srcRow;
        ++m_targetPoints;
        FAST_writeRgbPixel(_srcCol, _rgbRow, 0xffff00);
      }
      else
        FAST_writeRgbPixel(_srcCol, _rgbRow, _rgb888);
    }


    void proceedTwoYuyvPixels(const XDAS_UInt32 _yuyv,
                              const TrikCvImageDimension _srcCol,
                              const TrikCvImageDimension _srcRow,
                              XDAS_UInt16* _rgbRow)
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
      const XDAS_Int16 y1 = static_cast<XDAS_Int16>((_yuyv      ) & 0xff);
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

      proceedRgbPixel(_srcCol+0, _srcRow, _rgbRow,
                      range<XDAS_Int16>(0, (r12 + y1c) >> 6, 255),
                      range<XDAS_Int16>(0, (g12 + y1c) >> 6, 255),
                      range<XDAS_Int16>(0, (b12 + y1c) >> 6, 255));

      proceedRgbPixel(_srcCol+1, _srcRow, _rgbRow,
                      range<XDAS_Int16>(0, (r12 + y2c) >> 6, 255),
                      range<XDAS_Int16>(0, (g12 + y2c) >> 6, 255),
                      range<XDAS_Int16>(0, (b12 + y2c) >> 6, 255));
    }


    void /*__attribute__((noinline))*/ FAST_proceedTwoYuyvPixels(const XDAS_UInt32 _yuyv,
                                                             const TrikCvImageDimension _srcCol,
                                                             const TrikCvImageDimension _srcRow,
                                                             XDAS_UInt16* _rgbRow)
    {

      struct packs16
      {
        static packs16 s16(int16_t _s16) { packs16 res; res.v.s16 = _s16; return res; }
        static packs16 s8x2(int8_t _s8h, int8_t _s8l) { packs16 res; res.v.p16.h = _s8h; res.v.p16.l = _s8l; return res; }

        union {
          struct {
            int8_t l;
            int8_t h;
          } p16;
          int16_t s16;
        } v;
      };

      struct packu16
      {
        static packu16 u16(uint16_t _u16) { packu16 res; res.v.u16 = _u16; return res; }
        static packu16 u8x2(uint8_t _u8h, uint8_t _u8l) { packu16 res; res.v.p16.h = _u8h; res.v.p16.l = _u8l; return res; }

        union {
          struct {
            uint8_t l;
            uint8_t h;
          } p16;
          uint16_t u16;
        } v;
      };

      struct packs32
      {
        static packs32 s32(int32_t _s32) { packs32 res; res.v.s32 = _s32; return res; }
        static packs32 s16x2(int16_t _s16h, int16_t _s16l) { packs32 res; res.v.p32.h.v.s16 = _s16h; res.v.p32.l.v.s16 = _s16l; return res; }
        static packs32 s8x4(int8_t _s8hh, int8_t _s8hl, int8_t _s8lh, int8_t _s8ll) { packs32 res; res.v.p32.h.v.p16.h = _s8hh; res.v.p32.h.v.p16.l = _s8hl; res.v.p32.l.v.p16.h = _s8lh; res.v.p32.l.v.p16.l = _s8ll; return res; }

        union {
          struct {
            packs16 l;
            packs16 h;
          } p32;
          int32_t s32;
        } v;
      };

      struct packu32
      {
        static packu32 u32(uint32_t _u32) { packu32 res; res.v.u32 = _u32; return res; }
        static packu32 u16x2(uint16_t _u16h, uint16_t _u16l) { packu32 res; res.v.p32.h.v.u16 = _u16h; res.v.p32.l.v.u16 = _u16l; return res; }
        static packu32 u8x4(uint8_t _u8hh, uint8_t _u8hl, uint8_t _u8lh, uint8_t _u8ll) { packu32 res; res.v.p32.h.v.p16.h = _u8hh; res.v.p32.h.v.p16.l = _u8hl; res.v.p32.l.v.p16.h = _u8lh; res.v.p32.l.v.p16.l = _u8ll; return res; }

        union {
          struct {
            packu16 l;
            packu16 h;
          } p32;
          uint32_t u32;
        } v;
      };

      struct packs64
      {
        static packs64 s64(int64_t _s64) { packs64 res; res.v.s64 = _s64; return res; }
        static packs64 s32x2(int32_t _s32h, int32_t _s32l) { packs64 res; res.v.p64.h.v.s32 = _s32h; res.v.p64.l.v.s32 = _s32l; return res; }
        static packs64 s16x4(int16_t _s16hh, int16_t _s16hl, int16_t _s16lh, int16_t _s16ll) { packs64 res; res.v.p64.h.v.p32.h.v.s16 = _s16hh; res.v.p64.h.v.p32.l.v.s16 = _s16hl; res.v.p64.l.v.p32.h.v.s16 = _s16lh; res.v.p64.l.v.p32.l.v.s16 = _s16ll; return res; }

        union {
          struct {
            packs32 l;
            packs32 h;
          } p64;
          int64_t s64;
        } v;
      };


      const packu32 pu32_yuyv   = packu32::u32(_yuyv);
      const packs64 ps64_yuyv1  = packs64::s64(_mpyu4ll(pu32_yuyv.v.u32,
                                                        //packu32::u8x4(409/4, 298/4, 516/4, 298/4).v.u32));
                                                         (static_cast<uint32_t>(409/4)<<24)
                                                        |(static_cast<uint32_t>(298/4)<<16)
                                                        |(static_cast<uint32_t>(516/4)<< 8)
                                                        |(static_cast<uint32_t>(298/4)    )));
      const packs64 ps64_yuyv2a = packs64::s64(_mpyus4ll(pu32_yuyv.v.u32,
                                                         //packs32::s8x4(-208/4, 0, -100/4, 0).v.s32));
                                                         (static_cast<uint32_t>(static_cast<uint8_t>(-208/4))<<24)
                                                         |(static_cast<uint32_t>(static_cast<uint8_t>(-100/4))<< 8)));
      const packs32 ps32_yuyv2b = packs32::s32(_add2(ps64_yuyv2a.v.p64.h.v.s32, ps64_yuyv2a.v.p64.l.v.s32));
      //const packs64 ps64_rgb1a  = packs64::s16x4(0, ps64_yuyv1.v.p64.h.v.p32.h.v.s16, ps32_yuyv2b.v.p32.h.v.s16, ps64_yuyv1.v.p64.l.v.p32.h.v.s16);
      const packs64 ps64_rgb1a  = packs64::s32x2(_packlh2(ps32_yuyv2b.v.s32, ps64_yuyv1.v.p64.h.v.s32),
                                                 _packh2( ps32_yuyv2b.v.s32, ps64_yuyv1.v.p64.l.v.s32));
      const packs64 ps64_rgb1b  = packs64::s32x2(_add2(ps64_rgb1a.v.p64.h.v.s32,
                                                       //packs32::s16x2(0, 128/4 + (-128*409-16*298)/4).v.s32),
                                                       (static_cast<uint32_t>(static_cast<uint16_t>(128/4 + (-128*409-16*298)/4)))),
                                                 _add2(ps64_rgb1a.v.p64.l.v.s32,
                                                       //packs32::s16x2(128/4 + (+128*100+128*208-16*298)/4, 128/4 + (-128*516-16*298)/4).v.s32));
                                                        (static_cast<uint32_t>(static_cast<uint16_t>(128/4 + (+128*100+128*208-16*298)/4)<<16))
                                                       |(static_cast<uint32_t>(static_cast<uint16_t>(128/4 + (-128*516-16*298)/4)))));

      //const packs64 ps64_y1     = packs64::s16x4(0, ps64_yuyv1.v.p64.l.v.p32.l.v.s16, ps64_yuyv1.v.p64.l.v.p32.l.v.s16, ps64_yuyv1.v.p64.l.v.p32.l.v.s16);
      const packs64 ps64_y1     = packs64::s32x2(ps64_yuyv1.v.p64.l.v.p32.l.v.s16,
                                                 _pack2(ps64_yuyv1.v.p64.l.v.s32, ps64_yuyv1.v.p64.l.v.s32));
      //const packs64 ps64_y2     = packs64::s16x4(0, ps64_yuyv1.v.p64.h.v.p32.l.v.s16, ps64_yuyv1.v.p64.h.v.p32.l.v.s16, ps64_yuyv1.v.p64.h.v.p32.l.v.s16);
      const packs64 ps64_y2     = packs64::s32x2(ps64_yuyv1.v.p64.h.v.p32.l.v.s16,
                                                 _pack2(ps64_yuyv1.v.p64.h.v.s32, ps64_yuyv1.v.p64.h.v.s32));
      const packs32 ps32_rgb21h = packs32::s32(_add2(ps64_rgb1b.v.p64.h.v.s32, ps64_y1.v.p64.h.v.s32));
      const packs32 ps32_rgb21l = packs32::s32(_add2(ps64_rgb1b.v.p64.l.v.s32, ps64_y1.v.p64.l.v.s32));
      const packs64 ps64_rgb21  = packs64::s32x2(_shr2(ps32_rgb21h.v.s32, 6),
                                                 _shr2(ps32_rgb21l.v.s32, 6));
      const packs32 ps32_rgb22h = packs32::s32(_add2(ps64_rgb1b.v.p64.h.v.s32, ps64_y2.v.p64.h.v.s32));
      const packs32 ps32_rgb22l = packs32::s32(_add2(ps64_rgb1b.v.p64.l.v.s32, ps64_y2.v.p64.l.v.s32));
      const packs64 ps64_rgb22  = packs64::s32x2(_shr2(ps32_rgb22h.v.s32, 6),
                                                 _shr2(ps32_rgb22l.v.s32, 6));
      const packu32 pu32_rgb_p1 = packu32::u32(_spacku4(ps64_rgb21.v.p64.h.v.s32, ps64_rgb21.v.p64.l.v.s32));
      const packu32 pu32_rgb_p2 = packu32::u32(_spacku4(ps64_rgb22.v.p64.h.v.s32, ps64_rgb22.v.p64.l.v.s32));

      FAST_proceedRgbPixel(_srcCol+0, _srcRow, _rgbRow, pu32_rgb_p1.v.u32);

      FAST_proceedRgbPixel(_srcCol+1, _srcRow, _rgbRow, pu32_rgb_p2.v.u32);
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
      for (TrikCvImageDimension srcCol=0; srcCol < m_srcToDstColConv.size(); ++srcCol)
        m_srcToDstColConv[srcCol] = (srcCol*m_outImageDesc.m_width) / m_inImageDesc.m_width;

      m_srcToDstRowConv.resize(m_inImageDesc.m_height);
      for (TrikCvImageDimension srcRow=0; srcRow < m_srcToDstRowConv.size(); ++srcRow)
        m_srcToDstRowConv[srcRow] = (srcRow*m_outImageDesc.m_height) / m_inImageDesc.m_height;

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
      _outImage.m_size = m_outImageDesc.m_height * m_outImageDesc.m_lineLength;

      m_detectHueFrom = range<XDAS_Int16>(0, (_inArgs.detectHueFrom * 255) / 359, 255); // scaling 0..359 to 0..255
      m_detectHueTo   = range<XDAS_Int16>(0, (_inArgs.detectHueTo   * 255) / 359, 255); // scaling 0..359 to 0..255
      m_detectSatFrom = range<XDAS_Int16>(0, (_inArgs.detectSatFrom * 255) / 100, 255); // scaling 0..100 to 0..255
      m_detectSatTo   = range<XDAS_Int16>(0, (_inArgs.detectSatTo   * 255) / 100, 255); // scaling 0..100 to 0..255
      m_detectValFrom = range<XDAS_Int16>(0, (_inArgs.detectValFrom * 255) / 100, 255); // scaling 0..100 to 0..255
      m_detectValTo   = range<XDAS_Int16>(0, (_inArgs.detectValTo   * 255) / 100, 255); // scaling 0..100 to 0..255
      m_targetX = 0;
      m_targetY = 0;
      m_targetPoints = 0;

      for (TrikCvImageDimension srcRow=0; srcRow < m_inImageDesc.m_height; ++srcRow)
      {
        assert(srcRow < m_srcToDstRowConv.size());
        const TrikCvImageDimension dstRow = m_srcToDstRowConv[srcRow];

        const TrikCvImageSize srcRowOfs = srcRow*m_inImageDesc.m_lineLength;
        const XDAS_UInt32* srcImage = reinterpret_cast<XDAS_UInt32*>(_inImage.m_ptr + srcRowOfs);

        const TrikCvImageSize dstRowOfs = dstRow*m_outImageDesc.m_lineLength;
        XDAS_UInt16* dstImage = reinterpret_cast<XDAS_UInt16*>(_outImage.m_ptr + dstRowOfs);

        assert(m_inImageDesc.m_width % 32 == 0); // verified in setup
        for (TrikCvImageDimension srcCol=0; srcCol < m_inImageDesc.m_width; srcCol+=2)
          FAST_proceedTwoYuyvPixels(*srcImage++, srcCol, srcRow, dstImage);
      }

      const TrikCvImageDimension inImagePixels = m_inImageDesc.m_width * m_inImageDesc.m_height;
      if (inImagePixels > 0)
        _outArgs.targetMass = (m_targetPoints * 10000)/ inImagePixels; // scaling to 0..10000
      else
        _outArgs.targetMass = 0;

      if (m_targetPoints > 0)
      {
        drawRgbTargetCross(m_targetX / m_targetPoints, m_targetY / m_targetPoints, _outImage, 0xff, 0x00, 0xff);
        _outArgs.targetX = (((m_targetX / m_targetPoints) - m_inImageDesc.m_width/2)  * 100*2) / m_inImageDesc.m_width;
        _outArgs.targetY = (((m_targetY / m_targetPoints) - m_inImageDesc.m_height/2) * 100*2) / m_inImageDesc.m_height;
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


#endif // !TRIK_VIDTRANSCODE_CV_INTERNAL_CV_BALL_DETECTOR_HPP_
