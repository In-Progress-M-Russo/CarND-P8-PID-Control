#ifndef PID_H
#define PID_H

class PID {
 public:
  /**
   * Constructor
   */
  PID();

  /**
   * Destructor.
   */
  virtual ~PID();

  /**
   * Initialize PID.
   * @param (Kp_, Ki_, Kd_, do_tune_) The initial PID gains, tuning flag
   */
  void Init(double Kp_, double Ki_, double Kd_, bool do_tune_);

  /**
   * Initialize PID - case with tuning parameters.
   * @param (Kp_, Ki_, Kd_, do_tune_, init_it_, max_init_ ) The initial PID gains, tuning flag, iterations before
   * tuning, max iterations allowed for tuning.
   */
  void Init(double Kp_, double Ki_, double Kd_, bool do_tune_, int init_it_, int max_it_);

  /**
   * Set PID gains
   * @param (Kp_, Ki_, Kd_) The PID gains
   */
  void SetGains(double Kp_, double Ki_, double Kd_);

  /**
   * Get the current proportional gain.
   * @output Current value for Kp
   */
  double GetKp();

  /**
   * Get the current integral gain.
   * @output Current value for Ki
   */
  double GetKi();

  /**
   * Get the current integral gain.
   * @output Current value for Kd
   */
  double GetKd();

  /**
   * Get the current tuning flagn.
   * @output Current value for tuning flag
   */
  bool GetTuneFlag();

  /**
   * Update the PID error variables given cross track error.
   * @param cte The current cross track error
   */
  void UpdateError(double cte);

  /**
   * Calculate the total steering angle.
   * @output The total steering angle
   */
  double OutputSteeringAngle();

  /**
   * Tune PID gains using Coordinate Ascent (Twiddle) method.
   */
  void TuneGains();

 private:
  /**
   * PID Errors
   */
  double p_error;
  double i_error;
  double d_error;

  double s_error; // total cte squared (to be used by Tuning algorithm)

  /**
   * PID Coefficients
   */ 
  double Kp;
  double Ki;
  double Kd;

  /**
   * Parameters to be used for tuning (Twiddle)
   */
  bool do_tuning;                 // Flag indicating whether or not tuning is needed
  int it_count;                   // Counter of iterations
  int init_it;                    // Number of steps before running the algorithm
  int max_it;                     // Max number steps for running the algorithm
  double best_err;                // Best error, to be update
  double p[3];                    // Vector for the coefficients
  double dp[3];                   // Vector for coefficient changes
  int p_it;                       // Itreating index over p, dp vectors
  bool p_plus, p_minus,move_p_it; // Flags to use when iterating over p, dp vectors
  double threshold;               // Threshold for the tuning
  bool tuning_completed;          // Flag to indicate the end of the tuning phase
};

#endif  // PID_H