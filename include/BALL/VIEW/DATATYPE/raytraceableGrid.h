// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// $Id:$
//

#ifndef BALL_VIEW_DATATYPE_RAYTRACEABLEGRID_H
#define BALL_VIEW_DATATYPE_RAYTRACEABLEGRID_H

#ifndef BALL_DATATYPE_REGULARDATA3D_H
# include <BALL/DATATYPE/regularData3D.h>
#endif

namespace BALL
{
	namespace VIEW
	{
		class BALL_VIEW_EXPORT RaytraceableGrid
		{
			public:
				RaytraceableGrid() 
					throw()
				{ 
					grid_ = 0; 
				}

				RaytraceableGrid(RegularData3D* grid) 
					throw()
				{ 
					grid_ = grid; 
				}

				// Note: the grid_ - ptr is handled by the calling classes and we must not destroy it!
				virtual ~RaytraceableGrid() 
					throw()
				{}

				
				float getGridMinValue() throw();
				
				float getGridMaxValue() throw();

			protected:
				RegularData3D* grid_;
		};
	}
}
#endif
