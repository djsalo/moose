/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSSEFFAUX_H
#define RICHARDSSEFFAUX_H

#include "AuxKernel.h"

#include "RichardsSeff.h"

//Forward Declarations
class RichardsSeffAux;

template<>
InputParameters validParams<RichardsSeffAux>();

/**
 * Calculates effective saturation for a specified variable
 */
class RichardsSeffAux: public AuxKernel
{
public:
  RichardsSeffAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  /**
   * The user object that defines effective saturation
   * as function of porepressure (or porepressures in the
   * multiphase situation)
   */
  const RichardsSeff & _seff_UO;

  /**
   * the porepressure values (this will be length N
   * where N is the number of arguments that the _seff_UO requires)
   * Eg, for twophase simulations N=2
   */
  std::vector<VariableValue *> _pressure_vals;
};

#endif // RICHARDSSEFFAUX_H
