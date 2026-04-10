#ifndef VUERAPPORT_H
#define VUERAPPORT_H

#include <QWidget>
#include <memory>

class QComboBox;
class QDateEdit;

class VueRapport : public QWidget
{
    Q_OBJECT

public:
    VueRapport(QWidget* parent = nullptr);
    ~VueRapport();

private slots:
    void genererRapportJour();
    void genererRapportMois();
    void genererRapportPersonnalise();
    void exporterEnPDF();
    void exporterEnCSV();

private:
    void creerWidgets();
    void initialiserConnexions();

    std::unique_ptr<QComboBox> m_comboType;
    std::unique_ptr<QDateEdit> m_dateDebut;
    std::unique_ptr<QDateEdit> m_dateFin;
};

#endif // VUERAPPORT_H