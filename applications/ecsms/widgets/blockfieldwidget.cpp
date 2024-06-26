#define _USE_MATH_DEFINES
#define _USE_MATH_DEFINES

#include "blockfieldwidget.h"
#include "../controlls/controllerprocedure.h"
#include "../controlls/drawrectanglecontroller.h"
#include "../controlls/emptycontroller.h"
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
#include "../utility/selectionutility.h"
#include "connectnodewidget.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QVector>
#include <cmath>
#include <set>

static void drawArrow(QPainter& p,
                      QLineF line,
                      float arrow_head_length,
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
  auto g = k_segm == 0 ? M_PI / 2 : atan(-1 / k_segm);  //(tan(atan(k_segm));
  auto p2 =
      QPointF{p1.x() - a * cos(g), p1.y() - a * sin(g)};  // ����� ������� �����
  auto p3 = QPointF{p2.x() + 2 * a * cos(g),
                    p2.y() + 2 * a * sin(g)};  // ����� ������� �����
  p.drawLine(end, p2);
  p.drawLine(end, p3);
}

BlockFieldWidget::BlockFieldWidget(QWidget* parent) : QWidget(parent) {
  setMouseTracking(true);
  setFocus(Qt::FocusReason::ActiveWindowFocusReason);
  _field_model.Subscribe(this);
  _selection_model.Subscribe(this);
  _line_model.Subscribe(this);
  _rect_model.Subscribe(this);
  _vis_model.Subscribe(this);
}

void BlockFieldWidget::SetCommandManager(std::shared_ptr<CommandManager> cm) {
  _cm = cm;
  _controller.reset(new DefaultController(_field_model, _selection_model,
                                          _line_model, _rect_model, _vis_model,
                                          *_cm));
}

void BlockFieldWidget::AddBlock(const QString& block_name) {
  controller::execute::AddBlock(
      _block_name_maker, _field_model, _selection_model,
      _vis_model.MapToModel(rect().center()), block_name, _cm);
}

void BlockFieldWidget::Update(std::shared_ptr<Event> e) {
  switch (e->GetEventType()) {
    case drawEvent: {
      auto&& draw_e = std::static_pointer_cast<DrawEvent>(e);
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

      auto&& change_ctr_e = std::static_pointer_cast<ChangeControllerEvent>(e);
      switch (change_ctr_e->GetControllerType()) {
        case drawLineController: {
          _controller.reset(new DrawLineController(_field_model, _line_model,
                                                   _vis_model, *_cm));
          break;
        }
        case defaultController: {
          _controller.reset(
              new DefaultController(_field_model, _selection_model, _line_model,
                                    _rect_model, _vis_model, *_cm));
          break;
        }
        case drawRectangleController: {
          _controller.reset(new DrawRectangleController(
              _rect_model, _field_model, _selection_model, _vis_model));
          break;
        }
        case emptyCotroller: {
          _controller.reset(new EmptyController());
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
      auto&& change_e = std::static_pointer_cast<ChangeActiveNodeEvent>(e);
      auto&& node_id = change_e->GetNode();
      if (auto&& node = qobject_cast<ConnectNodeWidget*>(FindById(node_id))) {
        node->makeTransparent(!change_e->GetActivity());
      } else {
        assert(false);
        return;
      }
      break;
    }
    case addBlockEvent: {
      auto&& add_block_e = std::static_pointer_cast<AddBlockEvent>(e);
      auto&& block_data = add_block_e->GetBlockData();
      auto&& block = new BlockWidget(add_block_e->GetId(), _controller,
                                     block_data.text, this);
      block->show();
      actualizeBlock(add_block_e->GetId());
      break;
    }
    case updateBlockEvent: {
      auto&& update_block_e = std::static_pointer_cast<UpdateBlockEvent>(e);
      actualizeBlock(update_block_e->GetBlock());
      break;
    }
    case removeBlockEvent: {
      auto&& remove_block_e = std::static_pointer_cast<RemoveBlockEvent>(e);
      auto&& block_w = FindById(remove_block_e->GetBlock());
      if (!block_w) {
        assert(false);
        return;
      }
      delete block_w;
      repaint();
      break;
    }
    case visualModelUpdateEvent: {
      // move all blocks to their curr pos + center coord
      auto&& blocks_map = _field_model.GetBlocks();
      for (auto&& pair_iter = blocks_map.begin(); pair_iter != blocks_map.end();
           ++pair_iter) {
        auto&& block_id = pair_iter.key();
        actualizeBlock(block_id);
      }
      repaint();
      break;
    }
    default: {
      assert(false);
      break;
    }
  }
}

std::unique_ptr<IController>& BlockFieldWidget::GetController() {
  return _controller;
}

QWidget* BlockFieldWidget::FindById(Id id) {
  QWidget* res = nullptr;
  for (auto&& child : children()) {
    if (auto&& block = qobject_cast<BlockWidget*>(child)) {
      if (block->GetId() == id) {
        res = block;
        break;
      } else if (auto&& node = block->FindById(id)) {
        res = node;
        break;
      }
    }
  }
  return res;
}

void BlockFieldWidget::Clear() {
  _cm->ClearCommands();
  _selection_model = SelectionModel();
  _line_model = PhantomLineModel();
  _rect_model = PhantomRectangleModel();
  _vis_model = VisualizationModel();
  _field_model = FieldModel();

  _field_model.Subscribe(this);
  _selection_model.Subscribe(this);
  _line_model.Subscribe(this);
  _rect_model.Subscribe(this);
  _vis_model.Subscribe(this);
}

void BlockFieldWidget::GoToFirstBlock() {
  const QMap<BlockId, FieldModel::BlockData>& blocks_map =
      _field_model.GetBlocks();
  if (!blocks_map.empty()) {
    _current_block = 0;
    auto&& blocks_ids = blocks_map.keys();
    auto&& any_block = blocks_ids.at(_current_block);
    _vis_model.SetNewCoordCenter(-_field_model.GetBlockData(any_block)->pos +
                                 QPoint(width() / 2, height() / 2));
  }
}

void BlockFieldWidget::GoToNextBlock() {
  const QMap<BlockId, FieldModel::BlockData>& blocks_map =
      _field_model.GetBlocks();
  if (!blocks_map.empty()) {
    auto&& blocks_ids = blocks_map.keys();
    _current_block = (_current_block + 1) % blocks_ids.size();
    auto&& any_block = blocks_ids.at(_current_block);
    _vis_model.SetNewCoordCenter(-_field_model.GetBlockData(any_block)->pos +
                                 QPoint(width() / 2, height() / 2));
  }
}

const FieldModel& BlockFieldWidget::GetFieldModel() const {
  return _field_model;
}

const VisualizationModel& BlockFieldWidget::GetVisualizationModel() const {
  return _vis_model;
}

void BlockFieldWidget::LoadFieldModel(
    const FieldModel::Memento& field_model_memento) {
  _field_model.Load(field_model_memento);
}

void BlockFieldWidget::LoadVisualizationModel(
    const VisualizationModel::Memento& vis_model_memento) {
  _vis_model.Load(vis_model_memento);
}

int BlockFieldWidget::GetCounter() const {
  return _block_name_maker.GetCounter();
}

void BlockFieldWidget::SetCounter(int counter) {
  _block_name_maker.SetCounter(counter);
}

void BlockFieldWidget::mouseMoveEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton)
    setCursor(Qt::CursorShape::ClosedHandCursor);
  _controller->onMouseMoveEvent(this, event);
}

void BlockFieldWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::MiddleButton)
    setCursor(Qt::CursorShape::ClosedHandCursor);
  _controller->onMousePressEvent(this, event);
}

void BlockFieldWidget::keyPressEvent(QKeyEvent* event) {
  _controller->onKeyPressEvent(this, event);
}

void BlockFieldWidget::enterEvent(QEvent* event) {
  _controller->onEnterEvent(this, event);
}

void BlockFieldWidget::leaveEvent(QEvent* event) {
  _controller->onLeaveEvent(this, event);
}

void BlockFieldWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton)
    setCursor(Qt::CursorShape::ArrowCursor);
  _controller->onMouseReleaseEvent(this, event);
}

void BlockFieldWidget::paintEvent(QPaintEvent* event) {
  QPainter p(this);
  p.eraseRect(rect());

  // draw phantom rectangle
  auto&& model_p1 = _rect_model.GetP1();
  auto&& model_p2 = _rect_model.GetP2();
  if (model_p1 && model_p2) {
    setCursor(Qt::CursorShape::ArrowCursor);
    p.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
    QRect rect = {_vis_model.MapToVisualization(*model_p1),
                  _vis_model.MapToVisualization(*model_p2)};
    p.drawRect(rect);
  } else {
    p.eraseRect(rect());
  }

  /*-DRAW SELECTED AND NOT SELECTED LINES-*/
  std::vector<QLineF> unselected_lines, selected_lines;
  auto func = [this, &unselected_lines, &selected_lines](
                  QPoint model_start_pos, NodeId start_id, QPoint model_end_pos,
                  NodeId end_id) {
    auto&& connects_with_start =
        _selection_model.GetSelectedConnections()[start_id];
    auto&& iter = std::find(connects_with_start.begin(),
                            connects_with_start.end(), end_id);

    QPoint vis_start_pos = _vis_model.MapToVisualization(model_start_pos);
    QPoint vis_end_pos = _vis_model.MapToVisualization(model_end_pos);
    QLine line(vis_start_pos, vis_end_pos);
    if (iter != connects_with_start.end()) {
      selected_lines.push_back(line);
    } else {
      unselected_lines.push_back(line);
    }
  };

  forEachConnection(_field_model, func);

  for (auto&& [brush, lines] :
       std::vector<std::pair<QBrush, std::vector<QLineF>>>(
           {{Qt::green, selected_lines}, {Qt::red, unselected_lines}})) {
    p.setPen(QPen(brush, 3, Qt::SolidLine));
    p.drawLines(lines.data(), lines.size());
    for (auto&& line : lines)
      drawArrow(p, line, 10, M_PI / 3);
  }

  /*-------------------------------------*/

  /*-DRAW FRAME FOR SELECTED BLOCKS-*/
  p.setPen(QPen(Qt::green, 1, Qt::SolidLine));
  for (auto&& block_id : _selection_model.GetSelectedBlocks()) {
    if (auto&& block = qobject_cast<BlockWidget*>(FindById(block_id))) {
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
  auto&& model_begin = _line_model.GetBegin();
  auto&& model_end = _line_model.GetEnd();
  if (model_begin && model_end) {
    p.setPen(QPen(Qt::red, 3, Qt::SolidLine));
    QLine line = {_vis_model.MapToVisualization(*model_begin),
                  _vis_model.MapToVisualization(*model_end)};
    p.drawLine(line);
    drawArrow(p, line, 10, M_PI / 3);
  }
}

void BlockFieldWidget::actualizeBlock(BlockId id) {
  auto&& block = FindById(id);
  if (!block) {
    assert(false);
    return;
  }
  auto&& block_data = _field_model.GetBlockData(id);
  if (!block_data) {
    assert(false);
    return;
  }
  auto&& newPos = _vis_model.MapToVisualization(block_data->pos);
  block->move(newPos);

  if (auto blockWidget = qobject_cast<BlockWidget*>(block))
    blockWidget->SetText(block_data->text);
  else
    assert(false);

  repaint();
}