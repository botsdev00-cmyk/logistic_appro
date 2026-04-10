#include "AppException.h"

AppException::AppException(const QString& message)
    : m_message(message)
{
    m_stdMessage = message.toStdString();
}

const char* AppException::what() const noexcept
{
    return m_stdMessage.c_str();
}

QString AppException::getMessage() const
{
    return m_message;
}