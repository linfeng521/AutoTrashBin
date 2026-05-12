#include "kalman_1d_filter.h"

// 卡尔曼参数
static float last_estimate = 0.0f;	// 上一时刻最优估计值
static float process_noise = 0.01f; // 过程噪声
static float measure_noise = 0.5f;	// 测量噪声
static float error_cov = 1.0f;		// 上一时刻误差协方差

/**
 * @brief  一维卡尔曼滤波（适用于温湿度、电压、光照等模拟量）
 * @param  raw_data: 传感器原始数据
 * @retval 滤波后平滑数据
 */
float Kalman_Filter(float raw_data)
{
	if (last_estimate > 0 && fabs(raw_data - last_estimate) > 20.0f)
	{
		last_estimate = raw_data;
		error_cov = 1.0f;
		return last_estimate;
	}

	// 1.预测阶段
	float predict_estimate = last_estimate;			 // 预测状态,无速度，直接等于上一时刻值
	float predict_error = error_cov + process_noise; // 预测误差协方差
	// 2.更新阶段
	float kalman_gain = predict_error / (predict_error + measure_noise);			// 卡尔曼增益
	last_estimate = predict_estimate + kalman_gain * (raw_data - predict_estimate); // 更新最优估计
	error_cov = (1.0f - kalman_gain) * predict_error;								// 更新误差协方差

	return last_estimate;
}

//// 一维卡尔曼滤波
// static float xk = 0.0f;  // 上一次最优估计值
// static float Q = 0.01f;  // 过程噪声
// static float R = 0.2f;	  // 测量噪声
// static float Pk = 1.0f;  // 上一次估计误差协方差

// float klm(float zk)
//{
//	// 预测阶段
//	float xk_ = xk;		    // 状态预测，直接继承上一次估计值
//	float Pk_ = Pk + Q;     // 预测误差协方差

//	// 更新阶段
//	float Kk = Pk_ / (Pk_ + R);              // 卡尔曼增益
//	xk = xk_ + Kk * (zk - xk_);               // 更新最优估计值
//	Pk = (1.0f - Kk) * Pk_;                   // 更新误差协方差

//	return xk;
//}
