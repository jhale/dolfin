// Copyright (C) 2006-2014 Anders Logg
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
// Modified by Garth N. Wells 2011
//
// First added:  2006-06-02
// Last changed: 2014-05-22

#include <dolfin/geometry/Point.h>
#include "IntervalCell.h"
#include "TriangleCell.h"
#include "Cell.h"
#include "MeshTopology.h"
#include "Facet.h"

using namespace dolfin;

//-----------------------------------------------------------------------------
double Facet::normal(std::size_t i) const
{
  const std::size_t D = _mesh->topology().dim();
  _mesh->init(D - 1);
  _mesh->init(D - 1, D);
  dolfin_assert(_mesh->ordered());

  // Get cell to which face belong (first cell when there is more than one)
  const Cell cell(*_mesh, this->entities(D)[0]);

  // Get local index of facet with respect to the cell
  const std::size_t local_facet = cell.index(*this);

  return cell.normal(local_facet, i);
}
//-----------------------------------------------------------------------------
Point Facet::normal() const
{
  const std::size_t D = _mesh->topology().dim();
  _mesh->init(D - 1);
  _mesh->init(D - 1, D);
  dolfin_assert(_mesh->ordered());

  // Get cell to which face belong (first cell when there is more than one)
  const Cell cell(*_mesh, this->entities(D)[0]);

  // Get local index of facet with respect to the cell
  const std::size_t local_facet = cell.index(*this);

  return cell.normal(local_facet);
}
//-----------------------------------------------------------------------------
double Facet::squared_distance(const Point& point) const
{
  if (_dim == 1)
  {
    // Extract vertices
    const MeshGeometry& geometry = _mesh->geometry();
    const unsigned int* vertices = entities(0);
    const Point a = geometry.point(vertices[0]);
    const Point b = geometry.point(vertices[1]);

    // Compute squared distance
    return IntervalCell::squared_distance(point, a, b);
  }
  else if (_dim == 2)
  {
    // Extract vertices
    const MeshGeometry& geometry = _mesh->geometry();
    const unsigned int* vertices = entities(0);
    const Point a = geometry.point(vertices[0]);
    const Point b = geometry.point(vertices[1]);
    const Point c = geometry.point(vertices[2]);

    // Compute squared distance
    return TriangleCell::squared_distance(point, a, b, c);
  }

  dolfin_error("Facet.cpp",
               "compute (squared) distance to facet",
               "Not implemented for facets of dimension %d", _dim);

  return 0.0;
}
//-----------------------------------------------------------------------------
bool Facet::exterior() const
{
  const std::size_t D = _mesh->topology().dim();
  if (this->num_global_entities(D) == 1)
    return true;
  else
    return false;
}
//-----------------------------------------------------------------------------
std::pair<const Cell, const Cell>
Facet::adjacent_cells(const std::vector<std::size_t>* facet_orientation) const
{
  dolfin_assert(num_entities(dim() + 1) == 2);

  // Get cell indices
  const std::size_t D = dim() + 1;
  const std::size_t c0 = entities(D)[0];
  const std::size_t c1 = entities(D)[1];

  // Normal ordering
  if (!facet_orientation || (*facet_orientation)[this->index()] == c0)
    return std::make_pair(Cell(mesh(), c0), Cell(mesh(), c1));

  // Sanity check
  if ((*facet_orientation)[this->index()] != c1)
  {
    dolfin_error("Facet.cpp",
                 "extract adjacent cells of facet",
                 "Illegal facet orientation specified, cell %d is not a neighbor of facet %d",
                 (*facet_orientation)[this->index()], index());
  }

  // Opposite ordering
  return std::make_pair(Cell(mesh(), c1), Cell(mesh(), c0));
}
//-----------------------------------------------------------------------------
