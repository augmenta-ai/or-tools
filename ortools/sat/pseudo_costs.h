// Copyright 2010-2024 Google LLC
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OR_TOOLS_SAT_PSEUDO_COSTS_H_
#define OR_TOOLS_SAT_PSEUDO_COSTS_H_

#include <vector>

#include "absl/log/check.h"
#include "ortools/base/logging.h"
#include "ortools/base/strong_vector.h"
#include "ortools/sat/integer.h"
#include "ortools/sat/linear_programming_constraint.h"
#include "ortools/sat/model.h"
#include "ortools/sat/sat_base.h"
#include "ortools/sat/sat_parameters.pb.h"
#include "ortools/sat/util.h"
#include "ortools/util/strong_integers.h"

namespace operations_research {
namespace sat {

// Pseudo cost of a variable is measured as average observed change in the
// objective bounds per unit change in the variable bounds.
class PseudoCosts {
 public:
  explicit PseudoCosts(Model* model);

  // This must be called before we are about to branch.
  // It will record the current objective bounds.
  void BeforeTakingDecision(Literal decision);

  // Updates the pseudo costs for the given decision given to
  // BeforeTakingDecision().
  void AfterTakingDecision(bool conflict = false);

  // Returns the variable with best reliable pseudo cost that is not fixed.
  IntegerVariable GetBestDecisionVar();

  // Returns the pseudo cost of given variable. Currently used for testing only.
  double GetCost(IntegerVariable var) const {
    CHECK_LT(var, pseudo_costs_.size());
    return pseudo_costs_[var].CurrentAverage();
  }

  // Visible for testing.
  // Returns the number of recordings of given variable.
  int GetNumRecords(IntegerVariable var) const {
    CHECK_LT(var, pseudo_costs_.size());
    return pseudo_costs_[var].NumRecords();
  }

  // Alternative pseudo-costs. This relies on the LP more heavily and is more
  // in line with what a MIP solver would do.
  double LpPseudoCost(IntegerVariable var, double down_fractionality) const;

  // Returns the pseudo cost "reliability".
  int LpReliability(IntegerVariable var) const;

  // Experimental alternative pseudo cost based on the explanation for bound
  // increases.
  void UpdateBoolPseudoCosts(absl::Span<const Literal> reason,
                             IntegerValue objective_increase);
  double BoolPseudoCost(Literal lit, double lp_value) const;

  // Visible for testing.
  // Returns the bound delta associated with this decision.
  struct VariableBoundChange {
    IntegerVariable var = kNoIntegerVariable;
    IntegerValue lower_bound_change = IntegerValue(0);
    double lp_increase = 0.0;
  };
  std::vector<VariableBoundChange> GetBoundChanges(Literal decision);

 private:
  double CombineCosts(double down_branch, double up_branch) const;

  // Returns the current objective info.
  struct ObjectiveInfo {
    std::string DebugString() const;

    IntegerValue lb = kMinIntegerValue;
    IntegerValue ub = kMaxIntegerValue;
    double lp_bound = -std::numeric_limits<double>::infinity();
    bool lp_at_optimal = false;
  };
  ObjectiveInfo GetCurrentObjectiveInfo();

  // Model object.
  const SatParameters& parameters_;
  IntegerTrail* integer_trail_;
  IntegerEncoder* encoder_;
  ModelLpValues* lp_values_;
  LinearProgrammingConstraintCollection* lps_;
  IntegerVariable objective_var_ = kNoIntegerVariable;

  // Saved info by BeforeTakingDecision().
  ObjectiveInfo saved_info_;
  std::vector<VariableBoundChange> bound_changes_;

  // Current IntegerVariable pseudo costs.
  std::vector<IntegerVariable> relevant_variables_;
  absl::StrongVector<IntegerVariable, bool> is_relevant_;
  absl::StrongVector<IntegerVariable, double> scores_;
  absl::StrongVector<IntegerVariable, IncrementalAverage> pseudo_costs_;

  // This version is mainly based on the lp relaxation.
  absl::StrongVector<IntegerVariable, IncrementalAverage>
      average_unit_objective_increase_;

  // This version is based on objective increase explanation.
  absl::StrongVector<LiteralIndex, IncrementalAverage> lit_pseudo_costs_;
};

}  // namespace sat
}  // namespace operations_research

#endif  // OR_TOOLS_SAT_PSEUDO_COSTS_H_
