#ifndef PAGE_H
#define PAGE_H

#include "FlashTypes.h"

namespace NVM
{
	namespace FlashMemory
	{
		
		struct PageMetadata
		{
			LPA_type LPA;
		};

		class Page {
		public:
			Page()
			{
				Metadata.LPA = NO_LPA;
			};
			
			PageMetadata Metadata;

			void Write_metadata(const PageMetadata& metadata)
			{
				this->Metadata.LPA = metadata.LPA;
			}
			
			void Read_metadata(PageMetadata& metadata)
			{
				metadata.LPA = this->Metadata.LPA;
			}
		};
	}
}

#endif // !PAGE_H
