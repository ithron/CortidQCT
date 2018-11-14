/**
 * @file      SIMesh.cpp
 *
 * @brief     This file contains the implementation of the SIMesh file writer.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "SIMesh.h"
#include "Mesh.h"
#include "MeshHelpers.h"
#include "Optional.h"

#include <Eigen/Core>
#include <igl/adjacency_list.h>
#include <igl/edges.h>

#include <fstream>

namespace CortidQCT {
namespace Internal {

namespace {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
template <class T> struct Vertex {
  Eigen::Matrix<T, 3, 1> position;
  std::vector<typename Mesh<T>::Index> adjacentVertexIndices;
  std::vector<typename Mesh<T>::Index> incidentEdgeIndices;
  std::optional<typename Mesh<T>::Label> label;
};
#pragma clang diagnostic pop

template <class T> struct Edge {
  Eigen::Matrix<T, 2, 1> vertexIndices;
  Eigen::Matrix<T, 2, 1> vertexNeighboursIndices;
};

template <class T> struct Facet {
  Eigen::Matrix<T, 3, 1> vertexIndices;
  Eigen::Matrix<T, 3, 1> edgeIndices;
};

template <class T>
std::ostream &operator<<(std::ostream &out, Vertex<T> const &v) {
  out << v.position.transpose() << " " << v.adjacentVertexIndices.size() << " ";
  for (auto n : v.adjacentVertexIndices) out << n << " ";
  for (auto e : v.incidentEdgeIndices) out << e << " ";
  if (v.label) {
    out << *v.label;
  } else {
    out.seekp(-1, out.cur);
  }
  return out;
}

template <class T>
std::ostream &operator<<(std::ostream &out, Edge<T> const &e) {
  return out << e.vertexIndices.transpose() << " "
             << e.vertexNeighboursIndices.transpose();
}

template <class T>
std::ostream &operator<<(std::ostream &out, Facet<T> const &f) {
  return out << f.vertexIndices.transpose() << " " << f.edgeIndices.transpose();
}

} // namespace

template <class T>
void writeToSIMesh(Mesh<T> const &mesh, std::string const &filename,
                   bool includeLabels) {
  using namespace Eigen;
  using namespace gsl;
  using std::size_t;

  VertexMatrix<T> const V = vertexMatrix(mesh);
  FacetMatrix const F = facetMatrix(mesh);
  LabelVector const L = labelVector(mesh);

  std::vector<std::vector<typename Mesh<T>::Index>> vertexAdjacencyList;
  igl::adjacency_list(F, vertexAdjacencyList, true);

  Matrix<Index, Dynamic, 2> E;
  igl::edges(F, E);

  using Idx = typename Mesh<T>::Index;
  std::vector<Vertex<T>> vertices(mesh.vertexCount());
  std::vector<Edge<Idx>> edges(narrow_cast<size_t>(E.rows()));
  std::vector<Facet<Idx>> facets(mesh.triangleCount());

  typename Mesh<T>::Index vIdx{0};
  for (auto &&adjacentVertices : vertexAdjacencyList) {
    auto &vi = vertices[narrow_cast<size_t>(vIdx)];
    vi.position = V.row(narrow_cast<Index>(vIdx));
    vi.adjacentVertexIndices = adjacentVertices;

    if (includeLabels) { vi.label = L(vIdx); }

    for (auto j = 0; j < E.rows(); ++j) {
      if (E(j, 0) == vIdx || E(j, 1) == vIdx) {
        vi.incidentEdgeIndices.push_back(narrow_cast<Idx>(j));
      }
    }
    ++vIdx;
  }

  // Populate edge array
  for (auto i = 0; i < E.rows(); ++i) {
    auto &ei = edges[narrow_cast<size_t>(i)];
    ei.vertexIndices[0] = E(i, 0);
    ei.vertexIndices[1] = E(i, 1);

    auto j = 0;
    for (auto u :
         vertices[narrow_cast<size_t>(E(i, 0))].adjacentVertexIndices) {
      if (u == E(i, 1)) break;
      ++j;
    }
    ei.vertexNeighboursIndices[0] = j;

    j = 0;
    for (auto u :
         vertices[narrow_cast<size_t>(E(i, 1))].adjacentVertexIndices) {
      if (u == E(i, 0)) break;
      ++j;
    }
    ei.vertexNeighboursIndices[1] = j;
  }

  // Populate facet array
  for (auto i = 0; i < F.rows(); ++i) {
    auto &fi = facets[narrow_cast<size_t>(i)];

    fi.vertexIndices = F.row(i).transpose();

    for (auto k = 0; k < 3; ++k) {
      auto const vi = F(i, k);
      auto const vj = F(i, (k + 1) % 3);

      for (auto j = 0; j < E.rows(); ++j) {
        if ((E(j, 0) == vi && E(j, 1) == vj) ||
            (E(j, 0) == vj && E(j, 1) == vi)) {
          fi.edgeIndices[k] = j;
          break;
        }
      }
    }
  }

  // Output
  std::ofstream out(filename);
  out << mesh.vertexCount() << " " << mesh.triangleCount() << " "
      << edges.size() << std::endl;
  out << "Vertices" << std::endl;
  for (auto const &v : vertices) out << v << std::endl;
  out << "Edges" << std::endl;
  for (auto const &e : edges) out << e << std::endl;
  out << "Facets" << std::endl;
  for (auto const &f : facets) out << f << std::endl;
}

template void writeToSIMesh<float>(Mesh<float> const &, std::string const &,
                                   bool);
template void writeToSIMesh<double>(Mesh<double> const &, std::string const &,
                                    bool);

} // namespace Internal
} // namespace CortidQCT
