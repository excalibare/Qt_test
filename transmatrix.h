#ifndef TRANSMATRIX_H
#define TRANSMATRIX_H
#include<QPoint>
#include"point.h"

// �任����
class transMatrix {
private:
	int reference_x = 0, reference_y = 0;   //�ο���
	double tr[3][3] = { 1,0,0,
					  0,1,0,   //�任����
					  0,0,1 };

public:
	/**�������ʼ������**/
	transMatrix(int refer_X = 0, int refer_Y = 0) {  //�Բο���xy���죬Ĭ�϶�Ϊ0
		reference_x = refer_X;
		reference_y = refer_Y;
	}
	transMatrix(Point q) {   //������Pointֱ����Ϊ�ο��㹹�죬���ڳ���������꽻��
		reference_x = q.x;
		reference_y = q.y;
	}
	void setReference(int refer_X = 0, int refer_Y = 0) {  //��xy��Ϊ�����������òο���
		reference_x = refer_X;
		reference_y = refer_Y;
	}
	void setReference(Point q) {   //��Point���òο���
		reference_x = q.x;
		reference_y = q.y;
	}

	/**���ܺ���**/
	void Reset() {   //����������Ϊû���κα任Ч���ĳ�ֵ
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				if (i != j)
					tr[i][j] = 0;
				else
					tr[i][j] = 1;
			}
		}
	}

	void setRotateTrans(double angle) {   //������ת���ܣ����øú�������һ���Ƕ�ֵ���Զ�����������Ϊ��Ӧ�ı任���󣬲�����ԭ����
		Reset();
		angle = angle * 3.1415926 / 180;      //�Ƕ�ת����
		tr[0][0] = cos(angle);
		tr[0][1] = -sin(angle);
		tr[1][0] = -tr[0][1];
		tr[1][1] = tr[0][0];
	}

	void setZoomTrans(double z_X, double z_Y) {   //�������Ź��ܣ���������ת���ƣ�����Ϊx��y��������ű���
		Reset();
		tr[0][0] = z_X;
		tr[1][1] = z_Y;
	}

	void setMoveTrans(double m_X, double m_Y) {   //�����ƶ����ܣ���������ת���ƣ�����Ϊx��y������ƶ�����
		Reset();
		tr[0][2] = m_X;
		tr[1][2] = m_Y;
	}
	void setMoveTrans(Point movevec) {   //�����ƶ��������أ�ʹ��һ��point��Ϊ�����޸��ƶ�����
		Reset();
		tr[0][2] = movevec.Getx();
		tr[1][2] = movevec.Gety();
	}
	void setMoveTrans(QPoint movevec) {   //�����ƶ��������أ�ʹ��һ��Qpoint��Ϊ�����޸��ƶ�����
		Reset();
		tr[0][2] = movevec.x();
		tr[1][2] = movevec.y();
	}

	/**���������**/
	double* operator[](int index) {    //�±�����أ�����һ�η���һ����ָ�룬��Ҫ��ʹ��һ���±��ȡ��ָ��λ�þ���ʹ���Ϻ���ͨ��ά����һ�¡�
		return this->tr[index];
	}

	Point operator*(Point q) {       //���ҳ�һ��Point��ʱ������Ϊʩ�ӱ任������һ���任���QPoint
		float QMatrix[3] = { (float)-reference_x,(float)-reference_y,1 };//׼���������
		float QTransed[3] = { (float)reference_x,(float)reference_y,0 };//׼���������ĵ�����
		QMatrix[0] += q.x;          //�����ǼӶ����Ǹ�ֵ����������׼��������ʱ�����˲ο����������
		QMatrix[1] += q.y;          //��������Ϊ���������ĳ���ο�����з�������ת����ԭ�����ο�����Ҫ���������ƶ��任���������ֱ�������ڼӷ���
		float sum = 0;                //������ÿ��������ʼ�����յ�������Ԥ���ƶ��任�ķ�ʽ��ָ���ο���任��
		for (int i = 0; i < 3; ++i) { //����˷�
			sum = 0;
			for (int j = 0; j < 3; ++j) {
				sum += this->tr[i][j] * QMatrix[j];
			}
			QTransed[i] += sum;
		}
		q.rx() = (int)QTransed[0] + 0.5;//���õ��Ľ��ȡ����ȡ�����޸�QPoint������
		q.ry() = (int)QTransed[1] + 0.5;
		return q;
	}

	transMatrix operator*(transMatrix t) {  //���ҳ�ͬ�����ʱ��������ͨ����˷�����Ҫ����ͳһ�ο���
		float sum = 0;
		bool differRefer = false;
		transMatrix temp(t.reference_x, t.reference_y);//�����շ��ص����������Ҳ�任����Ĳο��㣬��Ϊ�Ҳ�������������������
		if (t.reference_y != reference_y || t.reference_x != reference_x) {   //����ο��㲻ͬ����Ҫ���ο���ͳһ
			differRefer = true;
			t.tr[0][2] -= reference_x;   //�����ƶ��任 �൱��ԭ�Ⱦ���������ʱ�����յ��ڳ˷��б��������˴�û��ֱ��������
			t.tr[1][2] -= reference_y;   //��˲ο�����˷�����ɶ���,�ȶ��Ҿ�����ƶ������ٶԼ��������ƶ���������ֱ�Ӽ�Ϊ��Ӧλ�üӷ���
		}
		for (int i = 0; i < 3; ++i) {  //��ͨ��������
			for (int j = 0; j < 3; ++j) {
				sum = 0;
				for (int k = 0; k < 3; ++k) {
					sum += this->tr[i][k] * t.tr[k][j];
				}
				temp[i][j] = sum;
			}
		}
		if (differRefer) {  //�����ƶ��任
			temp.tr[0][2] += reference_x;
			temp.tr[1][2] += reference_y;
		}
		return temp;
	}
};

#endif // !TRANSMATRIX_H
