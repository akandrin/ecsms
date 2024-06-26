#include "selectionmodel.h"
#include "../events/repaintevent.h"

SelectionModel& SelectionModel::operator=(const SelectionModel& other) {
  if (&other == this)
    return *this;

  Clear();

  _map_of_selected_nodes = other._map_of_selected_nodes;
  _selected_blocks = other._selected_blocks;

  IModel::operator=(other);

  return *this;
}

const QMap<NodeId, std::vector<NodeId>>&
SelectionModel::GetSelectedConnections() const {
  return _map_of_selected_nodes;
}

const std::set<BlockId>& SelectionModel::GetSelectedBlocks() const {
  return _selected_blocks;
}

void SelectionModel::AddSelection(const BlockId& block) {
  _selected_blocks.insert(block);
  Notify(std::make_shared<RepaintEvent>());
}

void SelectionModel::RemoveSelection(const BlockId& block) {
  _selected_blocks.erase(block);
  Notify(std::make_shared<RepaintEvent>());
}

void SelectionModel::RemoveSelectionWithNode(const NodeId& node) {
  _map_of_selected_nodes.remove(node);
  for (auto&& start : _map_of_selected_nodes.keys()) {
    for (auto&& end : _map_of_selected_nodes[start]) {
      auto&& connects = _map_of_selected_nodes[start];
      if (auto iter = std::find(connects.begin(), connects.end(), node);
          iter != connects.end())
        connects.erase(iter);
    }
  }
  _selected_blocks.erase(node.GetParentId());
}

void SelectionModel::AddSelection(const NodeId& start, const NodeId& end) {
  _map_of_selected_nodes[start].push_back(end);
  Notify(std::make_shared<RepaintEvent>());
}

void SelectionModel::RemoveSelection(const NodeId& start, const NodeId& end) {
  auto&& iter_to_remove = std::find(_map_of_selected_nodes[start].begin(),
                                    _map_of_selected_nodes[start].end(), end);
  if (iter_to_remove != _map_of_selected_nodes[start].end())
    _map_of_selected_nodes[start].erase(iter_to_remove);
  if (_map_of_selected_nodes[start].empty())
    _map_of_selected_nodes.remove(start);

  Notify(std::make_shared<RepaintEvent>());
}

void SelectionModel::ClearSelection() {
  _map_of_selected_nodes.clear();
  _selected_blocks.clear();
  Notify(std::make_shared<RepaintEvent>());
}

void SelectionModel::Clear() {
  ClearSelection();
  IModel::Clear();
}

SelectionModel::Memento SelectionModel::Save() const {
  Memento res{_map_of_selected_nodes, _selected_blocks};
  return res;
}

void SelectionModel::Load(const SelectionModel::Memento& m) {
  _map_of_selected_nodes = m._map_of_selected_nodes;
  _selected_blocks = m._selected_blocks;

  Notify(std::make_shared<RepaintEvent>());
}
