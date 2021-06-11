/*
 * (C) Crown copyright 2021, Met Office
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#ifndef UFO_COMPOSITEOPER_OBSCOMPOSITETLAD_H_
#define UFO_COMPOSITEOPER_OBSCOMPOSITETLAD_H_

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "oops/base/Variables.h"
#include "oops/util/ObjectCounter.h"

#include "ufo/LinearObsOperatorBase.h"

// Forward declarations
namespace eckit {
  class Configuration;
}

namespace ioda {
  class ObsSpace;
  class ObsVector;
}

namespace ufo {
  class GeoVaLs;
  class ObsBias;
  class ObsDiagnostics;

// -----------------------------------------------------------------------------
/// Composite TL/AD observation operator class
class ObsCompositeTLAD : public LinearObsOperatorBase,
                        private util::ObjectCounter<ObsCompositeTLAD> {
 public:
  static const std::string classname() { return "ufo::ObsCompositeTLAD"; }

  ObsCompositeTLAD(const ioda::ObsSpace &, const eckit::Configuration &);
  ~ObsCompositeTLAD() override;

  void setTrajectory(const GeoVaLs &, const ObsBias &, ObsDiagnostics &) override;
  void simulateObsTL(const GeoVaLs &, ioda::ObsVector &) const override;
  void simulateObsAD(GeoVaLs &, const ioda::ObsVector &) const override;

  const oops::Variables & requiredVars() const override { return requiredVars_; }

  oops::Variables simulatedVars() const override;

 private:
  void print(std::ostream &) const override;

 private:
  std::vector<std::unique_ptr<LinearObsOperatorBase>> components_;
  oops::Variables requiredVars_;
};

// -----------------------------------------------------------------------------

}  // namespace ufo
#endif  // UFO_COMPOSITEOPER_OBSCOMPOSITETLAD_H_
