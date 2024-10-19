#ifndef BEZIER_H
#define BEZIER_H
#include <iostream>
#include <QPainter>
#include <QPoint>
#include <cmath>
#include "point.h"
using namespace std;

// Bezier曲线
class Bezier {
private:
	vector<vector<int>> const* brush;
protected:
	QPen& _pen;
	QPainter& painter;
	vector<point2d> controlPoints;
public:
	Bezier(int penWidth, QPainter& p, const QVector<QPoint>& points, QPen pen)
		: _pen(pen), painter(p) {
		for (int i = 0; i < points.size(); ++i) {
			controlPoints.emplace_back(double(points[i].x()), double(points[i].y()));
		}
	}
	point2d recursive_bezier(const std::vector<point2d>& control_points, double t)
	{
		// Casteljau
		int n = control_points.size();
		if (n == 1) return control_points[0];
		std::vector<point2d> res;
		for (int i = 0; i < n - 1; ++i) {
			res.emplace_back((1 - t) * control_points[i].x + t * control_points[i + 1].x, (1 - t) * control_points[i].y + t * control_points[i + 1].y);
		}
		return recursive_bezier(res, t);
	}

	void drawBezier()
	{
		if (controlPoints.size() < 2) return;

		const int precision = 100; // 曲线精度
		QVector<QPointF> curvePoints;

		for (int i = 0; i <= precision; ++i) {
			double t = i / double(precision);
			double x = 0.0;
			double y = 0.0;
			int n = controlPoints.size() - 1;
			for (int j = 0; j <= n; ++j) {
				double binomial_coeff = combination(n, j) * std::pow(t, j) * std::pow(1 - t, n - j);
				x += binomial_coeff * controlPoints[j].x;
				y += binomial_coeff * controlPoints[j].y;
			}
			curvePoints.append(QPointF(x, y));
		}

		QPen originalPen = painter.pen();
		painter.setPen(_pen);
		for (int i = 0; i < curvePoints.size() - 1; ++i) {
			painter.drawLine(curvePoints[i], curvePoints[i + 1]);
		}
		painter.setPen(originalPen);
	}
	inline int combination(int n, int k) {
		if (k == 0 || k == n) return 1;
		return combination(n - 1, k - 1) + combination(n - 1, k);
	}
};

// B样曲线
class Bspline
{
private:
	vector<point2d> controlPoints;
	int k_step; // k阶 需要自定义
	int n; // n + 1控制点（屏幕上画出）
	double* U; // 节点向量
	QPen& _pen;
	QPainter& painter;
public:
	Bspline(QPainter& p, const QVector<QPoint>& points, int k, QPen pen) : _pen(pen), painter(p)
	{
		painter.setPen(_pen);
		for (int i = 0; i < points.size(); ++i) {
			controlPoints.emplace_back(double(points[i].x()), double(points[i].y()));
		}
		k_step = k;
		n = controlPoints.size() - 1; // n + 1 控制点
		U = nullptr;
	}
	double coefficient(int i, int r, double u)
	{
		if (fabs(U[i + k_step - r] - U[i]) <= 1e-7)
			return 0;
		else
		{
			return (u - U[i]) / (U[i + k_step - r] - U[i]);
		}
	}
	double DeBoor_Cox_X(int i, int r, double u)
	{
		if (r == 0)
		{
			return controlPoints[i].x;
		}
		else
		{
			return coefficient(i, r, u) * DeBoor_Cox_X(i, r - 1, u) + (1 - coefficient(i, r, u)) * DeBoor_Cox_X(i - 1, r - 1, u);
		}
	}
	double DeBoor_Cox_Y(int i, int r, double u)
	{
		if (r == 0)
		{
			return controlPoints[i].y;
		}
		else
		{
			return coefficient(i, r, u) * DeBoor_Cox_Y(i, r - 1, u) + (1 - coefficient(i, r, u)) * DeBoor_Cox_Y(i - 1, r - 1, u);
		}
	}
	void drawBspline()
	{
		U = new double[n + k_step + 2];
		for (int i = 0; i < n + k_step + 2; ++i) {
			U[i] = i; // 均匀分布 所有控制点的权重求和为1
		}

		double step = 0.0005;
		for (int i = k_step - 1; i < n + 1; ++i) {
			for (double theta_u = U[i]; theta_u < U[i + 1]; theta_u += step) {
				int temp_x = round(DeBoor_Cox_X(i, k_step - 1, theta_u));
				int temp_y = round(DeBoor_Cox_Y(i, k_step - 1, theta_u));
				painter.drawPoint(temp_x, temp_y); // 使用QPainter绘制点
			}
		}
		delete[] U;
	}
};

#endif // !BEZIER_H
