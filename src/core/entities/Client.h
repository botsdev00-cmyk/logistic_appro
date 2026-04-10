#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class Client
{
public:
    enum class ConditionsPaiement {
        Cash,
        Credit5Jours,
        Credit7Jours
    };

    Client();
    ~Client();

    // Getters
    QUuid getClientId() const { return m_clientId; }
    QString getNom() const { return m_nom; }
    QString getAdresse() const { return m_adresse; }
    QString getTelephone() const { return m_telephone; }
    QString getEmail() const { return m_email; }
    QUuid getRouteId() const { return m_routeId; }
    QString getPersonneContact() const { return m_personneContact; }
    ConditionsPaiement getConditionsPaiement() const { return m_conditionsPaiement; }
    bool estActif() const { return m_estActif; }
    QDateTime getDateCreation() const { return m_dateCreation; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }

    // Setters
    void setClientId(const QUuid& id) { m_clientId = id; }
    void setNom(const QString& nom) { m_nom = nom; }
    void setAdresse(const QString& adresse) { m_adresse = adresse; }
    void setTelephone(const QString& tel) { m_telephone = tel; }
    void setEmail(const QString& email) { m_email = email; }
    void setRouteId(const QUuid& id) { m_routeId = id; }
    void setPersonneContact(const QString& personne) { m_personneContact = personne; }
    void setConditionsPaiement(ConditionsPaiement conditions) { m_conditionsPaiement = conditions; }
    void setEstActif(bool actif) { m_estActif = actif; }
    void setDateCreation(const QDateTime& date) { m_dateCreation = date; }
    void setDateMiseAJour(const QDateTime& date) { m_dateMiseAJour = date; }

    // Utility methods
    static QString conditionsPaiementToString(ConditionsPaiement conditions);
    static ConditionsPaiement stringToConditionsPaiement(const QString& str);
    QString getConditionsPaiementLabel() const;

private:
    QUuid m_clientId;
    QString m_nom;
    QString m_adresse;
    QString m_telephone;
    QString m_email;
    QUuid m_routeId;
    QString m_personneContact;
    ConditionsPaiement m_conditionsPaiement;
    bool m_estActif;
    QDateTime m_dateCreation;
    QDateTime m_dateMiseAJour;
};

#endif // CLIENT_H