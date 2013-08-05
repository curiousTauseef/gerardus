/* 
 * CGAL_ALPHA_SHAPE3  Alpha-shape of a 3D set of points
 *
 * [ALPHALIM, TRI] = cgal_alpha_shape3(X, [], NCOMP)
 *
 *   X is a 3-column matrix. X(i, :) contains the xyz-coordinates of a 3D
 *   point.
 *
 *   NCOMP is a scalar with the number of connected components in the output
 *   alpha shape. By default, NCOMP=1.
 *
 *   ALPHALIM is a 3 vector:
 *
 *     ALPHALIM(1): minimum in the range of possible alpha values.
 *     ALPHALIM(2): optimal alpha value for NCOMP connected  components.
 *     ALPHALIM(3): maximum in the range of possible alpha values.
 *
 *   TRI is a cell containing a 3-column matrix with the alpha-shape
 *   triangulation produced by ALPHALIM(2). This alpha-shape has NCOMP
 *   connected components. Each row contains the 3 nodes that form one
 *   triangular facet in the mesh. The mesh can be visualised running:
 *
 *     >> trisurf(tri{1}, xyz)
 *
 * [~, TRI] = cgal_alpha_shape3(X, ALPHA)
 *
 *   ALPHA is a vector of scalar ALPHA values. TRI is a cell array of the
 *   same length as ALPHA. Cell TRI{i} contains the alpha shape
 *   triangulation for ALPHA{i}.
 *
 * This function uses CGAL's implementation of alpha shapes [1]. For
 * efficiency, all alpha shape's are computed internally, and then only
 * those requested by the user are extracted and provided at the output.
 *
 * [1] http://www.cgal.org/Manual/latest/doc_html/cgal_manual/Alpha_shapes_3/Chapter_main.html
 *
 */

 /*
  * Author: Ramon Casero <rcasero@gmail.com>
  * Copyright © 2013 University of Oxford
  * Version: 0.1.0
  * $Rev$
  * $Date$
  *
  * University of Oxford means the Chancellor, Masters and Scholars of
  * the University of Oxford, having an administrative office at
  * Wellington Square, Oxford OX1 2JD, UK. 
  *
  * This file is part of Gerardus.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details. The offer of this
  * program under the terms of the License is subject to the License
  * being interpreted in accordance with English Law and subject to any
  * action against the University of Oxford being under the jurisdiction
  * of the English Courts.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see
  * <http://www.gnu.org/licenses/>.
  */

#ifndef CGALALPHASHAPE3
#define CGALALPHASHAPE3

/* mex headers */
#include <mex.h>

/* C++ headers */
#include <iostream>

/* Gerardus headers */
#include "MatlabImportFilter.h"
#include "MatlabExportFilter.h"

/* CGAL headers */
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/Alpha_shape_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
// vertex
typedef CGAL::Triangulation_vertex_base_with_info_3<mwSize, K>  Vb;
typedef CGAL::Alpha_shape_vertex_base_3<K, Vb>       AsVb;
// cell
typedef CGAL::Alpha_shape_cell_base_3<K>             Fb;
// triangulation structure: vertex and cell
typedef CGAL::Triangulation_data_structure_3<AsVb, Fb> Tds;
typedef CGAL::Delaunay_triangulation_3<K, Tds>       Delaunay;
typedef CGAL::Alpha_shape_3<Delaunay>                Alpha_shape_3;

typedef K::Point_3                                   Point;
typedef std::pair<Point, mwIndex>                    PointWithIndex;
typedef Alpha_shape_3::Alpha_iterator                Alpha_iterator;

typedef Alpha_shape_3::Facet                         Facet;

/*
 * mexFunction(): entry point for the mex function
 */
void mexFunction(int nlhs, mxArray *plhs[], 
		 int nrhs, const mxArray *prhs[]) {

  // indices for inputs and outputs
  enum InputIndexType {IN_X, IN_ALPHA, IN_NCOMP, InputIndexType_MAX};
  enum OutputIndexType {OUT_ALPHAINT, OUT_TRI, OutputIndexType_MAX};

  // interface to deal with input arguments from Matlab
  MatlabImportFilter::Pointer matlabImport = MatlabImportFilter::New();
  matlabImport->SetMatlabArgumentsPointer(nrhs, prhs);

  // check the number of input arguments
  matlabImport->CheckNumberOfArguments(1, InputIndexType_MAX);

  // interface to deal with outputs to Matlab
  MatlabExportFilter::Pointer matlabExport = MatlabExportFilter::New();
  matlabExport->SetMatlabArgumentsPointer(nlhs, plhs);

  // check number of outputs the user is asking for
  matlabExport->CheckNumberOfArguments(0, OutputIndexType_MAX);

  // if the set of points is empty, the outputs are empty too
  if (mxIsEmpty(prhs[IN_X])) {
    plhs[0] = mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL);
    plhs[1] = mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL);
    return;
  }

  // default coordinates are NaN values, so that the user can spot
  // whether there was any problem reading them
  Point xDef(mxGetNaN(), mxGetNaN(), mxGetNaN());

  // get size of input matrix with the points
  mwSize nrowsX = mxGetM(prhs[IN_X]);
  mwSize ncolsX = mxGetN(prhs[IN_X]);
  if (ncolsX != 3) {
    mexErrMsgTxt("X must have 3 columns");
  }

  // read points from function
  std::vector<PointWithIndex> x(nrowsX);
  for (mwIndex i = 0; i < nrowsX; ++i) {

    // exit if user pressed Ctrl+C
    ctrlcCheckPoint(__FILE__, __LINE__);

    // read i-th row of input matrix as a point
    x[i] = std::make_pair(
			  matlabImport->GetRowVectorArgument<K, Point>(IN_X, i, "X", xDef),
			  i+1 // because this will be a row index in Matlab, 1, ..., Nrows
			  );
  }

  for (std::vector<PointWithIndex>::iterator it = x.begin(); it != x.end(); ++it) {
    std::cout << "point " << it->second << " = " << it->first << std::endl;
  }

  // read number of components from input
  mwSize numComponents = matlabImport->GetScalarArgument<mwSize>(IN_NCOMP, "NCOMP", 1);

  // // DEBUG
  // std::cout << "Computing Delaunay triangulation" << std::endl;

  // Delaunay triangulation
  // http://www.cgal.org/Manual/latest/doc_html/cgal_manual/Triangulation_3/Chapter_main.html#Subsection_39.5.3
  Delaunay delaunay(x.begin(), x.end());
  CGAL_assertion(delaunay.number_of_vertices() == nrowsX);

  // // DEBUG
  // std::cout << "Delaunay triangulation computed" << std::endl;

  // // DEBUG
  // std::cout << "Computing alpha shape in REGULARIZED mode by default"
  //           << std::endl;

  // compute alpha shape
  Alpha_shape_3 as(delaunay);

  // // DEBUG
  // std::cout << "Alpha shape computed in REGULARIZED mode by default"
  //           << std::endl;

  // create output for the alpha interval
  plhs[OUT_ALPHAINT] = mxCreateNumericMatrix(1, 3, mxDOUBLE_CLASS, mxREAL);
  if (plhs[OUT_ALPHAINT] == NULL) {
    mexErrMsgTxt("Cannot allocate memory for output ALPHAINT" );
  }
  
  // pointer to the alpha interval output
  double *alphaintOut = (double *)mxGetData(plhs[OUT_ALPHAINT]); // triangulation
  if (alphaintOut == NULL) {
    mexErrMsgTxt("Memory for output ALPHAINT has been allocated, but I cannot get a pointer to it");
  }

  // write alpha interval output
  Alpha_iterator alphaOptIt = as.find_optimal_alpha(numComponents);
  alphaintOut[1] = *alphaOptIt; // optimum alpha

  Alpha_iterator alphaIt = as.alpha_begin();
  alphaintOut[0] = *alphaIt; // minimum alpha

  alphaIt = as.alpha_end();
  alphaintOut[2] = *(--alphaIt); // maximum alpha

  // read vector of alpha values provided by the user
  std::vector<double> alphaDef(1, *alphaOptIt);
  std::vector<double> alpha = matlabImport
    ->GetArrayArgumentAsVector<double, std::vector<double> >(IN_ALPHA, "ALPHA", alphaDef);

  // create a cell per surface triangulation that we are going to
  // extract not that each cell is unpopulated, and we would get an
  // error if we try mxGetCell() before populating them
  const mwSize triDims[2] = {1, alpha.size()};
  plhs[OUT_TRI] = mxCreateCellArray(2, triDims);
  if (plhs[OUT_TRI] == NULL) {
    mexErrMsgTxt("Cannot allocate memory for output TRI" );
  }

  // for each alpha value provided by the user, extract the
  // corresponding surface triangulation
  for (mwIndex i = 0; i < alpha.size(); ++i) {

    // exit if user pressed Ctrl+C
    ctrlcCheckPoint(__FILE__, __LINE__);

    // // DEBUG:
    // std::cout << "Extracting surface triangulation for alpha = " << alpha[i]
    // 	      << std::endl;

    // set current alpha value
    as.set_alpha(alpha[i]);

    // get alpha-shape surface
    std::list<Facet>       facets;
    as.get_alpha_shape_facets(std::back_inserter(facets),
			      Alpha_shape_3::REGULAR);

    // // DEBUG:
    // std::cout << "Number of facets = " << facets.size() << std::endl;

    // allocate memory in the current cell for the surface triangulation
    if (facets.size() == 0) {
      mxSetCell(plhs[OUT_TRI], i, mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL));
      continue;
    } else {
      mxSetCell(plhs[OUT_TRI], i, mxCreateNumericMatrix(facets.size(), 3, mxDOUBLE_CLASS, mxREAL));
    }

    // pointer to the triangulation output
    double *triOut = (double *)mxGetData(mxGetCell(plhs[OUT_TRI], i));
    if (triOut == NULL) {
      mexErrMsgTxt("Memory for output TRI has been allocated, but I cannot get a pointer to it");
    }

    // write facets to Matlab output
    mwSize row = 0; // row index of Matlab output
    for (std::list<Facet>::iterator it = facets.begin(); it != facets.end(); ++it) {

      // exit if user pressed Ctrl+C
      ctrlcCheckPoint(__FILE__, __LINE__);

      // vertex 1 of the triangle
      triOut[row] 
	= it->first->vertex(Delaunay::vertex_triple_index(it->second,0))->info();
      
      // vertex 2 of the triangle
      triOut[row+facets.size()] 
	= it->first->vertex(Delaunay::vertex_triple_index(it->second,1))->info();
      
      // vertex 3 of the triangle
      triOut[row+2*facets.size()] 
	= it->first->vertex(Delaunay::vertex_triple_index(it->second,2))->info();
      
      // // DEBUG:
      // std::cout << "facet = "
      // 		<< it->first->vertex(Delaunay::vertex_triple_index(it->second,0))->info()
      // 		<< ", "
      // 		<< it->first->vertex(Delaunay::vertex_triple_index(it->second,1))->info()
      // 		<< ", "
      // 		<< it->first->vertex(Delaunay::vertex_triple_index(it->second,2))->info()
      // 		<< std::endl; 
      
      // increase row counter
      ++row;

    } // end loop for each facet
    


  } // end loop for each alpha


  as.set_alpha(*alphaOptIt);
  assert(as.number_of_solid_components() == numComponents);

  // get alpha-shape surface
  std::list<Facet>       facets;
  as.get_alpha_shape_facets(std::back_inserter(facets),
                            Alpha_shape_3::REGULAR);

  // // create output for the triangulation
  // plhs[OUT_TRI] = mxCreateNumericMatrix(facets.size(), 3, mxDOUBLE_CLASS, mxREAL);
  // if (plhs[OUT_TRI] == NULL) {
  //   mexErrMsgTxt("Cannot allocate memory for output TRI" );
  // }

  // // pointer to the triangulation output
  // double *f = (double *)mxGetData(plhs[OUT_TRI]); // triangulation
  // if (f == NULL) {
  //   mexErrMsgTxt("Memory for output TRI has been allocated, but I cannot get a pointer to it");
  // }

  // // write facets to Matlab output
  // mwSize i = 0; // row index of Matlab output
  // for (std::list<Facet>::iterator it = facets.begin(); it != facets.end(); ++it) {

  //   std::cout << "facet = "
  // 	      << it->first->vertex(Delaunay::vertex_triple_index(it->second,0))->info()
  // 	      << ", "
  // 	      << it->first->vertex(Delaunay::vertex_triple_index(it->second,1))->info()
  // 	      << ", "
  // 	      << it->first->vertex(Delaunay::vertex_triple_index(it->second,2))->info()
  // 	      << std::endl;

  //   // increase row index of Matlab output
  //   i++;
  // }
  
}

#endif /* CGALALPHASHAPE3 */