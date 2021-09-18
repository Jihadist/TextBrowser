#pragma once
#include <QDebug>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QtNetwork>
#include <memory>

namespace QtHelper {
struct deleteLaterDeletor {
    void operator()(QObject* object) const
    {
        if (object) {
            object->deleteLater();
        }
    }
};
}

template <typename T>
using UniquePointer = std::unique_ptr<T, QtHelper::deleteLaterDeletor>;

class RequestHelper : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QByteArray payload READ payload NOTIFY payloadChanged)
    Q_PROPERTY(bool success READ success NOTIFY successChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

public:
    explicit RequestHelper(QObject* parent = nullptr);

    const QString& url() const;
    void setUrl(const QString newUrl);
    Q_INVOKABLE void request();

    const QByteArray& payload() const;

    bool success() const;

    bool active() const;

signals:

    void urlChanged();
    void requestFinished();
    void payloadChanged();

    void successChanged();

    void activeChanged();

protected:
    void setPayload(const QByteArray& newPayload);
    void setSuccess(bool newSuccess);
    void setActive(bool newActive);

private:
    bool processHttpStatus(const QVariant& var) const;
#if QT_CONFIG(ssl)
    void sslErrors(const QList<QSslError>& errors);
#endif
    bool isText(const QString& str) const;
    bool isText(const QStringList& list) const
    {
        for (const auto& str : qAsConst(list)) {
            if (isText(str))
                return true;
        }
        return false;
    }
    void processResponse();

    QNetworkAccessManager qnam_;
    QByteArray m_payload;
    UniquePointer<QNetworkReply> currentReply_;

    QString m_url;
    bool m_success;
    bool m_active;
    bool hasPart_ = false;
};
