#ifndef LINE_H
#define LINE_H
#include <QLine>
#include <QColor>
#include "tools.h"

// 结构体来存储每条线的信息，包括起点、终点和线条宽度
struct Line {
	QLine line;
	int width;
	QColor colour;  // lyc:6
	line_Algorithm alg;
	// Qt::PenStyle penStyle; // 线型

	Line(QPoint p1, QPoint p2, int w, QColor c, line_Algorithm a) : line(p1, p2), width(w), colour(c), alg(a) {}
};
// 绘制预览实线(不更新MAP)
void drawPreviewSolid(QPainter& painter, QPoint p1, QPoint p2)
{
	int x1 = p1.x();
	int y1 = p1.y();
	int x2 = p2.x();
	int y2 = p2.y();
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;
	int err = dx - dy;

	while (x1 != x2 || y1 != y2) {
		painter.drawPoint(x1, y1);
		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y1 += sy;
		}
	}
}

// 绘制预览虚线(不更新MAP)
void drawPreviewDash(QPainter& painter, QPoint p1, QPoint p2)
{
	int x1 = p1.x();
	int y1 = p1.y();
	int x2 = p2.x();
	int y2 = p2.y();
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;
	int err = dx - dy;

	int dashLength = 8;   // 每段虚线的长度
	int gapLength = 8;    // 每段空白的长度
	int totalLength = dashLength + gapLength;  // 总周期长度
	int stepCount = 0;    // 计数步数，用于决定是否绘制

	while (x1 != x2 || y1 != y2) {
		// 只在虚线的部分绘制点
		if (stepCount % totalLength < dashLength) {
			painter.drawPoint(x1, y1);
		}

		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y1 += sy;
		}

		stepCount++;  // 增加步数计数器
	}
}
#endif // !LINE_H
