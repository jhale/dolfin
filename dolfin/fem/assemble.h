// Copyright (C) 2007-2013 Anders Logg
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// Modified by Garth N. Wells, 2008-2013.
// Modified by Johan Hake, 2009.
// Modified by Joachim B. Haga, 2012.
// Modified by Martin S. Alnaes, 2013.
//
// First added:  2007-01-17
// Last changed: 2013-02-13
//
// This file duplicates the Assembler::assemble* and SystemAssembler::assemble*
// functions in namespace dolfin, and adds special versions returning the value
// directly for scalars. For documentation, refer to Assemble.h and
// SystemAssemble.h

#ifndef __ASSEMBLE_H
#define __ASSEMBLE_H

#include <vector>

namespace dolfin
{

  class DirichletBC;
  class Form;
  class GenericMatrix;
  class GenericTensor;
  class GenericVector;
  template<typename T> class MeshFunction;

  //--- Copies of assembly functions in Assembler.h ---

  /// Assemble tensor
  void assemble(GenericTensor& A, const Form& a);

  /// Assemble system (A, b)
  void assemble_system(GenericMatrix& A,
                       GenericVector& b,
                       const Form& a,
                       const Form& L);

  /// Assemble system (A, b) and apply Dirichlet boundary condition
  void assemble_system(GenericMatrix& A,
                       GenericVector& b,
                       const Form& a,
                       const Form& L,
                       const DirichletBC& bc);

  /// Assemble system (A, b) and apply Dirichlet boundary conditions
  void assemble_system(GenericMatrix& A,
                       GenericVector& b,
                       const Form& a,
                       const Form& L,
                       const std::vector<const DirichletBC*> bcs);

  /// Assemble system (A, b) on sub domains and apply Dirichlet boundary
  /// conditions
  void assemble_system(GenericMatrix& A,
                       GenericVector& b,
                       const Form& a,
                       const Form& L,
                       const std::vector<const DirichletBC*> bcs,
                       const GenericVector& x0);

  //--- Specialized version for scalars ---

  /// Assemble scalar
  double assemble(const Form& a);

}

#endif
