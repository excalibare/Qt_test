#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QPen>
#include <cmath>
#include <QInputDialog>
#include <vector>
#include "stdafx.h"
using namespace std;

//TODO:���ƻ��ߵ� ��ʼ�� ���� ������ ����δ����

// ����SutherlandTrim
enum OutCode {
	INSIDE = 0, // 0000
	LEFT = 1,   // 0001
	RIGHT = 2,  // 0010
	BOTTOM = 4, // 0100
	TOP = 8     // 1000
};

// ����ʵ����䣨 pointData �� MAP ��
class pointData {
protected:
	QPoint pos;
	QColor color;
public:
	pointData(QPoint p, QColor c) {
		pos = p;
		color = c;
	}
	QColor getColor() {
		return color;
	}
	void setColor(QColor c) {
		color = c;
	}
};

void initMAP(vector<vector<pointData>>& MAP); 
void clearMAP(vector<vector<pointData>>& MAP);

// �������ͣ�ֱ�߻�Բ��
enum DrawMode {
	LineMode,
	ArcMode,
	CircleMode,
	PolygonMode,
	FillMode,
	TrimMode
};

// �߶λ����㷨
enum Algorithm {
	DDA,
	Bresenham,
	Midpoint,
	DashLine,
	SutherlandTrim,
	MidTrim
};

// �ṹ�����洢ÿ���ߵ���Ϣ��������㡢�յ���������
struct Line {
	QLine line;
	int width;
	QColor colour;  // lyc:6
	Algorithm alg;
	// Qt::PenStyle penStyle; // ����

	Line(QPoint p1, QPoint p2, int w, QColor c, Algorithm a) : line(p1, p2), width(w), colour(c), alg(a) {}
};

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

// �ṹ�����洢ÿ�������Ϣ��Ϊ����λ��Ʒ���
struct Point {
public:
	int x, y;
	Point(int x = 0, int y = 0) : x(x), y(y) {}

	int Getx() {
		return x;
	}
	int Gety() {
		return y;
	}
};

// �ṹ�����洢�����ÿ���ߵ���Ϣ��Ϊ����λ��Ʒ���
struct Edge {
	int yMax;
	float xMin, slopeReciprocal;
};

class Polygon {
public:
	std::vector<Point> points; // �������εĶ���
	QColor color;
	Polygon() {}

	void addPoint(const Point& point) {
		points.push_back(point);
	}

	bool isClosed() const {
		return points.size() > 2 && points.front().x == points.back().x && points.front().y == points.back().y;
	}

	void closePolygon() {
		if (points.size() > 2 && !isClosed()) {
			points.push_back(points.front()); // ��ն����
		}
	}
};

class Fill {
public:
	Point point; // �������ӵ�
	QColor color; // ���������ɫ

	Fill(Point t_point, QColor t_color) :point(t_point), color(t_color) {}
};

class ShapeDrawer : public QWidget {
	// Q_OBJECT;
private:
	DrawMode mode;              // ��ǰ����ģʽ��ֱ�� �� Բ��
	vector<vector<pointData>> MAP;

	int lineWidth = 5;          // �洢�������
	bool hasStartPoint = false; // �Ƿ�����㣨����ֱ�߻�Բ����
	bool drawing = false;       // �Ƿ����ڻ��ƣ�������������ͷ�ʱ�Ķ�����
	bool drawingrect = false;
	QColor currentLineColor = Qt::black;    // ��ǰ������ɫ

	QPoint startPoint;          // �߶ε����
	QPoint endPoint;            // �߶ε��յ�
	Algorithm algorithm = Midpoint;  // ��ǰѡ���ֱ�߶��㷨

	QPoint center;              // Բ�ģ�����Բ����
	int radius;                 // �뾶������Բ����
	int startAngle, endAngle;   // Բ������ʼ����ֹ�Ƕ�
	int counter = 0;            // �����������������0-2ѭ����

	vector<int> shape;          // ����ͼ�ε��ػ�˳�򣬷�ֹ˳����� 1.ֱ�� 2.Բ����Բ 3.�����
	QVector<Arc> arcs;          // �洢�ѻ��Ƶ�ֱ�߶�
	QVector<Line> lines;        // �洢�ѻ��Ƶ�ֱ�߶�

	std::vector<Polygon> polygons; // �洢��������
	Polygon currentPolygon; // ��ǰ���ڻ��ƵĶ����

	std::vector<Fill> fills; // �洢��������
	QStack<Point> stack;

	QPoint clipStartPoint;  // �ü���ڵ����
	QPoint clipEndPoint;    // �ü���ڵ��յ�

	float XL = 0, XR = 800, YB = 0, YT = 550;

protected:
	// ��д�����¼�
	void paintEvent(QPaintEvent* event) override
	{
		// ����ͼ�εļ����������ƴ�vector��ȡ����˳��
		int i1 = 0, i2 = 0, i3 = 0, i4 = 0;
		clearMAP(MAP);
		QPainter painter(this);

		// ÿ��ˢ�µ�ʱ�����»����Ѵ���ֱ��
		for (int c = 0; c < shape.size(); c++) {
			QPen pen;
			// �ػ�ֱ��
			if (shape.at(c) == 1 && lines.size() > i1) {
				const Line& line = lines.at(i1++);
				pen.setColor(line.colour);  // ����������ɫ(Ĭ�� black)
				pen.setWidth(line.width);   // ʹ�� width ����������ϸ
				painter.setPen(pen);
				switch (line.alg) {
				case DDA:
					drawDDALine(painter, line.line.p1(), line.line.p2());
					break;
				case Bresenham:
					drawBresenhamLine(painter, line.line.p1(), line.line.p2());
					break;
				case Midpoint:
					drawMidpointLine(painter, line.line.p1(), line.line.p2());
					break;
				case DashLine:
					drawDashLine(painter, line.line.p1(), line.line.p2());
					break;
				}
			}
			// �ػ�Բ�� �� Բ
			else if (shape.at(c) == 2 && arcs.size() > i2) {
				const Arc& arc = arcs.at(i2++);
				pen.setColor(arc.colour);   // ����������ɫ(Ĭ�� black)
				pen.setWidth(arc.width);    // ʹ�� width ����������ϸ
				painter.setPen(pen);
				if (arc.startAngle <= arc.endAngle) {
					drawMidpointArc(painter, arc.center, arc.radius, arc.startAngle, arc.endAngle);
				}
				else {
					drawMidpointArc(painter, arc.center, arc.radius, arc.startAngle, 360);
					drawMidpointArc(painter, arc.center, arc.radius, 0, arc.endAngle);
				}
			}
			// �ػ�����
			else if (shape.at(c) == 3 && polygons.size() > i3) {
				const Polygon& polygon = polygons.at(i3++);
				QPen pen;
				pen.setColor(polygon.color);  // ����������ɫ(Ĭ�� black)
				// pen.setWidth(lineWidth);		 // ʹ�� width ����������ϸ
				painter.setPen(pen);
				if (polygon.points.size() > 1) {
					for (size_t i = 0; i < polygon.points.size() - 1; ++i) {
						drawDDALine(painter, QPoint(polygon.points[i].x, polygon.points[i].y), QPoint(polygon.points[i + 1].x, polygon.points[i + 1].y));
						//painter.drawLine(polygon.points[i].x, polygon.points[i].y, polygon.points[i + 1].x, polygon.points[i + 1].y);
					}
				}
				if (polygon.isClosed()) {
					scanlineFill(painter, polygon);
				}
			}
			else if (shape.at(c) == 4 && fills.size() > i4) {
				const Fill& fill = fills.at(i4++);
				QPen pen;
				pen.setColor(fill.color);  // ����������ɫ(Ĭ�� black)
				painter.setPen(pen);
				qDebug() << "In paintevent : " << fill.color << " " << Fill(fill).point.Getx() << " " << Fill(fill).point.Gety();
				fillShape(painter, fill.point, fill.color);
			}
		}

		// ���Ƶ�ǰ���ڴ����Ķ����
		if (currentPolygon.points.size() > 1) {
			for (size_t i = 0; i < currentPolygon.points.size() - 1; ++i) {
				painter.drawLine(currentPolygon.points[i].x, currentPolygon.points[i].y, currentPolygon.points[i + 1].x, currentPolygon.points[i + 1].y);
			}
		}

		// �������㣬�����߶ε�Ԥ��
		if (hasStartPoint) {
			QPen pen;
			QColor temp_color = Qt::gray;   // ����Ԥ��������ɫ(��ɫ��
			pen.setColor(temp_color);
			pen.setWidth(lineWidth);        // ʹ�õ�ǰ���õ��������
			painter.setPen(pen);

			if (mode == LineMode) {
				switch (algorithm) {
				case DDA:
					drawDDALine(painter, startPoint, endPoint);
					break;
				case Bresenham:
					drawBresenhamLine(painter, startPoint, endPoint);
					break;
				case Midpoint:
					drawMidpointLine(painter, startPoint, endPoint);
					break;
				case DashLine:
					drawDashLine(painter, startPoint, endPoint);
					break;
				}
			}
			else if (mode == CircleMode) {
				drawMidpointArc(painter, center, radius, startAngle, endAngle);
			}
			else  if (mode == ArcMode) {
				if (startAngle <= endAngle)
					drawMidpointArc(painter, center, radius, startAngle, endAngle);
				else {
					drawMidpointArc(painter, center, radius, startAngle, 360);
					drawMidpointArc(painter, center, radius, 0, endAngle);
				}
			}
			else if (mode == TrimMode && hasStartPoint) {
				painter.setPen(Qt::DashLine);  // ʹ�����߱�ʾ�ü�����
				painter.drawRect(QRect(clipStartPoint, clipEndPoint));
			}
			else if (mode == TrimMode && hasStartPoint) {
				painter.setPen(Qt::DashLine);  // ʹ�����߱�ʾ�ü�����
				painter.drawRect(QRect(clipStartPoint, clipEndPoint));
			}
		}

		// ����������յ㣬����ʵ���߶�
		if (drawing) {
			QPen pen;
			pen.setColor(currentLineColor); // ʹ�õ�ǰ���õ���ɫ
			pen.setWidth(lineWidth);        // ʹ�õ�ǰ���õ��������
			painter.setPen(pen);

			if (mode == LineMode) {
				switch (algorithm) {
				case DDA:
					drawDDALine(painter, startPoint, endPoint);
					break;
				case Bresenham:
					drawBresenhamLine(painter, startPoint, endPoint);
					break;
				case Midpoint:
					drawMidpointLine(painter, startPoint, endPoint);
					break;
				case DashLine:
					drawDashLine(painter, startPoint, endPoint);
					break;
				}
			}
			else if (mode == CircleMode) {
				drawMidpointArc(painter, center, radius, startAngle, endAngle);
			}
			else  if (mode == ArcMode) {
				if (startAngle <= endAngle)
					drawMidpointArc(painter, center, radius, startAngle, endAngle);
				else {
					drawMidpointArc(painter, center, radius, startAngle, 360);
					drawMidpointArc(painter, center, radius, 0, endAngle);
				}
			}
		}

		//�����ǰ�� TrimMode�������û�ѡ��Ĳü����
		if (mode == TrimMode && hasStartPoint) {
			painter.setPen(Qt::DashLine);  // ʹ�����߱�ʾ�ü�����
			painter.drawRect(QRect(clipStartPoint, clipEndPoint));
		}
	}

	// �ж����Ƶ�ʱ���Ƿ���磨���MAP��
	// дfill��lyc��лл���ṩ���������
	// lyc�����ǣ����width��heightû��Ĭ�ϲ�����Ϊʲô���Լ�д�ж�
	bool checkLegalPos(int x, int y, int width, int height) {
		if (x >= 0 && x <= width && y >= 0 && y <= height) {
			return true;
		}
		else
			return false;
	}

	// �������ص��� MAP ��Ӧ����������
	void drawPixel(int x, int y, QPainter& painter) {
		//��ʹ�ñ�ˢ
		if (checkLegalPos(x, y, 800, 550)) {
			painter.drawPoint(x, y);
			// qDebug() << x << " " << y;
			MAP[x][y].setColor(painter.pen().color());
		}
	};

	void drawPixel(int x, int y, vector<vector<pointData>>& MAP2, QColor color) {
		if (checkLegalPos(x, y, 800, 550)) {  // ȷ������λ�úϷ�
			MAP2[x][y].setColor(color);  // ���� MAP2 ����ɫ
		}
	}

	// DDA �㷨ʵ��
	void drawDDALine(QPainter& painter, QPoint p1, QPoint p2) {
		int dx = p2.x() - p1.x();
		int dy = p2.y() - p1.y();
		int steps = std::max(abs(dx), abs(dy));
		float xIncrement = dx / static_cast<float>(steps);
		float yIncrement = dy / static_cast<float>(steps);

		float x = p1.x();
		float y = p1.y();
		for (int i = 0; i <= steps; ++i) {
			drawPixel(static_cast<int>(x), static_cast<int>(y), painter);
			x += xIncrement;
			y += yIncrement;
		}
	}

	// Bresenham �㷨ʵ��
	void drawBresenhamLine(QPainter& painter, QPoint p1, QPoint p2) {
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
			drawPixel(x1, y1, painter);
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

	// �е��㷨ʵ��
	void drawMidpointLine(QPainter& painter, QPoint p1, QPoint p2) {
		// ��ȡ��Ϣ
		int x1 = p1.x();
		int y1 = p1.y();
		int x2 = p2.x();
		int y2 = p2.y();

		int x = x1, y = y1;	//����ʼ��
		int dy = y1 - y2, dx = x2 - x1;
		int delta_x = (dx >= 0 ? 1 : (dx = -dx, -1));	//��dx>0�򲽳�Ϊ1������Ϊ-1��ͬʱdx����
		int delta_y = (dy <= 0 ? 1 : (dy = -dy, -1));	//ע������dy<0,���ǻ�����y����������

		drawPixel(x, y, painter);		//����ʼ��

		int d, incrE, incrNE;
		if (-dy <= dx)		// б�ʾ���ֵ <= 1
			//����-dy�������е�dy
		{
			d = 2 * dy + dx;	//��ʼ���ж�ʽd
			incrE = 2 * dy;		//ȡ����Eʱ�б�ʽ����
			incrNE = 2 * (dy + dx);//NE
			while (x != x2)
			{
				if (d < 0)
					y += delta_y, d += incrNE;
				else
					d += incrE;
				x += delta_x;
				drawPixel(x, y, painter);
			}
		}
		else				// б�ʾ���ֵ > 1
							// x��y�������
		{
			d = 2 * dx + dy;
			incrE = 2 * dx;
			incrNE = 2 * (dy + dx);
			while (y != y2)
			{
				if (d < 0)	//ע��d�仯���
					d += incrE;
				else
					x += delta_x, d += incrNE;
				y += delta_y;
				drawPixel(x, y, painter);
			}
		}
	}

	// ���߻��� Bresenham �㷨ʵ��
	void drawDashLine(QPainter& painter, QPoint p1, QPoint p2)
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
				drawPixel(x1, y1, painter);
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

	// �е�Բ���㷨ʵ��
	void drawMidpointArc(QPainter& painter, QPoint center, int radius, int startAngle, int endAngle) {
		// ���Ƕ�ת��Ϊ����
		float startRad = startAngle * M_PI / 180.0;
		float endRad = endAngle * M_PI / 180.0;

		int x = radius;
		int y = 0;
		int d = 1 - radius;

		drawSymmetricPointsArc(painter, center, x, y, startRad, endRad);

		while (x > y) {
			if (d < 0) {
				d += 2 * y + 3;
			}
			else {
				d += 2 * (y - x) + 5;
				--x;
			}
			++y;
			drawSymmetricPointsArc(painter, center, x, y, startRad, endRad);
		}
	}

	// ���ƶԳƵ㣬ֻ�����ڽǶȷ�Χ�ڵĵ�
	void drawSymmetricPointsArc(QPainter& painter, QPoint center, int x, int y, float startRad, float endRad) {
		drawPointInArc(painter, center, x, y, startRad, endRad);
		drawPointInArc(painter, center, -x, y, startRad, endRad);
		drawPointInArc(painter, center, x, -y, startRad, endRad);
		drawPointInArc(painter, center, -x, -y, startRad, endRad);
		drawPointInArc(painter, center, y, x, startRad, endRad);
		drawPointInArc(painter, center, -y, x, startRad, endRad);
		drawPointInArc(painter, center, y, -x, startRad, endRad);
		drawPointInArc(painter, center, -y, -x, startRad, endRad);
	}

	// �жϵ��Ƿ���Բ���Ƕȷ�Χ�ڲ�����
	void drawPointInArc(QPainter& painter, QPoint center, int x, int y, float startRad, float endRad) {
		float angle = atan2(y, x);
		if (angle < 0) angle += 2 * M_PI;
		if (angle >= startRad && angle <= endRad) {
			painter.drawPoint(center.x() + x, center.y() + y);
		}
	}

	// ɨ�����㷨
	void scanlineFill(QPainter& painter, const Polygon& polygon) {
		if (polygon.points.size() < 3) return;

		// ������С����� y ����
		int ymin = polygon.points[0].y, ymax = polygon.points[0].y;
		for (const Point& p : polygon.points) {
			ymin = std::min(ymin, p.y);
			ymax = std::max(ymax, p.y);
		}

		// ��ÿ��ɨ���ߴ� ymin �� ymax ���д���
		for (int y = ymin; y <= ymax; ++y) {
			std::vector<int> intersections;

			// ����ÿ�����뵱ǰɨ���ߵĽ���
			for (size_t i = 0; i < polygon.points.size() - 1; ++i) {
				const Point& p1 = polygon.points[i];
				const Point& p2 = polygon.points[i + 1];

				if ((p1.y <= y && p2.y > y) || (p1.y > y && p2.y <= y)) {
					int x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
					intersections.push_back(x);
				}
			}

			// �Խ����������
			std::sort(intersections.begin(), intersections.end());

			// ��������֮����߶�(Ԥ���棬��ʹ��Qt�ĺ���)
			for (size_t i = 0; i < intersections.size(); i += 2) {
				painter.drawLine(intersections[i], y, intersections[i + 1], y);
			}
		}
	}

	void fillShape(QPainter& painter, Point point, QColor newColor) {
		//�����λ�õ���ɫ����Ϊ��Ҫ�滻����ɫ
		QColor oldColor = MAP[point.Getx()][point.Gety()].getColor();
		if (oldColor == newColor)
			return;
		int xl, xr, i, x, y;
		x = point.Getx();
		y = point.Gety();
		bool spanNeedFill;
		Point pt(point);
		stack.push(pt);
		while (!stack.isEmpty()) {
			pt = stack.pop();
			y = pt.Gety();
			x = pt.Getx();
			//if (y >= 549 || y <= 0)continue;
			while (MAP[x][y].getColor() == oldColor && x < 799) {
				drawPixel(x, y, painter);
				x++;
			}
			xr = x - 1;
			x = pt.Getx() - 1;
			while (MAP[x][y].getColor() == oldColor && x > 0) {
				drawPixel(x, y, painter);
				x--;
			}
			xl = x + 1;
			x = xl;
			y = y + 1;
			while (x < xr) {
				spanNeedFill = false;
				while (MAP[x][y].getColor() == oldColor && x < 799) {
					spanNeedFill = true;
					x++;

				}
				if (spanNeedFill) {
					pt.x = x - 1;
					pt.y = y;
					stack.push(pt);
					spanNeedFill = false;

				}
				while (MAP[x][y].getColor() != oldColor && x < xr)x++;
			}
			x = xl;
			y = y - 2;
			while (x < xr) {
				spanNeedFill = false;
				while (MAP[x][y].getColor() == oldColor && x < 599) {
					spanNeedFill = true;
					x++;

				}
				if (spanNeedFill) {
					pt.x = x - 1;
					pt.y = y;
					stack.push(pt);
					spanNeedFill = false;

				}
				while (MAP[x][y].getColor() != oldColor && x < xr)x++;
			}
		}
	}

	// Sutherland �����
	OutCode computeOutCode(double x, double y, double xmin, double ymin, double xmax, double ymax) {
		OutCode code = INSIDE;

		if (x < xmin) code = OutCode(code | LEFT);
		else if (x > xmax) code = OutCode(code | RIGHT);
		if (y < ymin) code = OutCode(code | BOTTOM);
		else if (y > ymax) code = OutCode(code | TOP);

		return code;
	}

	// Sutherland �ü��㷨
	bool cohenSutherlandClip(QLineF& line, double xmin, double ymin, double xmax, double ymax) {
		double x0 = line.x1(), y0 = line.y1();
		double x1 = line.x2(), y1 = line.y2();

		OutCode outcode0 = computeOutCode(x0, y0, xmin, ymin, xmax, ymax);
		OutCode outcode1 = computeOutCode(x1, y1, xmin, ymin, xmax, ymax);
		bool accept = false;

		while (true) {
			if (!(outcode0 | outcode1)) {
				// Both endpoints inside rectangle
				accept = true;
				break;
			}
			else if (outcode0 & outcode1) {
				// Both endpoints outside rectangle
				break;
			}
			else {
				// Some segment of the line lies within the rectangle
				double x, y;
				// Pick an endpoint that is outside the rectangle
				OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

				if (outcodeOut & TOP) {          // point above the clip rectangle
					x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
					y = ymax;
				}
				else if (outcodeOut & BOTTOM) { // point below the clip rectangle
					x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
					y = ymin;
				}
				else if (outcodeOut & RIGHT) {  // point to the right of the clip rectangle
					y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
					x = xmax;
				}
				else if (outcodeOut & LEFT) {   // point to the left of the clip rectangle
					y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
					x = xmin;
				}

				if (outcodeOut == outcode0) {
					x0 = x;
					y0 = y;
					outcode0 = computeOutCode(x0, y0, xmin, ymin, xmax, ymax);
				}
				else {
					x1 = x;
					y1 = y;
					outcode1 = computeOutCode(x1, y1, xmin, ymin, xmax, ymax);
				}
			}
		}
		if (accept) {
			line.setP1(QPointF(x0, y0));
			line.setP2(QPointF(x1, y1));
		}
		return accept;
	}


	//中点算法裁剪
	bool liangBarskyClip(QLineF& line, double xmin, double ymin, double xmax, double ymax) {
		double x0 = line.x1(), y0 = line.y1();
		double x1 = line.x2(), y1 = line.y2();

		double dx = x1 - x0;
		double dy = y1 - y0;

		double t0 = 0.0, t1 = 1.0;
		double p[] = { -dx, dx, -dy, dy };
		double q[] = { x0 - xmin, xmax - x0, y0 - ymin, ymax - y0 };

		for (int i = 0; i < 4; i++) {
			if (p[i] == 0 && q[i] < 0) {
				// Line is parallel and outside the boundary
				return false;
			}

			double r = q[i] / p[i];
			if (p[i] < 0) {
				if (r > t1) return false;
				else if (r > t0) t0 = r;
			}
			else if (p[i] > 0) {
				if (r < t0) return false;
				else if (r < t1) t1 = r;
			}
		}

		if (t0 > t1) return false;

		line.setP1(QPointF(x0 + t0 * dx, y0 + t0 * dy));
		line.setP2(QPointF(x0 + t1 * dx, y0 + t1 * dy));
		return true;
	}



	// ������갴���¼�
	void mousePressEvent(QMouseEvent* event) override {
		if (!hasStartPoint) {
			if (mode == LineMode) {
				// ��һ�ε������¼���
				startPoint = event->pos();
				endPoint = startPoint;      // ��ʼ���յ�Ϊ���
				shape.push_back(1);
			}
			else if (mode == CircleMode) {
				center = event->pos();      // ��¼Բ��
				hasStartPoint = true;
				shape.push_back(2);
			}
			else if (mode == ArcMode && counter == 0) {
				center = event->pos();      // ��¼Բ��
				shape.push_back(2);
			}
			else if (mode == ArcMode && counter == 1) {
				// ���㵱ǰ���λ����Բ�ĵĽǶ�
				float angleRad = atan2(event->pos().y() - center.y(), event->pos().x() - center.x());
				startAngle = static_cast<int>(angleRad * 180.0 / M_PI); // ת��Ϊ�Ƕ�
				if (startAngle < 0) {
					startAngle += 360;  // ȷ���Ƕ�Ϊ��
				}
			}
			else if (mode == PolygonMode) {
				// ��ȡ�������λ��
				int x = event->pos().x();
				int y = event->pos().y();
				currentPolygon.addPoint(Point(x, y)); // ��Ӷ���
				currentPolygon.color = currentLineColor; // �޸Ķ������ɫ
				shape.push_back(3);
				update(); // ���������ػ�
			}
			else if (mode == FillMode) {
				fills.push_back(Fill(Point(event->pos().x(), event->pos().y()), currentLineColor));
				shape.push_back(4);
			}
			else if (mode == TrimMode) {
				clipStartPoint = event->pos();  // ��¼��갴�µ�λ����Ϊ���
				drawingrect = true;
			}

			hasStartPoint = true;
			drawing = false; // ��ʼ��Ϊ������ʵ��ֱ��
		}
	}

	// ��������ɿ��¼�
	void mouseReleaseEvent(QMouseEvent* event) override
	{
		if (hasStartPoint)
		{
			if (mode == LineMode) {
				endPoint = event->pos();  // ֱ��ģʽ�£��ɿ��趨�յ�
				lines.append(Line(startPoint, endPoint, lineWidth, currentLineColor, algorithm));
			}
			else if (mode == CircleMode) {
				endPoint = event->pos();  // Բģʽ�£�����뾶
				radius = std::sqrt(std::pow(endPoint.x() - center.x(), 2) + std::pow(endPoint.y() - center.y(), 2));
				arcs.append(Arc(center, radius, startAngle, endAngle, lineWidth, currentLineColor));
			}
			else if (mode == ArcMode && counter == 0) {
				counter += 1;
			}
			else if (mode == ArcMode && counter == 1) {
				endPoint = event->pos();  // Բ��ģʽ�£�����뾶
				radius = std::sqrt(std::pow(endPoint.x() - center.x(), 2) + std::pow(endPoint.y() - center.y(), 2));
				arcs.append(Arc(center, radius, startAngle, endAngle, lineWidth, currentLineColor));
				counter = 0;
			}
			else if (event->button() == Qt::LeftButton) {
				if (drawingrect) {
					drawingrect = false;
					clipEndPoint = event->pos();
					double xmin = min(clipStartPoint.x(), clipEndPoint.x());
					double ymin = min(clipStartPoint.y(), clipEndPoint.y());
					double xmax = max(clipStartPoint.x(), clipEndPoint.x());
					double ymax = max(clipStartPoint.y(), clipEndPoint.y());

					for (Line& line : lines) {
						QLineF lineF(line.line);
						if (algorithm == SutherlandTrim) {
							cohenSutherlandClip(lineF, xmin, ymin, xmax, ymax);
						}
						else if (algorithm == MidTrim) {
							liangBarskyClip(lineF, xmin, ymin, xmax, ymax);
						}
						line.line.setP1(lineF.p1().toPoint());
						line.line.setP2(lineF.p2().toPoint());
					}
					update();
				}
				else if (hasStartPoint) {
					clipEndPoint = event->pos();
					if (mode == LineMode) {
						Line line(clipStartPoint, clipEndPoint, lineWidth, currentLineColor, DDA);
						lines.push_back(line);
					}
					hasStartPoint = false;
					update();
				}
			}

			drawing = true;         // �������
			update();               // �����ػ�
			hasStartPoint = false;  // ���ã������ٴλ����µ��߶�
			drawing = false;
		}
	}

	// ��������ƶ��¼�
	void mouseMoveEvent(QMouseEvent* event) override {
		if (hasStartPoint)
		{
			if (mode == LineMode) {
				// �����յ�Ϊ��ǰ���λ��
				endPoint = event->pos();
			}
			else if (mode == CircleMode) {
				QPoint currentPos = event->pos();
				radius = std::sqrt(std::pow(currentPos.x() - center.x(), 2) + std::pow(currentPos.y() - center.y(), 2));
			}
			else if (mode == ArcMode && counter == 0) {
				return;
			}
			else if (mode == ArcMode && counter == 1) {
				QPoint currentPos = event->pos();
				radius = std::sqrt(std::pow(currentPos.x() - center.x(), 2) + std::pow(currentPos.y() - center.y(), 2));

				// ���㵱ǰ���λ����Բ�ĵĽǶ�
				float angleRad = atan2(currentPos.y() - center.y(), currentPos.x() - center.x());
				endAngle = static_cast<int>(angleRad * 180.0 / M_PI); // ת��Ϊ�Ƕ�

				if (endAngle < 0) {
					endAngle += 360;  // ȷ���Ƕ�Ϊ��
				}
			else if (drawingrect) {
				clipEndPoint = event->pos();
				update();
			}
			}

			update(); // �����ػ�
		}
	}

	// ����˫������¼�
	void mouseDoubleClickEvent(QMouseEvent* event) override {
		// ˫���¼���յ�ǰ�����
		if (mode == PolygonMode) {
			if (currentPolygon.points.size() > 2) {
				currentPolygon.closePolygon(); // ��յ�ǰ�����
				polygons.push_back(currentPolygon); // ���浽������б�
				currentPolygon = Polygon(); // ���õ�ǰ������Կ�ʼ�µĻ���
				update();
			}
		}
	}

public:
	ShapeDrawer(QWidget* parent = nullptr) : QWidget(parent), mode(LineMode), hasStartPoint(false), drawing(false), drawingrect(false),
		radius(0), startAngle(0), endAngle(360)
	{
		// ���ñ�����ɫ
		QPalette pal = this->palette();
		pal.setColor(QPalette::Window, Qt::white); // �������ñ�����ɫΪ��ɫ
		this->setAutoFillBackground(true);  // �����Զ���䱳��
		this->setPalette(pal);

		// ��ͼ���ڴ�С��
		setFixedSize(800, 550);
		initMAP(MAP);
	}

	// ���õ�ǰ��ͼģʽ
	void setDrawMode(DrawMode newMode) {
		mode = newMode;
		update();  // ģʽ�л������»���
	}

	// ����������������ȵĺ���
	void setLineWidth(int width) {
		lineWidth = width;
		// update();  // ÿ�θı���ʱ���������»���
	}

	//  ���������õ�ǰ������ɫ
	void setCurrentLineColor(QColor color) {
		currentLineColor = color;
		update();
	}

	//  ���������õ�ǰ�㷨
	void setAlgorithm(Algorithm algo)
	{
		algorithm = algo;
		// update(); // �ı��㷨�����»���
	}

	// ����������Բ������ʼ�ǶȺͽ����Ƕ�
	void setArcAngles(int start, int end) {
		startAngle = start;
		endAngle = end;
		update();  // �����ػ�
	}
};

class MainWindow : public QWidget {
	// Q_OBJECT

private:
	ShapeDrawer* shapeDrawer;       // ���������״������
	QComboBox* modeComboBox;        // ����ѡ�����ģʽ����Ͽ�
	QComboBox* algorithmComboBox;   // ������ֱ���㷨 ѡ��ť
	QSlider* widthSlider;           // ����������������� ������
	QPushButton* colorButton;       // ��������ɫ ѡ��ť
	QComboBox* lineTypeComboBox;    // ���������� ѡ��ť

public:
	MainWindow(QWidget* parent = nullptr) : QWidget(parent) {
		shapeDrawer = new ShapeDrawer(this); // ������ͼ����

		// ����ѡ���ͼģʽ����Ͽ�
		modeComboBox = new QComboBox(this);
		modeComboBox->addItem("Line", LineMode);
		modeComboBox->addItem("Circle", CircleMode);
		modeComboBox->addItem("Arc", ArcMode);
		modeComboBox->addItem("Shape", PolygonMode);
		modeComboBox->addItem("Fill", FillMode);
		modeComboBox->addItem("Trim", TrimMode);

		// ��������������㷨ѡ��
		algorithmComboBox = new QComboBox(this);
		algorithmComboBox->addItem("Midpoint", Midpoint);
		algorithmComboBox->addItem("Bresenham", Bresenham);
		algorithmComboBox->addItem("DDA", DDA);
		algorithmComboBox->addItem("DashLine", DashLine);
		algorithmComboBox->addItem("Cohen-Sutherland", SutherlandTrim);
		algorithmComboBox->addItem("Midpoint Trim", MidTrim);

		// ���������������������������
		widthSlider = new QSlider(Qt::Horizontal, this);
		widthSlider->setRange(1, 15);  // ����������ȷ�ΧΪ 1 �� 15 ����
		widthSlider->setValue(5);      // ��ʼֵΪ 5 ����

		// ������ɫѡ��ť
		colorButton = new QPushButton("Choose Color", this);

		// ˮƽ���ֹ��������˴���ʱֻ���Ҳࣩ
		QVBoxLayout* rightLayout = new QVBoxLayout();
		rightLayout->addWidget(new QLabel("Select Mode:"));
		rightLayout->addWidget(modeComboBox);       // ����������ģʽѡ��
		rightLayout->addWidget(algorithmComboBox);  // ֱ�߶��㷨ѡ��
		rightLayout->addWidget(widthSlider);        // ����������������ӵ��Ҳ಼��
		rightLayout->addWidget(colorButton);        // ����������ɫ��ť��ӵ�����
		rightLayout->addStretch();

		// ��ֱ���ֹ�����
		QHBoxLayout* mainLayout = new QHBoxLayout(this);
		mainLayout->addWidget(shapeDrawer);
		mainLayout->addLayout(rightLayout);

		// �����������ѡ��仯�ź�
		connect(algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
			shapeDrawer->setAlgorithm(static_cast<Algorithm>(algorithmComboBox->currentData().toInt()));
			});

		// ���ӻ�������ֵ�仯�źŵ� shapeDrawer �� setLineWidth ����
		connect(widthSlider, &QSlider::valueChanged, shapeDrawer, &ShapeDrawer::setLineWidth);

		// ������ɫѡ��ť������ɫѡ����
		connect(colorButton, &QPushButton::clicked, [=]() {
			QColor color = QColorDialog::getColor(Qt::black, this, "Choose Line Color");
			if (color.isValid()) {
				shapeDrawer->setCurrentLineColor(color);
			}
			});

		// ������Ͽ��ѡ��仯�ź�
		// ��ѡ��ģʽ�ı�ʱ�����¼�
		connect(modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
			DrawMode selectedMode = static_cast<DrawMode>(modeComboBox->itemData(index).toInt());
			shapeDrawer->setDrawMode(selectedMode);
			if (selectedMode == CircleMode) {
				// �����Ի���������ʼ�ǶȺͽ����Ƕ�
				bool ok;
				int startAngle = QInputDialog::getInt(this, tr("Set Start Angle"), tr("Enter start angle (degrees):"), 0, 0, 360, 1, &ok);
				if (ok) {
					int endAngle = QInputDialog::getInt(this, tr("Set End Angle"), tr("Enter end angle (degrees):"), 360, 0, 360, 1, &ok);
					if (ok) {
						// ���� ShapeDrawer �е���ʼ�ǶȺͽ����Ƕ�
						shapeDrawer->setArcAngles(startAngle, endAngle);
					}
				}
			}
			});

		setLayout(mainLayout);
		setWindowTitle("Drawing with Algorithms");
		// �����ڴ�С��
		setFixedSize(1000, 600);
	}
};

void initMAP(vector<vector<pointData>>& MAP) {
	for (int i = 0; i < 1000; i++) {
		vector<pointData> row;
		MAP.push_back(row);
		for (int j = 0; j < 1000; j++) {
			//��ÿһ���е�ÿһ�н�����ӵ�
			pointData point(QPoint(i, j), Qt::white);
			MAP[i].push_back(point);
		}
	}
}

void clearMAP(vector<vector<pointData>>& MAP) {
	for (int i = 0; i < 800; i++) {
		for (int j = 0; j < 550; j++) {
			MAP[i][j].setColor(Qt::white);
		}
	}
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	MainWindow window;
	window.show();

	return app.exec();
}