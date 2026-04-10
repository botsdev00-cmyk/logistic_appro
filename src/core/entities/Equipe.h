#ifndef EQUIPE_H
#define EQUIPE_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class Equipe
{
public:
    Equipe();
    ~Equipe();

    // Getters
    QUuid getEquipeId() const { return m_equipeId; }
    QString getNom() const { return m_nom; }
    QString getNomChef() const { return m_nomChef; }
    QString getTelephoneChef() const { return m_telephoneChef; }
    bool estActif() const { return m_estActif; }
    QDateTime getDateCreation() const { return m_dateCreation; }

    // Setters
    void setEquipeId(const QUuid& id) { m_equipeId = id; }
    void setNom(const QString& nom) { m_nom = nom; }
    void setNomChef(const QString& nom) { m_nomChef = nom; }
    void setTelephoneChef(const QString& tel) { m_telephoneChef = tel; }
    void setEstActif(bool actif) { m_estActif = actif; }
    void setDateCreation(const QDateTime& date) { m_dateCreation = date; }

private:
    QUuid m_equipeId;
    QString m_nom;
    QString m_nomChef;
    QString m_telephoneChef;
    bool m_estActif;
    QDateTime m_dateCreation;
};

#endif // EQUIPE_H