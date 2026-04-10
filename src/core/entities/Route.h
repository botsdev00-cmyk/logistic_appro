#ifndef ROUTE_H
#define ROUTE_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class Route
{
public:
    Route();
    ~Route();

    // Getters
    QUuid getRouteId() const { return m_routeId; }
    QString getNom() const { return m_nom; }
    QString getDescription() const { return m_description; }
    bool estActif() const { return m_estActif; }
    QDateTime getDateCreation() const { return m_dateCreation; }

    // Setters
    void setRouteId(const QUuid& id) { m_routeId = id; }
    void setNom(const QString& nom) { m_nom = nom; }
    void setDescription(const QString& desc) { m_description = desc; }
    void setEstActif(bool actif) { m_estActif = actif; }
    void setDateCreation(const QDateTime& date) { m_dateCreation = date; }

private:
    QUuid m_routeId;
    QString m_nom;
    QString m_description;
    bool m_estActif;
    QDateTime m_dateCreation;
};

#endif // ROUTE_H