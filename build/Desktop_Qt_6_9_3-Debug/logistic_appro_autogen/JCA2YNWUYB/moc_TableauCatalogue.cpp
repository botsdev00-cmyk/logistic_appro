/****************************************************************************
** Meta object code from reading C++ file 'TableauCatalogue.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/ui/widgets/TableauCatalogue.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TableauCatalogue.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN16TableauCatalogueE_t {};
} // unnamed namespace

template <> constexpr inline auto TableauCatalogue::qt_create_metaobjectdata<qt_meta_tag_ZN16TableauCatalogueE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "TableauCatalogue",
        "produitAjoute",
        "",
        "produitModifie",
        "produitSupprime",
        "categorieAjoutee",
        "categorieModifiee",
        "categorieSupprimee",
        "onRecherche",
        "texte",
        "onAjouterProduit",
        "onAjouterCategorie",
        "onMenuContextuel",
        "position",
        "onModifierProduit",
        "produitId",
        "onBasculeStatut",
        "onSupprimerProduit",
        "onModifierCategorie",
        "categorieId",
        "onBasculeStatutCategorie",
        "onSupprimerCategorie"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'produitAjoute'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'produitModifie'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'produitSupprime'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'categorieAjoutee'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'categorieModifiee'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'categorieSupprimee'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onRecherche'
        QtMocHelpers::SlotData<void(const QString &)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Slot 'onAjouterProduit'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAjouterCategorie'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMenuContextuel'
        QtMocHelpers::SlotData<void(const QPoint &)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPoint, 13 },
        }}),
        // Slot 'onModifierProduit'
        QtMocHelpers::SlotData<void(const QUuid &)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QUuid, 15 },
        }}),
        // Slot 'onBasculeStatut'
        QtMocHelpers::SlotData<void(const QUuid &)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QUuid, 15 },
        }}),
        // Slot 'onSupprimerProduit'
        QtMocHelpers::SlotData<void(const QUuid &)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QUuid, 15 },
        }}),
        // Slot 'onModifierCategorie'
        QtMocHelpers::SlotData<void(const QUuid &)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QUuid, 19 },
        }}),
        // Slot 'onBasculeStatutCategorie'
        QtMocHelpers::SlotData<void(const QUuid &)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QUuid, 19 },
        }}),
        // Slot 'onSupprimerCategorie'
        QtMocHelpers::SlotData<void(const QUuid &)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QUuid, 19 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TableauCatalogue, qt_meta_tag_ZN16TableauCatalogueE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject TableauCatalogue::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16TableauCatalogueE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16TableauCatalogueE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16TableauCatalogueE_t>.metaTypes,
    nullptr
} };

void TableauCatalogue::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TableauCatalogue *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->produitAjoute(); break;
        case 1: _t->produitModifie(); break;
        case 2: _t->produitSupprime(); break;
        case 3: _t->categorieAjoutee(); break;
        case 4: _t->categorieModifiee(); break;
        case 5: _t->categorieSupprimee(); break;
        case 6: _t->onRecherche((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->onAjouterProduit(); break;
        case 8: _t->onAjouterCategorie(); break;
        case 9: _t->onMenuContextuel((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 10: _t->onModifierProduit((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 11: _t->onBasculeStatut((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 12: _t->onSupprimerProduit((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 13: _t->onModifierCategorie((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 14: _t->onBasculeStatutCategorie((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 15: _t->onSupprimerCategorie((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TableauCatalogue::*)()>(_a, &TableauCatalogue::produitAjoute, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TableauCatalogue::*)()>(_a, &TableauCatalogue::produitModifie, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TableauCatalogue::*)()>(_a, &TableauCatalogue::produitSupprime, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TableauCatalogue::*)()>(_a, &TableauCatalogue::categorieAjoutee, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (TableauCatalogue::*)()>(_a, &TableauCatalogue::categorieModifiee, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (TableauCatalogue::*)()>(_a, &TableauCatalogue::categorieSupprimee, 5))
            return;
    }
}

const QMetaObject *TableauCatalogue::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TableauCatalogue::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16TableauCatalogueE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int TableauCatalogue::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void TableauCatalogue::produitAjoute()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void TableauCatalogue::produitModifie()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void TableauCatalogue::produitSupprime()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void TableauCatalogue::categorieAjoutee()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void TableauCatalogue::categorieModifiee()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void TableauCatalogue::categorieSupprimee()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
