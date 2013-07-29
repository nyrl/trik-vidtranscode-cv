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



class V4L2::Handle
{
  public:
    Handle(const QString& _path)
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

    ~Handle()
    {
      if (m_fd)
        v4l2_close(m_fd);
    }

    bool opened() const
    {
      return m_opened;
    }

  private:
    int  m_fd;
    bool m_opened;

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

  m_handle.reset(new Handle(m_path));
  if (!m_handle->opened())
  {
    qWarning() << "V4L2 open failed";
    m_handle.reset();
    return false;
  }

  return true;
}

bool
V4L2::close()
{
  m_handle.reset();
  return true;
}

void
V4L2::reportFPS()
{
#warning TODO
}




#if 0
    struct ImageFormat
    {
      int16_t  m_width;
      int16_t  m_height;
      uint32_t m_format;
      size_t   m_lineLength;
      size_t   m_size;
    };

  signals:
    void opened();
    void closed();
    void formatChanged(const ImageFormat& _format);
#endif


