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

#ifndef RIMLS_H
#define RIMLS_H

#include "mlssurface.h"

namespace GaelMls {

template<typename _MeshType>
class RIMLS : public MlsSurface<_MeshType>
{
		typedef _MeshType MeshType;
		typedef MlsSurface<_MeshType> Base;

		typedef typename Base::Scalar Scalar;
		typedef typename Base::VectorType VectorType;
		using Base::mCachedQueryPointIsOK;
		using Base::mCachedQueryPoint;
		using Base::mNeighborhood;
		using Base::mCachedWeights;
		using Base::mCachedWeightGradients;
		using Base::mBallTree;
		using Base::mPoints;
		using Base::mFilterScale;
		using Base::mMaxNofProjectionIterations;
		using Base::mAveragePointSpacing;
		using Base::mProjectionAccuracy;

	public:

		RIMLS(const MeshType& points)
			: Base(points)
		{
			mSigmaR = 0;
			mSigmaN = 0.8;
			mRefittingThreshold = 1e-3;
			mMinRefittingIters = 1;
			mMaxRefittingIters = 3;
		}

		virtual Scalar potential(const VectorType& x, int* errorMask = 0) const;
		virtual VectorType gradient(const VectorType& x, int* errorMask = 0) const;
		virtual VectorType project(const VectorType& x, VectorType* pNormal = 0, int* errorMask = 0) const;

		void setSigmaR(Scalar v);
		void setSigmaN(Scalar v);
		void setRefittingThreshold(Scalar v);
		void setMinRefittingIters(int n);
		void setMaxRefittingIters(int n);

	protected:
		bool computePotentialAndGradient(const VectorType& x) const;

	protected:

		int mMinRefittingIters;
		int mMaxRefittingIters;
		Scalar mRefittingThreshold;
		Scalar mSigmaN;
		Scalar mSigmaR;

		// cached values:
		mutable VectorType mCachedGradient;
		mutable Scalar mCachedPotential;
};

}

#include "rimls.tpp"

#endif
