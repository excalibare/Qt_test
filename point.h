#ifndef POINT_H
#define POINT_H

// 结构体来存储每个点的信息，为多边形绘制服务
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

	// 重载，支持相减
	Point operator-(const Point& b) {
		Point ret;
		ret.x = this->x - b.x;
		ret.y = this->y - b.y;
		return ret;
	}

	// 重载，支持相加
	Point operator+(const Point& b) {
		Point ret;
		ret.x = this->x + b.x;
		ret.y = this->y + b.y;
		return ret;
	}
};

// 用于实现Bezier，增广到了double精度
class point2d {
public:
	double x, y;
	point2d() {}
	point2d(double X, double Y) :x(X), y(Y) {}
	void seX(double X) { x = X; }
	void seY(double Y) { x = Y; }
};
#endif // !POINT_H
