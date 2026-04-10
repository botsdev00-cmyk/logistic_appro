#ifndef VUEREPARTITION_H
#define VUEREPARTITION_H

#include <QWidget>
#include <memory>

class QComboBox;
class TableauRepartition;

class VueRepartition : public QWidget
{
    Q_OBJECT

public:
    VueRepartition(QWidget* parent = nullptr);
    ~VueRepartition();

private slots:
    void creerRepartition();
    void verifierStatut();
    void chargerRetours();
    void filtrerParStatut();

private:
    void creerWidgets();
    void initialiserConnexions();

    std::unique_ptr<QComboBox> m_comboStatut;
    std::unique_ptr<TableauRepartition> m_tableauRepartition;
};

#endif // VUEREPARTITION_H