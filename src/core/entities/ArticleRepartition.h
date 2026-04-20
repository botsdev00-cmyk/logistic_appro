#ifndef ARTICLEREPARTITION_H
#define ARTICLEREPARTITION_H

#include <QUuid>
#include <QString>

class ArticleRepartition
{
public:
    ArticleRepartition();
    ~ArticleRepartition();

    QUuid getArticleRepartitionId() const { return m_articleRepartitionId; }
    QUuid getRepartitionId() const { return m_repartitionId; }
    QUuid getProduitId() const { return m_produitId; }
    int getQuantiteVente() const { return m_quantiteVente; }
    int getQuantiteCadeau() const { return m_quantiteCadeau; }
    int getQuantiteDegustation() const { return m_quantiteDegustation; }
    int getQuantiteTotale() const { return m_quantiteVente + m_quantiteCadeau + m_quantiteDegustation; }
    QString getObservation() const { return m_observation; }

    void setArticleRepartitionId(const QUuid& id) { m_articleRepartitionId = id; }
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }
    void setProduitId(const QUuid& id) { m_produitId = id; }
    void setQuantiteVente(int qty) { m_quantiteVente = qty; }
    void setQuantiteCadeau(int qty) { m_quantiteCadeau = qty; }
    void setQuantiteDegustation(int qty) { m_quantiteDegustation = qty; }
    void setObservation(const QString& obs) { m_observation = obs; }

private:
    QUuid m_articleRepartitionId;
    QUuid m_repartitionId;
    QUuid m_produitId;
    int m_quantiteVente = 0;
    int m_quantiteCadeau = 0;
    int m_quantiteDegustation = 0;
    QString m_observation;
};

#endif // ARTICLEREPARTITION_H