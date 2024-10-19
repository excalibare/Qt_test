#ifndef POLYGON_H
#define POLYGON_H
#include <QPoint>
#include <QColor>
#include <vector>
#include "point.h"
using namespace std;

// ��Point���͵ĵ�ת��ΪQPoint
QPoint P2QP(Point p) {
	QPoint res;
	res.setX(p.x);
	res.setY(p.y);
	return res;
}

// ��Point���͵ĵ�ת��ΪQPoint
Point QP2P(QPoint p) {
	Point res;
	res.x = p.x();
	res.y = p.y();
	return res;
}

// �ṹ�����洢�����ÿ���ߵ���Ϣ��Ϊ����λ��Ʒ���
struct Edge {
	int yMax;
	float xMin, slopeReciprocal;
};

// �����
class Polygon {
public:
	vector<Point> points; // �������εĶ���
	QColor color;
	Polygon() {}

	void addPoint(const Point& point) {
		points.push_back(point);
	}

	void addPoint(const Polygon& polygon) {
		for (int i = 0; i < polygon.points.size(); ++i) {
			points.push_back(polygon.points[i]);
		}
	}

	bool isClosed() const {
		return points.size() > 2 && points.front().x == points.back().x && points.front().y == points.back().y;
	}

	void closePolygon() {
		if (points.size() > 2 && !isClosed()) {
			points.push_back(points.front()); // ��ն����
		}
	}

	Polygon operator=(const Polygon& poly) {
		this->points = poly.points;
		this->color = poly.color;
		return *this;
	}

	int length() {
		return points.size();
	}

	void clear() {
		vector<Point> tem;
		points = tem;
		color = Qt::black;
	}

	void remove(int i) {
		points.erase(points.begin() + i);
	}

	void print() {
		qDebug() << "The shape is: ";
		for (int j = 0; j < points.size(); ++j) {
			qDebug() << points[j].Getx() << " & " << points[j].Gety() << " || ";
		}
		qDebug() << "\n";
	}
};

// ���Զ���Polygon���͵�PolygonתΪQVector(QPoint)
QVector<QPoint> P2QV(Polygon p) {
	QVector<QPoint> res;
	for (int i = 0; i < p.points.size() - 1; i++) {
		res.push_back(P2QP(p.points[i]));
	}
	return res;
}

// ��QVector(QPoint)תΪ�Զ���Polygon���͵�Polygon
Polygon QV2P(QVector<QPoint> q) {
	Polygon res;
	for (int i = 0; i < q.size(); i++) {
		res.points.push_back(QP2P(q[i]));
	}
	res.points.push_back(res.points[0]);
	return res;
}
#endif // !POLYGON_H
