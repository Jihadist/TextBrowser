#include "requesthelper.h"
#include <QByteArray>
#include <QTextStream>
#ifdef Q_OS_WINDOWS

#include <urlmon.h>
namespace detail {
static std::wstring mimeTypeFromString(void* buffer, quint32 size)
{

    LPWSTR pwzMimeOut = NULL;
    HRESULT hr = FindMimeFromData(NULL, NULL, buffer, size,
        NULL, FMFD_DEFAULT, &pwzMimeOut, 0x0);
    if (SUCCEEDED(hr)) {
        std::wstring strResult(pwzMimeOut);
        // Despite the documentation stating to call operator delete, the
        // returned string must be cleaned up using CoTaskMemFree
        CoTaskMemFree(pwzMimeOut);
        return strResult;
    }

    return L"";
}
}

static QString mimeType(const QByteArray& ba)
{
    return QString::fromStdWString(detail::mimeTypeFromString(const_cast<char*>(ba.data()), ba.size()));
}
#endif

constexpr int timeout = 10000;
constexpr int validStatus = 200;
constexpr int bufferSize = 512;

RequestHelper::RequestHelper(QObject* parent)
    : QObject(parent)
    , m_success(true)
{
    qnam_.setTransferTimeout(timeout);
}

const QString& RequestHelper::url() const { return m_url; }

void RequestHelper::setUrl(const QString newUrl)
{
    if (m_url == newUrl)
        return;
    m_url = newUrl;
    emit urlChanged();
}

bool RequestHelper::processHttpStatus(const QVariant& var) const
{
    if (!var.isValid())
        return false;

    bool ok = false;
    auto status = var.toInt(&ok);
    if (!ok) {
        return false;
    }
    if (status == validStatus)
        return true;

    return false;
}

bool RequestHelper::isText(const QString& str) const
{
    if ((str.contains("text")) //
        || str.contains("html") //
        || str.contains("json") //
        || str.contains("xml")) //
        return true;

    return false;
}

void RequestHelper::processResponse()
{
    if (hasPart_)
        return;
    hasPart_ = true;
    QByteArray buffer = currentReply_->read(bufferSize);

    auto headerMimeType = currentReply_->header(QNetworkRequest::ContentTypeHeader).toString().split("/");

    bool isBinary = true;
    if (isText(headerMimeType)) {
        isBinary = false;
    } else if (isText(mimeType(buffer).split("/"))) {
        isBinary = false;
    }

    // represent binary as string of bytes 0x00...
    if (isBinary) {

        QByteArray codedArray;
        QByteArray encodedArray = buffer.toHex();

        for (const auto& i : qAsConst(encodedArray)) {
            codedArray.append("0x").append(i);
        }

        setPayload(codedArray);
    } else {
        setPayload(buffer);
    }
}

void RequestHelper::request()
{
    hasPart_ = false;
    setActive(true);

    QString tempUrl = url();
    if (!((tempUrl.startsWith("http://")) ^ (tempUrl.startsWith("https://")))) {
        tempUrl.prepend("https://");
    }

    currentReply_ = UniquePointer<QNetworkReply>(
        qnam_.get(QNetworkRequest(tempUrl)));

    QObject::connect(currentReply_.get(), &QNetworkReply::finished, this, [&] {
        setActive(false);
        QVariant status_code = currentReply_->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        setSuccess(processHttpStatus(status_code));
    });

    QObject::connect(currentReply_.get(), &QNetworkReply::finished, this,
        &RequestHelper::requestFinished);

    QObject::connect(currentReply_.get(), &QNetworkReply::errorOccurred, this,
        [&](QNetworkReply::NetworkError code) {
            auto ba = QVariant::fromValue(code).toByteArray();
            setPayload(ba);
        });

    QObject::connect(currentReply_.get(), &QIODevice::readyRead, this, &RequestHelper::processResponse);

#if QT_CONFIG(ssl)
    QObject::connect(currentReply_.get(), &QNetworkReply::sslErrors, this,
        &RequestHelper::sslErrors);
#endif
}

const QByteArray& RequestHelper::payload() const
{
    return m_payload;
}

void RequestHelper::setPayload(const QByteArray& newPayload)
{
    if (m_payload == newPayload)
        return;
    m_payload = newPayload;

    emit payloadChanged();
}

bool RequestHelper::success() const { return m_success; }

void RequestHelper::setSuccess(bool newSuccess)
{
    if (m_success == newSuccess)
        return;
    m_success = newSuccess;
    emit successChanged();
}

bool RequestHelper::active() const
{
    return m_active;
}

void RequestHelper::setActive(bool newActive)
{
    if (m_active == newActive)
        return;
    m_active = newActive;
    emit activeChanged();
}

void RequestHelper::sslErrors(const QList<QSslError>& errors)
{
    // smth special for ssl errors
}
