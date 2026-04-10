#include "BusinessException.h"

BusinessException::BusinessException(const QString& message)
    : AppException(message),
      m_errorCode(0)
{
}

BusinessException::BusinessException(const QString& message, int errorCode)
    : AppException(message),
      m_errorCode(errorCode)
{
    // Ajouter le code d'erreur au message
    QString fullMessage = message + " [Code: " + QString::number(errorCode) + "]";
    m_message = fullMessage;
    m_stdMessage = fullMessage.toStdString();
}