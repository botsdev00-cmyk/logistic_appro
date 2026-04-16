#ifndef BOITE_DIALOG_ENTREE_STOCK_H
#define BOITE_DIALOG_ENTREE_STOCK_H

#include <QDialog>
#include <QUuid>
#include <QList>
#include <memory>

class GestionnaireStock;
class Produit;
class QTableWidget;
class QPushButton;
class QLineEdit;
class QLabel;

class BoiteDialogEntreeStock : public QDialog
{
    Q_OBJECT

public:
    explicit BoiteDialogEntreeStock(GestionnaireStock* gestionnaire, 
                                    const QUuid& utilisateurId, 
                                    QWidget* parent = nullptr);
    ~BoiteDialogEntreeStock();

private slots:
    void onAjouterLigne();
    void onSupprimerLigne();
    void onSelectionnerProduit(int row, int col);
    void onValider();
    void onAnnuler();
    void onCellChanged(int row, int col);
    void onImprimer();
    void onExporter();

private:
    void initializeUI();
    void connectSignals();
    void chargerProduits();
    void chargerSources();
    void ajouterLigneVide();
    bool validerLignes();
    bool sauvegarderEntrees();
    bool validerLigne(int row);

    GestionnaireStock* m_gestionnaire;
    QUuid m_utilisateurId;

    // UI Components
    QTableWidget* m_tableWidget;
    QPushButton* m_btnAjouter;
    QPushButton* m_btnSupprimer;
    QPushButton* m_btnValider;
    QPushButton* m_btnAnnuler;
    QPushButton* m_btnImprimer;
    QPushButton* m_btnExporter;
    QLabel* m_labelTotal;
    QLabel* m_labelStatut;
    QLabel* m_labelNbLignes;

    // Données
    QMap<QString, QUuid> m_produitsMap;
    QMap<QString, QUuid> m_sourcesMap;
};

#endif // BOITE_DIALOG_ENTREE_STOCK_H