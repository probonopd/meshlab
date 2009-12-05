/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <vcg/complex/trimesh/clean.h>
#include <vcg/complex/trimesh/inertia.h>
#include <vcg/complex/trimesh/stat.h>

#include <vcg/complex/trimesh/update/selection.h>
#include<vcg/complex/trimesh/append.h>
#include<vcg/simplex/face/pos.h>
#include<vcg/complex/trimesh/bitquad_support.h>
#include<vcg/complex/trimesh/bitquad_optimization.h>
#include "filter_measure.h"

using namespace std;
using namespace vcg;

// Constructor 
FilterMeasurePlugin::FilterMeasurePlugin() 
{ 
	typeList << 
	FP_MEASURE_TOPO <<
	FP_MEASURE_TOPO_QUAD <<
	FP_MEASURE_GAUSSCURV <<
	FP_MEASURE_VERTEX_QUALITY_DISTRIBUTION <<
	FP_MEASURE_GEOM;
  
  foreach(FilterIDType tt , types())
	  actionList << new QAction(filterName(tt), this);
}

// ST() return the very short string describing each filtering action 
 QString FilterMeasurePlugin::filterName(FilterIDType filterId) const
{
  switch(filterId) {
		case FP_MEASURE_TOPO :  return QString("Compute Topological Measures"); 
		case FP_MEASURE_TOPO_QUAD :  return QString("Compute Topological Measures for Quad Meshes"); 
		case FP_MEASURE_GEOM :  return QString("Compute Geometric Measures"); 
		case FP_MEASURE_GAUSSCURV :  return QString("Compute Integral of Gaussian Curvature"); 
		case FP_MEASURE_VERTEX_QUALITY_DISTRIBUTION :  return QString("Measure Quality Per Vertex"); 
		default : assert(0); 
	}
}

// Info() return the longer string describing each filtering action 
 QString FilterMeasurePlugin::filterInfo(FilterIDType filterId) const
{
  switch(filterId) {
		case FP_MEASURE_TOPO :  return QString("Selected faces are moved (or duplicated) in a new layer"); 
		case FP_MEASURE_TOPO_QUAD :  return QString("Selected faces are moved (or duplicated) in a new layer"); 
		case FP_MEASURE_GEOM :  return QString("Create a new layer containing the same model as the current one");
		case FP_MEASURE_GAUSSCURV :  return QString("Compute Integral of Gaussian Curvature");
		case FP_MEASURE_VERTEX_QUALITY_DISTRIBUTION : return QString("Compute some measures (min, max, med, stdev, variance, about the distribution of per vertex quality values");
		default : assert(0); 
	}
}

// Return true if the specified action has an automatic dialog.
// return false if the action has no parameters or has an self generated dialog.
bool FilterMeasurePlugin::autoDialog(QAction *action)
{
	 switch(ID(action))
	 {
			case FP_MEASURE_GEOM :
			case FP_MEASURE_TOPO:
			case FP_MEASURE_TOPO_QUAD:
			case FP_MEASURE_GAUSSCURV:
			case FP_MEASURE_VERTEX_QUALITY_DISTRIBUTION:
			 return false;
	 }
  return false;
}

// This function define the needed parameters for each filter. 
void FilterMeasurePlugin::initParameterSet(QAction *action, MeshDocument &m, RichParameterSet & parlst) 
{
	 switch(ID(action))	 
	 {
		default : assert(0); 
	}
}

// Core Function doing the actual mesh processing.
bool FilterMeasurePlugin::applyFilter(QAction *filter, MeshDocument &md, RichParameterSet & par, vcg::CallBackPos *cb)
{
	CMeshO::FaceIterator fi;
	int numFacesSel,numVertSel;

	switch(ID(filter))
  {
		case FP_MEASURE_TOPO : 
			{
				CMeshO &m=md.mm()->cm;	
				md.mm()->updateDataMask(MeshModel::MM_FACEFACETOPO);				
				md.mm()->updateDataMask(MeshModel::MM_VERTFACETOPO);				
				tri::UpdateTopology<CMeshO>::FaceFace(m);
				tri::UpdateTopology<CMeshO>::VertexFace(m);
				
				bool edgeManif = tri::Clean<CMeshO>::IsTwoManifoldFace(m);
				bool vertManif = tri::Clean<CMeshO>::IsTwoManifoldVertexFFVF(m);
				int edgeNum=0,borderNum=0;
				tri::Clean<CMeshO>::CountEdges(m, edgeNum, borderNum);
				int holeNum;
				Log("V: %6i E: %6i F:%6i",m.vn,edgeNum,m.fn);
				Log("Boundary Edges %i",borderNum); 
				
				int connectedComponentsNum = tri::Clean<CMeshO>::ConnectedComponents(m);
				Log("Mesh is composed by %i connected component(s)",connectedComponentsNum);
				
				if(edgeManif && vertManif) 
					Log("Mesh has is two-manifold ");
					
				if(!edgeManif) Log("Mesh has some non two manifold edges\n");
				if(!vertManif) Log("Mesh has some non two manifold vertexes\n");
				
				// For Manifold meshes compute some other stuff
				if(vertManif && edgeManif)
				{
					holeNum = tri::Clean<CMeshO>::CountHoles(m);
					Log("Mesh has %i holes",holeNum);
					
					int genus = tri::Clean<CMeshO>::MeshGenus(m, holeNum, connectedComponentsNum, edgeNum);
					Log("Genus is %i",genus);
				}
			}
		break;
		/************************************************************/ 
		case FP_MEASURE_TOPO_QUAD : 
			{
					CMeshO &m=md.mm()->cm;	
					md.mm()->updateDataMask(MeshModel::MM_FACEFACETOPO);				
					md.mm()->updateDataMask(MeshModel::MM_FACEQUALITY);				

					if (! tri::Clean<CMeshO>::IsFFAdjacencyConsistent(m)){
											Log("Error: mesh has a not consistent FF adjacency");
											return false;
										}
					if (! tri::Clean<CMeshO>::HasConsistentPerFaceFauxFlag(m)) {
											Log("Warning: mesh has a not consistent FauxEdge tagging");
											return false;
										}
					if (! tri::Clean<CMeshO>::IsBitTriQuadOnly(m)) {
											Log("Warning: IsBitTriQuadOnly");
											//return false;
										}
					//										if (! tri::Clean<CMeshO>::HasConsistentEdges(m)) lastErrorDetected |= NOT_EDGES_CONST;
					int nsinglets= tri::BitQuadOptimization< tri::BitQuad<CMeshO> >::MarkSinglets(m);
					if ( nsinglets  )  {
											Log("Warning: MarkSinglets");
											//return false;
										}
					
					if (! tri::BitQuad<CMeshO>::HasConsistentValencyFlag(m))
					 {
											Log("Warning: HasConsistentValencyFlag");
											//return false;
										}
				
			int nQuads = tri::Clean<CMeshO>::CountBitQuads(m);
			int nTris = tri::Clean<CMeshO>::CountBitTris(m);
			int nPolys = tri::Clean<CMeshO>::CountBitPolygons(m);
			
			Log("Mesh has %i tri %i quad and %i polig",nTris,nQuads,nPolys);

			}
			break;
		/************************************************************/ 
		case FP_MEASURE_GEOM : 
			{
				CMeshO &m=md.mm()->cm;
				tri::Inertia<CMeshO> I;
				I.Compute(m);
				
				tri::UpdateBounding<CMeshO>::Box(m); 
				float Area = tri::Stat<CMeshO>::ComputeMeshArea(m);
				float Volume = I.Mass(); 
				Log("Mesh Bounding Box Size %f %f %f", m.bbox.DimX(), m.bbox.DimY(), m.bbox.DimZ());			
				Log("Mesh Bounding Box Diag %f ", m.bbox.Diag());			
				Log("Mesh Volume  is %f", Volume);			
				Log("Center of Mass  is %f %f %f", I.CenterOfMass()[0], I.CenterOfMass()[1], I.CenterOfMass()[2]);		
				
				
				Matrix33f IT;
				I.InertiaTensor(IT);
				Log("Inertia Tensor is :");		
				Log("    | % 8.4f  8.4f  8.4f |",IT[0][0],IT[0][1],IT[0][2]);		
				Log("    | % 8.4f  8.4f  8.4f |",IT[1][0],IT[1][1],IT[1][2]);		
				Log("    | % 8.4f  8.4f  8.4f |",IT[2][0],IT[2][1],IT[2][2]);								
				
				Log("Mesh Surface is %f", Area);
				
			}
		break;
		case FP_MEASURE_VERTEX_QUALITY_DISTRIBUTION : 
			{
				CMeshO &m=md.mm()->cm;
				Distribution<float> DD;
			  tri::Stat<CMeshO>::ComputePerVertexQualityDistribution( m, DD, false);
				
				Log("   Min %f Max %f",DD.Min(),DD.Max());		
				Log("   Avg %f Med %f",DD.Avg(),DD.Percentile(0.5f));		
				Log("   StdDev		%f",DD.StandardDeviation());		
				Log("   Variance  %f",DD.Variance());						
			}
		break;

		case FP_MEASURE_GAUSSCURV : 
			{
				CMeshO &m=md.mm()->cm;
				SimpleTempData<CMeshO::VertContainer, float> TDArea(m.vert,0.0f);
				SimpleTempData<CMeshO::VertContainer, float> TDAngleSum(m.vert,0);

				tri::UpdateQuality<CMeshO>::VertexConstant(m,0);
				float angle[3];
				CMeshO::FaceIterator fi;
				for(fi=m.face.begin(); fi!= m.face.end(); ++fi)
					{
						angle[0] = math::Abs(Angle(	(*fi).P(1)-(*fi).P(0),(*fi).P(2)-(*fi).P(0) ));
						angle[1] = math::Abs(Angle(	(*fi).P(0)-(*fi).P(1),(*fi).P(2)-(*fi).P(1) ));
						angle[2] = M_PI-(angle[0]+angle[1]);

						float area= DoubleArea(*fi)/6.0f;
							for(int i=0;i<3;++i)
							{
								TDArea[(*fi).V(i)]+=area;
								TDAngleSum[(*fi).V(i)]+=angle[i];
							}
					}
				CMeshO::VertexIterator vi;
				float totalSum=0;
				for(vi=m.vert.begin(); vi!= m.vert.end(); ++vi)
					{
						(*vi).Q() = (2.0*M_PI-TDAngleSum[vi]);//*TDArea[vi];
						//if((*vi).IsS()) 
						totalSum += (*vi).Q(); 
					}
				Log("integrated is %f (%f*pi)", totalSum,totalSum/M_PI);												
			} break;
		default: assert(0);
	}
	return true;
}

 FilterMeasurePlugin::FilterClass FilterMeasurePlugin::getClass(QAction *a)
{
  switch(ID(a))
  {
    case FP_MEASURE_GAUSSCURV :
    case FP_MEASURE_GEOM :
    case FP_MEASURE_TOPO :
    case FP_MEASURE_TOPO_QUAD :
		 case FP_MEASURE_VERTEX_QUALITY_DISTRIBUTION:
      return MeshFilterInterface::Measure; 
		default :  assert(0);
			return MeshFilterInterface::Generic;
  }
}

Q_EXPORT_PLUGIN(FilterMeasurePlugin)
