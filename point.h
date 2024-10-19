#ifndef POINT_H
#define POINT_H

// �ṹ�����洢ÿ�������Ϣ��Ϊ����λ��Ʒ���
struct Point {
public:
	int x, y;
	Point(int x = 0, int y = 0) : x(x), y(y) {}
	Point(QPoint p) : x(p.x()), y(p.y()) {}

	int Getx() {
		return x;
	}
	int Gety() {
		return y;
	}

	constexpr inline int& rx() noexcept
	{
		return x;
	}

	constexpr inline int& ry() noexcept
	{
		return y;
	}

	// ���أ�֧�����
	Point operator-(const Point& b) {
		Point ret;
		ret.x = this->x - b.x;
		ret.y = this->y - b.y;
		return ret;
	}

	// ���أ�֧�����
	Point operator+(const Point& b) {
		Point ret;
		ret.x = this->x + b.x;
		ret.y = this->y + b.y;
		return ret;
	}
};

// ����ʵ��Bezier�����㵽��double����
class point2d {
public:
	double x, y;
	point2d() {}
	point2d(double X, double Y) :x(X), y(Y) {}
	void seX(double X) { x = X; }
	void seY(double Y) { x = Y; }
};
#endif // !POINT_H
