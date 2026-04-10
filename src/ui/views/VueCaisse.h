#ifndef VUECAISSE_H
#define VUECAISSE_H

#include <QWidget>
#include <memory>

class QLabel;
class QPushButton;

class VueCaisse : public QWidget
{
    Q_OBJECT

public:
    VueCaisse(QWidget* parent = nullptr);
    ~VueCaisse();

private slots:
    void enregistrerReception();
    void afficherHistorique();
    void genererRapportCaisse();
    void afficherDiscrepances();

private:
    void creerWidgets();
    void initialiserConnexions();
    void mettreAJourStatistiques();

    std::unique_ptr<QLabel> m_labelMontantJour;
    std::unique_ptr<QLabel> m_labelMontantAttente;
    std::unique_ptr<QLabel> m_labelDiscrepances;
};

#endif // VUECAISSE_H