#include "ValidateurEntrees.h"
#include <QRegularExpression>
#include <QDate>
#include <QDebug>

// Initialisation du membre statique
QString ValidateurEntrees::m_lastError;

bool ValidateurEntrees::validerEmail(const QString& email)
{
    if (email.isEmpty()) {
        setError("L'email ne peut pas être vide");
        return false;
    }

    // Regex pour valider une adresse email
    QRegularExpression emailRegex(
        R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
    );

    if (!emailRegex.match(email).hasMatch()) {
        setError("L'email n'est pas au bon format");
        return false;
    }

    clearError();
    return true;
}

bool ValidateurEntrees::validerMotDePasse(const QString& password, QStringList& erreurs)
{
    erreurs.clear();

    if (password.isEmpty()) {
        erreurs << "Le mot de passe ne peut pas être vide";
        setError("Le mot de passe ne peut pas être vide");
        return false;
    }

    if (password.length() < MIN_PASSWORD_LENGTH) {
        erreurs << QString("Le mot de passe doit contenir au moins %1 caractères")
                   .arg(MIN_PASSWORD_LENGTH);
    }

    if (!password.contains(QRegularExpression("[A-Z]"))) {
        erreurs << "Le mot de passe doit contenir au moins une majuscule (A-Z)";
    }

    if (!password.contains(QRegularExpression("[a-z]"))) {
        erreurs << "Le mot de passe doit contenir au moins une minuscule (a-z)";
    }

    if (!password.contains(QRegularExpression("[0-9]"))) {
        erreurs << "Le mot de passe doit contenir au moins un chiffre (0-9)";
    }

    if (!password.contains(QRegularExpression("[!@#$%^&*()_+\\-=\\[\\]{};:'\",.<>?/\\\\|`~]"))) {
        erreurs << "Le mot de passe doit contenir au moins un caractère spécial";
    }

    if (!erreurs.isEmpty()) {
        setError(erreurs.join("\n"));
        return false;
    }

    clearError();
    return true;
}

bool ValidateurEntrees::validerNomUtilisateur(const QString& username)
{
    if (username.isEmpty()) {
        setError("Le nom d'utilisateur ne peut pas être vide");
        return false;
    }

    if (username.length() < MIN_USERNAME_LENGTH) {
        setError(QString("Le nom d'utilisateur doit contenir au moins %1 caractères")
                 .arg(MIN_USERNAME_LENGTH));
        return false;
    }

    if (username.length() > MAX_USERNAME_LENGTH) {
        setError(QString("Le nom d'utilisateur ne doit pas dépasser %1 caractères")
                 .arg(MAX_USERNAME_LENGTH));
        return false;
    }

    // Regex: alphanumériques et underscores uniquement
    QRegularExpression usernameRegex(R"(^[a-zA-Z0-9_]+$)");

    if (!usernameRegex.match(username).hasMatch()) {
        setError("Le nom d'utilisateur ne peut contenir que des lettres, chiffres et underscores");
        return false;
    }

    clearError();
    return true;
}

bool ValidateurEntrees::validerTelephone(const QString& phone)
{
    if (phone.isEmpty()) {
        setError("Le numéro de téléphone ne peut pas être vide");
        return false;
    }

    // Supprimer les espaces, tirets et parenthèses
    QString cleanPhone = phone;
    cleanPhone.remove(' ');
    cleanPhone.remove('-');
    cleanPhone.remove('(');
    cleanPhone.remove(')');

    // Vérifier la longueur
    if (cleanPhone.length() < MIN_PHONE_LENGTH || cleanPhone.length() > MAX_PHONE_LENGTH) {
        setError(QString("Le numéro de téléphone doit contenir entre %1 et %2 chiffres")
                 .arg(MIN_PHONE_LENGTH).arg(MAX_PHONE_LENGTH));
        return false;
    }

    // Regex: commence par + ou chiffre, puis chiffres uniquement
    QRegularExpression phoneRegex(R"(^(\+|0)[0-9]{9,19}$)");

    if (!phoneRegex.match(cleanPhone).hasMatch()) {
        setError("Le numéro de téléphone n'est pas au bon format");
        return false;
    }

    clearError();
    return true;
}

bool ValidateurEntrees::validerURL(const QString& url)
{
    if (url.isEmpty()) {
        setError("L'URL ne peut pas être vide");
        return false;
    }

    QRegularExpression urlRegex(
        R"(^https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*)$)"
    );

    if (!urlRegex.match(url).hasMatch()) {
        setError("L'URL n'est pas au bon format");
        return false;
    }

    clearError();
    return true;
}

bool ValidateurEntrees::validerMontant(double montant, double min, double max)
{
    if (montant < min || montant > max) {
        setError(QString("Le montant doit être entre %1 et %2")
                 .arg(min).arg(max));
        return false;
    }

    clearError();
    return true;
}

bool ValidateurEntrees::validerQuantite(int quantite, int min, int max)
{
    if (quantite < min || quantite > max) {
        setError(QString("La quantité doit être entre %1 et %2")
                 .arg(min).arg(max));
        return false;
    }

    clearError();
    return true;
}

bool ValidateurEntrees::validerNonVide(const QString& str)
{
    if (str.isEmpty() || str.trimmed().isEmpty()) {
        setError("Le champ ne peut pas être vide");
        return false;
    }

    clearError();
    return true;
}

bool ValidateurEntrees::validerLongueur(const QString& str, int min, int max)
{
    int length = str.length();

    if (length < min || length > max) {
        setError(QString("La longueur doit être entre %1 et %2 caractères")
                 .arg(min).arg(max));
        return false;
    }

    clearError();
    return true;
}

bool ValidateurEntrees::validerDate(const QString& dateStr)
{
    if (dateStr.isEmpty()) {
        setError("La date ne peut pas être vide");
        return false;
    }

    QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");

    if (!date.isValid()) {
        setError("La date n'est pas au bon format (YYYY-MM-DD)");
        return false;
    }

    clearError();
    return true;
}

bool ValidateurEntrees::validerCodePostal(const QString& codePostal)
{
    if (codePostal.isEmpty()) {
        setError("Le code postal ne peut pas être vide");
        return false;
    }

    // Accepte les codes postaux avec tirets ou sans (ex: 75001 ou 75-001)
    QRegularExpression codePostalRegex(R"(^[0-9]{2,6}(-?[0-9]{2,4})?$)");

    if (!codePostalRegex.match(codePostal).hasMatch()) {
        setError("Le code postal n'est pas au bon format");
        return false;
    }

    clearError();
    return true;
}

QString ValidateurEntrees::getLastError()
{
    return m_lastError;
}

void ValidateurEntrees::clearError()
{
    m_lastError.clear();
}

void ValidateurEntrees::setError(const QString& message)
{
    m_lastError = message;
    qWarning() << "[VALIDATION]" << message;
}