#ifndef VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_
#define VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QSocketNotifier>
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

  protected:
    static int v4l2_fd(V4L2* _v4l2);
    static bool v4l2_ioctl(V4L2* _v4l2, int _request, void* _argp, bool _reportError = true, int* _errno = NULL);

  private:
    V4L2BufferMapper(const V4L2BufferMapper&) = delete;
    V4L2BufferMapper& operator=(const V4L2BufferMapper&) = delete;
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
    int fd() const;
    bool fd_ioctl(int _request, void* _argp, bool _reportError = true, int* _errno = NULL);

  protected slots:
    void frameReady();

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

    QScopedPointer<QSocketNotifier> m_frameNotifier;
};

} // namespace trik


#endif // !VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_
