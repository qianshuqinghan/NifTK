#ifndef XnatModel_h
#define XnatModel_h

#include "XnatRestWidgetsExports.h"

#include <QAbstractItemModel>

#include "XnatNode.h"


class XnatRestWidgets_EXPORT XnatModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  XnatModel(XnatNode* rootNode);
  ~XnatModel();

  QVariant data(const QModelIndex& index, int role) const;
  QModelIndex parent(const QModelIndex& child) const;
  QModelIndex index(int row, int column, const QModelIndex& parent) const;
  int rowCount(const QModelIndex& parent) const;
  int columnCount(const QModelIndex& parent) const;
  bool hasChildren(const QModelIndex& parent) const;
  bool canFetchMore(const QModelIndex& parent) const;
  void fetchMore(const QModelIndex& parent);
  bool removeAllRows(const QModelIndex& parent);

//  inline QVariant name(const QModelIndex& index) const
//  {
//    return data(index, Qt::DisplayRole);
//  }

  void downloadFile(const QModelIndex& index, const QString& zipFilename);
  void downloadFileGroup(const QModelIndex& index, const QString& zipFilename);
  void uploadFile(const QModelIndex& index, const QString& zipFilename);
  void addEntry(const QModelIndex& index, const QString& name);
  void removeEntry(const QModelIndex& index);

  static const int ModifiableChildKind;
  static const int ModifiableParentName;

private:
  XnatNode* root;
};

#endif