#ifndef UTILISATEUR_H
#define UTILISATEUR_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class Utilisateur
{
public:
    enum class Role {
        Admin,
        Logistique,
        Caissier,
        Gestionnaire
    };

    Utilisateur();
    ~Utilisateur();

    // Getters
    QUuid getUtilisateurId() const { return m_utilisateurId; }
    QString getNomUtilisateur() const { return m_nomUtilisateur; }
    QString getEmail() const { return m_email; }
    QString getHashMotPasse() const { return m_hashMotPasse; }
    QString getNomComplet() const { return m_nomComplet; }
    Role getRole() const { return m_role; }
    bool estActif() const { return m_estActif; }
    QDateTime getDateCreation() const { return m_dateCreation; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }

    // Setters
    void setUtilisateurId(const QUuid& id) { m_utilisateurId = id; }
    void setNomUtilisateur(const QString& nom) { m_nomUtilisateur = nom; }
    void setEmail(const QString& email) { m_email = email; }
    void setHashMotPasse(const QString& hash) { m_hashMotPasse = hash; }
    void setNomComplet(const QString& nom) { m_nomComplet = nom; }
    void setRole(Role role) { m_role = role; }
    void setEstActif(bool actif) { m_estActif = actif; }
    void setDateCreation(const QDateTime& date) { m_dateCreation = date; }
    void setDateMiseAJour(const QDateTime& date) { m_dateMiseAJour = date; }

    // Utility methods
    static QString roleToString(Role role);
    static Role stringToRole(const QString& roleStr);
    QString getRoleLabel() const;

private:
    QUuid m_utilisateurId;
    QString m_nomUtilisateur;
    QString m_email;
    QString m_hashMotPasse;
    QString m_nomComplet;
    Role m_role;
    bool m_estActif;
    QDateTime m_dateCreation;
    QDateTime m_dateMiseAJour;
};

#endif // UTILISATEUR_H