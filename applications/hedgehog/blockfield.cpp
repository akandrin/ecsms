#include "blockfield.h"
#include "connectnodewidget.h"
#include "controlls/defaultcontroller.h"
#include "controlls/drawlinecontroller.h"
#include "events/changeactivenodeevent.h"
#include "events/changecontrollerevent.h"
#include "events/drawevent.h"
#include "events/mypaintevent.h"
#include "events/repaintevent.h"
#include "models/nodetype.h"
#include "namemaker/blocknamemaker.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <set>

BlockField::BlockField(QWidget *parent) : QWidget(parent) {
  setMouseTracking(true);
  setFocus(Qt::FocusReason::ActiveWindowFocusReason);
  _controller.reset(new DefaultController(_field_model, _selection_model,
                                          _line_model, _active_nodes_model));
  _field_model.Subscribe(this);
  _selection_model.Subscribe(this);
  _line_model.Subscribe(this);
  _active_nodes_model.Subscribe(this);
}

void BlockField::AddNewBlock() {
  auto default_block =
      new BlockWidget(_block_name_maker.MakeName(), _controller, this);
  default_block->show();
  default_block->move(rect().center());
  auto &&left_p = default_block->GetLeftNode()->getCenterCoord();
  auto &&right_p = default_block->GetRightNode()->getCenterCoord();
  qDebug() << "left node coords: " << left_p;
  qDebug() << "right node coords: " << right_p;

  FieldModel::BlockData block_data = {
      default_block->pos(),
      {
          {NodeType::Incoming, left_p},
          {NodeType::Outgoing, right_p},
      }};
  QMap<NodeType, FieldModel::NodeData> node_data_map = {
      {NodeType::Incoming, {NodeType::Incoming}},
      {NodeType::Outgoing, {NodeType::Outgoing}}};
  _field_model.AddBlock(default_block->GetId(), block_data, node_data_map);
}

void BlockField::Update(std::shared_ptr<Event> e) {
  switch (e->GetEventType()) {
  case drawEvent: {
    auto &&draw_e = std::static_pointer_cast<DrawEvent>(e);
    switch (draw_e->GetDrawEventType()) {
    case repaintEvent: {
      repaint();
      break;
    }
    default: {
      assert(false);
      break;
    }
    }
    break;
  }
  case changeControllerEvent: {
    auto &&change_ctr_e = std::static_pointer_cast<ChangeControllerEvent>(e);
    switch (change_ctr_e->GetControllerType()) {
    case drawLineController: {
      _controller.reset(new DrawLineController(_field_model, _line_model,
                                               _active_nodes_model));
      break;
    }
    case defaultController: {
      _controller.reset(new DefaultController(
          _field_model, _selection_model, _line_model, _active_nodes_model));
      break;
    }
    default: {
      assert(false);
      break;
    }
    }
    break;
  }
  case changeActiveNodeEvent: {
    auto &&change_e = std::static_pointer_cast<ChangeActiveNodeEvent>(e);
    auto &&node_id = change_e->GetNode();
    if (auto &&node = qobject_cast<ConnectNodeWidget *>(FindById(node_id))) {
      node->makeTransparent(!change_e->GetActivity());
    } else {
      assert(false);
      return;
    }
    break;
  }
  default: {
    assert(false);
    break;
  }
  }
}

std::unique_ptr<IController> &BlockField::GetController() {
  return _controller;
}

QWidget *BlockField::FindById(Id id) {
  QWidget *res = nullptr;
  for (auto &&child : children()) {
    if (auto &&block = qobject_cast<BlockWidget *>(child)) {
      if (block->GetId() == id) {
        res = block;
        break;
      } else if (auto &&node = block->FindById(id)) {
        res = node;
        break;
      }
    }
  }
  return res;
}

void BlockField::mouseMoveEvent(QMouseEvent *event) {
  _controller->onMouseMoveEvent(this, event);
}

void BlockField::mousePressEvent(QMouseEvent *event) {
  _controller->onMousePressEvent(this, event);
}

void BlockField::keyPressEvent(QKeyEvent *event) {
  _controller->onKeyPressEvent(this, event);
}

void BlockField::enterEvent(QEvent *event) {
  _controller->onEnterEvent(this, event);
}

void BlockField::leaveEvent(QEvent *event) {
  _controller->onLeaveEvent(this, event);
}

void BlockField::mouseReleaseEvent(QMouseEvent *event) {
  _controller->onMouseReleaseEvent(this, event);
}

void BlockField::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.eraseRect(rect());
  p.setBackground(QBrush(Qt::white));

  /*-DRAW SELECTED AND NOT SELECTED LINES-*/
  std::vector<QLineF> unselected_lines, selected_lines;
  auto &&_connection_map = _field_model.GetConnectionMap();
  for (auto &&start_id : _connection_map.keys()) {
    for (auto &&end_id : _connection_map[start_id]) {
      auto &&start_data = _field_model.GetNodeData(start_id);
      if (!start_data) {
        assert(false);
        return;
      }

      auto &&end_data = _field_model.GetNodeData(end_id);
      if (!end_data) {
        assert(false);
        return;
      }

      NodeType start_type = start_data->node_type;
      NodeType end_type = end_data->node_type;

       QPoint start_pos, end_pos;
      if (auto &&start_pd = _field_model.GetBlockData(start_id.GetParentId())) {
        start_pos = start_pd->pos + start_pd->offset[start_type];
      }

      if (auto &&end_pd = _field_model.GetBlockData(end_id.GetParentId())) {
        end_pos = end_pd->pos + end_pd->offset[end_type];
      }

      auto &&connects_with_start = _selection_model.GetSelectionMap()[start_id];
      auto &&iter = std::find(connects_with_start.begin(),
                              connects_with_start.end(), end_id);
      if (iter != connects_with_start.end()) {
        selected_lines.push_back({start_pos, end_pos});
      } else {
        unselected_lines.push_back({start_pos, end_pos});
      }
    }
  }

  p.setPen(QPen(Qt::green, 1, Qt::SolidLine));
  p.drawLines(selected_lines.data(), selected_lines.size());

  p.setPen(QPen(Qt::red, 1, Qt::SolidLine));
  p.drawLines(unselected_lines.data(), unselected_lines.size());
  /*-------------------------------------*/

  /*-DRAW FRAME FOR SELECTED BLOCKS-*/
  p.setPen(QPen(Qt::green, 1, Qt::SolidLine));
  for (auto &&block_id : _selection_model.GetSelectedBlocks()) {
    if (auto &&block = qobject_cast<BlockWidget *>(FindById(block_id))) {
      QRect rect = block->rect();
      QRect mapped_rect(block->mapToParent({rect.x(), rect.y()}),
                        QSize({rect.width(), rect.height()}));
      p.drawRect(mapped_rect);
    } else {
      assert(false);
      return;
    }
  }
  /*--------------------------------*/

  auto &&begin = _line_model.GetBegin();
  auto &&end = _line_model.GetEnd();
  if (begin && end) { // draw connection line
    p.setPen(QPen(Qt::red, 1, Qt::SolidLine));
    p.drawLine(*begin, *end);
  }
}
