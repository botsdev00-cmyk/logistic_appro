#ifndef TABLEAUCATALOGUE_H
#define TABLEAUCATALOGUE_H

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QUuid>
#include "../../business/managers/GestionnaireCatalogue.h"
#include "../dialogs/BoiteDialogCategorie.h"
#include "../dialogs/BoiteDialogProduit.h"

class TableauCatalogue : public QWidget
{
    Q_OBJECT

public:
    explicit TableauCatalogue(QWidget* parent = nullptr);
    explicit TableauCatalogue(GestionnaireCatalogue* gestionnaire, const QUuid& utilisateurId, QWidget* parent = nullptr);
    ~TableauCatalogue();

    void setGestionnaireCatalogue(GestionnaireCatalogue* gestionnaire);
    void setUtilisateurId(const QUuid& utilisateurId);

    void rafraichir();

signals:
    void produitAjoute();
    void produitModifie();
    void produitSupprime();
    void categorieAjoutee();
    void categorieModifiee();
    void categorieSupprimee();

private slots:
    void onRecherche(const QString& texte);
    void onAjouterProduit();
    void onAjouterCategorie();
    void onMenuContextuel(const QPoint& position);
    void onModifierProduit(const QUuid& produitId);
    void onBasculeStatut(const QUuid& produitId);
    void onSupprimerProduit(const QUuid& produitId);
    void onModifierCategorie(const QUuid& categorieId);
    void onBasculeStatutCategorie(const QUuid& categorieId);
    void onSupprimerCategorie(const QUuid& categorieId);

private:
    void initializeUI();
    void chargerProduits();
    void chargerCategories();

    GestionnaireCatalogue* m_gestionnaireCatalogue;
    QUuid m_utilisateurId;
    QTableView* m_tableauProduits;
    QTableView* m_tableauCategories;
    QLineEdit* m_champRecherche;
    QStandardItemModel* m_modeleTableau;
    QStandardItemModel* m_modeleCategories;
    QSortFilterProxyModel* m_proxyModele;
    QSortFilterProxyModel* m_proxyModeleCategories;
};

#endif // TABLEAUCATALOGUE_H