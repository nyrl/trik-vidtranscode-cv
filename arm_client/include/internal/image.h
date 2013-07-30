#ifndef VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_IMAGE_H_
#define VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_IMAGE_H_


#include <inttypes.h>

#include <QString>
#include <QDataStream>

namespace trik
{


class FormatID
{
  public:
    FormatID() : m_fmt() {}
    explicit FormatID(uint32_t _fmt) : m_fmt(_fmt) {}

    operator bool() const { return m_fmt != 0; }

    uint32_t id() const { return m_fmt; }

    QString toString() const
    {
      QString s;
      if (m_fmt == 0)
        s.append("<undefined>");
      else
        s.append((m_fmt    )&0xff)
         .append((m_fmt>>8 )&0xff)
         .append((m_fmt>>16)&0xff)
         .append((m_fmt>>24)&0xff);

      return s;
    }

  private:
    uint32_t m_fmt;
};


struct ImageFormat
{
  ImageFormat() : m_width(), m_height(), m_format(), m_lineLength(), m_size() {}

  int16_t  m_width;
  int16_t  m_height;
  FormatID m_format;
  size_t   m_lineLength;
  size_t   m_size;
};

} // namespace trik


#endif // !VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_IMAGE_H_
