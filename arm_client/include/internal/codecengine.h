#ifndef VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_CODECENGINE_H_
#define VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_CODECENGINE_H_

#include <QObject>
#include <QString>
#include <QScopedPointer>


class CodecEngine : public QObject
{
  Q_OBJECT
  public:
    explicit CodecEngine(QObject* _parent);
    virtual ~CodecEngine();

  signals:
    void opened();
    void closed();

  public slots:
    void setServerName(const QString& _name);
    void setServerPath(const QString& _path);

    bool open();
    bool close();


  private:
    class EngineControl;
    static EngineControl s_engineControl;

    class Handle;
    QScopedPointer<Handle> m_handle;

    QString m_serverName;
    QString m_serverPath;
};


#endif // !VIDTRANSCODE_CV_ARM_CLIENT_INCLUDE_INTERNAL_CODECENGINE_H_
