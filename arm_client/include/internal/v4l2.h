#ifndef VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_
#define VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QPointer>
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

    virtual size_t buffersCount() const = 0;

    virtual bool map(const QPointer<V4L2>& _v4l2) = 0;
    virtual bool unmap(const QPointer<V4L2>& _v4l2) = 0;

    virtual bool queue(const QPointer<V4L2>& _v4l2, size_t _index) = 0;
    virtual bool dequeue(const QPointer<V4L2>& _v4l2, size_t _index) = 0;

  protected:
    static int v4l2_fd(const QPointer<V4L2>& _v4l2);
    static bool v4l2_ioctl(const QPointer<V4L2>& _v4l2, int _request, void* _argp, bool _reportError = true, int* _errno = NULL);

  private:
    V4L2BufferMapper(const V4L2BufferMapper&) = delete;
    V4L2BufferMapper& operator=(const V4L2BufferMapper&) = delete;
};
inline V4L2BufferMapper::~V4L2BufferMapper() = default; // gcc workaround


class V4L2BufferMapperMemoryMmap : public V4L2BufferMapper
{
  public:
    V4L2BufferMapperMemoryMmap(size_t _desiredBuffersCount);
    virtual ~V4L2BufferMapperMemoryMmap();

    virtual size_t buffersCount() const;

    virtual bool map(const QPointer<V4L2>& _v4l2);
    virtual bool unmap(const QPointer<V4L2>& _v4l2);

    virtual bool queue(const QPointer<V4L2>& _v4l2, size_t _index);
    virtual bool dequeue(const QPointer<V4L2>& _v4l2, size_t _index);

  private:
    size_t m_desiredBuffersCount;

    class Storage;
    QScopedPointer<Storage> m_storage;
};




class V4L2 : public QObject
{
  Q_OBJECT
  public:
    explicit V4L2(QObject* _parent);
    virtual ~V4L2();

  signals:
    void opened(const ImageFormat& _format);
    void closed();
    void started();
    void stopped();
    void fpsReported(qreal _fps);

  public slots:
    void setDevicePath(const QString& _path);
    void setFormat(const ImageFormat& _format);
    void setBufferMapper(const QSharedPointer<V4L2BufferMapper>& _bufferMapper);

    bool open();
    bool close();

    bool start();
    bool stop();

  protected:
    int fd() const;
    bool fd_ioctl(int _request, void* _argp, bool _reportError = true, int* _errno = NULL);

  protected slots:
    void frameReady();
    void reportFps();

  private:
    friend class V4L2BufferMapper;

    struct Config
    {
      QString                          m_path;
      ImageFormat                      m_imageFormat;
      QSharedPointer<V4L2BufferMapper> m_bufferMapper;
    };
    Config m_config;

    class OpenCloseHandler;
    friend class OpenCloseHandler;
    QScopedPointer<OpenCloseHandler> m_openCloseHandler;

    class RunningHandler;
    friend class RunningHandler;
    QScopedPointer<RunningHandler> m_runningHandler;
};

} // namespace trik


#endif // !VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_
