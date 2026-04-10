#ifndef ENTREESTOCK_H
#define ENTREESTOCK_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class EntreeStock
{
public:
    enum class Source {
        Production,
        Retour,
        Ajustement
    };

    EntreeStock();
    ~EntreeStock();

    // Getters
    QUuid getEntreeStockId() const { return m_entreeStockId; }
    QUuid getProduitId() const { return m_produitId; }
    int getQuantite() const { return m_quantite; }
    Source getSource() const { return m_source; }
    QUuid getIdentifiantReference() const { return m_identifiantReference; }
    QDateTime getDate() const { return m_date; }
    QUuid getCreePar() const { return m_creePar; }
    QString getNotes() const { return m_notes; }

    // Setters
    void setEntreeStockId(const QUuid& id) { m_entreeStockId = id; }
    void setProduitId(const QUuid& id) { m_produitId = id; }
    void setQuantite(int qty) { m_quantite = qty; }
    void setSource(Source source) { m_source = source; }
    void setIdentifiantReference(const QUuid& id) { m_identifiantReference = id; }
    void setDate(const QDateTime& date) { m_date = date; }
    void setCreePar(const QUuid& id) { m_creePar = id; }
    void setNotes(const QString& notes) { m_notes = notes; }

    // Utility methods
    static QString sourceToString(Source source);
    static Source stringToSource(const QString& str);
    QString getSourceLabel() const;

private:
    QUuid m_entreeStockId;
    QUuid m_produitId;
    int m_quantite;
    Source m_source;
    QUuid m_identifiantReference;
    QDateTime m_date;
    QUuid m_creePar;
    QString m_notes;
};

#endif // ENTREESTOCK_H