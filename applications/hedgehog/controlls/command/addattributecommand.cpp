#include "addattributecommand.h"

AddAttributeCommand::AddAttributeCommand(
    int row_to_insert, QStandardItemModel *table_view_model,
    QItemSelectionModel *table_selection_model)
    : _row_to_insert(row_to_insert),
      _table_view_model(table_view_model),
      _table_selection_model(table_selection_model) {}

void AddAttributeCommand::Execute() {
  QStandardItem *parent_tag = _table_view_model->invisibleRootItem();
  parent_tag->insertRow(_row_to_insert, {new QStandardItem("attribute_name"),
                                        new QStandardItem("attribute_value")});
  auto &&index_to_insert = _table_view_model->index(_row_to_insert, 0);
  _table_selection_model->select(index_to_insert,
                                 QItemSelectionModel::ClearAndSelect);
}

void AddAttributeCommand::UnExecute() {
  int row_to_insert = _table_view_model->rowCount();
  QStandardItem *parent_tag = _table_view_model->invisibleRootItem();
  parent_tag->removeRow(row_to_insert - 1);
}
