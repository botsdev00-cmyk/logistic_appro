#ifndef SERVICEVALIDATION_H
#define SERVICEVALIDATION_H

#include <QString>
#include <QStringList>
#include <QUuid>
#include <QRegularExpression>

class ServiceValidation
{
public:
    ServiceValidation();

    // Email validation
    static bool validateEmail(const QString& email);
    
    // Username validation
    static bool validateUsername(const QString& username);
    
    // Password validation
    static bool validatePassword(const QString& password, QStringList& errors);
    
    // Phone validation
    static bool validatePhone(const QString& phone);
    
    // Numeric validation
    static bool validatePositiveNumber(double number);
    static bool validatePositiveInteger(int number);
    
    // Monetary validation
    static bool validateAmount(double amount);
    
    // UUID validation
    static bool validateUuid(const QUuid& uuid);
    
    // String validation
    static bool validateNotEmpty(const QString& str);
    static bool validateStringLength(const QString& str, int minLength, int maxLength);
    
    // Quantity validation
    static bool validateQuantity(int quantity);
    
    // Get error messages
    static QString getEmailErrorMessage();
    static QString getPasswordErrorMessage(const QStringList& errors);
    static QString getPhoneErrorMessage();
    static QString getAmountErrorMessage();

private:
    static const QRegularExpression EMAIL_REGEX;
    static const QRegularExpression PHONE_REGEX;
    static const QRegularExpression USERNAME_REGEX;
    static const int MIN_PASSWORD_LENGTH;
    static const int MAX_PASSWORD_LENGTH;
};

#endif // SERVICEVALIDATION_H