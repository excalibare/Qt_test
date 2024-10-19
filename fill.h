#ifndef FILL_H
#define FILL_H
#include <QColor>
#include "point.h"

// 填充（种子点）
class Fill {
public:
	Point point; // 储存种子点
	QColor color; // 储存填充颜色

	Fill(Point t_point, QColor t_color) :point(t_point), color(t_color) {}
	Fill() :point(Point(10, 10)), color(Qt::black) {}
};
#endif // !FILL_H
