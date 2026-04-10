#include "ServiceValidation.h"
#include <QDebug>

const QRegularExpression ServiceValidation::EMAIL_REGEX(
    "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"
);

const QRegularExpression ServiceValidation::PHONE_REGEX(
    "^[+]?[0-9\\s\\-\\(\\)]{10,}$"
);

const QRegularExpression ServiceValidation::USERNAME_REGEX(
    "^[a-zA-Z0-9_]{3,50}$"
);

const int ServiceValidation::MIN_PASSWORD_LENGTH = 8;
const int ServiceValidation::MAX_PASSWORD_LENGTH = 255;

ServiceValidation::ServiceValidation()
{
}

bool ServiceValidation::validateEmail(const QString& email)
{
    if (email.isEmpty()) {
        return false;
    }
    
    QRegularExpressionMatch match = EMAIL_REGEX.match(email);
    return match.hasMatch();
}

bool ServiceValidation::validateUsername(const QString& username)
{
    if (username.isEmpty()) {
        return false;
    }
    
    QRegularExpressionMatch match = USERNAME_REGEX.match(username);
    return match.hasMatch();
}

bool ServiceValidation::validatePassword(const QString& password, QStringList& errors)
{
    errors.clear();
    
    if (password.length() < MIN_PASSWORD_LENGTH) {
        errors << QString("Le mot de passe doit contenir au moins %1 caractères").arg(MIN_PASSWORD_LENGTH);
    }
    
    if (!password.contains(QRegularExpression("[A-Z]"))) {
        errors << "Le mot de passe doit contenir au moins une majuscule";
    }
    
    if (!password.contains(QRegularExpression("[a-z]"))) {
        errors << "Le mot de passe doit contenir au moins une minuscule";
    }
    
    if (!password.contains(QRegularExpression("[0-9]"))) {
        errors << "Le mot de passe doit contenir au moins un chiffre";
    }
    
    if (!password.contains(QRegularExpression("[!@#$%^&*()_+-=\\[\\]{};:',.<>?]"))) {
        errors << "Le mot de passe doit contenir au moins un caractère spécial";
    }
    
    return errors.isEmpty();
}

bool ServiceValidation::validatePhone(const QString& phone)
{
    if (phone.isEmpty()) {
        return false;
    }
    
    QRegularExpressionMatch match = PHONE_REGEX.match(phone);
    return match.hasMatch();
}

bool ServiceValidation::validatePositiveNumber(double number)
{
    return number > 0.0;
}

bool ServiceValidation::validatePositiveInteger(int number)
{
    return number > 0;
}

bool ServiceValidation::validateAmount(double amount)
{
    return amount > 0.0 && amount < 999999.99;
}

bool ServiceValidation::validateUuid(const QUuid& uuid)
{
    return !uuid.isNull();
}

bool ServiceValidation::validateNotEmpty(const QString& str)
{
    return !str.isEmpty() && !str.trimmed().isEmpty();
}

bool ServiceValidation::validateStringLength(const QString& str, int minLength, int maxLength)
{
    int length = str.length();
    return length >= minLength && length <= maxLength;
}

bool ServiceValidation::validateQuantity(int quantity)
{
    return quantity > 0;
}

QString ServiceValidation::getEmailErrorMessage()
{
    return "Veuillez entrer une adresse email valide";
}

QString ServiceValidation::getPasswordErrorMessage(const QStringList& errors)
{
    if (errors.isEmpty()) {
        return "";
    }
    return "Mot de passe invalide :\n- " + errors.join("\n- ");
}

QString ServiceValidation::getPhoneErrorMessage()
{
    return "Veuillez entrer un numéro de téléphone valide";
}

QString ServiceValidation::getAmountErrorMessage()
{
    return "Veuillez entrer un montant valide (entre 0 et 999999.99)";
}