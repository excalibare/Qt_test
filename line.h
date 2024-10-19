#ifndef LINE_H
#define LINE_H
#include <QLine>
#include <QColor>
#include "tools.h"

// �ṹ�����洢ÿ���ߵ���Ϣ��������㡢�յ���������
struct Line {
	QLine line;
	int width;
	QColor colour;  // lyc:6
	line_Algorithm alg;
	// Qt::PenStyle penStyle; // ����

	Line(QPoint p1, QPoint p2, int w, QColor c, line_Algorithm a) : line(p1, p2), width(w), colour(c), alg(a) {}
};
// ����Ԥ��ʵ��(������MAP)
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

// ����Ԥ������(������MAP)
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

	int dashLength = 8;   // ÿ�����ߵĳ���
	int gapLength = 8;    // ÿ�οհ׵ĳ���
	int totalLength = dashLength + gapLength;  // �����ڳ���
	int stepCount = 0;    // �������������ھ����Ƿ����

	while (x1 != x2 || y1 != y2) {
		// ֻ�����ߵĲ��ֻ��Ƶ�
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

		stepCount++;  // ���Ӳ���������
	}
}
#endif // !LINE_H
