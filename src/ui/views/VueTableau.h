#ifndef VUETABLEAU_H
#define VUETABLEAU_H

#include <QWidget>
#include <memory>

class QLabel;
class QProgressBar;
class QLCDNumber;

class VueTableau : public QWidget
{
    Q_OBJECT

public:
    VueTableau(QWidget* parent = nullptr);
    ~VueTableau();

private slots:
    void rafraichirDonnees();
    void afficherAlertes();

private:
    void creerWidgets();
    void initialiserConnexions();
    void mettreAJourStatistiques();

    std::unique_ptr<QLCDNumber> m_lcdStockTotal;
    std::unique_ptr<QLCDNumber> m_lcdVentesJour;
    std::unique_ptr<QLCDNumber> m_lcdCreditsEnCours;
    std::unique_ptr<QLCDNumber> m_lcdCreditEnRetard;
    std::unique_ptr<QLabel> m_labelStockBas;
    std::unique_ptr<QLabel> m_labelAlertes;
    std::unique_ptr<QProgressBar> m_progressbar;
};

#endif // VUETABLEAU_H