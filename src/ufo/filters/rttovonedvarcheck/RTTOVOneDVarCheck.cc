/*
 * (C) Copyright 2020 Met Office
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

/* 1-D var qc and retrieval of atmospheric state
 *   J(x) = (x-xb)T B-1 (x-xb) + (y-H(x))T R-1 (y-H(x))
 *   Code adapted from Met Office OPS System
 */

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

#include "ufo/filters/rttovonedvarcheck/RTTOVOneDVarCheck.h"
#include "ufo/filters/rttovonedvarcheck/RTTOVOneDVarCheck.interface.h"
#include "ufo/GeoVaLs.h"

#include "eckit/exception/Exceptions.h"

namespace ufo {

// -----------------------------------------------------------------------------

RTTOVOneDVarCheck::RTTOVOneDVarCheck(ioda::ObsSpace & obsdb, const Parameters_ & parameters,
                                 std::shared_ptr<ioda::ObsDataVector<int> > flags,
                                 std::shared_ptr<ioda::ObsDataVector<float> > obserr)
  : FilterBase(obsdb, parameters, flags, obserr), channels_(), retrieved_vars_(),
    hoxdiags_retrieved_vars_(), parameters_(parameters)
{
  oops::Log::trace() << "RTTOVOneDVarCheck contructor starting" << std::endl;

  // Check only one variable has been defined - BT
  // Get channels from filter variables
  if (filtervars_.size() != 1) {
     throw eckit::UserError("RTTOVOneDVarCheck contructor:"
                            " only one variable allowed, aborting.");
  }
  channels_ = filtervars_[0].channels();

  // Check at least one channel has been defined
  if (channels_.empty()) {
     throw eckit::UserError("RTTOVOneDVarCheck contructor: no channels defined, aborting.");
  }

  // Setup Fortran object
  ufo_rttovonedvarcheck_create_f90(keyRTTOVOneDVarCheck_, obsdb, parameters_.toConfiguration(),
              channels_.size(), channels_[0], retrieved_vars_, QCflags::onedvar, QCflags::pass);

  // Create hofxdiags
  for (size_t jvar = 0; jvar < retrieved_vars_.size(); ++jvar) {
    for (size_t jch = 0; jch < channels_.size(); ++jch) {
      hoxdiags_retrieved_vars_.push_back("brightness_temperature_jacobian_"+
                   retrieved_vars_[jvar]+"_"+std::to_string(channels_[jch]));
    }
  }

  // Populate variables list - which makes sure this is not run as a pre-process filter
  // because model data is needed
  Variables model_vars(retrieved_vars_);
  allvars_ += Variables(model_vars, "GeoVaLs");

  oops::Log::trace() << "RTTOVOneDVarCheck contructor complete. " << std::endl;
}

// -----------------------------------------------------------------------------

RTTOVOneDVarCheck::~RTTOVOneDVarCheck() {
  ufo_rttovonedvarcheck_delete_f90(keyRTTOVOneDVarCheck_);
  oops::Log::trace() << "RTTOVOneDVarCheck destructed" << std::endl;
}

// -----------------------------------------------------------------------------

void RTTOVOneDVarCheck::applyFilter(const std::vector<bool> & apply,
                               const Variables & filtervars,
                               std::vector<std::vector<bool>> &) const {
  oops::Log::trace() << "RTTOVOneDVarCheck Filter starting" << std::endl;

// Get GeoVaLs
  const ufo::GeoVaLs * gvals = data_.getGeoVaLs();

// Create oops variable with the list of channels
  oops::Variables variables = filtervars.toOopsVariables();

// Convert apply to char for passing to fortran
// needed for channel selection
  std::vector<char> apply_char(apply.size(), 'F');
  for (size_t i = 0; i < apply_char.size(); i++) {
    if (apply[i]) {apply_char[i]='T';}
  }

// Save qc flags to database for retrieval in fortran - needed for channel selection
  flags_->save("FortranQC");    // temporary measure as per ROobserror qc

// Pass it all to fortran
  ufo_rttovonedvarcheck_apply_f90(keyRTTOVOneDVarCheck_, parameters_.ModOptions.value(),
                                  variables, hoxdiags_retrieved_vars_,
                                  gvals->toFortran(),
                                  apply_char.size(), apply_char[0]);

// Read qc flags from database
  flags_->read("FortranQC");    // temporary measure as per ROobserror qc

  oops::Log::trace() << "RTTOVOneDVarCheck Filter complete" << std::endl;
}

// -----------------------------------------------------------------------------

void RTTOVOneDVarCheck::print(std::ostream & os) const {
  os << "RTTOVOneDVarCheck: config =  " << parameters_ << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace ufo
