#ifndef ARC_H
#define ARC_H
#include<QPoint>
#include<QColor>

// 结构体来存储每条圆弧的信息，包括圆心、半径、起始角和结束角
struct Arc {
	QPoint center;  // 圆心
	int radius;     // 半径
	int startAngle; // 起始角度
	int endAngle;   // 终止角度
	int width;
	QColor colour;
	// Qt::PenStyle penStyle; // 线型

	Arc(QPoint p, int r, int sa, int ea, int w, QColor c) :center(p), radius(r), startAngle(sa), endAngle(ea), width(w), colour(c) {}
};
#endif // !ARC_H
