#include "ValidationException.h"

ValidationException::ValidationException(const QString& message)
    : AppException(message)
{
}

ValidationException::ValidationException(const QString& message, const QStringList& errors)
    : AppException(message),
      m_errors(errors)
{
    // Ajouter les erreurs au message principal
    if (!errors.isEmpty()) {
        QString fullMessage = message + "\n";
        for (const QString& error : errors) {
            fullMessage += "  • " + error + "\n";
        }
        m_message = fullMessage;
        m_stdMessage = fullMessage.toStdString();
    }
}