#ifndef VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_
#define VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <inttypes.h>

#include "internal/image.h"


namespace trik
{

class V4L2 : public QObject
{
  Q_OBJECT
  public:
    explicit V4L2(QObject* _parent);
    virtual ~V4L2();

  signals:
    void opened();
    void closed();
    void formatChanged(const ImageFormat& _format);

  public slots:
    void setDevicePath(const QString& _path);
    void setFormat(const ImageFormat& _format);

    bool open();
    bool close();

    void reportFPS();

  private:
    class Handle;
    QScopedPointer<Handle> m_handle;

    QString     m_path;
    ImageFormat m_formatConfigured;
    ImageFormat m_formatActual;
};

} // namespace trik


#endif // !VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_V4L2_H_
