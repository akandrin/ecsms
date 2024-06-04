#define _USE_MATH_DEFINES

#include "blockfieldwidget.h"
#include "../controlls/controllerprocedure.h"
#include "../events/addblockevent.h"
#include "../events/changeactivenodeevent.h"
#include "../events/changecontrollerevent.h"
#include "../events/drawevent.h"
#include "../events/removeblockevent.h"
#include "../events/repaintevent.h"
#include "../events/updateblockevent.h"
#include "../models/nodetype.h"
#include "../models/visualizationmodel.h"
#include "../namemaker/namemaker.h"
#include "connectnodewidget.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QVector>
#include <cmath>
#include <set>

static void drawArrow(QPainter &p, QLine line, float arrow_head_length,
                      float arrow_head_angle) {
  auto b = arrow_head_length, alpha = arrow_head_angle / 2;
  auto c = b / cos(alpha);
  auto a = c * sin(alpha);
  auto &&begin = line.p1(), &&end = line.p2();
  auto len = sqrt(pow(end.y() - begin.y(), 2) + pow(end.x() - begin.x(), 2));
  auto beta = acos((end.x() - begin.x()) / len);
  auto h = end.y() > begin.y() ? 1 : (end.y() == begin.y() ? 0 : -1);
  auto p1 = QPointF{end.x() - b * cos(beta), end.y() - h * b * sin(beta)};
  auto k_segm = float(end.y() - begin.y()) / (end.x() - begin.x());
  auto g = k_segm == 0 ? M_PI / 2 : atan(-1 / k_segm); //(tan(atan(k_segm));
  auto p2 =
      QPointF{p1.x() - a * cos(g), p1.y() - a * sin(g)}; // ����� ������� �����
  auto p3 = QPointF{p2.x() + 2 * a * cos(g),
                    p2.y() + 2 * a * sin(g)}; // ����� ������� �����
  p.drawLine(end, p2);
  p.drawLine(end, p3);
}

BlockFieldWidget::BlockFieldWidget(QWidget *parent) : QWidget(parent) {
  setMouseTracking(true);
  setFocus(Qt::FocusReason::ActiveWindowFocusReason);
  _field_model.Subscribe(this);
  _selection_model.Subscribe(this);
  _line_model.Subscribe(this);
  _vis_model.Subscribe(this);
}

void BlockFieldWidget::SetCommandManager(std::shared_ptr<CommandManager> cm) {
  _cm = cm;
  _controller.reset(new DefaultController(_field_model, _selection_model,
                                          _line_model, _vis_model, *_cm));
}

void BlockFieldWidget::AddBlock() {
  controller::execute::AddBlock(_block_name_maker, _field_model,
                                _selection_model,
                                _vis_model.MapToModel(rect().center()), _cm);
}

void BlockFieldWidget::Update(std::shared_ptr<Event> e) {
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
    _controller.reset();

    auto &&change_ctr_e = std::static_pointer_cast<ChangeControllerEvent>(e);
    switch (change_ctr_e->GetControllerType()) {
    case drawLineController: {
      _controller.reset(
          new DrawLineController(_field_model, _line_model, _vis_model, *_cm));
      break;
    }
    case defaultController: {
      _controller.reset(new DefaultController(_field_model, _selection_model,
                                              _line_model, _vis_model, *_cm));
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
  case addBlockEvent: {
    auto &&add_block_e = std::static_pointer_cast<AddBlockEvent>(e);
    auto &&block_data = add_block_e->GetBlockData();
    auto &&block = new BlockWidget(add_block_e->GetId(), _controller,
                                   block_data.text, this);
    block->move(block_data.pos);
    block->show();
    repaint();
    break;
  }
  case updateBlockEvent: {
    auto &&update_block_e = std::static_pointer_cast<UpdateBlockEvent>(e);
    auto &&block = FindById(update_block_e->GetBlock());
    if (!block) {
      assert(false);
      return;
    }
    auto &&block_data = update_block_e->GetBlockData();
    block->move(block_data.pos);
    qobject_cast<BlockWidget *>(block)->SetText(block_data.text);
    repaint();
    break;
  }
  case removeBlockEvent: {
    auto &&remove_block_e = std::static_pointer_cast<RemoveBlockEvent>(e);
    auto &&block_w = FindById(remove_block_e->GetBlock());
    if (!block_w) {
      assert(false);
      return;
    }
    delete block_w;
    repaint();
    break;
  }
  default: {
    assert(false);
    break;
  }
  }
}

std::unique_ptr<IController> &BlockFieldWidget::GetController() {
  return _controller;
}

QWidget *BlockFieldWidget::FindById(Id id) {
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

void BlockFieldWidget::Clear() {
  _selection_model.Clear();
  _field_model.RemoveAll();
}

void BlockFieldWidget::GoToFirstBlock() {
  const QMap<BlockId, FieldModel::BlockData> &blocks_map =
      _field_model.GetBlocks();
  if (!blocks_map.empty()) {
    _current_block = 0;
    auto &&blocks_ids = blocks_map.keys();
    auto &&any_block = blocks_ids.at(_current_block);
    _vis_model.SetNewCoordCenter(-_field_model.GetBlockData(any_block)->pos +
                                 QPoint(width() / 2, height() / 2));
  }
}

void BlockFieldWidget::GoToNextBlock() {
  const QMap<BlockId, FieldModel::BlockData> &blocks_map =
      _field_model.GetBlocks();
  if (!blocks_map.empty()) {
    auto &&blocks_ids = blocks_map.keys();
    _current_block = (_current_block + 1) % blocks_ids.size();
    auto &&any_block = blocks_ids.at(_current_block);
    _vis_model.SetNewCoordCenter(-_field_model.GetBlockData(any_block)->pos +
                                 QPoint(width() / 2, height() / 2));
  }
}

void BlockFieldWidget::mouseMoveEvent(QMouseEvent *event) {
  _controller->onMouseMoveEvent(this, event);
}

void BlockFieldWidget::mousePressEvent(QMouseEvent *event) {
  _controller->onMousePressEvent(this, event);
}

void BlockFieldWidget::keyPressEvent(QKeyEvent *event) {
  _controller->onKeyPressEvent(this, event);
}

void BlockFieldWidget::enterEvent(QEvent *event) {
  _controller->onEnterEvent(this, event);
}

void BlockFieldWidget::leaveEvent(QEvent *event) {
  _controller->onLeaveEvent(this, event);
}

void BlockFieldWidget::mouseReleaseEvent(QMouseEvent *event) {
  _controller->onMouseReleaseEvent(this, event);
}

void BlockFieldWidget::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.eraseRect(rect());
  p.setBackground(QBrush(Qt::white));

  // move all blocks to their curr pos + center coord
  // todo : cysch otsyuda
  auto &&blocks_map = _field_model.GetBlocks();
  for (auto &&pair_iter = blocks_map.begin(); pair_iter != blocks_map.end();
       ++pair_iter) {
    auto &&blockId = pair_iter.key();
    auto &&blockData = pair_iter.value();
    if (auto &&widget = FindById(blockId)) {
      auto &&newPos = _vis_model.MapToVisualization(blockData.pos);
      widget->move(newPos);
    } else
      assert(false);
  }

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

      QPoint model_start_pos, model_end_pos;
      if (auto &&start_pd = _field_model.GetBlockData(start_id.GetParentId())) {
        model_start_pos = start_pd->pos + start_pd->offset[start_type];
      }

      if (auto &&end_pd = _field_model.GetBlockData(end_id.GetParentId())) {
        model_end_pos = end_pd->pos + end_pd->offset[end_type];
      }

      auto &&connects_with_start = _selection_model.GetSelectionMap()[start_id];
      auto &&iter = std::find(connects_with_start.begin(),
                              connects_with_start.end(), end_id);

      QPoint vis_start_pos = _vis_model.MapToVisualization(model_start_pos);
      QPoint vis_end_pos = _vis_model.MapToVisualization(model_end_pos);
      QLine line(vis_start_pos, vis_end_pos);
      if (iter != connects_with_start.end()) {
        selected_lines.push_back(line);
        p.setPen(QPen(Qt::green, 3, Qt::SolidLine));
        drawArrow(p, line, 10, M_PI / 3);
      } else {
        unselected_lines.push_back(line);
        p.setPen(QPen(Qt::red, 3, Qt::SolidLine));
        drawArrow(p, line, 10, M_PI / 3);
      }
    }
  }

  p.setPen(QPen(Qt::green, 3, Qt::SolidLine));
  p.drawLines(selected_lines.data(), selected_lines.size());

  p.setPen(QPen(Qt::red, 3, Qt::SolidLine));
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

  // draw connection phantom line
  auto &&model_begin = _line_model.GetBegin();
  auto &&model_end = _line_model.GetEnd();
  if (model_begin && model_end) {
    p.setPen(QPen(Qt::red, 3, Qt::SolidLine));
    QLine line = {_vis_model.MapToVisualization(*model_begin),
                  _vis_model.MapToVisualization(*model_end)};
    p.drawLine(line);
    drawArrow(p, line, 10, M_PI / 3);
  }
}
