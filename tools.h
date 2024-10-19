#ifndef TOOLS_H
#define TOOLS_H
#include <vector>
#include <QColor>
#include <QPoint>
using namespace std;

// 用于SutherlandTrim
enum OutCode {
	INSIDE = 0, // 0000
	LEFT = 1,   // 0001
	RIGHT = 2,  // 0010
	BOTTOM = 4, // 0100
	TOP = 8     // 1000
};

// 功能类型：直线、圆弧、多边形、填充、裁剪、变换、曲线
enum DrawMode {
	LineMode,
	ArcMode,
	CircleMode,
	PolygonMode,
	FillMode,
	TrimMode,
	TransMode,
	BezierMode,
	BsplineMode,
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

// 变形模式
enum transMode {
	MOVE,
	ZOOM,
	ROTATE,
	BEZIER,
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

#endif // !TOOLS_H
