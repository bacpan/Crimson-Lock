#include "core/VaultEntry.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUuid>
#include <QtCore/QDateTime>
#include <QtCore/QCryptographicHash>
#include <QtCore/QSysInfo>
#include <stdexcept>

namespace crimson {
namespace core {

std::string VaultEntry::toJson() const {
    QJsonObject obj;
    obj["id"] = QString::fromStdString(id);
    obj["label"] = QString::fromStdString(label);
    obj["username"] = QString::fromStdString(username);
    obj["password"] = QString::fromStdString(password);
    obj["created_at"] = QString::fromStdString(created_at);
    obj["device_fingerprint"] = QString::fromStdString(device_fingerprint);
    
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact).toStdString();
}

VaultEntry VaultEntry::fromJson(const std::string& json) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json), &error);
    
    if (error.error != QJsonParseError::NoError) {
        throw std::runtime_error("Failed to parse vault entry JSON: " + error.errorString().toStdString());
    }
    
    if (!doc.isObject()) {
        throw std::runtime_error("Invalid vault entry JSON: not an object");
    }
    
    QJsonObject obj = doc.object();
    VaultEntry entry;
    
    entry.id = obj["id"].toString().toStdString();
    entry.label = obj["label"].toString().toStdString();
    entry.username = obj["username"].toString().toStdString();
    entry.password = obj["password"].toString().toStdString();
    entry.created_at = obj["created_at"].toString().toStdString();
    entry.device_fingerprint = obj["device_fingerprint"].toString().toStdString();
    
    return entry;
}

std::string VaultEntry::generateUuid() {
    QUuid uuid = QUuid::createUuid();
    return uuid.toString(QUuid::WithoutBraces).toStdString();
}

std::string VaultEntry::getCurrentTimestamp() {
    QDateTime now = QDateTime::currentDateTimeUtc();
    return now.toString(Qt::ISODate).toStdString();
}

std::string VaultEntry::getDeviceFingerprint() {
    // Create device fingerprint from various system properties
    QString deviceInfo;
    
    // Add platform identification
#ifdef _WIN32
    deviceInfo += "WINDOWS_";
#elif __linux__
    deviceInfo += "LINUX_";
#elif __APPLE__
    deviceInfo += "MACOS_";
#else
    deviceInfo += "UNKNOWN_";
#endif
    
    deviceInfo += QSysInfo::machineHostName();
    deviceInfo += QSysInfo::machineUniqueId();
    deviceInfo += QSysInfo::bootUniqueId();
    deviceInfo += QSysInfo::productType();
    deviceInfo += QSysInfo::productVersion();
    deviceInfo += QSysInfo::kernelType();
    deviceInfo += QSysInfo::kernelVersion();
    
    // Hash the device info to create a fingerprint
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(deviceInfo.toUtf8());
    
    return hash.result().toHex().toStdString();
}

} // namespace core
} // namespace crimson
