#pragma once

namespace hawkes_market_making {

constexpr double kEpsilon = 1e-12;
constexpr double kDefaultRiskAversion = 1.0;
constexpr double kDefaultSpreadPenalty = 1.0;
constexpr int kMaxInventory = 100;

// Default Avellaneda-Stoikov parameters. They are the ones used in the numerical experiment
// of the original paper (section 3.3), so the default-constructed model can be checked
// directly against the published tables.
constexpr double kAsDefaultRiskAversion = 0.1;   // gamma
constexpr double kAsDefaultVolatility = 2.0;     // sigma
constexpr double kAsDefaultArrivalScale = 140.0; // A
constexpr double kAsDefaultArrivalDecay = 1.5;   // kappa
constexpr double kAsDefaultHorizon = 1.0;        // T

} // namespace hawkes_market_making
