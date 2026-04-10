#ifndef VALIDATIONEXCEPTION_H
#define VALIDATIONEXCEPTION_H

#include "AppException.h"
#include <QStringList>

class ValidationException : public AppException
{
public:
    explicit ValidationException(const QString& message);
    ValidationException(const QString& message, const QStringList& errors);
    
    QStringList getErrors() const { return m_errors; }

private:
    QStringList m_errors;
};

#endif // VALIDATIONEXCEPTION_H