// BinarySearchController
// A small, header-only templated helper that wraps two callables to perform
// a binary search over an input value to reach a desired measured output.
//
// Concept / contract:
// - ApplyFunc: callable taking a single float (input) and applying it to the
//   system (write DAC, PWM, set drive level...). Should return void.
// - ReadFunc: callable taking no arguments and returning float (measured
//   output corresponding to the last applied input). This function is
//   responsible for any required settling/waiting behavior for the hardware.
//
// Usage:
//   auto apply = [](float v){ analogWrite(dacPin, (int)v); };
//   auto read  = [&]()->float{ return (float)analogRead(sensorPin); };
//   BinarySearchController<decltype(apply), decltype(read)> bs(
//       apply, read, /*inMin*/0, /*inMax*/255, /*outMin*/0, /*outMax*/1023,
//       /*tolerance*/2.0f, /*maxIter*/20, /*increasing*/true);
//   float finalIn, finalOut;
//   bool ok = bs.setTarget(512.0f, &finalIn, &finalOut);
//
// Notes:
// - The class assumes the measured output is (mostly) monotonic with the
//   input. Set `increasing` to false if the measured value decreases when the
//   input increases.
// - The ReadFunc should handle any required delays/settling. This class does
//   not call delay() so it is safe for environments where Arduino.h is not
//   available.

#ifndef SRC_HELPERS_SEARCH_H
#define SRC_HELPERS_SEARCH_H

// Avoid <cmath> to keep compatibility with small Arduino toolchains.
// Provide a tiny fast absolute-value helper for floats.
static inline float absf(float v) { return v < 0.0f ? -v : v; }

template <typename ApplyFunc, typename ReadFunc>
class BinarySearchController {
public:
	// Constructor
	// inMin/inMax: allowed input domain
	// outMin/outMax: expected measured output domain (used only for quick checks)
	// tolerance: acceptable absolute error on the measured output
	// maxIter: safety cap on iterations
	// increasing: true if measured output increases when input increases
    // settle_cb: optional pointer to a function called after apply and before read
    // to allow hardware to settle (e.g., a small delay). The function pointer
    // must be non-capturing (can be a lambda `[](){ delay(5); }`).
    BinarySearchController(ApplyFunc apply, ReadFunc read,
                            float inMin, float inMax,
                            float outMin, float outMax,
                            float tolerance = 1.0f,
                            int maxIter = 20,
                            bool increasing = true,
                            void (*settle_cb)() = nullptr): 
                apply_(apply), read_(read),
                inMin_(inMin), inMax_(inMax), outMin_(outMin), outMax_(outMax),
                tol_(tolerance), maxIter_(maxIter), increasing_(increasing), settle_cb_(settle_cb) {}

	// Attempt to drive the measured output to `target` using binary search.
	// finalInput/finalOutput are optional out parameters that receive the
	// last applied input and measured output. Returns true if converged
	// within tolerance, false otherwise (still applies a final input).
	bool setTarget(float target, float* finalInput = nullptr, float* finalOutput = nullptr) {
		// quick sanity check: target should be within expected output bounds
		if (!(target >= outMin_ && target <= outMax_)) {
			// target outside expected range; refuse to run
			return false;
		}

		float low = inMin_;
		float high = inMax_;
		float bestInput = low;
		float bestOutput = 0.0f;

		for (int iter = 0; iter < maxIter_; ++iter) {
			float mid = (low + high) * 0.5f;

			// apply the mid input
			apply_(mid);

			// allow optional settle callback to run after applying the input
			if (settle_cb_) settle_cb_();

			// read measured output (ReadFunc should still handle any further waits)
			float measured = read_();

			bestInput = mid;
			bestOutput = measured;

			// check convergence
			if (absf(measured - target) <= tol_) {
				if (finalInput) *finalInput = bestInput;
				if (finalOutput) *finalOutput = bestOutput;
				return true;
			}

			// move search bounds depending on monotonic direction
			if (increasing_) {
				if (measured < target) {
					low = mid; // need more input
				} else {
					high = mid; // overshoot
				}
			} else {
				// measured decreases with increasing input
				if (measured < target) {
					high = mid; // increasing input would decrease measured further
				} else {
					low = mid;
				}
			}

			// if bounds close enough break early
			if (absf(high - low) <= 1e-6f) break;
		}

		// apply best found value one last time
		apply_(bestInput);
		bestOutput = read_();
		if (finalInput) *finalInput = bestInput;
		if (finalOutput) *finalOutput = bestOutput;
	return absf(bestOutput - target) <= tol_;
	}

	// Accessors
	float getInputMin() const { return inMin_; }
	float getInputMax() const { return inMax_; }
	float getTolerance() const { return tol_; }
	void setTolerance(float t) { tol_ = t; }
	void setMaxIter(int m) { maxIter_ = m; }
	// Provide a setter for the settle callback if you want to change it after
	// construction. The callback must be a non-capturing function or lambda.
	void setSettleCallback(void (*cb)()) { settle_cb_ = cb; }

private:
	ApplyFunc apply_;
	ReadFunc read_;
	float inMin_;
	float inMax_;
	float outMin_;
	float outMax_;
	float tol_;
	int maxIter_;
	bool increasing_;
	// Optional settle callback (non-capturing function pointer)
	void (*settle_cb_)();
};

#endif // SRC_HELPERS_SEARCH_H

