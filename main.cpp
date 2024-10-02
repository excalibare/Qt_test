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

//TODO:绘制弧线的 起始角 大于 结束角 功能未完善

// 用于SutherlandTrim
enum OutCode {
	INSIDE = 0, // 0000
	LEFT = 1,   // 0001
	RIGHT = 2,  // 0010
	BOTTOM = 4, // 0100
	TOP = 8     // 1000
};

// 用于实现填充（ pointData 与 MAP ）
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
// QVector<QPoint> cropPolygon(const QVector<QPoint>& polygon, const QVector<QPoint>& cropPolygon);

// 功能类型：直线、圆弧、多边形、填充、裁剪
enum DrawMode {
	LineMode,
	ArcMode,
	CircleMode,
	PolygonMode,
	FillMode,
	TrimMode
};

// 线段绘制算法
enum line_Algorithm {
	DDA,
	Bresenham,
	Midpoint,
	DashLine,
};

// 裁剪控制算法
enum clip_Algorithm {
	SutherlandTrim,
	MidTrim,
	CropPolygon
};

// 结构体来存储每条线的信息，包括起点、终点和线条宽度
struct Line {
	QLine line;
	int width;
	QColor colour;  // lyc:6
	line_Algorithm alg;
	// Qt::PenStyle penStyle; // 线型

	Line(QPoint p1, QPoint p2, int w, QColor c, line_Algorithm a) : line(p1, p2), width(w), colour(c), alg(a) {}
};

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

// 结构体来存储每个点的信息，为多边形绘制服务
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

// 结构体来存储多边形每个边的信息，为多边形绘制服务
struct Edge {
	int yMax;
	float xMin, slopeReciprocal;
};

// 多边形
class Polygon {
public:
	vector<Point> points; // 储存多边形的顶点
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
			points.push_back(points.front()); // 封闭多边形
		}
	}
};

// 填充（种子点）
class Fill {
public:
	Point point; // 储存种子点
	QColor color; // 储存填充颜色

	Fill(Point t_point, QColor t_color) :point(t_point), color(t_color) {}
};

class ShapeDrawer : public QWidget {
	// Q_OBJECT;
private:
	// 全局
	DrawMode mode;              // 当前绘制模式：直线 或 圆弧
	vector<vector<pointData>> MAP;
	int lineWidth = 5;          // 存储线条宽度
	bool hasStartPoint = false; // 是否有起点
	bool drawing = false;       // 是否正在绘制（用来控制鼠标释放时的动作）
	QColor currentLineColor = Qt::black;    // 当前线条颜色
	QVector<int> shape;          // 控制图形的重绘顺序，防止顺序错乱 1.直线 2.圆弧或圆 3.多边形
	float XL = 0, XR = 800, YB = 0, YT = 550;

	// 直线段
	QPoint startPoint;          // 线段的起点
	QPoint endPoint;            // 线段的终点
	line_Algorithm line_algo = Midpoint;  // 当前选择的直线段算法
	QVector<Line> lines;        // 存储已绘制的直线段

	// 圆、圆弧
	QPoint center;              // 圆心（对于圆弧）
	int radius;                 // 半径（对于圆弧）
	int startAngle, endAngle;   // 圆弧的起始和终止角度
	int counter = 0;            // 计数（点击鼠标次数，0-2循环）
	QVector<Arc> arcs;          // 存储已绘制的直线段
	
	// 多边形
	vector<Polygon> polygons; // 存储多个多边形
	Polygon currentPolygon; // 当前正在绘制的多边形
	vector<Fill> fills; // 存储多个多边形
	QStack<Point> stack;

	// 裁剪
	QPoint clipStartPoint;  // 裁剪窗口的起点
	QPoint clipEndPoint;    // 裁剪窗口的终点
	clip_Algorithm  clip_algo = SutherlandTrim;// 当前选择的直线段裁剪算法
	QVector<QPoint>cropPolygon; // 裁切多边形

protected:
	// 重写绘制事件
	void paintEvent(QPaintEvent* event) override
	{
		// 各类图形的计数器，控制从vector中取出的顺序
		int i1 = 0, i2 = 0, i3 = 0, i4 = 0;
		clearMAP(MAP);
		QPainter painter(this);

		// 每次刷新的时候重新绘制已存在直线
		for (int c = 0; c < shape.size(); c++) {
			QPen pen;
			// 重绘直线
			if (shape.at(c) == 1 && lines.size() > i1) {
				const Line& line = lines.at(i1++);
				pen.setColor(line.colour);  // 设置线条颜色(默认 black)
				pen.setWidth(line.width);   // 使用 width 设置线条粗细
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
			// 重绘圆弧 或 圆
			else if (shape.at(c) == 2 && arcs.size() > i2) {
				const Arc& arc = arcs.at(i2++);
				pen.setColor(arc.colour);   // 设置线条颜色(默认 black)
				pen.setWidth(arc.width);    // 使用 width 设置线条粗细
				painter.setPen(pen);
				if (arc.startAngle <= arc.endAngle) {
					drawMidpointArc(painter, arc.center, arc.radius, arc.startAngle, arc.endAngle);
				}
				else {
					drawMidpointArc(painter, arc.center, arc.radius, arc.startAngle, 360);
					drawMidpointArc(painter, arc.center, arc.radius, 0, arc.endAngle);
				}
			}
			// 重绘多边形
			else if (shape.at(c) == 3 && polygons.size() > i3) {
				const Polygon& polygon = polygons.at(i3++);
				QPen pen;
				pen.setColor(polygon.color);  // 设置线条颜色(默认 black)
				// pen.setWidth(lineWidth);		 // 使用 width 设置线条粗细
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
			// 重绘填充区域
			else if (shape.at(c) == 4 && fills.size() > i4) {
				const Fill& fill = fills.at(i4++);
				QPen pen;
				pen.setColor(fill.color);  // 设置线条颜色(默认 black)
				painter.setPen(pen);
				qDebug() << "In paintevent : " << fill.color << " " << Fill(fill).point.Getx() << " " << Fill(fill).point.Gety();
				fillShape(painter, fill.point, fill.color);
			}
		}

		// 绘制当前正在创建的多边形
		if (currentPolygon.points.size() > 1) {
			for (size_t i = 0; i < currentPolygon.points.size() - 1; ++i) {
				painter.drawLine(currentPolygon.points[i].x, currentPolygon.points[i].y, currentPolygon.points[i + 1].x, currentPolygon.points[i + 1].y);
			}
		}

		// 如果有起点，绘制线段的预览
		if (hasStartPoint) {
			QPen pen;
			QColor temp_color = Qt::gray;   // 设置预览线条颜色(灰色）
			pen.setColor(temp_color);
			pen.setWidth(lineWidth);        // 使用当前设置的线条宽度
			painter.setPen(pen);

			if (mode == LineMode) {
				if (line_algo == DashLine)
					drawPreviewDash(painter, startPoint, endPoint);
				else
					drawPreviewSolid(painter, startPoint, endPoint);
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
			else if (mode == TrimMode) {
				// 更新预览用画笔
				pen.setWidth(2);
				painter.setPen(pen);
				// 获取预览裁剪矩形框的四个顶点
				QPoint top_left(min(clipStartPoint.x(), clipEndPoint.x()), max(clipStartPoint.y(), clipEndPoint.y()));
				QPoint top_right(max(clipStartPoint.x(), clipEndPoint.x()), max(clipStartPoint.y(), clipEndPoint.y()));
				QPoint bottom_left(min(clipStartPoint.x(), clipEndPoint.x()), min(clipStartPoint.y(), clipEndPoint.y()));
				QPoint bottom_right(max(clipStartPoint.x(), clipEndPoint.x()), min(clipStartPoint.y(), clipEndPoint.y()));
				// 绘制预览矩形框
				drawPreviewDash(painter, top_left, top_right);
				drawPreviewDash(painter, top_right, bottom_right);
				drawPreviewDash(painter, bottom_right, bottom_left);
				drawPreviewDash(painter, bottom_left, top_left);
			}
		}

		// 如果有起点和终点，绘制实际线段
		if (drawing) {
			QPen pen;
			pen.setColor(currentLineColor); // 使用当前设置的颜色
			pen.setWidth(lineWidth);        // 使用当前设置的线条宽度
			painter.setPen(pen);

			if (mode == LineMode) {
				switch (line_algo) {
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
	}

	// 判定绘制的时候是否出界（针对MAP）
	// 写fill的lyc：谢谢你提供了这个东西
	// lyc：不是，如果width和height没有默认参数我为什么不自己写判断
	// lty: 那你自己设置个默认参数嘛，就自己动手丰衣足食，懂吧
	bool checkLegalPos(int x, int y, int width, int height) {
		if (x >= 0 && x <= width && y >= 0 && y <= height) {
			return true;
		}
		else
			return false;
	}

	// 绘制像素点与 MAP 对应的像素数据
	void drawPixel(int x, int y, QPainter& painter) {
		//不使用笔刷
		if (checkLegalPos(x, y, 800, 550)) {
			painter.drawPoint(x, y);
			// qDebug() << x << " " << y;
			MAP[x][y].setColor(painter.pen().color());
		}
	};

	// 这是...？什么？不知道！就放着吧~
	void drawPixel(int x, int y, vector<vector<pointData>>& MAP2, QColor color) {
		if (checkLegalPos(x, y, 800, 550)) {  // 确保像素位置合法
			MAP2[x][y].setColor(color);  // 更新 MAP2 的颜色
		}
	}

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

	// DDA 算法实现
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

	// Bresenham 算法实现
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

	// 中点算法实现
	void drawMidpointLine(QPainter& painter, QPoint p1, QPoint p2) {
		// 提取信息
		int x1 = p1.x();
		int y1 = p1.y();
		int x2 = p2.x();
		int y2 = p2.y();

		int x = x1, y = y1;	//赋初始点
		int dy = y1 - y2, dx = x2 - x1;
		int delta_x = (dx >= 0 ? 1 : (dx = -dx, -1));	//若dx>0则步长为1，否则为-1，同时dx变正
		int delta_y = (dy <= 0 ? 1 : (dy = -dy, -1));	//注意这里dy<0,才是画布中y的增长方向

		drawPixel(x, y, painter);		//画起始点

		int d, incrE, incrNE;
		if (-dy <= dx)		// 斜率绝对值 <= 1
			//这里-dy即画布中的dy
		{
			d = 2 * dy + dx;	//初始化判断式d
			incrE = 2 * dy;		//取像素E时判别式增量
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
		else				// 斜率绝对值 > 1
							// x和y情况互换
		{
			d = 2 * dx + dy;
			incrE = 2 * dx;
			incrNE = 2 * (dy + dx);
			while (y != y2)
			{
				if (d < 0)	//注意d变化情况
					d += incrE;
				else
					x += delta_x, d += incrNE;
				y += delta_y;
				drawPixel(x, y, painter);
			}
		}
	}

	// 虚线绘制 Bresenham 算法实现
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

		int dashLength = 8;   // 每段虚线的长度
		int gapLength = 8;    // 每段空白的长度
		int totalLength = dashLength + gapLength;  // 总周期长度
		int stepCount = 0;    // 计数步数，用于决定是否绘制

		while (x1 != x2 || y1 != y2) {
			// 只在虚线的部分绘制点
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

			stepCount++;  // 增加步数计数器
		}
	}

	// 中点圆弧算法实现
	void drawMidpointArc(QPainter& painter, QPoint center, int radius, int startAngle, int endAngle) {
		// 将角度转换为弧度
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

	// 绘制对称点，只绘制在角度范围内的点
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

	// 判断点是否在圆弧角度范围内并绘制
	void drawPointInArc(QPainter& painter, QPoint center, int x, int y, float startRad, float endRad) {
		float angle = atan2(y, x);
		if (angle < 0) angle += 2 * M_PI;
		if (angle >= startRad && angle <= endRad) {
			drawPixel(center.x() + x, center.y() + y,painter);
		}
	}

	// 扫描线算法
	void scanlineFill(QPainter& painter, const Polygon& polygon) {
		if (polygon.points.size() < 3) return;

		// 计算最小和最大 y 坐标
		int ymin = polygon.points[0].y, ymax = polygon.points[0].y;
		for (const Point& p : polygon.points) {
			ymin = min(ymin, p.y);
			ymax = max(ymax, p.y);
		}

		// 对每条扫描线从 ymin 到 ymax 进行处理
		for (int y = ymin; y <= ymax; ++y) {
			vector<int> intersections;

			// 计算每条边与当前扫描线的交点
			for (size_t i = 0; i < polygon.points.size() - 1; ++i) {
				const Point& p1 = polygon.points[i];
				const Point& p2 = polygon.points[i + 1];

				if ((p1.y <= y && p2.y > y) || (p1.y > y && p2.y <= y)) {
					int x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
					intersections.push_back(x);
				}
			}

			// 对交点进行排序
			sort(intersections.begin(), intersections.end());

			// 画出交点之间的线段(预览版，故使用Qt的函数)
			for (size_t i = 0; i < intersections.size(); i += 2) {
				painter.drawLine(intersections[i], y, intersections[i + 1], y);
			}
		}
	}

	// 种子点填充算法
	void fillShape(QPainter& painter, Point point, QColor newColor) {
		//将点击位置的颜色设置为需要替换的颜色
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

	// Sutherland 点编码
	OutCode computeOutCode(double x, double y, double xmin, double ymin, double xmax, double ymax) {
		OutCode code = INSIDE;

		if (x < xmin) code = OutCode(code | LEFT);
		else if (x > xmax) code = OutCode(code | RIGHT);
		if (y < ymin) code = OutCode(code | BOTTOM);
		else if (y > ymax) code = OutCode(code | TOP);

		return code;
	}

	// Sutherland 裁剪直线段算法
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

	// 中点算法裁剪直线段
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



	// 处理鼠标按下事件
	void mousePressEvent(QMouseEvent* event) override {
		if (!hasStartPoint) {
			if (mode == LineMode) {
				// 第一次点击：记录起点
				startPoint = event->pos();
				endPoint = startPoint;      // 初始化终点为起点
				shape.push_back(1);
			}
			else if (mode == CircleMode) {
				center = event->pos();      // 记录圆心
				hasStartPoint = true;
				shape.push_back(2);
			}
			else if (mode == ArcMode && counter == 0) {
				center = event->pos();      // 记录圆心
				shape.push_back(2);
			}
			else if (mode == ArcMode && counter == 1) {
				// 计算当前鼠标位置与圆心的角度
				float angleRad = atan2(event->pos().y() - center.y(), event->pos().x() - center.x());
				startAngle = static_cast<int>(angleRad * 180.0 / M_PI); // 转换为角度
				if (startAngle < 0) {
					startAngle += 360;  // 确保角度为正
				}
			}
			else if (mode == PolygonMode) {
				// 获取鼠标点击的位置
				int x = event->pos().x();
				int y = event->pos().y();
				currentPolygon.addPoint(Point(x, y)); // 添加顶点
				currentPolygon.color = currentLineColor; // 修改多边形颜色
				shape.push_back(3);
				update(); // 触发界面重绘
			}
			else if (mode == FillMode) {
				fills.push_back(Fill(Point(event->pos().x(), event->pos().y()), currentLineColor));
				shape.push_back(4);
			}
			else if (mode == TrimMode) {
				clipStartPoint = event->pos();  // 记录鼠标按下的位置作为起点
				clipEndPoint = clipStartPoint;
			}
			//else if (mode == TrimMode && clip_algo == CropPolygon) {
			//	QPoint a;
			//	a.setX(event->pos().x());
			//	a.setY(event->pos().y());
			//	cropPolygon.append(a);
			//	shape.push_back(5);
			//	update();
			//}

			hasStartPoint = true;
			drawing = false; // 初始化为不绘制实际直线
		}
	}

	// 处理鼠标松开事件
	void mouseReleaseEvent(QMouseEvent* event) override
	{
		if (hasStartPoint)
		{
			if (mode == LineMode) {
				endPoint = event->pos();  // 直线模式下，松开设定终点
				lines.append(Line(startPoint, endPoint, lineWidth, currentLineColor, line_algo));
			}
			else if (mode == CircleMode) {
				endPoint = event->pos();  // 圆模式下，计算半径
				radius = std::sqrt(std::pow(endPoint.x() - center.x(), 2) + std::pow(endPoint.y() - center.y(), 2));
				arcs.append(Arc(center, radius, startAngle, endAngle, lineWidth, currentLineColor));
			}
			else if (mode == ArcMode && counter == 0) {
				counter += 1;
			}
			else if (mode == ArcMode && counter == 1) {
				endPoint = event->pos();  // 圆弧模式下，计算半径
				radius = std::sqrt(std::pow(endPoint.x() - center.x(), 2) + std::pow(endPoint.y() - center.y(), 2));
				arcs.append(Arc(center, radius, startAngle, endAngle, lineWidth, currentLineColor));
				counter = 0;
			}
			else if (mode == TrimMode) {
				// drawingrect = false;
				clipEndPoint = event->pos();
				// 确定裁剪矩形的左上、右下两个顶点
				double xmin = min(clipStartPoint.x(), clipEndPoint.x());
				double ymin = min(clipStartPoint.y(), clipEndPoint.y());
				double xmax = max(clipStartPoint.x(), clipEndPoint.x());
				double ymax = max(clipStartPoint.y(), clipEndPoint.y());

				// 更新在裁剪框内的直线的起始与结束端点
				for (Line& line : lines) {
					QLineF lineF(line.line);
					if (clip_algo == SutherlandTrim) {
						cohenSutherlandClip(lineF, xmin, ymin, xmax, ymax);
					}
					else if (clip_algo == MidTrim) {
						liangBarskyClip(lineF, xmin, ymin, xmax, ymax);
					}
					line.line.setP1(lineF.p1().toPoint());
					line.line.setP2(lineF.p2().toPoint());
				}
			}

			drawing = true;         // 绘制完成
			update();               // 触发重绘
			hasStartPoint = false;  // 重置，允许再次绘制新的线段
			drawing = false;
		}
	}

	// 处理鼠标移动事件
	void mouseMoveEvent(QMouseEvent* event) override {
		if (hasStartPoint)
		{
			if (mode == LineMode) {
				// 更新终点为当前鼠标位置
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

				// 计算当前鼠标位置与圆心的角度
				float angleRad = atan2(currentPos.y() - center.y(), currentPos.x() - center.x());
				endAngle = static_cast<int>(angleRad * 180.0 / M_PI); // 转换为角度

				if (endAngle < 0) {
					endAngle += 360;  // 确保角度为正
				}
			}
			else if (mode == TrimMode) {
				// 这个else if为什么要放在 ArcMode 里面？？？
				// 放里面当然用不了重绘啦
				clipEndPoint = event->pos();
			}

			update(); // 触发重绘
		}
	}

	// 处理双击鼠标事件
	void mouseDoubleClickEvent(QMouseEvent* event) override {
		// 双击事件封闭当前多边形
		if (mode == PolygonMode) {
			if (currentPolygon.points.size() > 2) {
				currentPolygon.closePolygon(); // 封闭当前多边形
				polygons.push_back(currentPolygon); // 保存到多边形列表
				currentPolygon = Polygon(); // 重置当前多边形以开始新的绘制
				update();
			}
		}
	}

	// 处理按键事件
	void keyPressEvent(QKeyEvent* event) override {
		if (event->key() == Qt::Key_Escape) {
			// 裁切多边形绘制结束
			if (mode == TrimMode && clip_algo == CropPolygon) {
				//	update();
				//	QVector<QVector<QPoint>> newPolygon;
				//	// QVector<QPen> newBrush;
				//	vector<int> deleteIndex;
				//	int k = 0;
				//	for (int i = 0; i < shape.length(); i++) {
				//		if (shape.at(i) == 3) {
				//			// QPen pen = _brush.at(i);
				//			QVector<QPoint> polygon = crop_Polygon(polygons.at(k++).points, cropPolygon);
				//			if (polygon.length() >= 3) {
				//				// 如果返回的多边形的长度大于等于3，则说明裁切后的多边形不为空，将他们追加到新的多边形数组中
				//				newPolygon.append(polygon);
				//				// newBrush.append(pen);
				//			}
				//			deleteIndex.push_back(i);
				//		}
				//	}
				//	sort(deleteIndex.rbegin(), deleteIndex.rend());
				//	for (int i = 0; i < deleteIndex.size(); ++i) {
				//		_polygon.remove(deleteIndex.size() - i - 1);
				//		shape.remove(deleteIndex[i]);
				//		// _brush.remove(deleteIndex[i]);
				//	}
				//	// 找到裁切多边形，并删除它
				//	for (int i = 0; i < shape.length(); i++) {
				//		if (shape.at(i) == 8) {
				//			shape.remove(i);
				//			// _brush.remove(i);
				//		}
				//	}
				//	cropPolygon.clear();
				//	for (int i = 0; i < newPolygon.length(); ++i) {
				//		_polygon.append(newPolygon.at(i));
				//		// _brush.append(newBrush.at(i));
				//		shape.append(7);
				//		update();
				//	}
				//}
			}
		}
	}

public:
	ShapeDrawer(QWidget* parent = nullptr) : QWidget(parent), mode(LineMode), hasStartPoint(false), drawing(false),
		radius(0), startAngle(0), endAngle(360)
	{
		// 设置背景颜色
		QPalette pal = this->palette();
		pal.setColor(QPalette::Window, Qt::white); // 这里设置背景颜色为白色
		this->setAutoFillBackground(true);  // 启用自动填充背景
		this->setPalette(pal);

		// 绘图窗口大小↓
		setFixedSize(800, 550);
		initMAP(MAP);
	}

	// 设置当前绘图模式
	void setDrawMode(DrawMode newMode) {
		mode = newMode;
		update();  // 模式切换后重新绘制
	}

	// 新增：设置线条宽度的函数
	void setLineWidth(int width) {
		lineWidth = width;
		// update();  // 每次改变宽度时，触发重新绘制
	}

	//  新增：设置当前线条颜色
	void setCurrentLineColor(QColor color) {
		currentLineColor = color;
		update();
	}

	//  新增：设置当前算法
	void setAlgorithm(line_Algorithm algo)
	{
		line_algo = algo;
		// update(); // 改变算法后重新绘制
	}

	// 新增设置裁剪直线段算法
	void setclipAlgorithm(clip_Algorithm algo)
	{
		clip_algo = algo;
	}

	// 新增：设置圆弧的起始角度和结束角度
	void setArcAngles(int start, int end) {
		startAngle = start;
		endAngle = end;
		update();  // 触发重绘
	}
};

class MainWindow : public QWidget {
	// Q_OBJECT

private:
	ShapeDrawer* shapeDrawer;       // 负责绘制形状的区域
	QComboBox* modeComboBox;        // 用于选择绘制模式的组合框
	QComboBox* line_algorithmComboBox;   // 新增：直线算法 选择按钮
	QComboBox* clip_algorithmComboBox;	// 新增：直线段裁剪算法 选择按钮
	QSlider* widthSlider;           // 新增：控制线条宽度 滑动条
	QPushButton* colorButton;       // 新增：颜色 选择按钮
	QComboBox* lineTypeComboBox;    // 新增：线型 选择按钮

public:
	MainWindow(QWidget* parent = nullptr) : QWidget(parent) {
		shapeDrawer = new ShapeDrawer(this); // 创建绘图区域

		// 创建选择绘图模式的组合框
		modeComboBox = new QComboBox(this);
		modeComboBox->addItem("Line", LineMode);
		modeComboBox->addItem("Circle", CircleMode);
		modeComboBox->addItem("Arc", ArcMode);
		modeComboBox->addItem("Shape", PolygonMode);
		modeComboBox->addItem("Fill", FillMode);
		modeComboBox->addItem("Trim", TrimMode);

		// 创建下拉框并添加算法选项
		line_algorithmComboBox = new QComboBox(this);
		line_algorithmComboBox->addItem("Midpoint", Midpoint);
		line_algorithmComboBox->addItem("Bresenham", Bresenham);
		line_algorithmComboBox->addItem("DDA", DDA);
		line_algorithmComboBox->addItem("DashLine", DashLine);

		// 创建下拉框并添加裁剪算法选项
		clip_algorithmComboBox = new QComboBox(this);
		clip_algorithmComboBox->addItem("Cohen-Sutherland", SutherlandTrim);
		clip_algorithmComboBox->addItem("Midpoint Trim", MidTrim);

		// 新增：创建滑动条控制线条宽度
		widthSlider = new QSlider(Qt::Horizontal, this);
		widthSlider->setRange(1, 15);  // 设置线条宽度范围为 1 到 15 像素
		widthSlider->setValue(5);      // 初始值为 5 像素

		// 创建颜色选择按钮 
		colorButton = new QPushButton("Choose Painter Color", this);

		// 水平布局管理器（此处暂时只有右侧）
		QVBoxLayout* rightLayout = new QVBoxLayout();
		rightLayout->addWidget(new QLabel("Select Painting Mode:"));
		rightLayout->addWidget(modeComboBox);       // 新增：绘制模式选择
		rightLayout->addWidget(new QLabel("Select Line Mode:"));
		rightLayout->addWidget(line_algorithmComboBox);  // 新增：直线段算法选择
		rightLayout->addWidget(new QLabel("Select Clip Mode:"));
		rightLayout->addWidget(clip_algorithmComboBox);  // 新增：直线段裁剪算法选择
		rightLayout->addWidget(new QLabel("Select Line Width:"));
		rightLayout->addWidget(widthSlider);        // 新增：将滑动条添加到右侧布局
		rightLayout->addWidget(colorButton);        // 新增：将颜色按钮添加到布局
		rightLayout->addStretch();

		// 垂直布局管理器
		QHBoxLayout* mainLayout = new QHBoxLayout(this);
		mainLayout->addWidget(shapeDrawer);
		mainLayout->addLayout(rightLayout);

		// 连接下拉框的选择变化信号
		connect(line_algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
			shapeDrawer->setAlgorithm(static_cast<line_Algorithm>(line_algorithmComboBox->currentData().toInt()));
			});

		// 连接下拉框的选择变化信号
		connect(clip_algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
			shapeDrawer->setclipAlgorithm(static_cast<clip_Algorithm>(clip_algorithmComboBox->currentData().toInt()));
			});

		// 连接滑动条的值变化信号到 shapeDrawer 的 setLineWidth 函数
		connect(widthSlider, &QSlider::valueChanged, shapeDrawer, &ShapeDrawer::setLineWidth);

		// 连接颜色选择按钮，打开颜色选择器
		connect(colorButton, &QPushButton::clicked, [=]() {
			QColor color = QColorDialog::getColor(Qt::black, this, "Choose Line Color");
			if (color.isValid()) {
				shapeDrawer->setCurrentLineColor(color);
			}
			});

		// 连接组合框的选择变化信号
		// 当选择模式改变时触发事件
		connect(modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
			DrawMode selectedMode = static_cast<DrawMode>(modeComboBox->itemData(index).toInt());
			shapeDrawer->setDrawMode(selectedMode);
			if (selectedMode == CircleMode) {
				// 弹出对话框，输入起始角度和结束角度
				bool ok;
				int startAngle = QInputDialog::getInt(this, tr("Set Start Angle"), tr("Enter start angle (degrees):"), 0, 0, 360, 1, &ok);
				if (ok) {
					int endAngle = QInputDialog::getInt(this, tr("Set End Angle"), tr("Enter end angle (degrees):"), 360, 0, 360, 1, &ok);
					if (ok) {
						// 设置 ShapeDrawer 中的起始角度和结束角度
						shapeDrawer->setArcAngles(startAngle, endAngle);
					}
				}
			}
			});

		setLayout(mainLayout);
		setWindowTitle("Drawing with Algorithms");
		// 主窗口大小↓
		setFixedSize(1000, 600);
	}
};

void initMAP(vector<vector<pointData>>& MAP) {
	for (int i = 0; i < 1000; i++) {
		vector<pointData> row;
		MAP.push_back(row);
		for (int j = 0; j < 1000; j++) {
			//对每一行中的每一列进行添加点
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

//bool outsideOneEdgeOfPolygon(QVector<QPoint> polygon, QPoint p, int x) {
//	QPoint p1 = polygon[x];
//	QPoint p2 = polygon[(x + 1) % int(polygon.length())];
//	QPoint p3 = polygon[(x + 2) % int(polygon.length())];
//	int a = p2.y() - p1.y();
//	int b = p1.x() - p2.x();
//	int c = p2.x() * p1.y() - p1.x() * p2.y();
//	if (a < 0) {
//		a = -a;
//		b = -b;
//		c = -c;
//	}
//	int pointD = a * p.x() + b * p.y() + c;
//	int polyNextPointD = a * p3.x() + b * p3.y() + c;
//	if (pointD * polyNextPointD >= 0) {
//		return true;
//	}
//	else {
//		return false;
//	}
//}
//
//QPoint intersection(QPoint p1, QPoint p2, QPoint p3, QPoint p4) {
//	QPoint p;
//	double a1, b1, c1, a2, b2, c2, x, y;
//	a1 = p2.y() - p1.y();
//	b1 = p1.x() - p2.x();
//	c1 = p2.x() * p1.y() - p1.x() * p2.y();
//	a2 = p4.y() - p3.y();
//	b2 = p3.x() - p4.x();
//	c2 = p4.x() * p3.y() - p3.x() * p4.y();
//	x = int((b1 * c2 - b2 * c1) / (a1 * b2 - a2 * b1));
//	y = int((a2 * c1 - a1 * c2) / (a1 * b2 - a2 * b1));
//	if ((x <= max(p1.x(), p2.x()) && x >= min(p1.x(), p2.x()))) {
//		p.setX(x);
//		p.setY(y);
//		return p;
//	}
//	else {
//		return QPoint(-1, -1);
//	}
//}
//
//QVector<QPoint> crop_Polygon(const QVector<QPoint>& polygon, const QVector<QPoint>& cropPolygon) {
//	QVector<QPoint> result, originalPolygon = polygon;
//	for (int i = 0; i < cropPolygon.length(); ++i) {
//		result.clear();
//		for (int j = 0; j < originalPolygon.length(); ++j) {
//			QPoint p1 = originalPolygon[j];
//			QPoint p2 = originalPolygon[(j + 1) % int(originalPolygon.length())];
//			bool p1_inPolygonEdge = outsideOneEdgeOfPolygon(cropPolygon, p1, i);
//			bool p2_inPolygonEdge = outsideOneEdgeOfPolygon(cropPolygon, p2, i);
//			if (p1_inPolygonEdge && p2_inPolygonEdge) {
//				result.append(p2);
//			}
//			else if (p1_inPolygonEdge || p2_inPolygonEdge) {
//				QPoint k1 = cropPolygon[i];
//				QPoint k2 = cropPolygon[(i + 1) % int(cropPolygon.length())];
//				QPoint temp = intersection(p1, p2, k1, k2);
//				if (temp.x() != -1 && temp.y() != -1) {
//					result.append(temp);
//				}
//			}
//			if (!p1_inPolygonEdge && p2_inPolygonEdge) {
//				result.append(p2);
//			}
//		}
//		originalPolygon = result;
//	}
//	return result;
//}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	MainWindow window;
	window.show();

	return app.exec();
}