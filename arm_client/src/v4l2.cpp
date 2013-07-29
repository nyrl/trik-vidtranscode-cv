#include "internal/v4l2.h"

#include <QDebug>
#include <QByteArray>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/videodev2.h>
#include <libv4l2.h>



namespace trik
{

namespace
{


class V4L2FdHandle
{
  public:
    V4L2FdHandle(const QString& _path)
     :m_fd(-1),
      m_opened(false)
    {
      QByteArray path(_path.toLocal8Bit()); // pin temporary, convert to char*

      m_fd = v4l2_open(path.data(), O_RDWR|O_NONBLOCK, 0);
      if (m_fd < 0)
      {
        qWarning() << "v4l2_open(" << _path << ") failed:" << errno;
        return;
      }

      m_opened = true;
    }

    ~V4L2FdHandle()
    {
      if (m_fd)
        v4l2_close(m_fd);
    }

    bool opened() const
    {
      return m_opened;
    }

    bool ioctl(int _request, void* _argp, bool _reportError = true, int* _err = NULL)
    {
      if (!m_opened)
      {
        if (_err)
          *_err = ENOTCONN;
        if (_reportError)
          qWarning() << "v4l2_ioctl(" << _request << ") attempt on closed device";
        return false;
      }

      if (v4l2_ioctl(m_fd, _request, _argp) == -1)
      {
        if (_err)
          *_err = errno;
        if (_reportError)
          qWarning() << "v4l2_ioctl(" << _request << ") failed:" << errno;
        return false;
      }

      if (_err)
        *_err = 0;

      return true;
    }

  private:
    int  m_fd;
    bool m_opened;

    V4L2FdHandle(const V4L2FdHandle&) = delete;
    V4L2FdHandle& operator=(const V4L2FdHandle&) = delete;
};


} // namespace unnamed


class V4L2::Handle
{
  public:
    Handle(const QString& _path, const ImageFormat& _imageFormat)
     :m_fd(_path),
      m_opened(false),
      m_v4l2Format(),
      m_imageFormat()
    {
      if (!m_fd.opened())
        return;

      if (!setFormat(_imageFormat))
        return;


#warning TODO set

#warning TODO image format

      m_opened = true;
    }

    ~Handle()
    {
    }

    bool opened() const
    {
      return m_opened;
    }

  private:
    V4L2FdHandle m_fd;
    bool         m_opened;
    v4l2_format  m_v4l2Format;
    ImageFormat  m_imageFormat;

    bool reportEmulatedFormats()
    {
      for (size_t fmtIdx = 0; ; ++fmtIdx)
      {
        v4l2_fmtdesc fmtDesc;
        fmtDesc.index = fmtIdx;
        fmtDesc.type = m_v4l2Format.type;

        int err;
        if (!m_fd.ioctl(VIDIOC_ENUM_FMT, &fmtDesc, false, &err))
        {
          if (err != EINVAL)
          {
            qWarning() << "v4l2_ioctl(VIDIOC_ENUM_FMT) failed:" << err;
            return false;
          }
          break;
        }

        if (fmtDesc.pixelformat == m_v4l2Format.fmt.pix.pixelformat)
        {
          if (fmtDesc.flags & V4L2_FMT_FLAG_EMULATED)
            qWarning() << "V4L2 format" << FormatID(m_v4l2Format.fmt.pix.pixelformat).toString() << "is emulated, performance will be degraded";

          break;
        }
      }
      return true;
    }

    bool setFormat(const ImageFormat& _imageFormat)
    {
      static const v4l2_format s_zeroFormat = v4l2_format();
      m_v4l2Format = s_zeroFormat;

      m_v4l2Format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      m_v4l2Format.fmt.pix.width       = _imageFormat.m_width;
      m_v4l2Format.fmt.pix.height      = _imageFormat.m_height;
      m_v4l2Format.fmt.pix.pixelformat = _imageFormat.m_format.id();
#warning TODO setup field
#if 0
      m_v4l2Format.fmt.pix.field       = V4L2_FIELD_NONE;
#endif

      if (!m_fd.ioctl(VIDIOC_S_FMT, &m_v4l2Format))
        return false;

#warning TEMPORARY
#if 1
      qDebug() << "V4L2 pix field" << m_v4l2Format.fmt.pix.field;
#endif

      m_imageFormat.m_width      = m_v4l2Format.fmt.pix.width;
      m_imageFormat.m_height     = m_v4l2Format.fmt.pix.height;
      m_imageFormat.m_format     = FormatID(m_v4l2Format.fmt.pix.pixelformat);
      m_imageFormat.m_lineLength = m_v4l2Format.fmt.pix.bytesperline;
      m_imageFormat.m_size       = m_v4l2Format.fmt.pix.sizeimage;

      return reportEmulatedFormats();
    }

    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
};




V4L2::V4L2(QObject* _parent)
 :QObject(_parent),
  m_handle(),
  m_path("/dev/video0"),
  m_formatConfigured(),
  m_formatActual()
{
}

V4L2::~V4L2()
{
}

void
V4L2::setDevicePath(const QString& _path)
{
  m_path = _path;
}

void
V4L2::setFormat(const ImageFormat& _format)
{
  m_formatConfigured = _format;
}

bool
V4L2::open()
{
  if (m_handle)
  {
    qWarning() << "V4L2 already opened";
    return true;
  }

  m_handle.reset(new Handle(m_path, m_formatConfigured));
  if (!m_handle->opened())
  {
    qWarning() << "V4L2 open failed";
    m_handle.reset();
    return false;
  }

  emit opened();

  return true;
}

bool
V4L2::close()
{
  m_handle.reset();

  emit closed();

  return true;
}

void
V4L2::reportFPS()
{
#warning TODO
}

} // namespace trik
