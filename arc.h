#ifndef ARC_H
#define ARC_H
#include<QPoint>
#include<QColor>

// �ṹ�����洢ÿ��Բ������Ϣ������Բ�ġ��뾶����ʼ�Ǻͽ�����
struct Arc {
	QPoint center;  // Բ��
	int radius;     // �뾶
	int startAngle; // ��ʼ�Ƕ�
	int endAngle;   // ��ֹ�Ƕ�
	int width;
	QColor colour;
	// Qt::PenStyle penStyle; // ����

	Arc(QPoint p, int r, int sa, int ea, int w, QColor c) :center(p), radius(r), startAngle(sa), endAngle(ea), width(w), colour(c) {}
};
#endif // !ARC_H
