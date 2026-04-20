#include "kalman_1d_filter.h"

// 卡尔曼参数
static float xk = 0.0f; // 上一时刻最优估计值
static float Q = 0.01f; // 过程噪声
static float R = 0.2f;	// 测量噪声
static float Pk = 1.0f; // 上一时刻误差协方差

float klm(float zk)
{

	// 预测阶段
	float xk_ = xk;		// 预测状态,无速度，直接等于上一时刻值
	float Pk_ = Pk + Q; // 预测误差协方差
	// 更新阶段
	float Kk = Pk_ / (Pk_ + R); // 卡尔曼增益
	xk = xk_ + Kk * (zk - xk_); // 更新最优估计值
	Pk = (1.0f - Kk) * Pk_;		// 更新误差协方差

	return xk;
}
