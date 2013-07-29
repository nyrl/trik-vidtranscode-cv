#ifndef VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_IMAGE_H_
#define VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_IMAGE_H_


#include <inttypes.h>

namespace trik
{

struct ImageFormat
{
  ImageFormat() : m_width(-1), m_height(-1), m_format(), m_lineLength(), m_size() {}

  int16_t  m_width;
  int16_t  m_height;
  uint32_t m_format;
  size_t   m_lineLength;
  size_t   m_size;
};

} // namespace trik


#endif // !VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_IMAGE_H_
