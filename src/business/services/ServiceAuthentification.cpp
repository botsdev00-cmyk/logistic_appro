#include "ServiceAuthentification.h"
#include "../exceptions/ValidationException.h"
#include "../../data/repositories/RepositoryUtilisateur.h"
#include "../../core/entities/Utilisateur.h"
#include <QDebug>
#include <memory>
#include <cstring>
#include <../../vendor/bcrypt/bcrypt.h>

ServiceAuthentification::ServiceAuthentification()
    : m_currentUserId(QUuid())
{
    // Rien à initialiser pour bcrypt
}

bool ServiceAuthentification::authenticate(const QString& username, const QString& password)
{
    try {
        if (username.isEmpty() || password.isEmpty()) {
            m_lastError = "Nom d'utilisateur et mot de passe requis";
            return false;
        }

        RepositoryUtilisateur userRepo;
        Utilisateur user = userRepo.getByUsername(username);

        if (user.getUtilisateurId().isNull()) {
            m_lastError = "Utilisateur non trouvé";
            return false;
        }

        if (!user.estActif()) {
            m_lastError = "Utilisateur inactif";
            return false;
        }

        qDebug() << "[AUTH] Username:" << username;
        qDebug() << "[AUTH] Password saisi:" << password;
        qDebug() << "[AUTH] Hash BDD:" << user.getHashMotPasse();

        if (!verifyPassword(password, user.getHashMotPasse())) {
            m_lastError = "Mot de passe incorrect";
            return false;
        }

        m_currentUserId = user.getUtilisateurId();
        m_currentUser = std::make_unique<Utilisateur>(user);
        m_lastError = "";

        qDebug() << "Authentification réussie pour" << username;
        return true;

    } catch (const std::exception& e) {
        m_lastError = QString::fromStdString(e.what());
        return false;
    }
}

bool ServiceAuthentification::logout()
{
    m_currentUserId = QUuid();
    m_currentUser.reset();
    m_lastError = "";
    return true;
}

QString ServiceAuthentification::hashPassword(const QString& plainPassword)
{
    QByteArray pwdUtf8 = plainPassword.toUtf8();
    char salt[64] = {0};
    char hash[128] = {0};

    // Génère un salt bcrypt (workfactor 12)
    if (bcrypt_gensalt(12, salt, sizeof(salt)) != 0) {
        qWarning() << "[BCRYPT] Erreur génération salt";
        return QString();
    }

    if (bcrypt_hashpw(pwdUtf8.constData(), salt, hash, sizeof(hash)) != 0) {
        qWarning() << "[BCRYPT] Erreur génération hash";
        return QString();
    }

    qDebug() << "[BCRYPT] Mot de passe hashé:" << QString::fromUtf8(hash);

    return QString::fromUtf8(hash);
}

bool ServiceAuthentification::verifyPassword(const QString& plainPassword, const QString& hash)
{
    if (plainPassword.isEmpty() || hash.isEmpty()) {
        qDebug() << "[BCRYPT] Paramètres vides!";
        return false;
    }

    QByteArray pwdUtf8 = plainPassword.toUtf8();
    QByteArray hashUtf8 = hash.toUtf8();

    qDebug() << "[BCRYPT] Vérification bcrypt_checkpw --";
    qDebug() << "[BCRYPT] password =" << pwdUtf8;
    qDebug() << "[BCRYPT] hash     =" << hashUtf8;

    int result = bcrypt_checkpw(pwdUtf8.constData(), hashUtf8.constData());
    qDebug() << "[BCRYPT] Résultat =" << result;

    return result == 0;
}

bool ServiceAuthentification::hasRole(const QString& requiredRole) const
{
    if (!m_currentUser) {
        return false;
    }

    return Utilisateur::roleToString(m_currentUser->getRole()) == requiredRole.toLower();
}

bool ServiceAuthentification::hasPermission(const QString& permission) const
{
    if (!isAuthenticated()) {
        return false;
    }

    QString role = Utilisateur::roleToString(m_currentUser->getRole());

    if (role == "admin") {
        return true;
    }

    if (role == "logistique") {
        return permission.contains("dispatch") ||
               permission.contains("stock") ||
               permission.contains("sales");
    }

    if (role == "caissier") {
        return permission.contains("cash") ||
               permission.contains("credit");
    }

    if (role == "gestionnaire") {
        return permission.contains("report") ||
               permission.contains("analytics");
    }

    return false;
}
