#ifndef BUSINESSEXCEPTION_H
#define BUSINESSEXCEPTION_H

#include "AppException.h"

class BusinessException : public AppException
{
public:
    explicit BusinessException(const QString& message);
    BusinessException(const QString& message, int errorCode);
    
    int getErrorCode() const { return m_errorCode; }

private:
    int m_errorCode;
};

#endif // BUSINESSEXCEPTION_H