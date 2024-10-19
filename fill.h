#ifndef FILL_H
#define FILL_H
#include <QColor>
#include "point.h"

// ��䣨���ӵ㣩
class Fill {
public:
	Point point; // �������ӵ�
	QColor color; // ���������ɫ

	Fill(Point t_point, QColor t_color) :point(t_point), color(t_color) {}
	Fill() :point(Point(10, 10)), color(Qt::black) {}
};
#endif // !FILL_H
