#pragma once

#include <stddef.h>

// This header intentionally has no Arduino dependency so its model calculation can
// be compiled and checked with the firmware translation unit.
namespace LoadFeatureModel {
constexpr size_t WINDOW_SIZE = 20;
constexpr size_t CELL_COUNT = 4;
constexpr size_t FEATURE_COUNT = 16;
constexpr double FEATURE_EPSILON = 1e-6;

struct Window {
  double samples[WINDOW_SIZE][CELL_COUNT];
};

struct FeatureVector {
  double values[FEATURE_COUNT];
};

#if __cplusplus >= 201402L
#define LOAD_FEATURE_CONSTEXPR constexpr
#else
#define LOAD_FEATURE_CONSTEXPR inline
#endif

LOAD_FEATURE_CONSTEXPR double absoluteValue(double value) {
  return value < 0.0 ? -value : value;
}

LOAD_FEATURE_CONSTEXPR double squareRootIteration(double value, double estimate,
                                                   int remainingIterations) {
  return remainingIterations == 0
             ? estimate
             : squareRootIteration(value, 0.5 * (estimate + value / estimate),
                                   remainingIterations - 1);
}

LOAD_FEATURE_CONSTEXPR double squareRoot(double value) {
  if (value <= 0.0) return 0.0;

  // Normalize before Newton iteration.  Seeding from an unscaled variance
  // makes a fixed iteration budget wildly inaccurate for HX711-scale values.
  double normalized = value;
  double scale = 1.0;
  while (normalized > 4.0) {
    normalized *= 0.25;
    scale *= 2.0;
  }
  while (normalized < 1.0) {
    normalized *= 4.0;
    scale *= 0.5;
  }
  return scale * squareRootIteration(normalized, 1.5, 12);
}

LOAD_FEATURE_CONSTEXPR FeatureVector calculateWindow(const Window& history,
                                                      size_t historyIndex) {
  double totalMean = 0.0;
  double w1Mean = 0.0;
  double w2Mean = 0.0;
  double w3Mean = 0.0;
  double w4Mean = 0.0;
  double frontMean = 0.0;
  double rearMean = 0.0;
  double leftMean = 0.0;
  double rightMean = 0.0;
  double cxMean = 0.0;
  double cyMean = 0.0;

  for (size_t index = 0; index < WINDOW_SIZE; ++index) {
    const size_t sourceIndex = (historyIndex + index) % WINDOW_SIZE;
    const double w1 = history.samples[sourceIndex][0];
    const double w2 = history.samples[sourceIndex][1];
    const double w3 = history.samples[sourceIndex][2];
    const double w4 = history.samples[sourceIndex][3];
    const double total = w1 + w2 + w3 + w4;
    const double front = w1 + w2;
    const double rear = w3 + w4;
    const double left = w1 + w3;
    const double right = w2 + w4;
    // These signed, per-sample Cx/Cy equations match the trained 1.ino model.
    const double cx = (w2 + w4) * 280.0 / (total + FEATURE_EPSILON);
    const double cy = (w3 + w4) * 700.0 / (total + FEATURE_EPSILON);

    totalMean += total;
    w1Mean += w1;
    w2Mean += w2;
    w3Mean += w3;
    w4Mean += w4;
    frontMean += front;
    rearMean += rear;
    leftMean += left;
    rightMean += right;
    cxMean += cx;
    cyMean += cy;
  }

  const double sampleCount = static_cast<double>(WINDOW_SIZE);
  totalMean /= sampleCount;
  w1Mean /= sampleCount;
  w2Mean /= sampleCount;
  w3Mean /= sampleCount;
  w4Mean /= sampleCount;
  frontMean /= sampleCount;
  rearMean /= sampleCount;
  leftMean /= sampleCount;
  rightMean /= sampleCount;
  cxMean /= sampleCount;
  cyMean /= sampleCount;

  double totalVariance = 0.0;
  double w1Variance = 0.0;
  double w2Variance = 0.0;
  double w3Variance = 0.0;
  double w4Variance = 0.0;
  double frontVariance = 0.0;
  double rearVariance = 0.0;
  double cxVariance = 0.0;
  double frontRearCovariance = 0.0;
  for (size_t index = 0; index < WINDOW_SIZE; ++index) {
    const size_t sourceIndex = (historyIndex + index) % WINDOW_SIZE;
    const double w1 = history.samples[sourceIndex][0];
    const double w2 = history.samples[sourceIndex][1];
    const double w3 = history.samples[sourceIndex][2];
    const double w4 = history.samples[sourceIndex][3];
    const double total = w1 + w2 + w3 + w4;
    const double front = w1 + w2;
    const double rear = w3 + w4;
    const double cx = (w2 + w4) * 280.0 / (total + FEATURE_EPSILON);

    totalVariance += (total - totalMean) * (total - totalMean);
    w1Variance += (w1 - w1Mean) * (w1 - w1Mean);
    w2Variance += (w2 - w2Mean) * (w2 - w2Mean);
    w3Variance += (w3 - w3Mean) * (w3 - w3Mean);
    w4Variance += (w4 - w4Mean) * (w4 - w4Mean);
    frontVariance += (front - frontMean) * (front - frontMean);
    rearVariance += (rear - rearMean) * (rear - rearMean);
    cxVariance += (cx - cxMean) * (cx - cxMean);
    frontRearCovariance += (front - frontMean) * (rear - rearMean);
  }

  const double frontStd = squareRoot(frontVariance / sampleCount);
  const double rearStd = squareRoot(rearVariance / sampleCount);
  FeatureVector features = {};
  // Do not reorder: model.c was trained against these exact 16 indices.
  features.values[0] = totalMean;
  features.values[1] = squareRoot(totalVariance / sampleCount);
  features.values[2] = w1Mean;
  features.values[3] = squareRoot(w1Variance / sampleCount);
  features.values[4] = w2Mean;
  features.values[5] = squareRoot(w2Variance / sampleCount);
  features.values[6] = w3Mean;
  features.values[7] = squareRoot(w3Variance / sampleCount);
  features.values[8] = w4Mean;
  features.values[9] = squareRoot(w4Variance / sampleCount);
  features.values[10] = (frontMean + FEATURE_EPSILON) / (rearMean + FEATURE_EPSILON);
  features.values[11] = (frontStd < FEATURE_EPSILON || rearStd < FEATURE_EPSILON)
                            ? 0.0
                            : (frontRearCovariance / sampleCount) / (frontStd * rearStd);
  features.values[12] = (leftMean + FEATURE_EPSILON) / (rightMean + FEATURE_EPSILON);
  features.values[13] = cxMean;
  features.values[14] = squareRoot(cxVariance / sampleCount);
  features.values[15] = cyMean;
  return features;
}

#if __cplusplus >= 201402L
constexpr Window makeSignedLowGolden() {
  Window history = {};
  for (size_t index = 0; index < WINDOW_SIZE; ++index) {
    history.samples[index][0] = -2.0;
    history.samples[index][1] = 4.0;
    history.samples[index][2] = 6.0;
    history.samples[index][3] = -1.0;
  }
  return history;
}

constexpr Window makeNegativeDenominatorGolden() {
  Window history = {};
  for (size_t index = 0; index < WINDOW_SIZE; ++index) {
    history.samples[index][0] = 0.0;
    history.samples[index][1] = -3.0;
    history.samples[index][2] = -2.0;
    history.samples[index][3] = -4.0;
  }
  return history;
}

constexpr Window makeVaryingSignedGolden() {
  Window history = {};
  for (size_t index = 0; index < WINDOW_SIZE; ++index) {
    const double sign = (index % 2 == 0) ? 1.0 : -1.0;
    history.samples[index][0] = sign * 8000000.0;
    history.samples[index][1] = sign * 4000000.0;
    history.samples[index][2] = sign * 2000000.0;
    history.samples[index][3] = sign * 1000000.0;
  }
  return history;
}

constexpr bool approximatelyEqual(double actual, double expected, double tolerance) {
  return absoluteValue(actual - expected) <= tolerance;
}

constexpr Window SIGNED_LOW_GOLDEN = makeSignedLowGolden();
constexpr FeatureVector SIGNED_LOW_GOLDEN_FEATURES = calculateWindow(SIGNED_LOW_GOLDEN, 0);
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[0], 7.0, 1e-9), "total mean");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[1], 0.0, 1e-9), "total std");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[2], -2.0, 1e-9), "w1 mean");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[3], 0.0, 1e-9), "w1 std");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[4], 4.0, 1e-9), "w2 mean");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[5], 0.0, 1e-9), "w2 std");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[6], 6.0, 1e-9), "w3 mean");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[7], 0.0, 1e-9), "w3 std");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[8], -1.0, 1e-9), "w4 mean");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[9], 0.0, 1e-9), "w4 std");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[10],
                                (2.0 + FEATURE_EPSILON) / (5.0 + FEATURE_EPSILON), 1e-9),
              "front rear ratio");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[11], 0.0, 1e-9), "correlation");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[12],
                                (4.0 + FEATURE_EPSILON) / (3.0 + FEATURE_EPSILON), 1e-9),
              "left right ratio");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[13], 120.0, 1e-3), "cx mean");
// Repeated division has a tiny constexpr floating-point residue; it is still
// below the model's FEATURE_EPSILON zero-variation threshold.
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[14], 0.0, 1e-6), "cx std");
static_assert(approximatelyEqual(SIGNED_LOW_GOLDEN_FEATURES.values[15], 500.0, 1e-3), "cy mean");

constexpr Window NEGATIVE_DENOMINATOR_GOLDEN = makeNegativeDenominatorGolden();
constexpr FeatureVector NEGATIVE_DENOMINATOR_GOLDEN_FEATURES =
    calculateWindow(NEGATIVE_DENOMINATOR_GOLDEN, 0);
static_assert(approximatelyEqual(NEGATIVE_DENOMINATOR_GOLDEN_FEATURES.values[10],
                                (-3.0 + FEATURE_EPSILON) / (-6.0 + FEATURE_EPSILON), 1e-9),
              "negative front rear denominator");
static_assert(approximatelyEqual(NEGATIVE_DENOMINATOR_GOLDEN_FEATURES.values[12],
                                (-2.0 + FEATURE_EPSILON) / (-7.0 + FEATURE_EPSILON), 1e-9),
              "negative left right denominator");

constexpr Window VARYING_SIGNED_GOLDEN = makeVaryingSignedGolden();
constexpr FeatureVector VARYING_SIGNED_GOLDEN_FEATURES =
    calculateWindow(VARYING_SIGNED_GOLDEN, 0);
constexpr double VARYING_CX_POSITIVE = 5000000.0 * 280.0 / (15000000.0 + FEATURE_EPSILON);
constexpr double VARYING_CX_NEGATIVE = -5000000.0 * 280.0 / (-15000000.0 + FEATURE_EPSILON);
constexpr double VARYING_CY_POSITIVE = 3000000.0 * 700.0 / (15000000.0 + FEATURE_EPSILON);
constexpr double VARYING_CY_NEGATIVE = -3000000.0 * 700.0 / (-15000000.0 + FEATURE_EPSILON);
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[0], 0.0, 1e-9),
              "varying total mean");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[1], 15000000.0, 1e-3),
              "varying total std");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[2], 0.0, 1e-9),
              "varying w1 mean");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[3], 8000000.0, 1e-3),
              "varying w1 std");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[4], 0.0, 1e-9),
              "varying w2 mean");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[5], 4000000.0, 1e-3),
              "varying w2 std");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[6], 0.0, 1e-9),
              "varying w3 mean");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[7], 2000000.0, 1e-3),
              "varying w3 std");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[8], 0.0, 1e-9),
              "varying w4 mean");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[9], 1000000.0, 1e-3),
              "varying w4 std");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[10], 1.0, 1e-12),
              "varying front rear ratio");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[11], 1.0, 1e-12),
              "varying front rear correlation");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[12], 1.0, 1e-12),
              "varying left right ratio");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[13],
                                (VARYING_CX_POSITIVE + VARYING_CX_NEGATIVE) / 2.0, 1e-9),
              "varying cx mean");
static_assert(VARYING_SIGNED_GOLDEN_FEATURES.values[14] > 0.0, "varying cx std is nonzero");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[14],
                                absoluteValue(VARYING_CX_POSITIVE - VARYING_CX_NEGATIVE) / 2.0, 1e-12),
              "varying cx std");
static_assert(approximatelyEqual(VARYING_SIGNED_GOLDEN_FEATURES.values[15],
                                (VARYING_CY_POSITIVE + VARYING_CY_NEGATIVE) / 2.0, 1e-9),
              "varying cy mean");
#endif

#undef LOAD_FEATURE_CONSTEXPR
}  // namespace LoadFeatureModel
