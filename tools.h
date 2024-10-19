#ifndef TOOLS_H
#define TOOLS_H
#include <vector>
#include <QColor>
#include <QPoint>
using namespace std;

// ����SutherlandTrim
enum OutCode {
	INSIDE = 0, // 0000
	LEFT = 1,   // 0001
	RIGHT = 2,  // 0010
	BOTTOM = 4, // 0100
	TOP = 8     // 1000
};

// �������ͣ�ֱ�ߡ�Բ��������Ρ���䡢�ü����任������
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

// �߶λ����㷨
enum line_Algorithm {
	DDA,
	Bresenham,
	Midpoint,
	DashLine,
};

// �ü������㷨
enum clip_Algorithm {
	SutherlandTrim,
	MidTrim,
	CropPolygon
};

// ����ģʽ
enum transMode {
	MOVE,
	ZOOM,
	ROTATE,
	BEZIER,
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

#endif // !TOOLS_H
