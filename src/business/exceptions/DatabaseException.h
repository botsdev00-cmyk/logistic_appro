#ifndef DATABASEEXCEPTION_H
#define DATABASEEXCEPTION_H

#include "AppException.h"

class DatabaseException : public AppException
{
public:
    explicit DatabaseException(const QString& message);
    DatabaseException(const QString& message, const QString& query);
    
    QString getQuery() const { return m_query; }

private:
    QString m_query;
};

#endif // DATABASEEXCEPTION_H