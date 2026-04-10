#include "DatabaseException.h"

DatabaseException::DatabaseException(const QString& message)
    : AppException(message)
{
}

DatabaseException::DatabaseException(const QString& message, const QString& query)
    : AppException(message),
      m_query(query)
{
    // Ajouter la requête au message pour le debugging
    QString fullMessage = message;
    if (!query.isEmpty()) {
        fullMessage += "\nRequête SQL: " + query;
    }
    m_message = fullMessage;
    m_stdMessage = fullMessage.toStdString();
}