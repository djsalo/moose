#include "AddPrimarySpeciesAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"


template<>
InputParameters validParams<AddPrimarySpeciesAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<NonlinearVariableName> >("primary_species", "The list of primary variables to add");

  return params;
}


AddPrimarySpeciesAction::AddPrimarySpeciesAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddPrimarySpeciesAction::act()
{
  std::vector<NonlinearVariableName> vars = getParam<std::vector<NonlinearVariableName> >("primary_species");

  for (unsigned int i=0; i < vars.size(); i++)
  {
    FEType fe_type(Utility::string_to_enum<Order>("first"),
                   Utility::string_to_enum<FEFamily>("lagrange"));
    Real scale_factor = 1.0;
    _problem->addVariable(vars[i], fe_type, scale_factor);
  }

}
