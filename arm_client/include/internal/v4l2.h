#ifndef VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_
#define VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <QSharedPointer>
#include <inttypes.h>

#include "internal/image.h"


namespace trik
{

class V4L2;

class V4L2BufferMapper
{
  public:
    V4L2BufferMapper() = default;
    virtual ~V4L2BufferMapper();

    virtual bool map(V4L2* _v4l2) = 0;
    virtual bool unmap(V4L2* _v4l2) = 0;
};
inline V4L2BufferMapper::~V4L2BufferMapper() = default; // old gcc workaround


class V4L2 : public QObject
{
  Q_OBJECT
  public:
    explicit V4L2(QObject* _parent);
    virtual ~V4L2();

  signals:
    void opened();
    void closed();
    void started();
    void stopped();
    void formatChanged(const ImageFormat& _format);

  public slots:
    void setDevicePath(const QString& _path);
    void setFormat(const ImageFormat& _format);
    void setBufferMapper(const QSharedPointer<V4L2BufferMapper>& _bufferMapper);


    bool open();
    bool close();

    bool start();
    bool stop();

    void reportFPS();

  protected:
    bool fd_ioctl(int _request, void* _argp, bool _reportError = true, int* _errno = NULL);

  private:
    class FdHandle;
    friend class FdHandle;
    QScopedPointer<FdHandle> m_fdHandle;

    class FormatHandler;
    friend class FormatHandler;
    QScopedPointer<FormatHandler> m_formatHandler;

    QString     m_path;

    ImageFormat m_formatConfigured;
    ImageFormat m_formatActual;

    friend class V4L2BufferMapper;
    QSharedPointer<V4L2BufferMapper> m_bufferMapperConfigured;

    class BufferMapperWrapper;
    friend class BufferMapperWrapper;
    QScopedPointer<BufferMapperWrapper> m_bufferMapperActual;
};

} // namespace trik


#endif // !VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_
