#ifndef SERVICEAUTHENTIFICATION_H
#define SERVICEAUTHENTIFICATION_H

#include <QString>
#include <QUuid>
#include <memory>

class Utilisateur;

class ServiceAuthentification
{
public:
    ServiceAuthentification();

    // Authentication
    bool authenticate(const QString& username, const QString& password);
    bool logout();
    
    // Password management
    static QString hashPassword(const QString& plainPassword);
    static bool verifyPassword(const QString& plainPassword, const QString& hash);
    
    // Session management
    QUuid getCurrentUserId() const { return m_currentUserId; }
    Utilisateur* getCurrentUser() const { return m_currentUser.get(); }
    bool isAuthenticated() const { return !m_currentUserId.isNull(); }
    
    // Authorization
    bool hasRole(const QString& requiredRole) const;
    bool hasPermission(const QString& permission) const;
    
    // Last error
    QString getLastError() const { return m_lastError; }

private:
    QUuid m_currentUserId;
    std::unique_ptr<Utilisateur> m_currentUser;
    QString m_lastError;
    
    bool validateCredentials(const QString& username, const QString& password);
};

#endif // SERVICEAUTHENTIFICATION_H