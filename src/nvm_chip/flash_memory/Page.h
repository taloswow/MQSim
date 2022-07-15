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

			void WriteMetadata(const PageMetadata& metadata)
			{
				this->Metadata.LPA = metadata.LPA;
			}
			
			void ReadMetadata(PageMetadata& metadata)
			{
				metadata.LPA = this->Metadata.LPA;
			}
		};
	}
}

#endif // !PAGE_H
