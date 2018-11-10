#include "kalman_filter.h"

using Eigen::MatrixXd;
using Eigen::VectorXd;

// Please note that the Eigen library does not initialize 
// VectorXd or MatrixXd objects with zeros upon creation.

KalmanFilter::KalmanFilter() {}

KalmanFilter::~KalmanFilter() {}

void KalmanFilter::Init(VectorXd &x_in, MatrixXd &P_in, MatrixXd &F_in,
                        MatrixXd &H_in, MatrixXd &R_in, MatrixXd &Q_in) {
  x_ = x_in;
  P_ = P_in;
  F_ = F_in;
  H_ = H_in;
  R_ = R_in;
  Q_ = Q_in;
}

void KalmanFilter::Predict() {
  x_ = F_ * x_;
  MatrixXd Ft = F_.transpose();
  P_ = F_ * P_ * Ft + Q_;
}

void KalmanFilter::Update(const VectorXd &z) {
  VectorXd z_pred = H_ * x_;
  VectorXd y = z - z_pred;
  MatrixXd Ht = H_.transpose();
  MatrixXd S = H_ * P_ * Ht + R_;
  MatrixXd Si = S.inverse();
  MatrixXd PHt = P_ * Ht;
  MatrixXd K = PHt * Si;

  // new estimate
  x_ = x_ + (K * y);
  long x_size = x_.size();
  MatrixXd I = MatrixXd::Identity(x_size, x_size);
  P_ = (I - K * H_) * P_;
}

/**
 * same as regular KF, except use
 * Hj => Jacobian matrix, set in FusionEKF
 * Fj => sensor noise, usually from sensor manufacturer, assumed to be 0 for this project
 * H_func => calculated from Multivariable Taylor Series, used instead of z_pred for radar
 */
void KalmanFilter::UpdateEKF(const VectorXd &z) {
  float px = x_(0);
  float py = x_(1);
  float vx = x_(2);
  float vy = x_(3);
  
  // increase values for eq1 if too small to avoid divide-by-zero error
  if (px < 0.00001) {
    px += 0.001;
  }
  if (py < 0.00001) {
    py += 0.001;
  }
  float eq1 = sqrt(pow(px, 2) + pow(py, 2));
  VectorXd H_func(3);
  H_func << eq1, atan2(py, px), (px * vx + py * vy) / eq1;
  
  VectorXd y = z - H_func;
  // adjust y(1) arctan value to be within -M_PI and M_PI
  while (y(1) < -M_PI) {
    y(1) += 2 * M_PI;
  }
  while (y(1) > M_PI) {
    y(1) -= 2 * M_PI;
  }
  MatrixXd Ht = H_.transpose();
  MatrixXd S = H_ * P_ * Ht + R_;
  MatrixXd Si = S.inverse();
  MatrixXd PHt = P_ * Ht;
  MatrixXd K = PHt * Si;

  // new estimate
  x_ = x_ + (K * y);
  long x_size = x_.size();
  MatrixXd I = MatrixXd::Identity(x_size, x_size);
  P_ = (I - K * H_) * P_;
}
