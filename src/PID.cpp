#include "PID.h"
#include <limits>
#include <iostream>
#include <math.h>
#include <jmorecfg.h>

PID::PID() {}

PID::~PID() {}

void PID::Init(double Kp_, double Ki_, double Kd_, bool do_tune_) {
  /**
   * Initialize PID.
   * @param (Kp_, Ki_, Kd_, do_tune_) The initial PID coefficients, tuning flag
   */

  // Initialize gains
  Kp = Kp_;
  Ki = Ki_;
  Kd = Kd_;

  // Initialize errors
  p_error = 0.0;
  i_error = 0.0;
  d_error = 0.0;

  // Initialize tuning parameters
  do_tuning = do_tune_;

  if (do_tuning){

    // Initialize counters
    it_count = 0;       // Iteration counter
    init_it = 350;      // Number of steps before running the algorithm
    max_it = 1000;      // Max number steps for running the algorithm

    // Initialize vectors
    p[0] = Kp;
    p[1] = Ki;
    p[2] = Kd;

    dp[0] = Kp*0.1;     // Delta vector initialized to 10% of gains
    dp[1] = Ki*0.1;
    dp[2] = Kd*0.1;

    p_it = 0;           // Iterator over p, dp vectors

    p_plus = true;
    p_minus = true;
    move_p_it = false;

    // Initialize error and tolerance
    best_err = std::numeric_limits<double>::max();  //Initialize to high value
    threshold = 0.01;

    // Initialize completed tuning flag
    tuning_completed = false;
  }
}

void PID::Init(double Kp_, double Ki_, double Kd_, bool do_tune_, int init_it_, int max_it_ ) {
  /**
    * Initialize PID - case with tuning parameters.
    * @param (Kp_, Ki_, Kd_, do_tune_, init_it_, max_init_ ) The initial PID gains, tuning flag, iterations before
    * tuning, max iterations allowed for tuning.
    */

  // Initialize gains
  Kp = Kp_;
  Ki = Ki_;
  Kd = Kd_;

  // Initialize errors
  p_error = 0.0;
  i_error = 0.0;
  d_error = 0.0;

  // Initialize tuning parameters
  do_tuning = do_tune_;

  if (do_tuning){

    // Initialize counters
    it_count = 0;             // Iteration counter
    init_it = init_it_;       // Number of steps before running the algorithm
    max_it = max_it_;         // Max number steps for running the algorithm

    // Initialize vectors
    p[0] = Kp;
    p[1] = Ki;
    p[2] = Kd;

    dp[0] = Kp*0.1;           // Delta vector initialized to 10% of gains
    dp[1] = Ki*0.1;
    dp[2] = Kd*0.1;

    p_it = 0;                 // Iterator over p, dp vectors

    p_plus = true;
    p_minus = true;
    move_p_it = false;

    // Initialize error and tolerance
    best_err = std::numeric_limits<double>::max();  //Initialize to high value
    threshold = 0.01;

    // Initialize completed tuning flag
    tuning_completed = false;
  }
}

void PID::SetGains(double Kp_, double Ki_, double Kd_) {
  /**
   * Set PID gains
   * @param (Kp_, Ki_, Kd_) The PID gains
   */

  // Set gains
  Kp = Kp_;
  Ki = Ki_;
  Kd = Kd_;
}

double PID::GetKp() {
  /**
   * Get the current proportional gain.
   * @output Current value for Kp
   */

  // Get Kp
  return Kp;
}

double PID::GetKi() {
  /**
   * Get the current integral gain.
   * @output Current value for Ki
   */

  // Get Ki
  return Ki;
}


double PID::GetKd() {
  /**
   * Get the current derivative gain.
   * @output Current value for Kd
   */

  // Get Kd
  return Kd;
}

bool PID::GetTuneFlag() {
  /**
   * Get the current tuning flagn.
   * @output Current value for tuning flag
   */

  // Get tuning flag
  return do_tuning;
}

void PID::UpdateError(double cte) {
  /**
   * Update the PID error variables given cross track error.
   * @param cte: The current cross track error
   */

  // NOTE: Previous cte is stored in previous p_error, and so the calculation of i_error must happen before the
  // update of p_error
  d_error = cte - p_error;
  p_error = cte;
  i_error += cte;

  s_error = sqrt(pow(cte,2));

  // Count iterations
  it_count += 1;

  #ifdef PID_DEBUG
    std::cout << "*************************" << std::endl;
    std::cout<<"Iteration : " << it_count << std::endl;
  #endif
}

double PID::OutputSteeringAngle() {
  /**
   * Calculate the total steering angle.
   * @output The total steering angle
   */

  double steering = -Kp * p_error - Ki * i_error -Kd * d_error;

  return steering;
}

void PID::TuneGains() {
  /**
   * Tune PID gains using Coordinate Ascent (Twiddle) method.
   */


  // Run tuning algorithm only after an intial transient to allow the vehicle to accelerate. In any case stop before
  // reaching maximum iterations

  if ((it_count > init_it) && (it_count < max_it) && (!tuning_completed)){

    double dp_avg = (fabs(dp[0]/p[0]) + fabs(dp[1]/p[1]) + fabs(dp[2]/p[2])) / 3.0;

    if (dp_avg > threshold) {

      // Evaluate average dp against threshold
      #ifdef PID_DEBUG
        std::cout << "-------------------------" << std::endl;
        std::cout << "Average dp/p = " << dp_avg << " against threshold : " << threshold << std::endl;
        std::cout << "-------------------------" << std::endl;
        std::cout << "TUNING" << std::endl;
        std::cout << "Current p index : " << p_it << std::endl;
      #endif

      if (p_plus) {
        #ifdef PID_DEBUG
          std::cout << "Cycle start - Increment p[p_it] by dp[p_it] " << std::endl;
        #endif
        p[p_it] += dp[p_it];

        // Set gains
        SetGains(p[0], p[1], p[2]);

        p_plus = false;
      } else {
        if (s_error < best_err) {
          #ifdef PID_DEBUG
            std::cout << "Case number one: best error found, no operations executed. Increment dp[p_it] " << std::endl;
            std::cout << "Changing best error from : " << best_err << " to : " << s_error << std::endl;
          #endif
          best_err = s_error;
          dp[p_it] *= 1.1;

          move_p_it = true;
        } else {
          if (p_minus) {
            #ifdef PID_DEBUG
              std::cout << "Case number two: increment p executed but NO best error found. Decrement p[it] by "
                           "2*dp[p_it]" << std::endl;
            #endif
            p[p_it] -= 2 * dp[p_it];

            // Set gains
            SetGains(p[0], p[1], p[2]);

            p_minus = false;
          } else {
            #ifdef PID_DEBUG
              std::cout << "Case number three: increment and decrement executed, but NO best error found. Reset and "
                         "reduce increment" << std::endl;
            #endif
            p[p_it] += dp[p_it];

            // Set gains
            SetGains(p[0], p[1], p[2]);

            dp[p_it] *= 0.9;

            move_p_it = true;
          }
        }
      }

      // Loop to the next element in p, dp
      if (move_p_it) {
        // Increment p_it, looping over [0,1,2]
        p_it = (p_it + 1) % 3;

        // reset flags
        p_plus = true;
        p_minus = true;
        move_p_it = false;
      }
    }
    else{
      std::cout << "=========================" << std::endl;
      std::cout << "Tuning threshold crossed:" << std::endl;
      std::cout << "Tuned parameters:" << std::endl;
      std::cout << "Kp = " << GetKp() << " Ki = " << GetKi() << " Kd = " << GetKd() << std::endl;
      std::cout << "=========================" << std::endl;

      tuning_completed = true;
    }

    #ifdef PID_DEBUG
      std::cout << "-------------------------" << std::endl;
      std::cout << "Adjusted parameters ..." << std::endl;
      std::cout << "Kp = " << GetKp() << " Ki = " << GetKi() << " Kd = " << GetKd() << std::endl;
      std::cout << "dKp = " << dp[0] << " dKi = " << dp[1] << " Kd = " << dp[2] << std::endl;
      std::cout << "Error = " << s_error << std::endl;
    #endif
  }
}