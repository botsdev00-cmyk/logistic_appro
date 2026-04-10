#ifndef APPEXCEPTION_H
#define APPEXCEPTION_H

#include <QString>
#include <exception>

class AppException : public std::exception
{
public:
    explicit AppException(const QString& message);
    virtual ~AppException() = default;

    const char* what() const noexcept override;
    QString getMessage() const;

protected:
    QString m_message;
    mutable std::string m_stdMessage;
};

#endif // APPEXCEPTION_H