#include "DerivativeTwoPhaseMaterial.h"

template<>
InputParameters validParams<DerivativeTwoPhaseMaterial>()
{
  InputParameters params = validParams<DerivativeBaseMaterial>();
  params.addClassDescription("Two phase material that combines two single phase materials using a switching function.");

  // Two base materials
  params.addRequiredParam<std::string>("fa_name", "Phase A material (at eta=0)");
  params.addRequiredParam<std::string>("fb_name", "Phase A material (at eta=1)");
  params.addParam<std::string>("h", "h", "Switching Function Material that provides h(eta)");
  params.addParam<std::string>("g", "g", "Barrier Function Material that provides g(eta)");

  // All arguments to the phase free energies
  params.addRequiredCoupledVar("args", "Arguments of fa and fb - use vector coupling");

  // Order parameter which determines the phase
  params.addRequiredCoupledVar("eta", "Order parameter");

  // Variables with applied tolerances and their tolerance values
  params.addParam<Real>("W", 0.0, "Energy barrier for the phase transformation from A to B");
  params.addParam<std::vector<Real> >("tol_values", std::vector<Real>(), "Vector of tolerance values for the variables in tol_names");

  return params;
}

DerivativeTwoPhaseMaterial::DerivativeTwoPhaseMaterial(const std::string & name,
                                                       InputParameters parameters) :
    DerivativeBaseMaterial(name, parameters),
    _eta(coupledValue("eta")),
    _eta_name(getVar("eta", 0)->name()),
    _eta_var(coupled("eta")),
    _fa_name(getParam<std::string>("fa_name")),
    _fb_name(getParam<std::string>("fb_name")),
    _h_name(getParam<std::string>("h")),
    _h(getMaterialProperty<Real>(_h_name)),
    _dh(getMaterialPropertyDerivative<Real>(_h_name, _eta_name)),
    _d2h(getMaterialPropertyDerivative<Real>(_h_name, _eta_name, _eta_name)),
    _d3h(getMaterialPropertyDerivative<Real>(_h_name, _eta_name, _eta_name, _eta_name)),
    _g_name(getParam<std::string>("g")),
    _g(getMaterialProperty<Real>(_h_name)),
    _dg(getMaterialPropertyDerivative<Real>(_g_name, _eta_name)),
    _d2g(getMaterialPropertyDerivative<Real>(_g_name, _eta_name, _eta_name)),
    _d3g(getMaterialPropertyDerivative<Real>(_g_name, _eta_name, _eta_name, _eta_name)),
    _W(getParam<Real>("W")),
    _prop_Fa(getMaterialProperty<Real>(_fa_name)),
    _prop_Fb(getMaterialProperty<Real>(_fb_name))
{
  // reserve space for phase A and B material properties
  _prop_dFa.resize(_nargs);
  _prop_d2Fa.resize(_nargs);
  _prop_d3Fa.resize(_nargs);
  _prop_dFb.resize(_nargs);
  _prop_d2Fb.resize(_nargs);
  _prop_d3Fb.resize(_nargs);
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _prop_dFa[i] = &getMaterialPropertyDerivative<Real>(_fa_name, _arg_names[i]);
    _prop_dFb[i] = &getMaterialPropertyDerivative<Real>(_fb_name, _arg_names[i]);

    _prop_d2Fa[i].resize(_nargs);
    _prop_d2Fb[i].resize(_nargs);

    // TODO: maybe we should reserve and initialize to NULL...
    if (_third_derivatives) {
      _prop_d3Fa[i].resize(_nargs);
      _prop_d3Fb[i].resize(_nargs);
    }

    for (unsigned int j = 0; j < _nargs; ++j)
    {
      _prop_d2Fa[i][j] = &getMaterialPropertyDerivative<Real>(_fa_name, _arg_names[i], _arg_names[j]);
      _prop_d2Fb[i][j] = &getMaterialPropertyDerivative<Real>(_fb_name, _arg_names[i], _arg_names[j]);

      if (_third_derivatives) {
        _prop_d3Fa[i][j].resize(_nargs);
        _prop_d3Fb[i][j].resize(_nargs);

        for (unsigned int k = 0; k < _nargs; ++k)
        {
          _prop_d3Fa[i][j][k] = &getMaterialPropertyDerivative<Real>(_fa_name, _arg_names[i], _arg_names[j], _arg_names[k]);
          _prop_d3Fb[i][j][k] = &getMaterialPropertyDerivative<Real>(_fb_name, _arg_names[i], _arg_names[j], _arg_names[k]);
        }
      }
    }
  }
}

Real
DerivativeTwoPhaseMaterial::computeF()
{
  return _h[_qp] * _prop_Fb[_qp] + (1.0 - _h[_qp]) * _prop_Fa[_qp] + _W * _g[_qp];
}

Real
DerivativeTwoPhaseMaterial::computeDF(unsigned int i_var)
{
  if (i_var == _eta_var)
    return _dh[_qp] * (_prop_Fb[_qp] - _prop_Fa[_qp]) + _W * _dg[_qp];
  else
  {
    unsigned int i = argIndex(i_var);
    return _h[_qp] * (*_prop_dFb[i])[_qp] + (1.0 - _h[_qp]) * (*_prop_dFa[i])[_qp];
  }
}

Real
DerivativeTwoPhaseMaterial::computeD2F(unsigned int i_var, unsigned int j_var)
{
  if (i_var == _eta_var && j_var == _eta_var)
    return _d2h[_qp] * (_prop_Fb[_qp] - _prop_Fa[_qp]) + _W * _d2g[_qp];

  unsigned int i = argIndex(i_var);
  unsigned int j = argIndex(j_var);

  if (i_var == _eta_var)
    return _dh[_qp] * ((*_prop_dFb[j])[_qp] - (*_prop_dFa[j])[_qp]);
  if (j_var == _eta_var)
    return _dh[_qp] * ((*_prop_dFb[i])[_qp] - (*_prop_dFa[i])[_qp]);

  return _h[_qp] * (*_prop_d2Fb[i][j])[_qp] + (1.0 - _h[_qp]) * (*_prop_d2Fa[i][j])[_qp];
}

Real
DerivativeTwoPhaseMaterial::computeD3F(unsigned int i_var, unsigned int j_var, unsigned int k_var)
{
  if (i_var == _eta_var && j_var == _eta_var && k_var == _eta_var)
    return _d3h[_qp] * (_prop_Fb[_qp] - _prop_Fa[_qp]) + _W * _d3g[_qp];

  unsigned int i = argIndex(i_var);
  unsigned int j = argIndex(j_var);
  unsigned int k = argIndex(k_var);

  if (j_var == _eta_var && k_var == _eta_var)
    return _d2h[_qp] * ((*_prop_dFb[i])[_qp] - (*_prop_dFa[i])[_qp]);
  if (i_var == _eta_var && k_var == _eta_var)
    return _d2h[_qp] * ((*_prop_dFb[j])[_qp] - (*_prop_dFa[j])[_qp]);
  if (i_var == _eta_var && j_var == _eta_var)
    return _d2h[_qp] * ((*_prop_dFb[k])[_qp] - (*_prop_dFa[k])[_qp]);

  if (i_var == _eta_var)
    return _dh[_qp] * (*_prop_d2Fb[j][k])[_qp] + (1.0 - _dh[_qp]) * (*_prop_d2Fa[j][k])[_qp];
  if (j_var == _eta_var)
    return _dh[_qp] * (*_prop_d2Fb[i][k])[_qp] + (1.0 - _dh[_qp]) * (*_prop_d2Fa[i][k])[_qp];
  if (k_var == _eta_var)
    return _dh[_qp] * (*_prop_d2Fb[i][j])[_qp] + (1.0 - _dh[_qp]) * (*_prop_d2Fa[i][j])[_qp];

  return _h[_qp] * (*_prop_d3Fb[i][j][k])[_qp] + (1.0 - _h[_qp]) * (*_prop_d3Fa[i][j][k])[_qp];
}
